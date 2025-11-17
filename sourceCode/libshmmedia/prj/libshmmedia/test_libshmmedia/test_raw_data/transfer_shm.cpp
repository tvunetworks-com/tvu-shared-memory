/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media.h"
#include "libshm_media_raw_data_opt.h"
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

static uint32_t gInShmItemLen = 0;
static uint32_t gInShmItemCount = 0;

static int read_raw_data(const char*input_name, libshmmedia_raw_head_param_t *ph, libshmmedia_raw_data_param_t *pi)
{
    static libshm_media_handle_t  ghInput = NULL;
    int ret = 0;

    if (!ghInput)
    {
        ghInput = LibShmMediaOpen(input_name, NULL, NULL);
        if(ghInput)
        {
            gInShmItemCount = LibShmMediaGetItemCounts(ghInput);
            gInShmItemLen = LibShmMediaGetItemLength(ghInput);
        }
    }

    if (!ghInput)
    {
        return 0;
    }

    ret = LibShmMediaRawDataRead(ghInput, ph, pi, 0);

    if (ret < 0)
    {
        LibShmMediaDestroy(ghInput);
        ghInput = NULL;
        printf("read raw data failed, ret %d, need destory handle\n", ret);
    }

    return ret;
}

static libshm_media_handle_t  ghOut = NULL;
static int write_raw_data(const char *output_name, const libshmmedia_raw_head_param_t *ph, const libshmmedia_raw_data_param_t *pi)
{
    static uint32_t item_len = 0;
    int ret = 0;

    if (!ghOut)
    {
        ghOut = LibShmMediaCreate(output_name,1024, 30, gInShmItemLen+1024);

        if (ghOut)
        {
            item_len = gInShmItemLen + 1024;
        }
    }

    if (!ghOut)
    {
        return -1;
    }

    if (ph && ph->pRawHead_ && ph->uRawHead_>0)
    {
        ret = LibShmMediaRawHeadWrite(ghOut, ph);
        if (ret <= 0)
            return ret;
    }

    int bHasReader = LibShmMediaHasReader(ghOut, 100);

    if (bHasReader)
    {
        printf("has reader\n");

        if (pi && pi->pRawData_ && pi->uRawData_>0)
        {
            uint8_t *pdest = LibShmMediaRawDataApply(ghOut, pi->uRawData_);

            if (pdest)
            {
                memcpy(pdest, pi->pRawData_, pi->uRawData_);

                LibShmMediaRawDataCommit(ghOut, pi->uRawData_);

                ret = pi->uRawData_;
            }
            else
            {
                if (item_len < pi->uRawData_)
                {
                    item_len = pi->uRawData_+1024;
                }
                ret = -1;
            }
        }

        if (ret < 0)
        {
            LibShmMediaDestroy(ghOut);
            ghOut = NULL;
            ret = 0;
        }
    }
    else
    {
        printf("no reader\n");
        ret = 0;
    }

    return ret;
}

static int transfer_shm(const char *input_name, const char *output_name)
{
    do {
        libshmmedia_raw_head_param_t   omh;
        {
            memset((void *)&omh, 0, sizeof(libshmmedia_raw_head_param_t));
        }
        libshmmedia_raw_data_param_t   omi;
        {
            memset((void *)&omi, 0, sizeof(libshmmedia_raw_data_param_t));
        }

        int ret = 0;

        ret = read_raw_data(input_name, &omh, &omi);

        if(ret <= 0)
        {
            if (ret < 0)
            {
                printf("read data ret [%d],  failed from [%s] shm\n", ret, input_name);
            }
            usleep(1000);
            continue;
        }

        ret = write_raw_data(output_name, &omh, &omi);
        if (ret <= 0)
        {
            printf("write data ret [%d], failed to [%s] shm\n", ret, output_name);
        }

    }while(!g_exit);

    if (ghOut)
        LibShmMediaDestroy(ghOut);

    return 0;
}

int get_shm_log(int level, const char *fmt, ...)
{
    char    slog[1024]  = {0};
    va_list ap;
    va_start(ap,fmt);

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
    va_end(ap);

    return 0;
}

static
void handle_sig(int sig)
{
    printf("get sig value[%d]\n", sig);
	g_exit	= 1;
}

int main(int argc, char *argv[])
{
    const char *input_name = NULL;
    const char *output_name = NULL;
    int ret = 0;

    LibShmMediaSetLogCb(get_shm_log);

    if (argc < 2) 
    {
        printf("Help:\n"
            "    %s -r<input shm> -w<output shm>\n"
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
    while ((ch = getopt_long(argc, argv, "r:w:", long_options, &opt_index)) != -1)
    {
        switch(ch)
        {
            case 'r':
            {
                input_name    = optarg;
            }
            break;
            case 'w':
            {
                output_name    = optarg;
            }
            break;
            default:
            {
                printf("unsupport command[%c]", ch);
            }
            break;
        }
    }

    if (!input_name || ! output_name)
    {
        printf("please check help usage\n");
        return -1;
    }

    ret = transfer_shm(input_name, output_name);

    return ret;
}

