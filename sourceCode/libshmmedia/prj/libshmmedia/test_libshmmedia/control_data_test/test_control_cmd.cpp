/*******************************************************************************************
 *  Description:
 *      used to test libshmmediawrap.
 *  CopyRight:
 *      TVU/lotus.
*******************************************************************************************/
#include "libshm_media_variable_item.h"
#include "libshm_data_protocol.h"
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
    libshmmedia_ctrlcmd_wrap_handle_t   h           = NULL;
    
    h       = LibShmMediaCtrlCmdWrapHandleCreate(name, 1024, count, count*(item_size+2048));

    if (!h)
    {
        printf("create libshm media handle failed\n");
        return -1;
    }

    libtvumedia_ctrlcmd_data_t oinsert;
    {
        oinsert.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        oinsert.u_command_type = kLibTvuMediaCtrlCmdInsertKF;
        oinsert.o_params.o_insertKF.u_program_index = 10;
    }
    libtvumedia_ctrlcmd_data_t oBitrate;
    {
        oBitrate.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        oBitrate.u_command_type = kLibTvuMediaCtrlCmdChangeBitRate;
        oBitrate.o_params.o_changeBitrate.u_vbitrate = 100000000;
        oBitrate.o_params.o_changeBitrate.u_abitrate = 64000;
    }
    libtvumedia_ctrlcmd_data_t oResolution;
    {
        oResolution.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        oResolution.u_command_type = kLibTvuMediaCtrlCmdChangeResolution;
        oResolution.o_params.o_changeResolution.u_width = 1920;
        oResolution.o_params.o_changeResolution.u_height = 1080;
    }

    libtvumedia_ctrlcmd_data_t oprog;
    {
        oprog.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        oprog.u_command_type = kLibTvuMediaCtrlCmdChangeProgramBitRate;
        oprog.o_params.o_progBitrate.u_programIndex = 11;
        oprog.o_params.o_progBitrate.u_vbitrate = 2*1024*1024;
        oprog.o_params.o_progBitrate.u_abitrate = 64000;
    }

    libtvumedia_ctrlcmd_data_t oaudio;
    {
        oinsert.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        oaudio.u_command_type = kLibTvuMediaCtrlCmdChangeAudioParams;
        oaudio.o_params.o_audio.u_audio_only = 1;
        oaudio.o_params.o_audio.u_tracks_layout = 2;
        oaudio.o_params.o_audio.u_aac_profile = 25;
    }

    libtvumedia_ctrlcmd_data_t odelay;
    {
        odelay.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        odelay.u_command_type = kLibTvuMediaCtrlCmdChangeDelayParams;
        odelay.o_params.o_delay.u_delay_ms = 2000;
    }

    libtvumedia_ctrlcmd_data_t ovolume;
    {
        ovolume.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        ovolume.u_command_type = kLibTvuMediaCtrlCmdSendAudioVolumeParams;
        ovolume.o_params.o_audioVolume.u_counts = 2;
        ovolume.o_params.o_audioVolume.u_volume[0] = 100;
        ovolume.o_params.o_audioVolume.u_volume[1] = 90;
    }

    libtvumedia_ctrlcmd_data_t o_start_live;
    {
        o_start_live.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        o_start_live.u_command_type = kLibTvuMediaCtrlCmdStartProgLiveParams;
        o_start_live.o_params.o_startLive.u_program_index = 2;
        o_start_live.o_params.o_startLive.u_video_bit_rate = 100000000;
        o_start_live.o_params.o_startLive.u_audio_bit_rate = 128000;
        o_start_live.o_params.o_startLive.u_video_codec_fourcc = 0x90754854;
        o_start_live.o_params.o_startLive.u_audio_codec_fourcc = 0x80754855;
        o_start_live.o_params.o_startLive.u_audio_tracks_layout = 0x00000022;
    }

    libtvumedia_ctrlcmd_data_t o_stop_live;
    {
        o_stop_live.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        o_stop_live.u_command_type = kLibTvuMediaCtrlCmdStopProgLiveParams;
        o_stop_live.o_params.o_stopLive.u_program_index = 2;
    }

    libtvumedia_ctrlcmd_data_t o_cameras;
    {
        o_cameras.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        o_cameras.u_command_type = kLibTvuMediaCtrlCmdCamerasParams;
        o_cameras.o_params.o_cameras.u_index = 3;
        o_cameras.o_params.o_cameras.u_video_connection = 90;
        o_cameras.o_params.o_cameras.u_audio_connection = 100;
        o_cameras.o_params.o_cameras.u_video_fmt = 0x567890;

    }

    const char json_cmd[] = "{\"ret\":\"0\"}";

    libtvumedia_ctrlcmd_data_t o_json;
    {
        o_json.u_structSize = sizeof(libtvumedia_ctrlcmd_data_t);
        o_json.u_command_type = kLibTvuMediaCtrlCmdCommonJsonParams;
        o_json.o_params.o_json.u_len = strlen(json_cmd);
        o_json.o_params.o_json.p_json = json_cmd;
    }

    libtvumedia_ctrlcmd_data_t oParam[12];
    {
        oParam[0] = oinsert;
        oParam[1] = oBitrate;
        oParam[2] = oResolution;
        oParam[3] = oprog;
        oParam[4] = oaudio;
        oParam[5] = ovolume;
        oParam[6] = odelay;
        oParam[7] = oinsert;
        oParam[8] = o_start_live;
        oParam[9] = o_stop_live;
        oParam[10] = o_cameras;
        oParam[11] = o_json;
    }

    libtvumedia_control_handle_t hctrl = LibTvuMediaControlHandleCreate();

    while (!g_exit) {
#if 1
        int ret = LibShmMediaCtrlCmdWrapHandleWrite(h, oParam, sizeof(oParam)/sizeof(oParam[0]));
        printf("write ret %d\n", ret);
#else
        const uint8_t *pout = NULL;
        int ret = LibTvuMediaControlHandleWrite(hctrl, oParam, 8, &pout);
        if (ret > 0)
        {
            int nw = LibShmMediaCtrlCmdWrapHandleWriteBinary(h, pout, ret);
            printf("write, data %d, nw %d\n", ret, nw);
        }
#endif

        Sleep(1000);
        i++;
    }

    if (h)
    {
        LibShmMediaCtrlCmdWrapHandleDestroy(h);
        h   = NULL;
    }
    return 0;
}

static int Read(const char *name)
{
    unsigned int                i       = 0;
    libshmmedia_ctrlcmd_wrap_handle_t       h       = NULL;
    const char                  *shmname= name;
    int                         ret     = -1;

    while (!g_exit) {
        const libtvumedia_ctrlcmd_data_t *pcmdlst = NULL;
        int nlist = 0;

        if (!h)
        {
            h = LibShmMediaCtrlCmdWrapHandleOpen(shmname);
        }

        if (!h)
        {
            Sleep(1000);
            printf("open handle failed, hadle name [%s], wait another 1000ms\n", shmname);
            continue;
        }


        ret = LibShmMediaCtrlCmdWrapHandleRead(h, &pcmdlst, &nlist);

        if (ret == 0)
        {
            Sleep(1);
            continue;
        }
        else if (ret < 0)
        {
            LibShmMediaCtrlCmdWrapHandleDestroy(h);
            h = NULL;
        }

        printf("get control data len: %d\n", ret);

        for (int i = 0; i < nlist; i++)
        {
            printf("get command: %u\n", pcmdlst[i].u_command_type);

            if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdInsertKF)
            {
                printf("    insertKF pindex:%hhu\n", pcmdlst[i].o_params.o_insertKF.u_program_index);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdChangeBitRate)
            {
                printf("    change bitrate to:%u\n", pcmdlst[i].o_params.o_changeBitrate.u_vbitrate);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdChangeResolution)
            {
                printf("    change resolution to:%hux%hu\n", pcmdlst[i].o_params.o_changeResolution.u_width, pcmdlst[i].o_params.o_changeResolution.u_height);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdChangeProgramBitRate)
            {
                printf("    change bitrate to:prog %hhu, bitrate %u\n", pcmdlst[i].o_params.o_progBitrate.u_programIndex, pcmdlst[i].o_params.o_progBitrate.u_vbitrate);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdChangeAudioParams)
            {
                printf("    change to:audio only %hhu, track layout 0x%lx, aac profile %hhu\n"
                       , pcmdlst[i].o_params.o_audio.u_audio_only
                       , pcmdlst[i].o_params.o_audio.u_tracks_layout
                       , pcmdlst[i].o_params.o_audio.u_aac_profile
                       );
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdChangeDelayParams)
            {
                printf("    change delay to: %u\n", pcmdlst[i].o_params.o_delay.u_delay_ms);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdSendAudioVolumeParams)
            {
                printf("    send audio volume to count %hhu\n", pcmdlst[i].o_params.o_audioVolume.u_counts);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdStartProgLiveParams)
            {
                libtvumedia_ctrlcmd_start_live_params_t *p = NULL;
                p = (libtvumedia_ctrlcmd_start_live_params_t *)&(pcmdlst[i].o_params.o_startLive);
                printf("    startLive comamnd, program index %u, bit rate %u, v codec fourcc 0x%08x, a codec fourcc 0x%08x, a track layout 0x%08lx\n"
                       , p->u_program_index, p->u_video_bit_rate, p->u_video_codec_fourcc, p->u_audio_codec_fourcc, p->u_audio_tracks_layout);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdStopProgLiveParams)
            {
                libtvumedia_ctrlcmd_stop_live_params_t *p = NULL;
                p = (libtvumedia_ctrlcmd_stop_live_params_t *)&(pcmdlst[i].o_params);
                printf("    stopLive comamnd, program index %u\n"
                       , p->u_program_index);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdCamerasParams)
            {
                libtvumedia_ctrlcmd_cameras_params_t *p = NULL;
                p = (libtvumedia_ctrlcmd_cameras_params_t *)&(pcmdlst[i].o_params);
                printf("    cameras comamnd, index %u, v connection %u, a connection %u, vidoe format 0x%x\n"
                       , p->u_index, p->u_video_connection, p->u_audio_connection, p->u_video_fmt);
            }
            else if (pcmdlst[i].u_command_type == kLibTvuMediaCtrlCmdCommonJsonParams)
            {
                libtvumedia_ctrlcmd_common_json_params_t *p = NULL;
                p = (libtvumedia_ctrlcmd_common_json_params_t *)&(pcmdlst[i].o_params);
                printf("    common json len %u, string %s\n", p->u_len, p->p_json);
            }
        }
    }

    if (h)
    {
        LibShmMediaCtrlCmdWrapHandleDestroy(h);
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

