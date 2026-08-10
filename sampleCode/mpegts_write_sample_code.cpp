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
 *  mpegts_write_sample_code -- MPEG-TS producer over shared memory
 *
 *  Writes a muxed MPEG-TS byte stream into a TVU shared-memory segment, using
 *  the same mechanism the TVU applications use.
 *
 *  Transport model
 *  ---------------
 *  TS is carried in the *user data* field of a variable-size shared-memory
 *  item, tagged with LIBSHM_MEDIA_TYPE_MPEG_TS_DATA ('TMTS', 0x53544D54):
 *
 *      ohi.i_userDataType = LIBSHM_MEDIA_TYPE_MPEG_TS_DATA;
 *      ohi.p_userData     = <muxed TS bytes>;
 *      ohi.i_userDataLen  = <byte count>;
 *
 *  The video/audio/subtitle fields of the item stay empty -- this is a pure TS
 *  byte stream, not decoded or frame-structured media. Consumers must check
 *  i_userDataType before interpreting the payload as a transport stream.
 *
 *  Every item holds a whole number of 188-byte TS packets. The packet count per
 *  item is not fixed by the protocol; 1316 bytes (7 x 188) is the suggested
 *  value because it matches the standard UDP/SRT payload size.
 *
 *  See section 8.5 of the MediaMesh Shared Memory SDK document for the full
 *  contract, including the flow-control rules a consumer must observe.
 ******************************************************************************/

#include "libshmmedia.h"
#include "libshmmedia_variableitem.h"
#include "libshm_media_extension_protocol.h"   /* LIBSHM_MEDIA_TYPE_MPEG_TS_DATA */

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
#define DEFAULT_ITEM_SIZE   1316            /* 7 x 188, matches UDP/SRT payload */
#define DEFAULT_ITEM_COUNT  1024
#define SHM_HEADER_LEN      1024

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

static void usage(const char *argv0)
{
    fprintf(stderr,
        "mpegts_write_sample_code -- write MPEG-TS into a shared-memory segment\n"
        "\n"
        "Usage:\n"
        "  %s -n <shm_name> [-i <file|->] [options]\n"
        "\n"
        "Options:\n"
        "  -n <name>    Shared-memory name. This is the <name> part of\n"
        "               tvutsshm://0?name=<name>.  (required)\n"
        "  -i <path>    Input TS file, or '-' for stdin.  (required)\n"
        "  -s <bytes>   Item size, must be a multiple of 188. Default %d (7 x 188).\n"
        "  -c <count>   Ring item count. Default %d.\n"
        "  -b <bps>     Pace output at this bitrate. 0 = as fast as input\n"
        "               arrives (correct for stdin/live). Default 0.\n"
        "  -l           Loop the input file forever (file input only).\n"
        "  -r           Vary the number of TS packets per item (1..7). Items stay\n"
        "               whole packets, but the size changes from item to item.\n"
        "               Use it to verify a consumer does not hardcode 1316 or\n"
        "               any other fixed item size.\n"
        "  -v           Verbose: per-second progress on stderr.\n"
        "\n"
        "Examples:\n"
        "  %s -n LiveTransmit -i clip.ts -b 8000000 -l\n"
        "  ffmpeg -re -i in.mp4 -c copy -f mpegts - | %s -n LiveTransmit -i -\n",
        argv0, DEFAULT_ITEM_SIZE, DEFAULT_ITEM_COUNT, argv0, argv0);
}

int main(int argc, char *argv[])
{
    const char *shm_name   = NULL;
    const char *input_path = NULL;
    int         item_size  = DEFAULT_ITEM_SIZE;
    int         item_count = DEFAULT_ITEM_COUNT;
    int64_t     bitrate    = 0;
    int         loop_input = 0;
    int         vary_packets = 0;
    int         verbose    = 0;
    int         ch;

    while ((ch = getopt(argc, argv, "n:i:s:c:b:lrvh")) != -1) {
        switch (ch) {
        case 'n': shm_name   = optarg;                 break;
        case 'i': input_path = optarg;                 break;
        case 's': item_size  = atoi(optarg);           break;
        case 'c': item_count = atoi(optarg);           break;
        case 'b': bitrate    = strtoll(optarg, NULL, 10); break;
        case 'l': loop_input = 1;                      break;
        case 'r': vary_packets = 1;                 break;
        case 'v': verbose    = 1;                      break;
        case 'h':
        default:  usage(argv[0]); return (ch == 'h') ? 0 : 1;
        }
    }

    if (!shm_name || !input_path) {
        usage(argv[0]);
        return 1;
    }

    /*
     * Item size must be a whole number of TS packets. The packet count per item
     * is free -- 1316 (7 x 188) is the suggested value because it matches the
     * standard UDP/SRT payload, but any multiple of 188 is valid.
     */
    if (item_size <= 0 || item_size % TS_PACKET_SIZE != 0) {
        fprintf(stderr,
            "error: item size %d must be a positive multiple of %d "
            "(one TS packet)\n", item_size, TS_PACKET_SIZE);
        return 1;
    }

    signal(SIGINT,  handle_sig);
    signal(SIGTERM, handle_sig);
    signal(SIGPIPE, SIG_IGN);

    if (verbose) {
        LibShmMediaSetLogCallback(shm_log_cb);
    }

    FILE *fin = NULL;
    int   from_stdin = (strcmp(input_path, "-") == 0);

    if (from_stdin) {
        fin = stdin;
        if (loop_input) {
            fprintf(stderr, "warning: -l ignored for stdin input\n");
            loop_input = 0;
        }
    } else {
        fin = fopen(input_path, "rb");
        if (!fin) {
            fprintf(stderr, "error: cannot open input '%s'\n", input_path);
            return 1;
        }
    }

    /* total_size covers item payloads plus per-item bookkeeping overhead. */
    const uint64_t total_size = (uint64_t)item_count * (uint64_t)(item_size + 1024);

    libshm_media_handle_t h =
        LibViShmMediaCreate(shm_name, SHM_HEADER_LEN, item_count, total_size);

    if (!h) {
        fprintf(stderr, "error: LibViShmMediaCreate('%s') failed\n", shm_name);
        if (!from_stdin) fclose(fin);
        return 1;
    }

    /*
     * The stream header carries no format description for a pure TS payload --
     * everything a consumer needs is inside the transport stream itself.
     */
    libshm_media_head_param_t ohp;
    memset(&ohp, 0, sizeof(ohp));

    fprintf(stderr,
        "writer: shm='%s' url=tvutsshm://0?name=%s shmver=0x%x\n"
        "writer: userDataType=0x%08X ('TMTS')\n"
        "writer: item_size=%d (%d TS packets) item_count=%d total=%llu bytes\n",
        shm_name, shm_name, LibViShmMediaGetVersion(h),
        (unsigned)LIBSHM_MEDIA_TYPE_MPEG_TS_DATA,
        item_size, item_size / TS_PACKET_SIZE, item_count,
        (unsigned long long)total_size);

    uint8_t *buf = (uint8_t *)malloc(item_size);
    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        LibViShmMediaDestroy(h);
        if (!from_stdin) fclose(fin);
        return 1;
    }

    int64_t  start_ms      = now_ms();
    int64_t  bytes_sent    = 0;
    int64_t  items_sent    = 0;
    int64_t  dropped_full  = 0;
    int64_t  last_report   = start_ms;
    int      checked_sync  = 0;
    int      rc            = 0;

    unsigned vary_seq = 0;

    while (!g_exit) {
        size_t want = (size_t)item_size;

        /*
         * Ragged mode walks the request size through a set of offsets that are
         * deliberately not multiples of 188, so consecutive items start at
         * different phases within a TS packet.
         */
        /*
         * Vary the number of TS packets per item. Every item is still a whole
         * number of packets -- what changes is how many. Use this to verify a
         * consumer does not hardcode 1316 or any other fixed item size.
         */
        if (vary_packets) {
            static const int counts[] = { 1, 7, 3, 2, 5, 4 };
            int n = counts[vary_seq++ % (sizeof(counts) / sizeof(counts[0]))];
            int max_n = item_size / TS_PACKET_SIZE;

            if (n > max_n) n = max_n;
            want = (size_t)(n * TS_PACKET_SIZE);
        }

        size_t got = fread(buf, 1, want, fin);

        if (got == 0) {
            if (loop_input && !from_stdin) {
                rewind(fin);
                continue;
            }
            break;                                  /* end of input */
        }

        /*
         * Send only whole packets. A short tail at end of file is dropped
         * rather than sent, so a consumer never receives a partial packet.
         */
        if (got % TS_PACKET_SIZE != 0) {
            size_t whole = got - (got % TS_PACKET_SIZE);
            if (whole == 0) break;
            got = whole;
        }

        /* One-time sanity check that the input really is a transport stream. */
        if (!checked_sync) {
            checked_sync = 1;
            if (buf[0] != TS_SYNC_BYTE) {
                fprintf(stderr,
                    "warning: first byte is 0x%02x, expected TS sync 0x47. "
                    "Input may not be a transport stream, or is not packet aligned.\n",
                    buf[0]);
            }
        }

        /*
         * The TS bytes go in as user data, tagged so a consumer can recognise
         * them. Video, audio and subtitle stay empty.
         */
        libshm_media_item_param_t ohi;
        memset(&ohi, 0, sizeof(ohi));

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_MPEG_TS_DATA;
        ohi.p_userData     = buf;
        ohi.i_userDataLen  = (int)got;
        ohi.i64_userDataCT = now_ms();

        /*
         * PollSendable reports whether a slot is free. 0 means the ring is
         * momentarily full: the writer must never block on readers, so drop
         * this chunk and carry on rather than waiting for a consumer.
         */
        int ret = LibViShmMediaPollSendable(h, 0);

        if (ret < 0) {
            fprintf(stderr, "error: PollSendable failed, ret=%d\n", ret);
            rc = 1;
            break;
        }

        if (ret == 0) {
            dropped_full++;
            continue;
        }

        ret = LibViShmMediaSendData(h, &ohp, &ohi);
        if (ret < 0) {
            fprintf(stderr, "error: SendData failed, ret=%d\n", ret);
            rc = 1;
            break;
        }

        bytes_sent += (int64_t)got;
        items_sent++;

        /* Optional constant-bitrate pacing for file playback. */
        if (bitrate > 0) {
            int64_t due     = start_ms + (bytes_sent * 8 * 1000) / bitrate;
            int64_t elapsed = now_ms();
            if (due > elapsed) {
                usleep((useconds_t)((due - elapsed) * 1000));
            }
        }

        if (verbose) {
            int64_t t = now_ms();
            if (t - last_report >= 1000) {
                double mbps = (double)bytes_sent * 8.0 / (double)(t - start_ms) / 1000.0;
                fprintf(stderr,
                    "writer: %lld items, %lld bytes, %.3f Mbps avg, "
                    "%lld ring-full drops\n",
                    (long long)items_sent, (long long)bytes_sent, mbps,
                    (long long)dropped_full);
                last_report = t;
            }
        }
    }

    fprintf(stderr,
        "writer: stopped. %lld items, %lld bytes, %lld ring-full drops\n",
        (long long)items_sent, (long long)bytes_sent, (long long)dropped_full);

    free(buf);
    LibViShmMediaDestroy(h);
    if (!from_stdin) fclose(fin);

    /*
     * Remove the segment from the system so no stale readable state is left
     * behind after the source stops.
     */
    LibViShmMediaRemoveShmFromSystem(shm_name);

    return rc;
}
