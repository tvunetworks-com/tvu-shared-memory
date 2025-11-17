/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media_variable_item.h"
#include "libshm_media.h"
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
    libshm_media_handle_t   h           = NULL;
    libshm_media_head_param_t   ohp     = {0};
    libshm_media_item_param_t   ohi     = {0};
    int64_t                     now     = 0;
    int64_t                     base    = 0;
    int64_t                     next_pts= 0;
    libshmmedia_extended_data_context_t hExtData = NULL;

    hExtData = LibshmMediaExtDataCreateHandle();
    
    h       = LibViShmMediaCreate(name, 1024, count, count*(item_size/*+2048*/));

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibViShmMediaGetVersion(h));
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
    ohp.u_audiofourcc   = 'a' << 24 | 'a' << 16 | 'c' << 8 | '0';
    ohp.i_channels      = 2;
    ohp.i_depth         = 16;
    ohp.i_samplerate    = 48000;

    now = TVUUTIL_GET_SYS_MS64();
    base = now;

    int vlen = item_size*30;
    uint8_t *video_data = (uint8_t *)malloc(vlen);

    memset((void *)video_data, 'v', vlen);

    while (!g_exit) {
        now = TVUUTIL_GET_SYS_MS64();
        next_pts    = base + i * 33;

        if (now < next_pts)
        {
            Sleep(1);
            continue;
        }

        vlen = (i%30)*item_size;

        ohi.i64_vdts    = 
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = vlen;
        ohi.i64_apts    = 
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = (uint8_t *)"a1a2a3a4a5";
        ohi.i_aLen      = 10;

        int ret = 0;

        libshmmedia_extended_entry_data_t   oEntry = {0};
        uint8_t aExtBuff[1024] = {0};
        uint32_t iExtBuffSize = 0;
        char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
        uint8_t cc608[73] = {0};
        memset(cc608, '6', 72);

        LibshmMediaExtDataResetEntry(hExtData);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID;
        oEntry.u_len = 36;
        oEntry.p_data = (const uint8_t*)uuid_str;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP;
        oEntry.u_len = 72;
        oEntry.p_data = cc608;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX;
        oEntry.u_len = 4;
        oEntry.p_data = cc608;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        iExtBuffSize = LibshmMediaExtDataGetEntryBuffSize(hExtData);

        assert(iExtBuffSize <= 1024);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;

        ret     = LibViShmMediaPollSendable(h, 0);

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
            ret = LibViShmMediaSendData(h, &ohp, &ohi);
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
        LibViShmMediaDestroy(h);
        h   = NULL;
    }
    return 0;
}

static int Write2(const char *name, int count, int item_size)
{
    unsigned int                i       = 0;
    libshm_media_handle_t   h           = NULL;
    libshm_media_head_param_t   ohp     = {0};
    libshm_media_item_param_t   ohiv     = {0};
    int64_t                     now     = 0;
    int64_t                     base    = 0;
    int64_t                     next_pts= 0;
    libshmmedia_extended_data_context_t hExtData = NULL;

    libshm_media_item_param_v1_t   &ohi     = ohiv.v0;

    hExtData = LibshmMediaExtDataCreateHandle();

    h       = LibViShmMediaCreate(name, 1024, count, count*(item_size+2048));

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    printf("shm version : %x\n", LibViShmMediaGetVersion(h));
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

        ohi.i64_vdts    =
        ohi.i64_vpts    = next_pts;
        ohi.p_vData     = (uint8_t *)video_data;
        ohi.i_vLen      = item_size;
        ohi.i64_apts    =
        ohi.i64_adts    = next_pts;
        ohi.p_aData     = (uint8_t *)"a1a2a3a4a5";
        ohi.i_aLen      = 10;

        int ret = 0;

        libshmmedia_extended_entry_data_t   oEntry = {0};
        uint8_t aExtBuff[1024] = {0};
        uint32_t iExtBuffSize = 0;
        char uuid_str[37] = "1234567890abcdefghijklfmopqrstuvwxyz";
        uint8_t cc608[73] = {0};
        memset(cc608, '6', 72);

        LibshmMediaExtDataResetEntry(hExtData);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID;
        oEntry.u_len = 36;
        oEntry.p_data = (const uint8_t*)uuid_str;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP;
        oEntry.u_len = 72;
        oEntry.p_data = cc608;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        oEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX;
        oEntry.u_len = 4;
        oEntry.p_data = cc608;
        LibshmMediaExtDataAddOneEntry(hExtData, &oEntry, aExtBuff, 1024);

        iExtBuffSize = LibshmMediaExtDataGetEntryBuffSize(hExtData);

        assert(iExtBuffSize <= 1024);

        ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        ohi.p_userData = aExtBuff;
        ohi.i_userDataLen = iExtBuffSize;

        int nbuffer = ohi.i_vLen+ohi.i_aLen+ohi.i_userDataLen;
        uint8_t *pbuf = LibViShmMediaItemApplyBuffer(h, nbuffer);
        libshm_media_item_addr_layout_t olayout;
        {
            memset(&olayout, 0, sizeof(olayout));
        }

        if (!pbuf)
        {
            printf("apply item buffer %d failed\n", nbuffer);
            return -1;
        }

        ret = LibViShmMediaItemPreGetWriteBufferLayout(h, &ohiv, pbuf, &olayout);
        if (ret < 0)
        {
            printf("get item layout failed\n");
            return -1;
        }

        memcpy(olayout.p_vData, ohi.p_vData, ohi.i_vLen);
        memcpy(olayout.p_aData, ohi.p_aData, ohi.i_aLen);
        memcpy(olayout.p_userData, ohi.p_userData, ohi.i_userDataLen);
        LibViShmMediaItemWriteBufferIgnoreInternalCopy(h, &ohp, &ohiv, pbuf);
        LibViShmMediaItemCommitBuffer(h, pbuf, nbuffer);

        i++;
#if defined _LIBSHM_MEDIA_V3_STEP_2
        free(ohi.pa_data);
#endif
    }

    if (h)
    {
        LibViShmMediaDestroy(h);
        h   = NULL;
    }
    return 0;
}

static int read_callback(void *opaq, libshm_media_item_param_t *datactx)
{
#if !defined _LIBSHM_MEDIA_V3_STEP_2
    printf("callback : v[%d, %lld, %lld, %s]\n a[%d, %lld, %lld, %s]\n s[%d, %lld, %lld, %5s]\n ext[%d, %5s]\n"
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
    }
#else
	for (int i = 0; i < datactx->i_counts; ++i) {
		fprintf(stderr, "callback : index %d [%d, %lld, %lld, %p = %s] \n",
			i,
			datactx->pa_data[i].i_len,
			datactx->pa_data[i].i64_pts,
			datactx->pa_data[i].i64_dts,
			datactx->pa_data[i].p_data,
			datactx->pa_data[i].p_data
		);

	}
#endif
    fflush(stdout);
    return 0;
}

int _search_callback(void *user, const libshm_media_head_param_t *head, const libshm_media_item_param_t*datactx)
{
    char sv[6] = {0};
    char sa[6] = {0};
    uint64_t now = _xxtvuutil_get_sys_ms64();
    uint64_t pts = datactx->i64_vpts>0?datactx->i64_vpts:datactx->i64_apts;

    snprintf(sv, sizeof(sv), "%s", (const char *)datactx->p_vData);
    snprintf(sa, sizeof(sa), "%s", (const char *)datactx->p_aData);
    printf("callback : pos:%u, -n:%ld,v:[%d, %ld, %ld, %s], a:[%d, %ld, %ld, %s], ul:%d, uc:%ld\n"
        , datactx->u_read_index
        , pts-now
        , datactx->i_vLen
        , datactx->i64_vpts
        , datactx->i64_vpts
        , sv
        , datactx->i_aLen
        , datactx->i64_apts
        , datactx->i64_adts
        , sa
        , datactx->i_userDataLen
        , datactx->i64_userDataCT
    );

    if (pts + 100 < now)
    {
        return 1;
    }

    static int i = 0;
    printf("i=%d\n", i++);
    return 0;
}

static int Read(const char *name)
{
    unsigned int                i       = 0;
    libshm_media_handle_t       h       = NULL;
    libshm_media_head_param_t   ohp     = {0};
    libshm_media_item_param_t   ohi     = {0};
    const char                  *shmname= name;
    int                         ret     = -1;
    int                         timeout = 3000;//3000 ms

    h       = LibViShmMediaOpen(shmname, NULL/*read_callback*/, NULL);

    if (!h) {
        printf("open [%s] libshm media handle failed\n", shmname);
        return -1;
    }

    printf("shm version : %x\n", LibViShmMediaGetVersion(h));

    usleep(1000000);

    int64_t t1 = _xxtvuutil_get_sys_ms64();
    uint64_t windex = LibViShmMediaGetWriteIndex(h);
    printf("search before." "wi:%ld\n", windex);
    int searchret = LibViShmMediaSearchItems(h, NULL, _search_callback);
    int64_t t2 = _xxtvuutil_get_sys_ms64();
    uint64_t rindex = LibViShmMediaGetReadIndex(h);
    windex = LibViShmMediaGetWriteIndex(h);
    printf("search result." "d:%ld, ret:%d, ri:%ld, wi:%ld\n", t2-t1, searchret, rindex, windex);

    //LibViShmMediaSeekReadIndexToWriteIndex(h);

    while (!g_exit) {
        libshm_media_item_param_t   ohi = {0};
        libshm_media_item_param_t   *datactx    = NULL;
        uint32_t r_index = 0;
        uint32_t w_index = 0;

        r_index = LibViShmMediaGetReadIndex(h);

        ret     = LibViShmMediaPollReadData(h, &ohp, &ohi, timeout);

        if (ret < 0)
        {
            printf("poll readdata failed, ret %d\n", ret);
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

        w_index = LibViShmMediaGetWriteIndex(h);

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

            {
                printf("readout, rindex %u, windex %u, v[%d, %lld, %lld, %s ...]\n a[%d, %lld, %lld, %s ...]\n "
                    "s[%d, %lld, %lld, %s]\n ext[%d, %s], user data[%d, 0x%x, %lld]\n"
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
                );
                fflush(stdout);
            }
        }
        //usleep(1*1000);
    }

    if (h)
    {
        LibViShmMediaDestroy(h);
        h   = NULL;
    }
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

    fprintf(stdout, "time %ld, %s", _xxtvuutil_get_sys_ms64(), slog);
    fflush(stdout);
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
        //while (!g_exit)
        {
            Read(name);
#ifdef TVU_WINDOWS
			::Sleep(1);
#else
            usleep(1000000);
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

