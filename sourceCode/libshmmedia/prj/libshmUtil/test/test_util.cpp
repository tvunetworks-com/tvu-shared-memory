/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_key_value.h"
#include "buffer_ctrl.h"
#include <string.h>
#include <stdio.h>
#if defined(TVU_MINGW)
#include <Windows.h>
#elif defined(TVU_WINDOWS)
#define snprintf(p, count, fmt, ...)   _snprintf(p, (count)-1, fmt, ##__VA_ARGS__)
#include <Windows.h>
#include <sys/timeb.h>
#define usleep(n)    Sleep(n/1000)
#else
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/time.h>
#include <inttypes.h>
#define Sleep(n)    usleep(1000*n)
#endif
#include <getopt.h>
#include <assert.h>

#define TEST_SHM_NM     "tst_xx0"
static int g_exit	= 0;

#define TVUUTIL_GET_SYS_MS64()                  _xxtvuutil_get_sys_ms64()
#define VIDEO_FRAME_SIZE                        10240

static inline
int64_t _xxtvuutil_get_sys_ms64()
{
#ifdef TVU_WINDOWS
    int64_t tmNow = 0;
    struct _timeb timebuffer;
    _ftime_s(&timebuffer);
    tmNow = timebuffer.time;
    tmNow *= 1000;
    tmNow += timebuffer.millitm;
    return tmNow;
#elif defined(TVU_MINGW)
    int64_t tmNow = 0;
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    tmNow = timebuffer.time;
    tmNow *= 1000;
    tmNow += timebuffer.millitm;
    return tmNow;
#elif defined(TVU_LINUX)
    int64_t tmNow = 0;
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    tmNow = tv.tv_sec * 1000 + tv.tv_usec/1000;
    return tmNow;
#endif
}

static int _trans_raw_data_to_hex_string(const uint8_t *data, int len, char hx_str[], int hx_str_len)
{
    int ix = 0;

    for (int i = 0; i < len; i++)
    {
        if (i == 0)
        {
            snprintf(hx_str + ix, hx_str_len - ix, "%02hhx", data[i]);
        }
        else
        {
            snprintf(hx_str + ix, hx_str_len - ix, ",%02hhx", data[i]);
        }
        ix = strlen(hx_str);
    }

    return ix;
}

static void print_bin_info(const uint8_t *pDest, uint32_t nDest)
{
    if (!pDest || !nDest)
    {
        return;
    }

    char hxstr[256] = {0};
    _trans_raw_data_to_hex_string(pDest, nDest, hxstr, sizeof(hxstr));
    printf("bin infor."
           "len:%u,bin:{%s}"
           "\n"
           , nDest, hxstr
           );
    return;
}

static int TestKeyValParam()
{
    tvushm::KeyValParam par;
    tvushm::BufferController_t buf;
    {
        tvushm::BufferCtrlInit(&buf);
    }

    par.SetParamAsU8(1, 0x10);
    par.SetParamAsU16(2, 0x100);
    par.SetParamAsU32(3, 0x100);
    par.SetParamAsU64(4, 0x100);
    tvushm::uint128_t v128;
    {
        v128.namedQwords.highQword = 0x101;
        v128.namedQwords.lowQword = 0x03;
    }

    par.SetParamAsU128(5, v128);

    std::string os = "hello";
    par.SetParamAsString(6, os);

    const uint8_t xx[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    par.SetParamAsBytes(7, xx, sizeof(xx));

    par.AppendToBuffer(buf);

    print_bin_info(tvushm::BufferCtrlGetOrigPtr(&buf), tvushm::BufferCtrlGetBufLen(&buf));

    int ret = 0;
    {
        tvushm::KeyValParam par2;
        tvushm::BufferCtrlRewind(&buf);
        ret = par2.ExtractFromBuffer(buf, false);
        if (ret <= 0)
        {
            printf("extract failed."
                   "ret:%d"
                   "\n"
                   , ret
                   );
        }
        tvushm::BufferController_t buf2;
        {
            tvushm::BufferCtrlInit(&buf2);
        }

        par2.AppendToBuffer(buf2);

        print_bin_info(tvushm::BufferCtrlGetOrigPtr(&buf2), tvushm::BufferCtrlGetBufLen(&buf2));

        {
            tvushm::KeyValParam par3;
            tvushm::BufferCtrlRewind(&buf2);
            ret = par3.ExtractFromBuffer(buf2, true);
            if (ret <= 0)
            {
                printf("extract failed2."
                       "ret:%d"
                       "\n"
                       , ret
                       );
            }
            tvushm::BufferController_t buf3;
            {
                tvushm::BufferCtrlInit(&buf3);
            }

            par3.AppendToBuffer(buf3);

            print_bin_info(tvushm::BufferCtrlGetOrigPtr(&buf3), tvushm::BufferCtrlGetBufLen(&buf3));

            tvushm::BufferCtrlRelease(&buf3);
        }

        tvushm::BufferCtrlRelease(&buf2);
    }

    tvushm::BufferCtrlRelease(&buf);

    return 0;
}

static int TestBufferCtrl()
{
    tvushm::BufferController_t op;
    {
        tvushm::BufferCtrlInit(&op);
    }
    tvushm::BufferCtrlCompactEncodeValueU32(&op, 0x32);

    tvushm::BufferController_t op2;
    {
        tvushm::BufferCtrlInit(&op2);
    }

    uint8_t *buf = tvushm::BufferCtrlGetOrigPtr(&op);
    uint32_t nbuf = tvushm::BufferCtrlGetBufLen(&op);
    tvushm::BufferCtrlAttachExternalReadBuffer(&op2, buf, nbuf);

    uint32_t v = 0;
    int x = tvushm::BufferCtrlCompactDecodeValueU32(&op2, v);

    print_bin_info(buf, nbuf);

    printf("get value."
           "x:%d, v:%u"
           , x, v
           );

    return 0;
}

int get_shm_log(int level, const char *fmt, va_list ap)
{
    char    slog[1024]  = {0};

    switch (level) {
        case 'i':
            {
                snprintf(slog, 1024, "info --");
                vsnprintf(slog + 7, 1024 - 7, fmt, ap);
            }
            break;
        case 'w':
            {
                snprintf(slog, 1024, "warn --");
                vsnprintf(slog + 7, 1024 - 7, fmt, ap);
            }
            break;
        case 'e':
            {
                snprintf(slog, 1024, "error--");
                vsnprintf(slog + 7, 1024 - 7, fmt, ap);
            }
            break;
        default:
            break;
    }

    fprintf(stderr, "%s", slog);
    fflush(stderr);

    return 0;
}

static
void handle_sig(int sig)
{
	g_exit	= 1;
}

int main(int argc, char *argv[])
{
    int mode = 0;
    const char *name = TEST_SHM_NM;
    int count = 10;
    int item_size = 1024;

    //LibShmMediaProtoSetLogCbInternalV2(get_shm_log);

//    if (argc < 2)
//    {
//        printf("Help:\n"
//            "    %s -m<r|w|d> -n<shmname> -c<counts> -s<item_size>\n"
//            , argv[0]
//        );
//        return 0;
//    }

#if defined(TVU_LINUX)
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);
#endif

    struct option long_options[] = {
        {0, 0, 0, 0}
    };

    int     ch          = '?';
    int     opt_index   = 0;
    while ((ch = getopt_long(argc, argv, "m:n:c:s:", long_options, &opt_index)) != -1)
    {
        switch(ch)
        {
            case 'm':
            {
                if (optarg && optarg[0] == 'w')
                {
                    mode    = 1;
                }
                else if (optarg && optarg[0] == 'd')
                {
                    mode    = 2;
                }
            }
            break;
            case 'n':
            {
                name    = optarg;
            }
            break;
            case 'c':
            {
                count    = atoi(optarg);
            }
            break;
            case 's':
            {
                item_size    = atoi(optarg);
            }
            break;
            default:
            {
                printf("unsupport command[%c]", ch);
            }
            break;
        }
    }

    //TestBufferCtrl();
    TestKeyValParam();

    return 0;
}

