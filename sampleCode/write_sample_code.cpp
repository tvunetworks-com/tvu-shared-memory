#include "libshmmedia.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

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

static void Sleep(uint32_t ms)
{
    usleep(ms*1000);
}

static bool g_exit = false;

static int WriteSampleCode(const char *name, int count, int item_size)
{
    unsigned int                i       = 0;
    libshm_media_handle_t   h           = NULL;
    libshm_media_head_param_t   ohp;
    {
        memset(&ohp, 0, sizeof(ohp));
        ohp.u_structSize = sizeof(libshm_media_head_param_t);
    }
    libshm_media_item_param_t   ohi;
    {
        memset(&ohi, 0, sizeof(ohi));
        ohi.u_structSize = sizeof(libshm_media_item_param_t);
    }

    int64_t                     now     = 0;
    int64_t                     base    = 0;
    int64_t                     next_pts= 0;
    
    h       = LibShmMediaCreate(name, 1024, count, item_size); // create shared memory handle with head size, item count, item size

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    ohp.i_dstw          = 1920;
    ohp.i_dsth          = 1080;
    ohp.u_videofourcc   = 'h' << 24 | '2' << 16 | '6' << 8  | '4';
    ohp.i_duration      = 1001;
    ohp.i_scale         = 30000;
    ohp.u_audiofourcc   = 'a' << 24 | 'a' << 16 | 'c' << 8 | '0';
    ohp.i_channels      = 2;
    ohp.i_depth         = 16;
    ohp.i_samplerate    = 48000;

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    uint8_t *video_data = (uint8_t *)malloc(item_size);

    memset((void *)video_data, 'v', item_size);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * 33;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        /* prepare video/audio data. */
        ohi.i64_vdts    = 
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = item_size;
        ohi.i64_apts    = 
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = (uint8_t *)"a1a2a3a4a5";
        ohi.i_aLen      = 10;

        int ret = 0;
        uint8_t aExtBuff[1024] = {0};
        uint32_t iExtBuffSize = 0;
        char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
        uint8_t cc608[73] = {0};
        memset(cc608, '6', 72);


        /* prepare extension data. */
        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.p_caption_text = cc608;
            myExt.i_cc608_cdp_length = 72;
        }

        {
            myExt.p_uuid_data = (const uint8_t *)uuid_str;
            myExt.i_uuid_length = strlen(uuid_str);
        }

        {
            uint32_t timecode = 0;
            myExt.p_timecode = (const uint8_t*)&timecode;
            myExt.i_timecode = sizeof (uint32_t);
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2; // extension data need this type.
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;

        ret     = LibShmMediaPollSendable(h, 0);

        if (ret < 0) {
            printf("sending failed\n");
            break;
        } else if (ret == 0) {
            printf("need wait to resend\n");
            Sleep(1);
            continue;
        }
        else {
            // poll success
            ret = LibShmMediaSendData(h, &ohp, &ohi); // write the item data to shared memory.
            if (ret < 0) {
                printf("sendable status, send failed, why?, ret %d\n", ret);
                break;
            }
        }

        i++;
    }

    if (h)
    {
        now = TVUUTIL_GET_SYS_MS64();
        printf("now %" PRId64 ", finish writing counts %d, would call LibShmMediaDestroy\n", now, count);
        LibShmMediaDestroy(h); // destory the shared memory.
        h   = NULL;
    }
    return 0;
}


int main()
{
    return WriteSampleCode("test", 10, 10240);
}


