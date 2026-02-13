/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the “License”);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an “AS IS” BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/
#ifndef LIBSHMMEDIA_CONTROL_PROTOCOL_H
#define LIBSHMMEDIA_CONTROL_PROTOCOL_H

//#include "libtvu_media_info.h"
#include <stdint.h>
#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_CONTROL_PRO_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_CONTROL_PRO_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_CONTROL_PRO_DLL_
#endif

#else
#define _LIBSHMMEDIA_CONTROL_PRO_DLL_
#endif

#ifndef __cplusplus
#include <stdbool.h>
#endif


#define _TVU_LIBSHMMEDIA_LE_FOURCCTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define _TVU_LIBSHMMEDIA_BE_FOURCCTAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

/**
 *  if LIBSHM_MEDIA_TYPE_CONTROL_DATA structure
 * the data would be stored
 * at shm's extended part, the matching type is LIBSHM_MEDIA_TYPE_CONTROL_DATA.
**/

enum eLibTvuMediaControlCommandType
{
    kLibTvuMediaCtrlCmdUnknown = 0,
    kLibTvuMediaCtrlCmdInsertKF = 1,
    kLibTvuMediaCtrlCmdChangeBitRate = 2,
    kLibTvuMediaCtrlCmdChangeResolution = 3,
    kLibTvuMediaCtrlCmdChangeProgramBitRate = 4,
    kLibTvuMediaCtrlCmdChangeAudioParams = 5,
    kLibTvuMediaCtrlCmdChangeDelayParams = 6,
    kLibTvuMediaCtrlCmdSendAudioVolumeParams = 7,
    kLibTvuMediaCtrlCmdStartProgLiveParams = 8,
    kLibTvuMediaCtrlCmdStopProgLiveParams = 9,
    kLibTvuMediaCtrlCmdCamerasParams = 10,
    kLibTvuMediaCtrlCmdChangeVideoParams = 11,

    /**************************************/
    kLibTvuMediaCtrlCmdCommonJsonParams = _TVU_LIBSHMMEDIA_LE_FOURCCTAG(0xFF, 'j', 's', 'n'),
};

#define    kLibTvuMediaCtrlCmdFourStrKeyVer                    "ver\0"
#define    kLibTvuMediaCtrlCmdFourStrKeyCmdType                "cmdt"
#define    kLibTvuMediaCtrlCmdFourStrKeyCmds                   "cmds"
#define    kLibTvuMediaCtrlCmdFourStrKeyCounts                 "cnts"
#define    kLibTvuMediaCtrlCmdFourStrKeyDelay                  "dely"
//#define    kLibTvuMediaCtrlCmdFourStrKeySourceInputType        "stpe"
#define    kLibTvuMediaCtrlCmdFourStrKeySourceInputUrl         "surl"
#define    kLibTvuMediaCtrlCmdFourStrKeyCmdParam               "cmdp"
#define    kLibTvuMediaCtrlCmdFourStrKeyCameraId               "cmri"
#define    kLibTvuMediaCtrlCmdFourStrKeyCameraStatus           "cmrs"
#define    kLibTvuMediaCtrlCmdFourStrKeyProgramId              "pgmi"
#define    kLibTvuMediaCtrlCmdFourStrKeyWidth                  "wdth"
#define    kLibTvuMediaCtrlCmdFourStrKeyHeigth                 "hgth"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoProgNetBitRate    "vpnb"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioProgNetBitRate    "apnb"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioTrackConfBitRate  "atcb"
#define    kLibTvuMediaCtrlCmdFourStrKeyTotalProgNetBitRate    "tpnb"
#define    kLibTvuMediaCtrlCmdFourStrKeyTotalNetBitRate        "tnbt"
#define    kLibTvuMediaCtrlCmdFourStrKeyTotalProgConfBitRate   "tpcb"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioOnly              "auon"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioTracksLayout      "atkl"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoCodecFourcc       "vcdf"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoCodecParam        "vcdp"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoEncBits           "vebt"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType          "veot"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoEncWorkingMode    "vewm"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioCodecFourcc       "acdf"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioCodecParam        "acdp"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioAacProfile        "aacp"
#define    kLibTvuMediaCtrlCmdFourStrKeyNetEncMode             "nemd"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioEncMode           "aemd"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoEncMode           "vemd"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoFormatId          "vfid"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoFormatFourcc      "vffc"
#define    kLibTvuMediaCtrlCmdFourStrKeyVideoConnection        "vcnt"
#define    kLibTvuMediaCtrlCmdFourStrKeyAudioConnection        "acnt"
#define    kLibTvuMediaCtrlCmdFourStrKeyRAspect16_9            "rsct"
#define    kLibTvuMediaCtrlCmdFourStrKeyROutputVideoHD         "rvhd"
#define    kLibTvuMediaCtrlCmdFourStrKeyReceiverVersion        "rver"
#define    kLibTvuMediaCtrlCmdFourStrKeyReceiverIdentity       "rid\0"
#define    kLibTvuMediaCtrlCmdFourStrKeyPlayerVersion          "pver"


enum eLibTvuMediaControlCommandVideoEncBits{
    kLibTvuMediaCtrlCmdFourStrKeyVideoEncBits_8 = 8,
    kLibTvuMediaCtrlCmdFourStrKeyVideoEncBits_10 = 10,
};

enum eLibTvuMediaControlCommandVideoEOTFType{
    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType_SDR = 0,
    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType_HLG,
    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType_HDR10,
    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType_HDR10Plus,
    kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType_DolbyVision,
};

enum eLibTvuMediaControlCommandVideoEncWorkingMode
{
  kLibTvuMediaCtrlCmdFourStrKeyVideoEncWorkingMode_UNKNOW = 0,
  kLibTvuMediaCtrlCmdFourStrKeyVideoEncWorkingMode_FAST_MOVTION,
  kLibTvuMediaCtrlCmdFourStrKeyVideoEncWorkingMode_INTERVIEW
};

/**
# common json format:
{
    kLibTvuMediaCtrlCmdFourStrKeyVer : "1.0",
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyCmds :
    [
        {
            kLibTvuMediaCtrlCmdFourStrKeyCmdType : "", # see eLibTvuMediaControlCommandType command type value
            kLibTvuMediaCtrlCmdFourStrKeyCmdParam:
            {
                // see every cmd's parameter json
            }
         }
    ]
}

# parameter json

Insert Frame params：
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
}

Change Encoding Resolution params：
{
    kLibTvuMediaCtrlCmdFourStrKeyWidth:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyHeigth:"<int>"
}

Change Network BitRate params：
{
    kLibTvuMediaCtrlCmdFourStrKeyTotalNetBitRate:"<int>"
}

Change Program Network BitRate params：
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyTotalProgNetBitRate:"<int>"
}

Change Audio Params:
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyAudioEncMode:"<int>" # 1, cbr, 2. vbr
    kLibTvuMediaCtrlCmdFourStrKeyAudioOnly: "<bool> or 0, 1"
    kLibTvuMediaCtrlCmdFourStrKeyAudioTracksLayout:"<int64>"
    kLibTvuMediaCtrlCmdFourStrKeyAudioCodecFourcc:"<int fourcc>"
    kLibTvuMediaCtrlCmdFourStrKeyAudioCodecParam:
    {
        kLibTvuMediaCtrlCmdFourStrKeyAudioTrackConfBitRate:"<int>"
        kLibTvuMediaCtrlCmdFourStrKeyAudioAacProfile: "<int>"
    }
}

Change Video Params:
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyVideoCodecFourcc:"<int fourcc>"
    kLibTvuMediaCtrlCmdFourStrKeyVideoCodecParam:
    {
        kLibTvuMediaCtrlCmdFourStrKeyVideoEncBits:"<int>" # see eLibTvuMediaControlCommandVideoEncBits
        kLibTvuMediaCtrlCmdFourStrKeyVideoEOTFType: "<int>" # see eLibTvuMediaControlCommandVideoEOTFType.
        kLibTvuMediaCtrlCmdFourStrKeyVideoEncWorkingMode: "<int>"  # see eLibTvuMediaControlCommandVideoEncWorkingMode.
    }
}

#Change Delay Params:
#{
#    kLibTvuMediaCtrlCmdFourStrKeyDelay:"<int>"
#}

Change Send Audio Volume Params:
{
    kLibTvuMediaCtrlCmdFourStrKeyCounts : <int>
    [
        <int>, <int>,...
    ]
}

Start Program Live Parameters:
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyNetEncMode:"<int>" # 1, cbr, 2. vbr
    kLibTvuMediaCtrlCmdFourStrKeyTotalProgConfBitRate:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyVideoCodecFourcc:"<int> video fourcc"
    kLibTvuMediaCtrlCmdFourStrKeyAudioCodecFourcc:"<int> video fourcc"
    kLibTvuMediaCtrlCmdFourStrKeyAudioTracksLayout:"<int64>"
    kLibTvuMediaCtrlCmdFourStrKeyDelay:"<int>"
    kLibTvuMediaCtrlCmdFourStrKeyRAspect16_9 : "<bool>, 0, 1"
    kLibTvuMediaCtrlCmdFourStrKeyROutputVideoHD : "<bool>, 0, 1"
    kLibTvuMediaCtrlCmdFourStrKeyVideoConnection: "" #see _eLibTvuMediaVideoConnection or kLibTvuMediaVideoConxxxString
    kLibTvuMediaCtrlCmdFourStrKeySourceInputUrl : "<string>" @ default null is from camera SDI source.
}

Stop Program Live Parameters:
{
    kLibTvuMediaCtrlCmdFourStrKeyProgramId:"<int>"
}

Cameras Parameter:
{
    kLibTvuMediaCtrlCmdFourStrKeyCameraId : "<int>"
    kLibTvuMediaCtrlCmdFourStrKeyCameraStatus: "<int>" # camera status 1: have camera, 0: no camera
    kLibTvuMediaCtrlCmdFourStrKeyVideoFormatId:"<int>" # see eLibTvuMediaVideoFormatId definition
    kLibTvuMediaCtrlCmdFourStrKeyVideoConnection: "" #see _eLibTvuMediaVideoConnection or kLibTvuMediaVideoConxxxString
    kLibTvuMediaCtrlCmdFourStrKeyAudioConnection: ""  #see _eLibTvuMediaAudioConnection or kLibTvuMediaAudioConxxxString
}

#SDI,Embedded,9,1920,1080,29970

**/

typedef struct SLibTvuMediaCtrlCmdInsertKFParams
{
    uint8_t  u_program_index;
}libtvumedia_ctrlcmd_insert_key_frame_params_t;

typedef struct SLibTvuMediaCtrlCmdChangeBitRateParams
{
    uint32_t  u_vbitrate;
    uint32_t  u_abitrate;

}libtvumedia_ctrlcmd_change_bitrate_params_t;

typedef struct SLibTvuMediaCtrlCmdChangeResolutionParams
{
    uint16_t  u_width;
    uint16_t  u_height;
}libtvumedia_ctrlcmd_change_resolution_params_t;

typedef struct SLibTvuMediaCtrlCmdChangeProgBitRateParams
{
    uint8_t   u_programIndex;
    uint32_t  u_vbitrate;
    uint32_t  u_abitrate;
}libtvumedia_ctrlcmd_change_prog_bitrate_params_t;

typedef struct SLibTvuMediaCtrlCmdChangeAudioParams
{
    uint8_t     u_audio_only;
    uint8_t     u_aac_profile;
    uint64_t    u_tracks_layout;
}libtvumedia_ctrlcmd_change_audio_params_t;

typedef struct SLibTvuMediaCtrlCmdChangeDelayParams
{
    uint32_t    u_delay_ms;
}libtvumedia_ctrlcmd_change_delay_params_t;

typedef struct SLibTvuMediaCtrlCmdSendAudioVolumeParams
{
    uint8_t     u_counts;
    uint16_t    u_volume[32];
}libtvumedia_ctrlcmd_send_audio_volumes_params_t;

typedef struct SLibTvuMediaCtrlCmdStartLiveParams
{
    uint32_t    u_program_index; //-1 means all programs
    uint8_t     b_vbr; // 0, cbr, 1, vbr
    uint32_t    u_video_bit_rate; // video bit rate
    uint32_t    u_audio_bit_rate; // audio bit rate
    uint32_t    u_video_codec_fourcc; // 0 means unset
    uint32_t    u_audio_codec_fourcc; // 0 means unset
    uint64_t    u_audio_tracks_layout; //0x0062, the 1st track is stereo, the 2nd track is 5.1
}libtvumedia_ctrlcmd_start_live_params_t;

typedef struct SLibTvuMediaCtrlCmdStopLiveParams
{
    uint32_t    u_program_index;//-1 means all programs
}libtvumedia_ctrlcmd_stop_live_params_t;

typedef struct SLibTvuMediaCtrlCmdCamerasParams
{
    uint8_t     u_index;// should be matching to the encoder's program id.
    uint8_t     u_video_connection; // would be defined
    uint8_t     u_audio_connection; // would be defined
    uint32_t    u_video_fmt; // -1 means no plugin in.
}libtvumedia_ctrlcmd_cameras_params_t;

typedef struct SLibTvuMediaCtrlCmdCommonJsonParams
{
    uint32_t     u_len;
    const char * p_json;
}libtvumedia_ctrlcmd_common_json_params_t;

typedef struct SLibTvuMediaControlCmdParams
{
    union {
        struct SLibTvuMediaCtrlCmdInsertKFParams        o_insertKF;
        struct SLibTvuMediaCtrlCmdChangeBitRateParams   o_changeBitrate;
        struct SLibTvuMediaCtrlCmdChangeResolutionParams o_changeResolution;
        struct SLibTvuMediaCtrlCmdChangeProgBitRateParams o_progBitrate;
        struct SLibTvuMediaCtrlCmdChangeAudioParams     o_audio;
        struct SLibTvuMediaCtrlCmdChangeDelayParams     o_delay;
        struct SLibTvuMediaCtrlCmdSendAudioVolumeParams o_audioVolume;
        struct SLibTvuMediaCtrlCmdStartLiveParams       o_startLive;
        struct SLibTvuMediaCtrlCmdStopLiveParams        o_stopLive;
        struct SLibTvuMediaCtrlCmdCamerasParams         o_cameras;
        struct SLibTvuMediaCtrlCmdCommonJsonParams      o_json;
    };
}libtvumedia_ctrlcmd_params_t;

typedef struct SLibTvuMediaCtrlCmdDataV1
{
    uint32_t    u_structSize;
    uint32_t    u_command_type; /* enum ELibTvuMediaControlCommandType */
    libtvumedia_ctrlcmd_params_t o_params;
}libtvumedia_ctrlcmd_data_v1_t;

typedef libtvumedia_ctrlcmd_data_v1_t libtvumedia_ctrlcmd_data_t;

typedef void * libtvumedia_control_handle_t;

/* endif LIBSHM_MEDIA_TYPE_CONTROL_DATA structure */


#ifdef __cplusplus
extern "C" {
#endif

/**
 *  if LIBSHM_MEDIA_TYPE_CONTROL_DATA structure
**/
/**
 *  Functionality:
 *      used to create media contorl data context.
 *  Parameter:
 *  Return:
 *      NULL, failed
 *      else, success
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
libtvumedia_control_handle_t LibTvuMediaControlHandleCreate();

/**
 *  Functionality:
 *      used to destory media contorl data context.
 *  Parameter:
 *  Return:
 *      void
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
void LibTvuMediaControlHandleDestory(libtvumedia_control_handle_t h);

/**
 *  Functionality:
 *      write @pinfo data to @dest_buffer.
 *  Parameter:
 *      @pinfo, control data ptr.
 *      @ppout,  the binary data point point.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length.
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibTvuMediaControlHandleWrite(/*IN*/const libtvumedia_control_handle_t h, /*IN*/const libtvumedia_ctrlcmd_data_t *param, /*IN*/int counts, /*OUT*/const uint8_t **ppOut);

/**
 *  Functionality:
 *      parse out control data from @src_buffer.
 *  Parameter:
 *      @src_buffer  : protocol data point
 *      @src_buffer_len : protocol data len
 *  Return:
 *      < 0 : failed
 *      ==0 : read 0 bytes, invalide
 *      > 0 : read out buffer len
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibTvuMediaControlHandleRead(/*OUT*/libtvumedia_control_handle_t h, /*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len
                                 , /*OUT*/const libtvumedia_ctrlcmd_data_t **pp_param, /*OUT*/int *pcounts);


/* endif LIBSHM_MEDIA_TYPE_CONTROL_DATA apis */

typedef void *libshmmedia_ctrlcmd_wrap_handle_t;
/**
 *  Functionality:
 *      used to create the share memory, or just open it if the share memory had existed.
 *  Parameter:
 *      @pMemoryName:
 *          share memory entry name
 *      @header_len:
 *          the share memory head size, which would store the media head data.
 *      @item_count:
 *          how many counts of share memory item counts.
 *      @total_size:
 *          total shm size.
 *  Return:
 *      NULL, open failed. Or return the share memory handle.
 */
_LIBSHMMEDIA_CONTROL_PRO_DLL_
libshmmedia_ctrlcmd_wrap_handle_t LibShmMediaCtrlCmdWrapHandleCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
);

/**
 *  Functionality:
 *      used to open the existed share memory.
 *  Parameter:
 *      @pMemoryName:
 *          share memory entry name
 *      @timeout :
 *          0   - non-block
 *          >0  - block mode
 *      @cb      :
 *          user callback function
 *      @opaq    :
 *          user self data
 *  Return:
 *      NULL, open failed. Or return the share memory handle.
 */
_LIBSHMMEDIA_CONTROL_PRO_DLL_
libshmmedia_ctrlcmd_wrap_handle_t LibShmMediaCtrlCmdWrapHandleOpen
(
    const char * pMemoryName
);


/**
 *  Functionality:
 *      used to destory media contorl data context.
 *  Parameter:
 *      h: handle
 *  Return:
 *      void
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
void LibShmMediaCtrlCmdWrapHandleDestroy
(
    libshmmedia_ctrlcmd_wrap_handle_t h
);

/**
 *  Functionality:
 *      write @pcmd data to viriable shm.
 *  Parameter:
 *      @h , handle
 *      @pcmd, control command point.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length, equal control data length.
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibShmMediaCtrlCmdWrapHandleWrite(/*IN*/const libshmmedia_ctrlcmd_wrap_handle_t h, const libtvumedia_ctrlcmd_data_t *pcmd, int counts);

/**
 *  Functionality:
 *      parse out control data from @shm's user data.
 *  Parameter:
 *      @h , handle
 *      @ppCmds  : parameter point point
 *      @pcounts : counts point
 *  Return:
 *      < 0 : failed
 *      ==0 : not ready
 *      > 0 : read out buffer len.
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibShmMediaCtrlCmdWrapHandleRead(/*OUT*/libshmmedia_ctrlcmd_wrap_handle_t h
                                 , /*OUT*/const libtvumedia_ctrlcmd_data_t **ppCmds, /*OUT*/int *pcounts);


/**
 *  Functionality:
 *      used to write control data binary to shm..
 *  Parameter:
 *      @h , handle
 *      @pbin, binary point.
 *      @nbin, binary legth
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : how many bytes to be written to shm, should be equal to @nbin.
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibShmMediaCtrlCmdWrapHandleWriteBinary(/*IN*/const libshmmedia_ctrlcmd_wrap_handle_t h, const uint8_t *pbin, int nbin);

/**
 *  Functionality:
 *      used to read out control data binary from shm.
 *  Parameter:
 *      @h , handle
 *      @ppBin  : parameter point point
 *  Return:
 *      < 0 : failed, need to reopen handle.
 *      ==0 : not ready
 *      > 0 : binary length of *@ppBin.
**/
_LIBSHMMEDIA_CONTROL_PRO_DLL_
int LibShmMediaCtrlCmdWrapHandleReadBinary(/*OUT*/libshmmedia_ctrlcmd_wrap_handle_t h
                                 , /*OUT*/const uint8_t **ppBin);
#ifdef __cplusplus
}
#endif


#endif // LIBSHMMEDIA_CONTROL_PROTOCOL_H
