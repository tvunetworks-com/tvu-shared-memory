#include "libshmmedia.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#if defined(TVU_WINDOWS)
#include <sys/timeb.h>
#include <time.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif


#define TVUUTIL_GET_SYS_MS64()                  _xxtvuutil_get_sys_ms64()
#define VIDEO_FRAME_SIZE                        10240
#define TVU_LINUX 1

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

#if defined(TVU_WINDOWS)

#else
static void Sleep(uint32_t ms)
{
    usleep(ms*1000);
}
#endif

static bool g_exit = false;

static int ReadSampleCode(const char *name)
{
    unsigned int                i       = 0;
    libshm_media_handle_t       h       = NULL;
    libshm_media_head_param_t   ohp     = {0};
    const char                  *shmname= name;
    int                         ret     = -1;
    int                         timeout = 3000;//3000 ms

    h       = LibShmMediaOpen(shmname, NULL, NULL); // open the shared memory, the callback can be NULL here.

    if (!h) { /* failed if the handle was null */
        printf("open [%s] libshm media handle failed\n", shmname);
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));

    while (!g_exit) {
        libshm_media_item_param_t   ohi = {0};
        libshm_media_item_param_t   *datactx    = NULL;

        ret     = LibShmMediaPollReadData(h, &ohp, &ohi, timeout); // read out the data with timeout.
        uint32_t r_index = LibShmMediaGetReadIndex(h);
        uint32_t w_index = LibShmMediaGetWriteIndex(h);
        int64_t now = TVUUTIL_GET_SYS_MS64(); // to get the current system time.

        if (ret < 0)
        {
            printf("poll readable ret %d\n", ret);
            break;
        }
        else if (ret == 0) {
            /**
             *  timeout, but no data
             *  you can go on to wait, or break
             *  Here, 3 seconds no data, I choose break
             */
            //break;
            Sleep(1);
            continue;
        }

        datactx     = &ohi;
        {
            char vsample[5] = {"null"};
            if (datactx->p_vData)
            {
                strncpy(vsample, (char *)datactx->p_vData, 4);
            }
            char asample[5] = {"null"};
            if (datactx->p_aData)
            {
                strncpy(asample, (char *)datactx->p_aData, 4);
            }
            char exsample[5] = {"null"};
            if (datactx->p_CCData)
            {
                strncpy(exsample, (char *)datactx->p_CCData, 4);
            }
            char subsample[5] = {"null"};
            if (datactx->p_sData)
            {
                strncpy(subsample, (char *)datactx->p_sData, 4);
            }
            printf("now %" PRId64 " readout, rindex %u, windex %u, head[video fourcc 0x%08x, audio fourcc 0x%08x"
                ",vbr %d, sarw %d, sarh %d, src %dx%d, dst %dx%d, duration %d, scale %d"
                ", channels %x, depth %d, samplerate %d]"
                ", v[%d, %" PRId64 ", %" PRId64 ", %s ...]\n a[%d, %" PRId64 ", %" PRId64 ", %s ...]\n "
                "s[%d, %" PRId64 ", %" PRId64 ", %s]\n user data:[%d, 0x%x, %" PRId64 "]\n"
                , now
                , r_index, w_index
                , ohp.u_videofourcc, ohp.u_audiofourcc
                , ohp.i_vbr, ohp.i_sarw, ohp.i_sarh
                , ohp.i_srcw, ohp.i_srch
                , ohp.i_dstw, ohp.i_dsth
                , ohp.i_duration, ohp.i_scale
                , ohp.i_channels, ohp.i_depth, ohp.i_samplerate
                , datactx->i_vLen
                , datactx->i64_vpts
                , datactx->i64_vpts
                , vsample
                , datactx->i_aLen
                , datactx->i64_apts
                , datactx->i64_adts
                , asample
                , datactx->i_sLen
                , datactx->i64_spts
                , datactx->i64_sdts
                , subsample
                , datactx->i_userDataLen
                , datactx->i_userDataType
                , datactx->i64_userDataCT
            );

            if (ohi.p_userData && ohi.i_userDataLen>0 && ohi.i_userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2)
            {
                libshmmedia_extend_data_info_t myExt;
                {
                    memset(&myExt, 0, sizeof(myExt));
                }
                LibShmMeidaParseExtendData(&myExt, ohi.p_userData, ohi.i_userDataLen, ohi.i_userDataType); // parse out the extension data.
            }
        }
    }

    if (h)
    {
        LibShmMediaDestroy(h); // destroyed the shared memory.
        h   = NULL;
    }
    return 0;
}

static void help(const char *prog)
{
    printf("Usage:\n"
           "    %s <shmname>\n"
           "ex:%s vx1"
           "\n"
           , prog
           , prog
           );
    return;
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        help(argv[0]);
        return -1;
    }

    return ReadSampleCode(argv[1]);
}


