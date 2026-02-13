/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media.h"
#include "libshm_media_raw_data_opt.h"
#include "libshm_media_audio_track_channel_protocol.h"
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

static const char* _xxxTransBinaryToHexString(const uint8_t *data, int len, char hx_str[], int hx_str_len)
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

    return hx_str;
}

static int Delete(const char *name)
{
#ifdef TVU_LINUX
    LibShmMediaRemoveShmidFromSystem(name);
#endif
    return 0;
}

static uint64_t basetvutimestamp = 0x0900000000000000 | 0x10;

static libshm_media_handle_t _createAndWriteShm(const char *shmName, int wcount)
{
    libshm_media_head_param_t   ohp     = {0};
    {
        LibShmMediaHeadParamInit(&ohp, sizeof (ohp));
    }
    libshm_media_item_param_t   ohi     = {0};
    {
        LibShmMediaItemParamInit(&ohi, sizeof (ohi));
    }
    libshm_media_handle_t   h           = NULL;
    int count = 1024;
    int item_size = 1024;
    int videolen = 16;
    h       = LibShmMediaCreate(shmName, 1024, count, item_size);

    ohp.i_vbr           = 10240;
    ohp.i_sarw          = 1;
    ohp.i_sarh          = 1;
    ohp.i_srcw          = 720;
    ohp.i_srch          = 480;
    ohp.i_dstw          = 1920;
    ohp.i_dsth          = 1080;
    ohp.u_videofourcc   = 'h' << 24 | '2' << 16 | '6' << 8  | '4';
    ohp.i_duration      = 1001;
    ohp.i_scale         = 30000;
    ohp.u_audiofourcc   = 'a' << 24 | 'a' << 16 | 0x02 << 8 | 0x02;
    ohp.i_channels      = 2;
    ohp.i_depth         = 16;
    ohp.i_samplerate    = 48000;

    uint64_t now = TVUUTIL_GET_SYS_MS64();
    uint64_t base = now;

    uint8_t *video_data = (uint8_t *)malloc(videolen + 32);

    memcpy(video_data, "1234567890abcdef", videolen);

    for (int i = 0; i < wcount; i++)
    {
        uint64_t next_pts    = base + i * 20;

        ohi.i64_vdts    =
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = videolen;
        ohi.i64_apts    =
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = NULL;
        ohi.i_aLen      = 0;

        int ret = 0;


#if 1
        uint8_t aExtBuff[1024] = {0};
        uint32_t iExtBuffSize = 0;
        uint64_t tvutimestamp = basetvutimestamp + i;


        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.i_timecode_fps_index = 8;
            myExt.p_timecode_fps_index = (const uint8_t *)&tvutimestamp;
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;


        libshmmedia_extend_data_info_t ext2;
        {
            memset(&ext2, 0, sizeof (ext2));
        }

        LibShmMeidaParseExtendDataV2(&ext2, aExtBuff, iExtBuffSize);
        if (i == 0)
            printf("n:%d, p:%" PRIx64 "\n"
                , ext2.i_timecode_fps_index
                , *(uint64_t *)ext2.p_timecode_fps_index
            );

#endif
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
            ret = LibShmMediaSendData(h, &ohp, &ohi);
            if (ret < 0) {
                printf("sendable status, send failed, why?, ret %d\n", ret);
                break;
            }
        }
    }

    return h;
}

static void _destroyShm(libshm_media_handle_t h)
{
    LibShmMediaDestroy(h);
}

static void gtest_tvutimestamp_searching()
{
    const char *shmName = "test000";
    libshm_media_handle_t hwrite =_createAndWriteShm(shmName, 100);

    /* test tvutimestamp search. */
    {
        libshm_media_handle_t h = LibShmMediaOpen(shmName, NULL, NULL);
        LibShmMediaSeekReadIndex(h, 0);
        libshm_media_item_param_t ohi;
        {
            LibShmMediaItemParamInit(&ohi, sizeof(libshm_media_item_param_t));
        }
        bool bGot = LibShmMediaSearchItemWithTvutimestamp(h, basetvutimestamp+10, &ohi);
        if (bGot)
        {
            printf("got tvutimestamp.\n");
        }
        else
        {
            printf("not got tvutimestamp.\n");
        }
    }

    _destroyShm(hwrite);
    return;
}

#define  _DEBUG_WRITE_DESTROY   0

static int Write(const char *name, int count, int item_size)
{
    unsigned int                i       = 0;
    libshm_media_handle_t   h           = NULL;
    libshm_media_head_param_t   ohp     = {0};
    {
        LibShmMediaHeadParamInit(&ohp, sizeof (ohp));
    }
    libshm_media_item_param_t   ohiv     = {0};
    {
        LibShmMediaItemParamInit(&ohiv, sizeof (ohiv));
    }
    int64_t                     now     = 0;
    int64_t                     base    = 0;
    int64_t                     next_pts= 0;

    libshm_media_item_param_t   &ohi = ohiv;
    
    h       = LibShmMediaCreate(name, 1024, count, item_size+2048);

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    libshmmedia_audio_channel_layout_handle_t hChannel = NULL;
    hChannel = LibshmmediaAudioChannelLayoutCreate();
    if (!hChannel)
    {
        printf("create channellayout handle failed.\n");
        return -1;
    }
    uint16_t channellayout[] = {0x02, 0x02, 0x06, 0x08};
    uint16_t nChannelLayout = sizeof(channellayout)/sizeof(channellayout[0]);

    if (LibshmmediaAudioChannelLayoutSerializeToBinary(hChannel, channellayout, nChannelLayout,false))
    {
        ohp.h_channel = hChannel;
        const uint8_t *pBin = NULL;
        uint32_t nBin = 0;
        char str[256] = {0};
        LibshmmediaAudioChannelLayoutGetBinaryAddr(hChannel, &pBin, &nBin);

        printf("write channe bin."
               "n:%u, bin:{%s}"
               "\n"
               , nBin, _xxxTransBinaryToHexString(pBin, nBin, str, sizeof(str))
               );
    }
    else
    {
        printf("impossile the channel serial failed.\n");
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    ohp.i_vbr           = 10240;
    ohp.i_sarw          = 1;
    ohp.i_sarh          = 1;
    ohp.i_srcw          = 720;
    ohp.i_srch          = 480;
    ohp.i_dstw          = 1920;
    ohp.i_dsth          = 1080;
    ohp.u_videofourcc   = 'h' << 24 | '2' << 16 | '6' << 8  | '4';
    ohp.i_duration      = 1001;
    ohp.i_scale         = 30000;
    ohp.u_audiofourcc   = 'a' << 24 | 'a' << 16 | 0x02 << 8 | 0x02;
    ohp.i_channels      = 2;
    ohp.i_depth         = 16;
    ohp.i_samplerate    = 48000;

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    uint8_t *video_data = (uint8_t *)malloc(item_size);

    memcpy(video_data, "1234567890abcdef", 16);
    memset((void *)(video_data+16), 'v', item_size-16);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * 33;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        ohi.i64_vdts    = 
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = item_size;
        ohi.i64_apts    = 
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = (uint8_t *)"1234567890abcdef";
        ohi.i_aLen      = 16;

        int ret = 0;

#if 1
        uint8_t aExtBuff[1024] = {0};
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

        uint32_t timecode = 0;
        {
            myExt.p_timecode = (const uint8_t*)&timecode;
            myExt.i_timecode = sizeof (uint32_t);
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;

#endif

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
            ret = LibShmMediaSendData(h, &ohp, &ohiv);
            if (ret < 0) {
                printf("sendable status, send failed, why?, ret %d\n", ret);
                break;
            }
        }

        i++;
#if defined _LIBSHM_MEDIA_V3_STEP_2
		free(ohi.pa_data);
#endif
    }

    if (h)
    {
        now = TVUUTIL_GET_SYS_MS64();
        printf("now %" PRId64 ", finish writing counts %d, would call LibShmMediaDestroy\n", now, count);
        LibShmMediaDestroy(h);
        h   = NULL;
    }

    LibshmmediaAudioChannelLayoutDestroy(hChannel);
    hChannel = NULL;

    if (video_data)
        free(video_data);
    return 0;
}

static int create_subtitle_private_protocol(uint8_t **ppSubtitle)
{
    int ret = 0;
    LibShmmediaSubtitlePrivateProtocolEntries entry = {};
    {
        const int counts = 3;
        entry.head.counts = counts;
        for (int i = 0; i < counts; i++)
        {
            libshmmedia_subtitle_private_proto_entry_item_t &item = entry.entries[i];

            if (i == 0)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT;
                item.timestamp = 323412341;
                item.duration = 2000;
                item.dataLen = 4;
                item.data = (const uint8_t *)"4444";
            }
            else if (i == 1)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT;
                item.timestamp = 666666;
                item.duration = 2000;
                item.dataLen = 5;
                item.data = (const uint8_t *)"11111";
            }
            else if (i == 2)
            {
                item.strucSize = sizeof (libshmmedia_subtitle_private_proto_entry_item_t);
                item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP;
                item.timestamp = 77777;
                item.duration = 3000;
                item.dataLen = 3;
                item.data = (const uint8_t *)"22222";
            }
        }


        int prelen = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entry);
        uint8_t *buffer = NULL;

        if (prelen < 0)
        {
            return -1;
        }

        buffer = (uint8_t *)malloc(prelen);
        if (!buffer)
        {
            return -1;
        }

        int len = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entry, buffer, prelen);

        if (len>prelen)
        {
            return -1;
        }

        *ppSubtitle = buffer;
        ret = len;
    }

    return ret;
}

static int Write2(const char *name, int count, int item_size)
{
    unsigned int                i       = 0;
    libshm_media_handle_t   h           = NULL;
    libshm_media_head_param_t   ohp;
    {
        LibShmMediaHeadParamInit(&ohp, sizeof (ohp));
    }
    libshm_media_item_param_t   ohiv;
    {
        LibShmMediaItemParamInit(&ohiv, sizeof (ohiv));
    }

    libshm_media_item_param_t   &ohi = ohiv;
    int64_t                     now     = 0;
    int64_t                     base    = 0;
    int64_t                     next_pts= 0;

    h       = LibShmMediaCreate(name, 1024, count, item_size+2048);

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));
    fflush(stdout);

    libshmmedia_audio_channel_layout_handle_t hChannel = NULL;
    hChannel = LibshmmediaAudioChannelLayoutCreate();
    if (!hChannel)
    {
        printf("create channellayout handle failed.\n");
        return -1;
    }
    uint16_t channellayout[] = {0x02, 0x02, 0x06, 0x08};
    uint16_t nChannelLayout = sizeof(channellayout)/sizeof(channellayout[0]);

    if (LibshmmediaAudioChannelLayoutSerializeToBinary(hChannel, channellayout, nChannelLayout, false))
    {
        ohp.h_channel = hChannel;
        const uint8_t *pBin = NULL;
        uint32_t nBin = 0;
        char str[256] = {0};
        LibshmmediaAudioChannelLayoutGetBinaryAddr(hChannel, &pBin, &nBin);

        printf("write channe bin."
               "n:%u, bin:{%s}"
               "\n"
               , nBin, _xxxTransBinaryToHexString(pBin, nBin, str, sizeof(str))
               );
    }
    else
    {
        printf("impossile the channel serial failed.\n");
    }

    ohp.i_vbr           = 10240;
    ohp.i_sarw          = 1;
    ohp.i_sarh          = 1;
    ohp.i_srcw          = 720;
    ohp.i_srch          = 480;
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

    memcpy(video_data, "1234567890abcdef", 16);
    memset((void *)(video_data+16), 'v', item_size-16);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * 33;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        ohi.i64_vdts    =
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = item_size;
        ohi.i64_apts    =
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = (uint8_t *)"1234567890abcdef";
        ohi.i_aLen      = 16;

        int ret = 0;

        libshmmedia_extend_data_info_t oe;
        {
            memset(&oe, 0, sizeof(oe));
        }


        uint8_t aExtBuff[1024] = {0};
        uint8_t subtitle[1024] = {"webvtt"};
        uint32_t iExtBuffSize = 0;
        char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
        uint8_t cc608[73] = {0};
        memset(cc608, '6', 72);

        {
            oe.i_uuid_length = 36;
            oe.p_uuid_data =  (const uint8_t*)uuid_str;
        }

        {
            oe.i_cc608_cdp_length = 72;
            oe.p_cc608_cdp_data =  cc608;
        }

        {
            oe.i_timecode = 4;
            oe.p_timecode =  cc608;
        }

        uint8_t *buffer = NULL;
        {
            int bufflen = create_subtitle_private_protocol(&buffer);
            oe.i_subtitle = bufflen;
            oe.p_subtitle =  buffer;
            oe.u_subtitle_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PRIVATE_PROTOCOL_EXTENTION_STRUCTURE;
        }

        iExtBuffSize = LibShmMediaEstimateExtendDataSize(&oe);

        assert(iExtBuffSize <= 1024);

        LibShmMediaWriteExtendData(aExtBuff, iExtBuffSize, &oe);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;

        libshm_media_item_addr_layout_t olayout;
        {
            memset(&olayout, 0, sizeof(olayout));
        }

        ret = LibShmMediaItemApplyBuffer(h, &ohiv, &olayout);

        if (ret <= 0)
        {
            printf("apply item buffer failed, ret %d\n", ret);
            {
                if (buffer)
                    free(buffer);
            }
            return -1;
        }

        memcpy(olayout.p_vData, ohi.p_vData, ohi.i_vLen);
        memcpy(olayout.p_aData, ohi.p_aData, ohi.i_aLen);
        memcpy(olayout.p_userData, ohi.p_userData, ohi.i_userDataLen);
        LibShmMediaItemCommitBuffer(h, &ohp, &ohiv);

        i++;
        {
            if (buffer)
                free(buffer);
        }
    }

    if (h)
    {
        now = TVUUTIL_GET_SYS_MS64();
        printf("now %" PRId64 ", finish writing counts %d, would call LibShmMediaDestroy\n", now, count);
        LibShmMediaDestroy(h);
        h   = NULL;
    }

    if (video_data)
        free(video_data);

    LibshmmediaAudioChannelLayoutDestroy(hChannel);
    hChannel = NULL;
    return 0;
}

static void print_ext_data(const uint8_t *puserData, const libshmmedia_extend_data_info_t *p)
{
    printf("userDataAddr %p, uuid[%p, %d], cc608[%p, %d], captionText[%p, %d], "
           "pp_stream[%p, %d], receiver_info[%p, %d], scte104[%p, %d],"
           " scte35[%p, %d], timecodIndex[%p, %d], startTimecood[%p, %d], "
           "hdr[%p, %d], fpsTimecode[%p, %d], pic_struct[%p, %d], "
           "src_timestamp[%p, %d], timecode[%p, %d], metaPts[%p, %d]\n"
           , puserData, p->p_uuid_data
           , p->i_uuid_length
           , p->p_cc608_cdp_data
           , p->i_cc608_cdp_length
           , p->p_caption_text
           , p->i_caption_text_length
           , p->p_producer_stream_info
           , p->i_producer_stream_info_length
           , p->p_receiver_info
           , p->i_receiver_info_length
           , p->p_scte104_data
           , p->i_scte104_data_len
           , p->p_scte35_data
           , p->i_scte35_data_len
           , p->p_timecode_index
           , p->i_timecode_index_length
           , p->p_start_timecode
           , p->i_start_timecode_length
           , p->p_hdr_metadata
           , p->i_hdr_metadata
           , p->p_timecode_fps_index
           , p->i_timecode_fps_index
           , p->p_pic_struct
           , p->i_pic_struct
           , p->p_source_timestamp
           , p->i_source_timestamp
           , p->p_timecode
           , p->i_timecode
           , p->p_metaDataPts
           , p->i_metaDataPts
    );
}

static int read_callback(void *opaq, libshm_media_item_param_t *datactxV)
{ 
    libshm_media_item_param_v1_t *datactx = (libshm_media_item_param_v1_t*)datactxV;
    printf("callback : v[%d, %lld, %lld, %s]\n a[%d, %lld, %lld, %s]\n s[%d, %lld, %lld, %s]\n ext[%d, %s]\n"
        , datactx->i_vLen
        , datactx->i64_vpts
        , datactx->i64_vpts
		, datactx->p_vData
        , datactx->i_aLen
        , datactx->i64_apts
        , datactx->i64_adts
		, datactx->p_aData
        , datactx->i_sLen
        , datactx->i64_spts
        , datactx->i64_sdts
		, datactx->p_sData
        , datactx->i_CCLen
		, datactx->p_CCData
    );

    if(datactx->i_userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2)
    {
#if 1
        libshmmedia_extend_data_info_t oExt;
        {
            memset(&oExt, 0, sizeof(oExt));
        }
        int failed = LibShmMeidaParseExtendData(&oExt, datactx->p_userData, datactx->i_userDataLen, datactx->i_userDataType);

        if (!failed)
        {
            print_ext_data(datactx->p_userData, &oExt);
        }

        libshmmedia_extended_data_context_t v2Ctx = LibshmMediaExtDataCreateHandle();
        {
            memset(&oExt, 0, sizeof(oExt));
        }
        failed = LibShmMediaReadExtendData(&oExt, datactx->p_userData, datactx->i_userDataLen, datactx->i_userDataType,v2Ctx);
        if (!failed)
        {
            print_ext_data(datactx->p_userData, &oExt);
        }
        LibshmMediaExtDataDestroyHandle(&v2Ctx);
#else
        libshmmedia_extended_data_context_t hExtDataR = LibshmMediaExtDataCreateHandle();

        if(!hExtDataR)
        {
            printf("create ext data handle failed\n");
        }

        if (hExtDataR)
        {
            LibshmMediaExtDataParseBuff(hExtDataR, datactx->p_userData, datactx->i_userDataLen);

            int entry_couts = LibshmMediaExtDataGetEntryCounts(hExtDataR);
            int i = 0;
            for (i = 0; i<entry_couts;i++)
            {
                libshmmedia_extended_entry_data_t   oEntryR = {0};
                LibshmMediaExtDataGetOneEntry(hExtDataR, i, &oEntryR);
                printf("entry index %d, type %u, len %u, data 0x%lx\n"
                    , i
                    , oEntryR.u_type
                    , oEntryR.u_len
                    , oEntryR.p_data
                );
            }

            LibshmMediaExtDataDestroyHandle(&hExtDataR);
        }

#endif
    }
    fflush(stdout);
    return 0;
}

static int Read(const char *name)
{
    unsigned int                i       = 0;
    libshm_media_handle_t       h       = NULL;
    libshm_media_head_param_t   ohp;
    {
        LibShmMediaHeadParamInit(&ohp,sizeof(libshm_media_head_param_t));
    }
    const char                  *shmname= name;
    int                         ret     = -1;
    int                         timeout = 3000;//3000 ms

    h       = LibShmMediaOpen(shmname, NULL, NULL);

    if (!h) {
        printf("open [%s] libshm media handle failed\n", shmname);
        return -1;
    }

    libshmmedia_audio_channel_layout_object_t *hChannel = LibshmmediaAudioChannelLayoutCreate();
    if (!hChannel)
    {
        return -1;
    }
    ohp.h_channel = hChannel;

    printf("shm version : %x\n", LibShmMediaGetVersion(h));

    while (!g_exit) {
        libshm_media_item_param_t   ohiv = {0};
        libshm_media_item_param_t   *datactx    = NULL;
        ret     = LibShmMediaPollReadData(h, &ohp, &ohiv, timeout);
        uint32_t r_index = LibShmMediaGetReadIndex(h);
        uint32_t w_index = LibShmMediaGetWriteIndex(h);
        int64_t now = TVUUTIL_GET_SYS_MS64();

        libshm_media_item_param_t   &ohi = ohiv;

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
            usleep(1000);
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

            const uint16_t *pChannel = NULL;
            uint16_t nChannel = 0;
            char hxStr[256] = {0};
            bool bGot = LibshmmediaAudioChannelLayoutGetChannelArr(hChannel, &pChannel, &nChannel);
            if (nChannel>0 && pChannel)
            {
                _xxxTransBinaryToHexString((const uint8_t *)pChannel, nChannel*sizeof(uint16_t),hxStr, sizeof(hxStr));
            }

            printf("now %" PRId64 " readout, rindex %u, windex %u, head[video fourcc 0x%08x, audio fourcc 0x%08x"
                ",vbr %d, sarw %d, sarh %d, src %dx%d, dst %dx%d, duration %d, scale %d"
                ", channels %x, depth %d, samplerate %d]"
                ", v[%d, %" PRId64 ", %" PRId64 ", %s ...]\n a[%d, %" PRId64 ", %" PRId64 ", %s ...]\n "
                "s[%d, %" PRId64 ", %" PRId64 ", %s]\n ext[%d, %s], user data[%d, 0x%x, %" PRId64 "]\n"
                "nchan:%hu, chan:{%s}\n"
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
                , datactx->i_CCLen
                , exsample
                , datactx->i_userDataLen
                , datactx->i_userDataType
                , datactx->i64_userDataCT
                , nChannel,hxStr
            );

            if (ohi.p_userData && ohi.i_userDataLen>0 && ohi.i_userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2)
            {
                libshmmedia_extend_data_info_t myExt;
                {
                    memset(&myExt, 0, sizeof(myExt));
                }
                LibShmMeidaParseExtendData(&myExt, ohi.p_userData, ohi.i_userDataLen, ohi.i_userDataType);
            }
            fflush(stdout);
        }
        //usleep(100*1000);
    }

    if (h)
    {
        LibShmMediaDestroy(h);
        h   = NULL;
    }

    LibshmmediaAudioChannelLayoutDestroy(hChannel);
    hChannel = NULL;
    return 0;
}

static int ReadSampleCode(const char *name)
{
    unsigned int                i       = 0;
    libshm_media_handle_t       h       = NULL;
    libshm_media_head_param_t   ohp     = {0};
    const char                  *shmname= name;
    int                         ret     = -1;
    int                         timeout = 3000;//3000 ms

    h       = LibShmMediaOpen(shmname, NULL, NULL);

    if (!h) {
        printf("open [%s] libshm media handle failed\n", shmname);
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));

    while (!g_exit) {
        libshm_media_item_param_t   ohiv = {0};
        libshm_media_item_param_t   *datactx    = NULL;
        ret     = LibShmMediaPollReadData(h, &ohp, &ohiv, timeout);
        uint32_t r_index = LibShmMediaGetReadIndex(h);
        uint32_t w_index = LibShmMediaGetWriteIndex(h);
        int64_t now = TVUUTIL_GET_SYS_MS64();

        libshm_media_item_param_t   &ohi = ohiv;

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
            usleep(1000);
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
                "s[%d, %" PRId64 ", %" PRId64 ", %s]\n ext[%d, %s], user data[%d, 0x%x, %" PRId64 "]\n"
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
                , datactx->i_CCLen
                , exsample
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
                LibShmMeidaParseExtendData(&myExt, ohi.p_userData, ohi.i_userDataLen, ohi.i_userDataType);
            }
        }
        //usleep(100*1000);
    }

    if (h)
    {
        LibShmMediaDestroy(h);
        h   = NULL;
    }
    return 0;
}

static int Read2(const char *name)
{
    unsigned int                i       = 0;
    libshm_media_handle_t       h       = NULL;
    libshmmedia_raw_head_param_t   ohp     = {0};
    {
        LibShmMediaRawHeadParamInit(&ohp, sizeof(ohp));
    }
    const char                  *shmname= name;
    int                         ret     = -1;
    int                         timeout = 3000;//3000 ms

    h       = LibShmMediaOpen(shmname, NULL/*read_callback*/, NULL);

    if (!h) {
        printf("open [%s] libshm media handle failed\n", shmname);
        return -1;
    }

    printf("shm version : %x\n", LibShmMediaGetVersion(h));

    libshmmedia_audio_channel_layout_object_t *hChannel = LibshmmediaAudioChannelLayoutCreate();
    if (!hChannel)
    {
        return -1;
    }

    while (!g_exit) {
        libshmmedia_raw_data_param_t   ohi = {0};
        {
            LibShmMediaRawDataParamInit(&ohi, sizeof(ohi));
        }
        libshmmedia_raw_data_param_t   *datactx1    = NULL;
        ret     = LibShmMediaRawDataRead(h, &ohp, &ohi, timeout);
        uint32_t r_index = LibShmMediaGetReadIndex(h);
        uint32_t w_index = LibShmMediaGetWriteIndex(h);
        int64_t now = TVUUTIL_GET_SYS_MS64();

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
            usleep(1000);
            continue;
        }

        datactx1     = &ohi;

        uint8_t *bin_buffer = (uint8_t *)malloc(datactx1->uRawData_);
        libshm_media_item_param_t oip;
        {
            LibShmMediaItemParamInit(&oip, sizeof(oip));
        }
        libshm_media_head_param_t ohp1;
        {
            LibShmMediaHeadParamInit(&ohp1, sizeof(ohp1));
            ohp1.h_channel = hChannel;
        }

        memcpy(bin_buffer, datactx1->pRawData_, datactx1->uRawData_);

        ret = LibShmMediaProtoReadItemBufferLayout(&ohp1, &oip, bin_buffer, datactx1->uRawData_);

        if (ret <= 0)
        {
            continue;
        }

        libshm_media_item_param_t * datactx     = &oip;
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

        const uint16_t *pChannel = NULL;
        uint16_t nChannel = 0;
        char hxStr[256] = {0};
        bool bGot = LibshmmediaAudioChannelLayoutGetChannelArr(hChannel, &pChannel, &nChannel);
        if (nChannel>0 && pChannel)
        {
            _xxxTransBinaryToHexString((const uint8_t *)pChannel, nChannel*sizeof(uint16_t),hxStr, sizeof(hxStr));
        }

        printf("now %" PRId64 " readout, rindex %u, windex %u, v[%d, %ld, %ld, %s ...]\n a[%d, %ld, %ld, %s ...]\n "
            "s[%d, %ld, %ld, %s]\n ext[%d, %s], user data[%d, 0x%x, %ld], nChan:%hu,chan:{%s}\n"
            , now
            , r_index, w_index
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
            , datactx->i_CCLen
            , exsample
            , datactx->i_userDataLen
            , datactx->i_userDataType
            , datactx->i64_userDataCT
            , nChannel, hxStr
        );
        fflush(stdout);


#if 1
        if (bin_buffer)
        {
            printf("now %" PRId64 " readout, rindex %u, windex %u, copy %lu raw data to buffer\n"
                , now
                , r_index, w_index
                , datactx1->uRawData_
            );
            memcpy(bin_buffer, datactx1->pRawData_, datactx1->uRawData_);
            free(bin_buffer);
        }

#endif

        usleep(10*1000);
    }

    if (h)
    {
        LibShmMediaDestroy(h);
        h   = NULL;
    }

    LibshmmediaAudioChannelLayoutDestroy(hChannel);
    hChannel = NULL;
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
	g_exit	= 1;
}

int main(int argc, char *argv[])
{
    int mode = 0;
    const char *name = TEST_SHM_NM;
    int count = 10;
    int item_size = 1024;
#if 1
    LibShmMediaSetLogCb(get_shm_log);

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
                else if (optarg && optarg[0] == '3')
                {
                    mode    = 3;
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
            Read2(name);
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
    else if (mode == 3)
    {
        gtest_tvutimestamp_searching();
    }
#elif 0 /* test robust */
    libshm_media_handle_t h = LibShmMediaCreate(argv[1], 1024, 100, 1024*1024*10);
    int ms = 3* 1024*1024;
    char *line = (char *)malloc(ms);

    if (!h)
        return 0;

    memset(line, 'a', ms);

   for (int i = 1; i != 0; i++)
    {
        libshm_media_head_param_t  omh = {0};
        libshm_media_item_param_t  omi = {0};
        int ret = 0;
        omi.i64_vpts = 1;
        omi.i64_vdts = 1;
        omi.p_vData = (const uint8_t *)line;
        omi.i_vLen = ms;
        ret = LibShmMediaSendData(h, &omh, &omi);
        printf("send ret %d\n", ret);
        usleep(100000);
    }
    
    if (h)
        LibShmMediaDestroy(h);
#elif 0 /* test ext data APIs */
    libshmmedia_extended_data_context_t hExtDataW = NULL;
    libshmmedia_extended_data_context_t hExtDataR = NULL;
    libshmmedia_extended_entry_data_t   oEntryW = {0};
    uint8_t aExtBuff[1024] = {0};
    uint32_t iExtBuffSize = 0;
    char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
    uint8_t cc608[73] = {0};
    memset(cc608, '6', 72);

    hExtDataW = LibshmMediaExtDataCreateHandle();

    if(!hExtDataW)
    {
        printf("create ext data handle failed\n");
        return -1;
    }

    LibshmMediaExtDataResetEntry(hExtDataW);

    oEntryW.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID;
    oEntryW.u_len = 36;
    oEntryW.p_data = (const uint8_t*)uuid_str;
    LibshmMediaExtDataAddOneEntry(hExtDataW, &oEntryW, aExtBuff, 1024);

    oEntryW.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP;
    oEntryW.u_len = 72;
    oEntryW.p_data = cc608;
    LibshmMediaExtDataAddOneEntry(hExtDataW, &oEntryW, aExtBuff, 1024);
    oEntryW.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX;
    oEntryW.u_len = 4;
    oEntryW.p_data = cc608;
    LibshmMediaExtDataAddOneEntry(hExtDataW, &oEntryW, aExtBuff, 1024);

    iExtBuffSize = LibshmMediaExtDataGetEntryBuffSize(hExtDataW);

    hExtDataR = LibshmMediaExtDataCreateHandle();

    if(!hExtDataR)
    {
        printf("create ext data handle failed\n");
        return -1;
    }
    LibshmMediaExtDataParseBuff(hExtDataR, aExtBuff, iExtBuffSize);

    int entry_couts = LibshmMediaExtDataGetEntryCounts(hExtDataR);
    int i = 0;
    for (i = 0; i<entry_couts;i++)
    {
        libshmmedia_extended_entry_data_t   oEntryR = {0};
        LibshmMediaExtDataGetOneEntry(hExtDataR, i, &oEntryR);
        printf("entry index %d, type %u, len %u, data 0x%lx\n"
            , i
            , oEntryR.u_type
            , oEntryR.u_len
            , oEntryR.p_data
        );
    }

    LibshmMediaExtDataDestroyHandle(&hExtDataW);
    LibshmMediaExtDataDestroyHandle(&hExtDataR);
#endif
    return 0;
}

