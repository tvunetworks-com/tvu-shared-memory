/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media_variable_item.h"
#include "libshm_data_protocol.h"
#ifdef _TVULIVE_TEST_ENABLE
#include "libtvulive2.h"
#endif
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
#define Sleep(n)    usleep(1000*n)
#endif
#include <getopt.h>
#include <assert.h>
#include <inttypes.h>

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

static int Delete(const char *name)
{
#ifdef TVU_LINUX
    LibViShmMediaRemoveShmFromSystem(name);
#endif
    return 0;
}

static int Write(const char *name, int count, int item_size)
{
    unsigned int                i       = 0;
    libshmmedia_tvulive_wrap_handle_t   h           = NULL;

    h       = LibShmMediaTvuliveWrapHandleCreate(name, 1024, count, count*(item_size+2048));

    if (!h)
    {
        printf("create libshmmedia tvulive handle failed\n");
        return -1;
    }

    int frame_index = 0;

    int gopend = 0;
    int fileend = 0;
    int gopval = 0;
    int gopNum = 0;

    while (!g_exit) {

        libtvumedia_tvulive_data_sections_t osec;
        {
            memset(&osec, 0, sizeof(osec));
            osec.u_struct_size = sizeof(libtvumedia_tvulive_data_sections_t);
        }

        libtvumedia_tvulive_section_pair_t opair[3];
        {
            opair[0].i_section = 10;
            opair[0].p_section = (uint8_t *)"1234567890";
            opair[1].i_section = 5;
            opair[1].p_section = (uint8_t *)"YYYYYYY";
            opair[2].i_section = 20;
            opair[2].p_section = (uint8_t *)"abcdefghigklmnopqrstuvwxyz";
        }

        osec.u_createTime = _xxtvuutil_get_sys_ms64();
        osec.o_info.i_type = kLibShmMediaDataTypeRawJpeg;
        osec.o_info.u_frame_index = frame_index++;
        osec.o_info.u_program_index = 1;
        osec.o_info.u_stream_index = 10;
        osec.o_info.u_frame_timestamp_ms = _xxtvuutil_get_sys_ms64();

        if (gopval>=30)
        {
            gopend = 1;
        }

        osec.o_info.ext.u_gop_poc = (1<<31)|(fileend<<30)|(gopend<<29)|gopval;
        osec.u_section_counts = 3;
        osec.p_sections = opair;

        int ret = LibShmMediaTvuliveWrapHandleWrite(h, &osec);

        if (ret <= 0)
        {
            printf("shm tvulive write failed, ret %d", ret);
            break;
        }

        gopval++;
        if (gopend)
        {
            gopend = 0;
            gopval = 0;
            gopNum++;
        }

        if (gopNum >= 60 )
        {
            fileend = 1;
            gopNum = 0;
        }

        Sleep(30);
        i++;

    }

    if (h)
    {
        LibShmMediaTvuliveWrapHandleDestroy(h);
        h   = NULL;
    }
    return 0;
}

#ifdef _TVULIVE_TEST_ENABLE
static
int copy_frame(
      void      *userdata
    , const libtvulive_frame_info_t *pInfo
)
{
    uint64_t   pts      = pInfo->pts;
    uint64_t   dts      = pInfo->dts;
    uint8_t   flag      = pInfo->flag;
    uint16_t  frame_index = pInfo->frame_index;
    uint8_t   *buffer   = pInfo->buffer;
    int       len      = pInfo->buf_len;
    uint64_t  position_offset = pInfo->position_offset;

    int64_t now = _xxtvuutil_get_sys_ms64();
    printf("pts[%lu], dts[%lu], now [%lu], dts-now[%ld], flag[0x%02x], frame index[%hd], pos[%lld], len[%d], data['%c']\n"
        , pts, dts, now, dts - now, flag, frame_index, position_offset, len, buffer[0]);

    return 0;
}
#endif

#define _MIN(a, b) ((a<b)?a:b)

void _print_data(libtvumedia_tvulive_data_t *p)
{
    char data[256] = {0};

    int cplen = _MIN(p->i_data, (sizeof(data)-1));
    cplen = _MIN(4, cplen);
    memcpy(data, p->p_data, cplen);
    data[cplen] = 0;

    printf("data type 0x%08x, "
           "create time %" PRId64 ", "
           "frame index %hd, stream index %hd, "
           "program index %hd, frame timestamp ms %" PRId64 ", "
           "gopV:0x%x,"
           "tvulive len %d, tvulive data[%s]\n"
           , p->o_info.i_type
           , p->u_createTime
           , p->o_info.u_frame_index
           , p->o_info.u_stream_index
           , p->o_info.u_program_index
           , p->o_info.u_frame_timestamp_ms
           , p->o_info.ext.u_gop_poc
           , p->i_data
           , data
    );

#ifdef _TVULIVE_TEST_ENABLE
    {
        static libtvulive2_handle_t      hdemux = NULL;

        if (!hdemux)
        {
            hdemux = libtvulive2_demux_handle_create();
        }

        if (hdemux)
        {
            const uint8_t *buff = p->p_data;
            int nbuff = p->i_data;

            int ret = libtvulive2_demux_header(hdemux, buff, nbuff);

            if (ret < 0)
            {
                libtvulive2_demux_frame(hdemux, buff, nbuff, NULL, copy_frame);
            }
        }
    }
#endif
}

static int Read(const char *name)
{
    unsigned int                i       = 0;
    libshmmedia_tvulive_wrap_handle_t       h       = NULL;
    const char                  *shmname= name;
    int                         ret     = -1;

    while (!g_exit) {
        if (!h)
        {
            h = LibShmMediaTvuliveWrapHandleOpen(shmname);
        }

        if (!h)
        {
            Sleep(1000);
            printf("open handle failed, hadle name [%s], wait another 1000ms\n", shmname);
            continue;
        }

        libtvumedia_tvulive_data_t odata;
        {
            memset(&odata, 0, sizeof(odata));
            odata.u_struct_size = sizeof(libtvumedia_tvulive_data_t);
        }

        ret = LibShmMediaTvuliveWrapHandleRead(h, &odata);

        if (ret == 0)
        {
            Sleep(1);
            continue;
        }
        else if (ret < 0)
        {
            printf("read shm tvulive failed, ret %d\n", ret);
            LibShmMediaTvuliveWrapHandleDestroy(h);
            h = NULL;
        }

        printf("get tvulive data len: %d\n", ret);

        _print_data(&odata);
    }

    if (h)
    {
        LibShmMediaTvuliveWrapHandleDestroy(h);
        h   = NULL;
    }
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
    LibShmMediaSetLogCallback(get_shm_log);

    if (argc < 2)
    {
        printf("Help:\n"
            "    %s -m<r|w|d> -n<shmname> -c<counts> -s<item_size>\n"
            , argv[0]
        );
        return 0;
    }

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

    if (mode == 0)
    {
        while (!g_exit)
        {
            Read(name);
#ifdef TVU_WINDOWS
            ::Sleep(1);
#else
            usleep(100000);
#endif
        }
    }
    else if (mode == 1)
    {
        Write(name, count, item_size);
    }
    else if (mode == 2)
    {
        Delete(name);
    }

    return 0;
}

