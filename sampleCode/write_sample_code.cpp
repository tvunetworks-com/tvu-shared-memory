#include "libshmmedia.h"
#include "libtvu_media_fourcc.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string>

#define TVUUTIL_GET_SYS_MS64()                  _xxtvuutil_get_sys_ms64()
#define VIDEO_FRAME_SIZE                        10240
#define TVU_LINUX 1

#define _MAX(a,b)    ((a)>(b))?(a):(b)
#define _MIN(a,b)    ((a)<(b))?(a):(b)

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

static uint8_t *CreateUYVYBlackVideoFrame(int w, int h, uint32_t &frameSize)
{
    uint8_t *pFrame = NULL;

    frameSize = 2 *  w * h;

    pFrame = (uint8_t *)malloc(frameSize);

    if (!pFrame)
    {
        return NULL;
    }

    /* fill black */
    for (unsigned int i = 0;i < frameSize / 4; i++)
    {
         ((uint32_t *)pFrame)[i] = 0x10801080;
    }

    return pFrame;
}

static void DestroyUYVYBlackVideoFrame(uint8_t *videoFrame)
{
    if (videoFrame)
    {
        free(videoFrame);
    }
    return;
}

/**
 * The audio channel layout is 32bit, every 4bit express the channel number of one track, so it support maximum 8 tracks.
 * As channel layout is 0x22222222, which means 8 tracks stereo.
 * However for support 16 track mono, just to use 0xFFFF10 to express 16 track mono.
**/
struct AudioChannelLayout
{
    int ntrack;
    int nChannelCounts;
    uint8_t tracks[16];
};

static bool parseAudioChannelLayout(AudioChannelLayout &out, uint32_t layout)
{
    int i = 0;
    if (((layout & 0xF) == 0))
    {
        if (layout == 0xFFFF10)
        {
            out.nChannelCounts = 16;
            out.ntrack = 16;
            for (i = 0; i < out.ntrack; i++)
            {
                out.tracks[i] = 1;
            }
            return true;
        }
        return false;
    }

    uint64_t tmp = layout;
    memset(&out, 0, sizeof(out));

    while ((tmp & 0xF))
    {
        int chan = (tmp & 0xF);
        out.nChannelCounts += chan;
        out.tracks[out.ntrack] = chan;
        out.ntrack += 1;
        tmp = (tmp >> 4);
    }

    return true;
}


static uint8_t *CreateMuteAudioFrame(int depth, const AudioChannelLayout &chLayout, uint32_t sampleCount, uint32_t &frameSize)
{
    uint8_t *pFrame = NULL;

    uint32_t sampleBytes = (depth>>3);

    frameSize = sampleBytes * chLayout.nChannelCounts * sampleCount;


    pFrame = (uint8_t *)malloc(frameSize);

    if (!pFrame)
    {
        return NULL;
    }

    /* fill mute */
    memset(pFrame, 0, frameSize);

    return pFrame;
}

static void DestroyMuteAudioFrame(uint8_t *pFrame)
{
    if (pFrame)
    {
        free(pFrame);
    }
    return;
}

static uint8_t *gpbuff = NULL;
static uint32_t gbuffSize = 0;

static uint8_t *AllocExtBuffer(uint32_t s)
{
    if (gbuffSize < s)
    {
        gpbuff = (uint8_t *)realloc(gpbuff,s);
        if (!gpbuff)
        {
            gbuffSize = 0;
            return NULL;
        }

        gbuffSize = s;
    }

    return gpbuff;
}

static void FreeExtBuffer()
{
    if (gpbuff)
    {
        free(gpbuff);
        gpbuff = NULL;
    }
    gbuffSize = 0;
    return;
}

static bool g_exit = false;

static int WriteSampleCode(const char *name)
{
    int count = 10;
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

    int64_t     now     = 0;
    int64_t     base    = 0;
    int64_t     next_pts= 0;
    uint32_t    videoFrameSize = 0;
    uint32_t    itemSize = 0;
    uint32_t    frameMs = 0;
    uint32_t audioFrameSize = 0;
    uint32_t sampleCounts = 0;
    uint8_t *audioFrame = NULL;
    uint32_t shmItemHeadSize = 1024;

    AudioChannelLayout layout;
    {
        memset(&layout, 0, sizeof(layout));
    }

    ohp.i_dstw          = 1920; /* video width */
    ohp.i_dsth          = 1080; /* video height */
    ohp.u_videofourcc   = K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422 ; /* UYVY */
    ohp.i_duration      = 1000; /* fps den */
    ohp.i_scale         = 25000; /* fps num */
    ohp.u_audiofourcc   = K_TVU_AUDIO_FOURCC_WAVE_48K_16; /* audio format, Singed 16, 48K sample rate */
    ohp.i_channels      = 0x22; /* channel layout, two tracks stereo */
    ohp.i_depth         = 16; /* audio depth */
    ohp.i_samplerate    = 48000; /* sample rate */
    frameMs = ohp.i_duration * 1000 / ohp.i_scale;

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    uint8_t *videoFrame = NULL;

    /*alloc video frame*/
    {
        videoFrame = CreateUYVYBlackVideoFrame(ohp.i_dstw, ohp.i_dsth, videoFrameSize);

        if (!videoFrame || !videoFrameSize)
        {
            goto EXT;
        }

        itemSize += videoFrameSize;
    }

    /* audio frame alloc */
    {
        parseAudioChannelLayout(layout, ohp.i_channels);
        sampleCounts = frameMs * ohp.i_samplerate / 1000;
        audioFrame = CreateMuteAudioFrame(ohp.i_depth, layout, sampleCounts, audioFrameSize);
        if (!audioFrame || !audioFrameSize)
        {
            goto EXT;
        }
        itemSize += audioFrameSize;
    }

    {
        #define SHAREDMEMORY_ITEM_METADATA_PRE_SIZE (2048)
        itemSize +=  SHAREDMEMORY_ITEM_METADATA_PRE_SIZE;
    }

    h = LibShmMediaCreate(name, shmItemHeadSize, count, itemSize); // create shared memory handle with head size, item count, item size

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * frameMs;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        /* prepare video/audio data. */
        ohi.i64_vdts    = 
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)videoFrame;
        ohi.i_vLen      = videoFrameSize;
        ohi.i64_apts    = 
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = audioFrame;
        ohi.i_aLen      = audioFrameSize;

        int ret = 0;

        /* prepare extension data. */
        {
            uint32_t timecode = 0x1232178;
            uint8_t *aExtBuff = NULL;
            uint32_t iExtBuffSize = 0;
            char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
            uint8_t cc608[73] = {0};
            memset(cc608, '6', 72);

            libshmmedia_extend_data_info_t myExt;
            {
                memset(&myExt, 0, sizeof (myExt));
            }

            {
                myExt.p_cc608_cdp_data = cc608;
                myExt.i_cc608_cdp_length = 72;
            }

            {
                myExt.p_uuid_data = (const uint8_t *)uuid_str;
                myExt.i_uuid_length = strlen(uuid_str);
            }

            {
                myExt.p_timecode = (const uint8_t*)&timecode;
                myExt.i_timecode = sizeof (uint32_t);
            }

            {
                myExt.p_metaDataPts = (const uint8_t *)&next_pts; /* make sure little endian here, & 64bit */
                myExt.i_metaDataPts = sizeof(next_pts);
            }

            int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
            aExtBuff = AllocExtBuffer(iExtBuffSizeBeforeAlloc);
            if (aExtBuff)
            {
                iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);
                ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2; // extension data need this type.
                ohi.p_userData = aExtBuff;
                ohi.i_userDataLen = iExtBuffSize;
                ohi.i64_userDataCT = next_pts;
            }
        }

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


EXT:
    if (videoFrame)
    {
        DestroyUYVYBlackVideoFrame(videoFrame);
    }

    if (audioFrame)
    {
        DestroyMuteAudioFrame(audioFrame);
    }

    FreeExtBuffer();

    if (h)
    {
        now = TVUUTIL_GET_SYS_MS64();
        printf("now %" PRId64 ", finish writing counts %d, would call LibShmMediaDestroy\n", now, count);
        LibShmMediaDestroy(h); // destory the shared memory.
        h   = NULL;
    }
    return 0;
}

static int WriteVideoSampleCode(const char *name)
{
    int count = 10;
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

    int64_t     now     = 0;
    int64_t     base    = 0;
    int64_t     next_pts= 0;
    uint32_t    videoFrameSize = 0;
    uint32_t    itemSize = 0;
    uint32_t    frameMs = 0;
    uint32_t shmHeadSize = 1024;

    ohp.i_dstw          = 1920; /* video width */
    ohp.i_dsth          = 1080; /* video height */
    ohp.u_videofourcc   = K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422 ; /* UYVY */
    ohp.i_duration      = 1000; /* fps den */
    ohp.i_scale         = 25000; /* fps num */
    frameMs = ohp.i_duration * 1000 / ohp.i_scale;

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    uint8_t *videoFrame = NULL;

    /*alloc video frame*/
    {
        videoFrame = CreateUYVYBlackVideoFrame(ohp.i_dstw, ohp.i_dsth, videoFrameSize);

        if (!videoFrame || !videoFrameSize)
        {
            goto EXT;
        }

        itemSize += videoFrameSize;
    }

    {
        #define SHAREDMEMORY_ITEM_METADATA_PRE_SIZE (2048)
        itemSize +=  SHAREDMEMORY_ITEM_METADATA_PRE_SIZE;
    }

    h = LibShmMediaCreate(name, shmHeadSize, count, itemSize); // create shared memory handle with head size, item count, item size

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * frameMs;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        /* prepare video/audio data. */
        ohi.i64_vdts    =
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)videoFrame;
        ohi.i_vLen      = videoFrameSize;

        int ret = 0;
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


EXT:
    if (videoFrame)
    {
        DestroyUYVYBlackVideoFrame(videoFrame);
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

static int WriteAudioSampleCode(const char *name)
{
    int count = 10;
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

    int64_t     now     = 0;
    int64_t     base    = 0;
    int64_t     next_pts= 0;
    uint32_t    itemSize = 0;
    uint32_t    frameMs = 20;
    uint32_t audioFrameSize = 0;
    uint32_t sampleCounts = 0;
    uint8_t *audioFrame = NULL;
    uint32_t shmItemHeadSize = 1024;

    AudioChannelLayout layout;
    {
        memset(&layout, 0, sizeof(layout));
    }

    ohp.u_audiofourcc   = K_TVU_AUDIO_FOURCC_WAVE_48K_16; /* audio format, Singed 16, 48K sample rate */
    ohp.i_channels      = 0x22; /* channel layout, two tracks stereo */
    ohp.i_depth         = 16; /* audio depth */
    ohp.i_samplerate    = 48000; /* sample rate */

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    /* audio frame alloc */
    {
        parseAudioChannelLayout(layout, ohp.i_channels);
        sampleCounts = frameMs * ohp.i_samplerate / 1000;
        audioFrame = CreateMuteAudioFrame(ohp.i_depth, layout, sampleCounts, audioFrameSize);
        if (!audioFrame || !audioFrameSize)
        {
            goto EXT;
        }
        itemSize += audioFrameSize;
    }

    h = LibShmMediaCreate(name, shmItemHeadSize, count, itemSize); // create shared memory handle with head size, item count, item size

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * frameMs;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        /* prepare video/audio data. */
        ohi.i64_apts    =
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = audioFrame;
        ohi.i_aLen      = audioFrameSize;

        int ret = 0;
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


EXT:

    if (audioFrame)
    {
        DestroyMuteAudioFrame(audioFrame);
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

static int WriteMetadataSampleCode(const char *name)
{
    int count = 10;
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

    int64_t     now     = 0;
    int64_t     base    = 0;
    int64_t     next_pts= 0;
    uint32_t    itemSize = 0;
    int    frameMs = 40;
    uint32_t shmItemHeadSize = 1024;
    int pretendMs = 300;
    int pretendCount = (pretendMs+frameMs)/frameMs;
    count = _MAX(count, pretendCount);

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    {
        #define SHAREDMEMORY_ITEM_METADATA_PRE_SIZE (2048)
        itemSize +=  SHAREDMEMORY_ITEM_METADATA_PRE_SIZE;
    }

    h = LibShmMediaCreate(name, shmItemHeadSize, count, itemSize); // create shared memory handle with head size, item count, item size

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * frameMs;

        if (now + pretendMs < next_pts)
        {
            Sleep(1);
            continue;
        }

        int ret = 0;

        /* prepare extension data. */
        {
            uint32_t timecode = 0x12345678;
            uint8_t *aExtBuff = NULL;
            uint32_t iExtBuffSize = 0;
            char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
            uint8_t cc608[73] = {0};
            memset(cc608, '6', 72);

            libshmmedia_extend_data_info_t myExt;
            {
                memset(&myExt, 0, sizeof (myExt));
            }

            {
                myExt.p_cc608_cdp_data = cc608;
                myExt.i_cc608_cdp_length = 72;
            }

            {
                myExt.p_uuid_data = (const uint8_t *)uuid_str;
                myExt.i_uuid_length = strlen(uuid_str);
            }

            {
                myExt.p_timecode = (const uint8_t*)&timecode;
                myExt.i_timecode = sizeof (uint32_t);
            }


            {
                myExt.p_metaDataPts = (const uint8_t *)&next_pts; /* make sure little endian here, & 64bit */
                myExt.i_metaDataPts = sizeof(next_pts);
            }

            int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
            aExtBuff = AllocExtBuffer(iExtBuffSizeBeforeAlloc);
            if (aExtBuff)
            {
                iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);
                ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2; // extension data need this type.
                ohi.p_userData = aExtBuff;
                ohi.i_userDataLen = iExtBuffSize;
                ohi.i64_userDataCT = next_pts;
            }
        }

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

    FreeExtBuffer();

    if (h)
    {
        now = TVUUTIL_GET_SYS_MS64();
        printf("now %" PRId64 ", finish writing counts %d, would call LibShmMediaDestroy\n", now, count);
        LibShmMediaDestroy(h); // destory the shared memory.
        h   = NULL;
    }
    return 0;
}

static int url_find_tag_value(char *valueBuffer, int value_buffer_size, const char *tag1, const char *url)
{
    const char *p;
    char tag[128], *q;

    p = url;
    if (*p == '?')
        p++;
    for (;;)
    {
        q = tag;
        while (*p != '\0' && *p != '=' && *p != '&')
        {
            if ((q - tag) < (int)(sizeof(tag) - 1))
                *q++ = *p;
            p++;
        }
        *q = '\0';
        q = valueBuffer;
        if (*p == '=')
        {
            p++;
            while (*p != '&' && *p != '\0') {
                if ((q - valueBuffer) < value_buffer_size - 1)
                {
                    if (*p == '+')
                        *q++ = ' ';
                    else
                        *q++ = *p;
                }
                p++;
            }
        }
        *q = '\0';
        if (!strcmp(tag, tag1))
            return 1;
        if (*p != '&')
            break;
        p++;
    }
    return 0;
}

static int parserShmURL(const char* url, std::string& videoShm,
                 std::string& audioShm,
                 std::string &dataShm)
{
    bool gotV = false, gotA = false, gotD = false;
    char buffer[256];
    if (NULL == url)
        return -1;
    if (strncasecmp(url, "tvushm://", 6))
    {
        return -1;
    }
    const char* p = strchr(url, '?');
    if (NULL == p)
    {
        return -1;
    }
    if (url_find_tag_value(buffer, sizeof(buffer), "v", p))
    {
        videoShm = buffer;
        gotV = true;
    }

    if (url_find_tag_value(buffer, sizeof(buffer), "a", p))
    {
        gotA = true;
        audioShm = buffer;
    }

    if (url_find_tag_value(buffer, sizeof(buffer), "d", p))
    {
        gotD = true;
        dataShm = buffer;
    }

    return (gotA || gotV || gotD) ? 0 : -1;
}

static void help(const char *prog)
{
    printf("Usage:\n"
           "    %s <url> [mode]\n"
           "    url:\n"
           "        ex as tvushm://0?v=v1&a=a1&d=d1, v1 is the video shm name, a1 is the audio shm name, d1 is the meta data shm name.\n"
           "    mode:\n"
           "        0 - default value, to write All shm data together\n"
           "        1 - to write video shm data\n"
           "        2 - to write audio shm data\n"
           "        3 - to write meta shm data\n"
           "ex:%s \"tvushm://0?v=vx1&a=ax1&d=dx1\" 1\n"
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

    std::string videoShmName, audioShmName, dataShmName;

    if (parserShmURL(argv[1], videoShmName, audioShmName, dataShmName) < 0)
    {
        help(argv[0]);
        return -1;
    }

    int mode = 0;

    if (argc >= 3)
    {
        mode = atoi(argv[2]);
    }

    if (mode == 1)
    {
        if (!videoShmName.empty())
        {
            WriteVideoSampleCode(videoShmName.c_str());
        }
    }
    else if (mode == 2)
    {
        if (!audioShmName.empty())
        {
            WriteAudioSampleCode(audioShmName.c_str());
        }
    }
    else if (mode == 3)
    {
        if (!dataShmName.empty())
        {
            WriteMetadataSampleCode(dataShmName.c_str());
        }
    }
    else
    {
        if (!videoShmName.empty())
        {
            WriteSampleCode(videoShmName.c_str());
        }
    }

    return 0;
}


