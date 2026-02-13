/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media_protocol.h"
#include "libshm_media_protocol_log_internal.h"
#include "libshm_media_bin_concat_protocol.h"
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

static void print_channel_info(libshmmedia_audio_channel_layout_object_t *h)
{
    const uint16_t *pDest = NULL;
    uint16_t nDest = 0;
    bool bPlanar = LibshmmediaAudioChannelLayoutIsPlanar(h);
    LibshmmediaAudioChannelLayoutGetChannelArr(h, &pDest, &nDest);

    char hxstr[256] = {0};
    _trans_raw_data_to_hex_string((const uint8_t*)pDest, nDest*sizeof(uint16_t), hxstr, sizeof(hxstr));

    printf("channel infor."
           "planar:%s,"
           "ch:%hu,bin{%s}"
           "\n"
           , bPlanar?"Yes":"No"
           , nDest, hxstr
           );

    return;
}

static void print_bin_info(libshmmedia_audio_channel_layout_object_t *h)
{
    const uint8_t *pDest = NULL;
    uint32_t nDest = 0;
    LibshmmediaAudioChannelLayoutGetBinaryAddr(h, &pDest, &nDest);
    char hxstr[256] = {0};
    _trans_raw_data_to_hex_string(pDest, nDest, hxstr, sizeof(hxstr));
    printf("bin infor."
           "len:%u,bin:{%s}"
           "\n"
           , nDest, hxstr
           );
    return;
}

static void print_bin_info2(const uint8_t *pDest, uint32_t nDest)
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

static int TestTrackChannelProto()
{
    libshmmedia_audio_channel_layout_object_t *h = LibshmmediaAudioChannelLayoutCreate();
    if (!h)
    {
        return -1;
    }

    uint16_t channel[256] = {0};
    for (unsigned int i = 0; i < sizeof(channel)/sizeof(uint16_t); i++)
    {
        channel[i] = 1;
    }
    uint16_t nChan = sizeof(channel)/sizeof(uint16_t);
    bool bret = false;
    bret = LibshmmediaAudioChannelLayoutSerializeToBinary(h, channel, nChan, true);
    if (!bret)
    {
        return -1;
    }

    {
        print_channel_info(h);
    }

    {
        const uint8_t *pDest = NULL;
        uint32_t nDest = 0;
        LibshmmediaAudioChannelLayoutGetBinaryAddr(h, &pDest, &nDest);
        char hxstr[256] = {0};
        _trans_raw_data_to_hex_string(pDest, nDest, hxstr, sizeof(hxstr));
        printf("bin infor."
               "len:%u,bin:{%s}"
               "\n"
               , nDest, hxstr
               );

        /* parse */
        libshmmedia_audio_channel_layout_object_t *h2 = LibshmmediaAudioChannelLayoutCreate();
        if (!h2)
        {
            return -1;
        }

        do
        {
            if (!LibshmmediaAudioChannelLayoutParseFromBinary(h2, pDest, nDest))
            {
                break;
            }

            {
                print_channel_info(h2);
                print_bin_info(h2);
            }
        } while (0);

        //char *p = (char *)malloc(1000);

        LibshmmediaAudioChannelLayoutDestroy(h2);
        h2 = NULL;

    }


    LibshmmediaAudioChannelLayoutDestroy(h);
    h = NULL;


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

static void print_ext_data(const libshmmedia_extend_data_info_t *p)
{
    char s1[16] = {"nil"};
    char s2[16] = {"nil"};
    char s3[16] = {"nil"};
    char s4[16] = {"nil"};

    if (p->bHasColorPrimariesVal_)
    {
        snprintf(s1, sizeof(s1), "%u", p->uColorPrimariesVal_);
    }
    if (p->bHasColorTransferCharacteristicVal_)
    {
        snprintf(s2, sizeof(s2), "%u", p->uColorTransferCharacteristicVal_);
    }
    if (p->bHasColorSpaceVal_)
    {
        snprintf(s3, sizeof(s3), "%u", p->uColorSpaceVal_);
    }
    if (p->bHasVideoFullRangeFlagVal_)
    {
        snprintf(s4, sizeof(s4), "%u", p->uVideoFullRangeFlagVal_);
    }

    printf(
        "extension data info."
        "uuid[%p, %d], cc608[%p, %d], captionText[%p, %d], "
        "pp_stream[%p, %d], receiver_info[%p, %d], scte104[%p, %d],"
        " scte35[%p, %d], timecodIndex[%p, %d], startTimecood[%p, %d], "
        "hdr[%p, %d], fpsTimecode[%p, %d], pic_struct[%p, %d], "
        "src_timestamp[%p, %d], timecode[%p, %d], metaPts[%p, %d]"
        ",colorParamies:%s"
        ",colorTransfer:%s"
        ",colorSpace:%s"
        ",videoFullRange:%s"
        "\n"
        , p->p_uuid_data
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
        , s1, s2, s3, s4
    );
}

static void TestExtensionDataProto()
{
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

    /* setting color primary */
    {
        oe.bHasColorPrimariesVal_ = true;
        oe.uColorPrimariesVal_ = 1;
    }

    /* setting color transfer characteristic */
    {
        oe.bHasColorTransferCharacteristicVal_ = true;
        oe.uColorTransferCharacteristicVal_ = 2;
    }

    /* setting color space */
    {
        oe.bHasColorSpaceVal_ = true;
        oe.uColorSpaceVal_ = 3;
    }

    /* setting video full range flag */
    {
        oe.bHasVideoFullRangeFlagVal_ = false;
        oe.uVideoFullRangeFlagVal_ = 1;
    }

    iExtBuffSize = LibShmMediaEstimateExtendDataSize(&oe);

    assert(iExtBuffSize <= 1024);

    int nw = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSize, &oe);

    if (buffer)
    {
        free(buffer);
    }

    if (nw <= 0)
    {
        return;
    }

    libshmmedia_extend_data_info_t oe1;
    {
        memset(&oe1, 0, sizeof(oe1));
    }
    LibShmMeidaParseExtendDataV2(&oe1, aExtBuff, nw);

    print_ext_data(&oe1);
    return;
}

static void TestBinConcatProto2(libshmmedia_bin_concat_proto_handle_t h)
{
    char s1[50] = {0};
    {
        memset(s1, 'a', sizeof(s1));
    }

    char s2[50] = {0};
    {
        memset(s2, 'b', sizeof(s2));
    }

    char s3[50] = {0};
    {
        memset(s3, 'c', sizeof(s3));
    }

    LibshmmediaBinConcatProtoReset(h);
    libshmmedia_bin_concat_proto_seg_t oseg = {0};
    {
        oseg.pSeg = (const uint8_t *)s1;
        oseg.nSeg = sizeof(s1);
    }
    if (!LibshmmediaBinConcatProtoConcatSegment(h, &oseg))
    {
        return;
    }

    {
        oseg.pSeg = (const uint8_t *)s2;
        oseg.nSeg = sizeof(s2);
    }
    if (!LibshmmediaBinConcatProtoConcatSegment(h, &oseg))
    {
        return;
    }

    {
        oseg.pSeg = (const uint8_t *)s3;
        oseg.nSeg = sizeof(s3);
    }
    if (!LibshmmediaBinConcatProtoConcatSegment(h, &oseg))
    {
        return;
    }

    const uint8_t *pBin = NULL;
    uint32_t nBin = 0;
    {
        if (!LibshmmediaBinConcatProtoFlushBinary(h, &pBin, &nBin))
        {
            return;
        }
    }

    //print_bin_info2(pBin, nBin);


    libshmmedia_bin_concat_proto_handle_t h2 = LibshmmediaBinConcatProtoCreate();
    LibshmmediaBinConcatProtoReset(h2);
    uint8_t xx[] = {0x1d,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x81,0x70,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23,0x01,0x00,0x01,0x06,0x0e,0x2b,0x34,0x02,0x05,0x01,0x01,0x0e,0x01,0x01,0x03,0x11,0x00,0x00,0x00,0x09,0x9f,0x00,0x06,0x41,0x1b,0xc4,0xa1,0x13,0xe2,0x23};
    libshmmedia_bin_concat_proto_seg_t *pseg = NULL;
    uint32_t nseg = 0;

    pBin = xx;
    nBin = sizeof(xx);

    if (!LibshmmediaBinConcatProtoSplitBinary(h2, pBin, nBin, true, &pseg, &nseg))
    {
        LibshmmediaBinConcatProtoDestroy(h2);
        return;
    }

    printf("got seg."
           "n:%u"
           "\n"
           ,nseg
           );

    for (unsigned int i = 0; i < nseg; i++)
    {
        print_bin_info2(pseg[i].pSeg, pseg[i].nSeg);
    }

    LibshmmediaBinConcatProtoDestroy(h2);
    return;
}

static int TestBinConcatProto()
{
    libshmmedia_bin_concat_proto_handle_t h = LibshmmediaBinConcatProtoCreate();
    if (!h)
    {
        return -1;
    }

    TestBinConcatProto2(h);

    //TestBinConcatProto2(h);

    LibshmmediaBinConcatProtoDestroy(h);

    return 0;
}

static int TestBufferCtrl()
{
//    tvushm::BufferController_t op;
//    {
//        tvushm::BufferCtrlInit(&op);
//    }
//    tvushm::BufferCtrlCompactEncodeValueU32(&op, 0x32);

//    tvushm::BufferController_t op2;
//    {
//        tvushm::BufferCtrlInit(&op2);
//    }

//    uint8_t *buf = tvushm::BufferCtrlGetOrigPtr(&op);
//    uint32_t nbuf = tvushm::BufferCtrlGetBufLen(&op);
//    tvushm::BufferCtrlAttachExternalReadBuffer(&op2, buf, nbuf);

//    uint32_t v = 0;
//    int x = tvushm::BufferCtrlCompactDecodeValueU32(&op2, v);

//    print_bin_info2(buf, nbuf);

//    printf("get value."
//           "x:%d, v:%u"
//           , x, v
//           );

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

    LibShmMediaProtoSetLogCbInternalV2(get_shm_log);

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
    printf("to begin with the test of track-channel-layout protocol\n");
    TestTrackChannelProto();
    printf("to end with the test of track-channel-layout protocol\n");
    printf("------------------------------------------------------------------------------\n");
    printf("to begin with the test of bin-concat protocol\n");
    TestBinConcatProto();
    printf("to end with the test of bin-concat protocol\n");

    printf("------------------------------------------------------------------------------\n");
    printf("to begin with extension data protocol\n");
    TestExtensionDataProto();
    printf("to end with extension data protocol\n");


    return 0;
}

