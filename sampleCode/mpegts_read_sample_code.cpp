/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/
/*******************************************************************************
 *  mpegts_read_sample_code -- MPEG-TS consumer over shared memory
 *
 *  Attaches to a TVU shared-memory segment carrying a muxed MPEG-TS byte stream
 *  (the tvutsshm://0?name=<name> convention), optionally writes the stream out
 *  to a file or stdout, and runs basic TS-level validation.
 *
 *  Transport model
 *  ---------------
 *  TS arrives in the *user data* field of each shared-memory item, tagged with
 *  LIBSHM_MEDIA_TYPE_MPEG_TS_DATA ('TMTS', 0x53544D54). A consumer must check
 *  the tag before treating the payload as a transport stream:
 *
 *      LibViShmMediaPollReadData(h, &ohp, &ohi, timeout);
 *      if (ohi.i_userDataType == LIBSHM_MEDIA_TYPE_MPEG_TS_DATA) {
 *          consume(ohi.p_userData, ohi.i_userDataLen);
 *      }
 *
 *  The video/audio/subtitle fields are empty on a TS segment.
 *
 *  IMPORTANT -- READER MUST KEEP UP
 *  ------------------------------------------------------------------------
 *  The writer never blocks and there is no backpressure or retransmission. A
 *  reader that consumes slower than the writer produces will silently miss
 *  items. Missed items look exactly like continuity-counter errors, PCR
 *  discontinuities and PID loss in the recovered stream -- they are artifacts
 *  of the reader falling behind, NOT defects in the source stream.
 *
 *  This sample detects that case explicitly. It monitors the distance between
 *  the write and read indices ("lag") and reports when the consumer is close to
 *  being lapped, so reader-side loss can be distinguished from genuine TS
 *  problems. Any analyser integrating against this interface should do the same.
 *
 *  See section 8.5 of the MediaMesh Shared Memory SDK document for the full
 *  contract.
 ******************************************************************************/

#include "libshmmedia.h"
#include "libshmmedia_variableitem.h"
#include "libshm_media_extension_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <sys/time.h>

#define TS_PACKET_SIZE      188
#define TS_SYNC_BYTE        0x47
#define TS_NULL_PID         0x1FFF
#define TS_MAX_PID          8192
#define OPEN_RETRY_MS       500

static volatile int g_exit = 0;

static void handle_sig(int sig)
{
    (void)sig;
    g_exit = 1;
}

static int64_t now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int shm_log_cb(int level, const char *fmt, va_list ap)
{
    char line[1024] = {0};

    vsnprintf(line, sizeof(line), fmt, ap);
    fprintf(stderr, "[shmlib:%c] %s", level, line);
    return 0;
}

/* ---------------------------------------------------------------- TS state */

/*
 * Item size is a whole number of 188-byte TS packets, but the packet count per
 * item is NOT fixed -- a consumer must not hardcode 1316 or any other size.
 *
 * This parser therefore treats the payload as a byte stream and locks onto the
 * 0x47 sync byte instead of indexing at fixed offsets. That also makes it
 * tolerant of a producer that violates the framing rule and splits a packet
 * across items, which is reported separately as `unaligned items`.
 *
 * `carry` holds whatever is left over between feeds. Two packets' worth of room
 * is enough: one for an incomplete packet, one for the resync scan window.
 */
typedef struct {
    int64_t  packets;
    int64_t  sync_errors;               /* resync events, not per-packet */
    int64_t  cc_errors;
    int64_t  null_packets;
    int      synced;
    int8_t   last_cc[TS_MAX_PID];       /* -1 = not seen yet */
    uint8_t  pid_seen[TS_MAX_PID];
    uint8_t  carry[TS_PACKET_SIZE * 2];
    size_t   carry_len;
} ts_state_t;

static void ts_state_init(ts_state_t *st)
{
    memset(st, 0, sizeof(*st));
    memset(st->last_cc, -1, sizeof(st->last_cc));
}

/* Validate one complete 188-byte packet, already known to start with 0x47. */
static void ts_validate_packet(ts_state_t *st, const uint8_t *p)
{
    st->packets++;

    uint16_t pid = (uint16_t)(((p[1] & 0x1F) << 8) | p[2]);

    if (pid == TS_NULL_PID) {               /* stuffing, no CC continuity */
        st->null_packets++;
        return;
    }

    st->pid_seen[pid] = 1;

    uint8_t afc = (uint8_t)((p[3] >> 4) & 0x03);
    uint8_t cc  = (uint8_t)(p[3] & 0x0F);

    /*
     * The continuity counter only advances on packets that carry a payload
     * (afc 1 = payload only, 3 = adaptation field + payload). For afc 0 and 2
     * the counter must stay unchanged.
     */
    int    has_payload = (afc == 1 || afc == 3);
    int8_t prev        = st->last_cc[pid];

    if (prev >= 0) {
        uint8_t expected = has_payload
                         ? (uint8_t)((prev + 1) & 0x0F)
                         : (uint8_t)prev;

        /* A single duplicated packet is legal and repeats the counter. */
        int duplicate_ok = has_payload && (cc == (uint8_t)prev);

        if (cc != expected && !duplicate_ok) {
            st->cc_errors++;
        }
    }

    st->last_cc[pid] = (int8_t)cc;
}

/*
 * Find a packet boundary in buf. A candidate 0x47 is accepted only if a second
 * 0x47 appears exactly 188 bytes later, which rejects payload bytes that happen
 * to be 0x47. Returns the offset, or -1 if no boundary is confirmable yet.
 */
static long ts_find_sync(const uint8_t *buf, size_t len)
{
    if (len < TS_PACKET_SIZE * 2) return -1;

    for (size_t i = 0; i + TS_PACKET_SIZE < len; i++) {
        if (buf[i] == TS_SYNC_BYTE && buf[i + TS_PACKET_SIZE] == TS_SYNC_BYTE) {
            return (long)i;
        }
    }
    return -1;
}

/*
 * Feed one item's payload into the parser. The item size is whatever the
 * producer chose, so nothing here assumes a fixed length.
 */
static void ts_feed(ts_state_t *st, const uint8_t *data, size_t len)
{
    while (len > 0) {
        /* Top the carry buffer up from the incoming item. */
        size_t room = sizeof(st->carry) - st->carry_len;
        size_t take = (len < room) ? len : room;

        memcpy(st->carry + st->carry_len, data, take);
        st->carry_len += take;
        data          += take;
        len           -= take;

        /* Consume as many whole packets as the carry buffer now holds. */
        for (;;) {
            if (!st->synced) {
                long off = ts_find_sync(st->carry, st->carry_len);

                if (off < 0) {
                    /*
                     * No boundary confirmable yet. Keep the tail so a boundary
                     * spanning this and the next item is still found.
                     */
                    if (st->carry_len > TS_PACKET_SIZE * 2 - 1) {
                        size_t keep = TS_PACKET_SIZE * 2 - 1;
                        memmove(st->carry, st->carry + st->carry_len - keep, keep);
                        st->carry_len = keep;
                    }
                    break;
                }

                if (off > 0) {              /* discard bytes before the boundary */
                    memmove(st->carry, st->carry + off, st->carry_len - (size_t)off);
                    st->carry_len -= (size_t)off;
                }
                st->synced = 1;

                /* The very first lock is normal; later ones mean lost sync. */
                if (st->packets > 0) st->sync_errors++;
            }

            if (st->carry_len < TS_PACKET_SIZE) break;

            if (st->carry[0] != TS_SYNC_BYTE) {
                st->synced = 0;             /* lost alignment, rescan */
                continue;
            }

            ts_validate_packet(st, st->carry);

            st->carry_len -= TS_PACKET_SIZE;
            memmove(st->carry, st->carry + TS_PACKET_SIZE, st->carry_len);
        }
    }
}

static int ts_count_pids(const ts_state_t *st)
{
    int n = 0;
    for (int i = 0; i < TS_MAX_PID; i++) {
        if (st->pid_seen[i]) n++;
    }
    return n;
}

/* ------------------------------------------------------------------- main */

static void usage(const char *argv0)
{
    fprintf(stderr,
        "mpegts_read_sample_code -- read MPEG-TS from a shared-memory segment\n"
        "\n"
        "Usage:\n"
        "  %s -n <shm_name> [options]\n"
        "\n"
        "Options:\n"
        "  -n <name>    Shared-memory name. This is the <name> part of\n"
        "               tvutsshm://0?name=<name>.  (required)\n"
        "  -o <path>    Write the recovered TS to this file, or '-' for stdout.\n"
        "  -t <ms>      Read timeout per call. Default 1000.\n"
        "  -d <sec>     Stop after this many seconds. 0 = run until signalled.\n"
        "  -q           Do not run TS validation, just pass bytes through.\n"
        "  -v           Verbose: per-second statistics on stderr.\n"
        "\n"
        "Examples:\n"
        "  %s -n LiveTransmit -v\n"
        "  %s -n LiveTransmit -o - -q | ffprobe -\n"
        "  %s -n LiveTransmit -o capture.ts -d 10\n",
        argv0, argv0, argv0, argv0);
}

int main(int argc, char *argv[])
{
    const char *shm_name    = NULL;
    const char *output_path = NULL;
    unsigned    timeout_ms  = 1000;
    int         duration_s  = 0;
    int         validate    = 1;
    int         verbose     = 0;
    int         ch;

    while ((ch = getopt(argc, argv, "n:o:t:d:qvh")) != -1) {
        switch (ch) {
        case 'n': shm_name    = optarg;                     break;
        case 'o': output_path = optarg;                     break;
        case 't': timeout_ms  = (unsigned)atoi(optarg);     break;
        case 'd': duration_s  = atoi(optarg);               break;
        case 'q': validate    = 0;                          break;
        case 'v': verbose     = 1;                          break;
        case 'h':
        default:  usage(argv[0]); return (ch == 'h') ? 0 : 1;
        }
    }

    if (!shm_name) {
        usage(argv[0]);
        return 1;
    }

    signal(SIGINT,  handle_sig);
    signal(SIGTERM, handle_sig);
    signal(SIGPIPE, SIG_IGN);

    if (verbose) {
        LibShmMediaSetLogCallback(shm_log_cb);
    }

    FILE *fout = NULL;
    if (output_path) {
        if (strcmp(output_path, "-") == 0) {
            fout = stdout;
        } else {
            fout = fopen(output_path, "wb");
            if (!fout) {
                fprintf(stderr, "error: cannot open output '%s'\n", output_path);
                return 1;
            }
        }
    }

    /* The producer may not exist yet; retry until it appears. */
    libshm_media_handle_t h = NULL;
    while (!g_exit && !h) {
        h = LibViShmMediaOpen(shm_name, NULL, NULL);
        if (!h) {
            fprintf(stderr, "reader: waiting for shm '%s' ...\n", shm_name);
            usleep(OPEN_RETRY_MS * 1000);
        }
    }

    if (!h) {
        if (fout && fout != stdout) fclose(fout);
        return 1;
    }

    const unsigned item_count = LibViShmMediaGetItemCounts(h);

    /*
     * The read/write indices are not free-running counters: they wrap within a
     * space of 2 x item_count. Differences must therefore be taken modulo that
     * value, otherwise a wrap looks like an enormous backwards jump.
     */
    const uint64_t index_modulus = 2ULL * (uint64_t)item_count;

    fprintf(stderr,
        "reader: attached to shm='%s' (tvutsshm://0?name=%s) shmver=0x%x "
        "items=%u payload=%u bytes\n",
        shm_name, shm_name, LibViShmMediaGetVersion(h),
        item_count, LibViShmMediaGetTotalPayloadSize(h));

    /*
     * Start from the newest data rather than replaying whatever happens to be
     * sitting in the ring. For live monitoring this is almost always what you
     * want; drop this call if you need the backlog.
     */
    LibViShmMediaSeekReadIndexToWriteIndex(h);

    ts_state_t st;
    ts_state_init(&st);

    int64_t start_ms     = now_ms();
    int64_t last_report  = start_ms;
    int64_t bytes_read   = 0;
    int64_t items_read   = 0;
    int64_t underruns    = 0;
    int64_t items_lost   = 0;
    int64_t unaligned    = 0;
    uint64_t prev_rindex = LibViShmMediaGetReadIndex(h);
    uint64_t max_lag     = 0;
    int64_t  lapped      = 0;
    int64_t  near_lapped = 0;
    int64_t  other_type  = 0;
    int      warned_lag  = 0;
    int      rc          = 0;

    /* Warn once the reader is using more than this share of the ring. */
    const uint64_t lag_warn_threshold = (uint64_t)item_count * 3 / 4;

    while (!g_exit) {
        libshm_media_head_param_t ohp;
        libshm_media_item_param_t ohi;

        memset(&ohp, 0, sizeof(ohp));
        memset(&ohi, 0, sizeof(ohi));

        int ret = LibViShmMediaPollReadData(h, &ohp, &ohi, timeout_ms);

        if (ret < 0) {
            fprintf(stderr, "reader: PollReadData failed, ret=%d -- "
                            "handle must be recreated\n", ret);
            rc = 1;
            break;
        }

        if (ret == 0) {                     /* timeout, no data yet */
            if (duration_s > 0 && now_ms() - start_ms >= duration_s * 1000) break;
            continue;
        }

        /*
         * Underrun detection. The read index should advance by exactly one item
         * per successful read. A larger jump means the writer lapped us and the
         * library skipped ahead -- data was lost on the reader side.
         *
         * Caveat: because the index space is finite, a reader that falls behind
         * by an exact multiple of 2 x item_count is not detectable this way. In
         * practice a reader that far behind will trip this check long before.
         */
        uint64_t rindex = LibViShmMediaGetReadIndex(h);
        uint64_t step   = (rindex + index_modulus - prev_rindex) % index_modulus;

        if (step > 1) {
            underruns++;
            items_lost += (int64_t)(step - 1);
            fprintf(stderr,
                "reader: *** SHM UNDERRUN *** skipped %llu items "
                "(read index %llu -> %llu). Any CC/PCR errors reported around "
                "this point are reader-side loss, not stream defects.\n",
                (unsigned long long)(step - 1),
                (unsigned long long)prev_rindex,
                (unsigned long long)rindex);
        }
        prev_rindex = rindex;

        /*
         * Lag monitoring -- the more reliable of the two signals.
         *
         * The index step above only catches the case where the library visibly
         * seeks the read index forward. It does NOT catch the more common
         * failure: the reader keeps stepping one slot at a time while the
         * writer laps it, so the slots being read have already been overwritten
         * with newer data. The read index looks perfectly healthy while the
         * recovered byte stream is silently discontinuous.
         *
         * Lag catches that. It is the distance between the write and read
         * index; the ring holds item_count items, so a lag above item_count
         * means the writer has definitely lapped us and data was lost.
         */
        uint64_t windex = LibViShmMediaGetWriteIndex(h);
        uint64_t lag    = (windex + index_modulus - rindex) % index_modulus;

        if (lag > max_lag) max_lag = lag;

        if (lag > (uint64_t)item_count) {
            lapped++;
        } else if (lag >= lag_warn_threshold) {
            near_lapped++;
            if (!warned_lag) {
                warned_lag = 1;
                fprintf(stderr,
                    "reader: warning: lag %llu of %u items -- this consumer is "
                    "close to being lapped by the writer. Reduce per-item work "
                    "or increase the ring size.\n",
                    (unsigned long long)lag, item_count);
            }
        }

        /*
         * Only items tagged as MPEG-TS carry a transport stream. Anything else
         * on this segment is not ours to interpret.
         */
        if (ohi.i_userDataType != LIBSHM_MEDIA_TYPE_MPEG_TS_DATA) {
            other_type++;
            if (other_type == 1) {
                fprintf(stderr,
                    "reader: warning: item userDataType 0x%08X is not "
                    "LIBSHM_MEDIA_TYPE_MPEG_TS_DATA (0x%08X) -- skipping. "
                    "Is this segment really a tvutsshm TS stream?\n",
                    (unsigned)ohi.i_userDataType,
                    (unsigned)LIBSHM_MEDIA_TYPE_MPEG_TS_DATA);
            }
            continue;
        }

        const uint8_t *data = ohi.p_userData;
        size_t         len  = (size_t)ohi.i_userDataLen;

        if (!data || len == 0) continue;

        items_read++;
        bytes_read += (int64_t)len;

        /*
         * Item size must be a whole number of TS packets. The parser below
         * recovers from a violation anyway, but it is a producer bug and worth
         * reporting rather than silently absorbing.
         */
        if (len % TS_PACKET_SIZE != 0) {
            unaligned++;
            if (unaligned == 1) {
                fprintf(stderr,
                    "reader: warning: item length %zu is not a multiple of %d. "
                    "The producer is violating the framing rule; TS packets are "
                    "being split across items.\n", len, TS_PACKET_SIZE);
            }
        }

        /*
         * data points into the shared segment and is only valid until the next
         * read call, so consume or copy it now.
         */
        if (fout) {
            if (fwrite(data, 1, len, fout) != len) {
                fprintf(stderr, "reader: output write failed\n");
                rc = 1;
                break;
            }
        }

        if (validate) {
            ts_feed(&st, data, len);
        }

        int64_t t = now_ms();

        if (verbose && t - last_report >= 1000) {
            double mbps = (double)bytes_read * 8.0 / (double)(t - start_ms) / 1000.0;
            fprintf(stderr,
                "reader: %lld items, %lld bytes, %.3f Mbps",
                (long long)items_read, (long long)bytes_read, mbps);
            if (validate) {
                fprintf(stderr, ", %lld TS packets, %d PIDs, "
                                "%lld CC errors, %lld sync errors",
                    (long long)st.packets, ts_count_pids(&st),
                    (long long)st.cc_errors, (long long)st.sync_errors);
            }
            fprintf(stderr, ", %lld underruns (%lld items lost), "
                            "lag %llu/%u max\n",
                (long long)underruns, (long long)items_lost,
                (unsigned long long)max_lag, item_count);
            last_report = t;
        }

        if (duration_s > 0 && t - start_ms >= duration_s * 1000) break;
    }

    int64_t elapsed = now_ms() - start_ms;
    if (elapsed <= 0) elapsed = 1;

    fprintf(stderr,
        "\n--- mpegts_read_sample_code summary ---------------------------\n"
        "  shm name          : %s\n"
        "  elapsed           : %.1f s\n"
        "  items read        : %lld\n"
        "  bytes read        : %lld (%.3f Mbps average)\n"
        "  index skips       : %lld (%lld items lost on the reader side)\n"
        "  max lag           : %llu of %u ring items\n"
        "  reads while lapped: %lld   <- data definitely lost here\n"
        "  reads near-lapped : %lld\n"
        "  non-TS items      : %lld\n"
        "  unaligned items   : %lld   <- non-zero means a producer bug\n",
        shm_name,
        (double)elapsed / 1000.0,
        (long long)items_read,
        (long long)bytes_read,
        (double)bytes_read * 8.0 / (double)elapsed / 1000.0,
        (long long)underruns, (long long)items_lost,
        (unsigned long long)max_lag, item_count,
        (long long)lapped,
        (long long)near_lapped,
        (long long)other_type,
        (long long)unaligned);

    if (validate) {
        fprintf(stderr,
            "  TS packets        : %lld\n"
            "  distinct PIDs     : %d\n"
            "  null packets      : %lld\n"
            "  resync events     : %lld\n"
            "  continuity errors : %lld%s\n",
            (long long)st.packets,
            ts_count_pids(&st),
            (long long)st.null_packets,
            (long long)st.sync_errors,
            (long long)st.cc_errors,
            (st.cc_errors > 0 && (underruns > 0 || lapped > 0))
                ? "   <-- the reader fell behind, so an unknown share of these\n"
                  "                      are reader-side loss rather than stream defects"
                : "");
    }
    fprintf(stderr,
        "--------------------------------------------------------------\n");

    if (fout && fout != stdout) fclose(fout);
    LibViShmMediaDestroy(h);

    return rc;
}
