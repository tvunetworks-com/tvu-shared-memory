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
/******************************************************************************
 *  File:
 *    libshm_data_protocol.h
 *  Description:
 *    used to declare data protocol apis here
 * Author:
 *    Lotus/TVU initialize on June 24th 2021
******************************************************************************/

#ifndef LIBSHM_DATA_PROTOCOL_INTERNAL_H
#define LIBSHM_DATA_PROTOCOL_INTERNAL_H

#include "libshm_media_variable_item.h"
#include "libshmmedia_control_protocol.h"
#include "buffer_ctrl.h"
#include <stdint.h>
#include <memory>
#include <string>

enum ELibTvuMediaControlDataProtocol
{
    kLibTvuMediaControlDataProV1 = 1,
};

#pragma pack(push, 1)

/* if LIBSHM_MEDIA_TYPE_CONTROL_DATA protocol */
/**
control data：
{
      ver:8bit,
      count:n bits(0xFF 0x01),
      [
         {
            Length:32 bits
            reserver:8bit
            command_type:32 bit
            param_offset:n bits(0xFF 0x01)
            ...#extended for future
            {
                params:n bit
            }
         }

         ...
      ]

}

Command Type
{
Insert KeyFrame = 1
Change BitRate = 2
Change Resolution = 3
Change Program BitRate = 4
Change Audio Parameter = 5
change Delay Parameters = 6
Send Audio Volumes = 7
Start Live = 8
Stop Live = 9
Camera Status = 10
}

Insert KeyFrame params：
{
    Length:n bits(0xFF 0x01)
    reserve:8bit
    programIndex:8bit
}

Change Encoding Resolution params：
{
    Length:n bits(0xFF 0x01)
    reserve:8 bit
    width:16 bit  //BE
    height:16 bit //BE
}

Change BitRate params：
{
    Length:n bits(0xFF 0x01)
    reserve:8 bit
    video_bitrate:32 bit //BE
    audio_bitrate:32bit //BE
}

Change Program BitRate params：
{
    Length:n bits(0xFF 0x01)
    reserve:8 bit
    programIndex:8bit
    video_bitrate:32 bit //BE
    audio_bitrate:32bit //BE
}

Change Audio Params:
{
    Length:n bits(0xFF 0x01)
    reserve : 8 bit
    audio only : 8 bit
    #channel number : 8 bit
    tracks_info_layout: 64 bit //BE 0x000000012
    aac profile : 8bit
}

Change Delay Params:
{
    Length : n bits(0xFF 0x01)
    reserve : 8 bit
    delay : 32 bit // BE, ms
}

Change Send Audio Volume Params:
{
    Length : n bits(0xFF 0x01)
    reserve : 8 bit
    count : 8 bit
    [
        v0(16bit), v1, ...
    ]
}

Start Program Live Parameters:
{
    Length : n bits(0xFF 0x01)
    reserve : 8 bit
    program_id:32bit // BE
    vbr:8bit // 0, cbr, 1 vbr
    video_bitrate:32bit //BE
    audio_bitrate:32bit // BE
    video_codec_fourcc:32bit //BE
    audio_codec_fourcc:32bit //BE
    audio_tracks_layout:64bit //BE
}

Stop Program Live Parameters:
{
    Length : n bits(0xFF 0x01)
    reserve : 8 bit
    program_id:32bit // BE
}

<CMD docver="1.0"><TYPE>6</TYPE><TIME>1652933655678</TIME><PEERID>4842174963571995439</PEERID>
<CONTENT>SDI,Embedded,9,1920,1080,29970 </CONTENT>
</CMD>

static VideoCfg g_v_cfg[] = {
    {0, bmdModeNTSC, "NTSC (720X486 29.97 FPS) 486-->480"}, // nMode, nValid, nDeckMode
    {1, bmdModeNTSC2398, "NTSC 23.98 (720X486 23.976 FPS)"}, // not used.
    {2, bmdModePAL, "PAL (720X576 25 FPS)"},
    {3, bmdModeHD1080p2398, "HD 1080p 23.98 (1920X1080 23.976 FPS)"},
    {4, bmdModeHD1080p24, "HD 1080p 24 (1920X1080 24 FPS)"},
    {5, bmdModeHD1080p25, "HD 1080p 25 (1920X1080 25 FPS)"},
    {6, bmdModeHD1080p2997, "HD 1080p 29.97 (1920X1080 29.97 FPS)"},
    {7, bmdModeHD1080p30, "HD 1080p 30 (1920X1080 30 FPS)"},
    {8, bmdModeHD1080i50, "HD 1080i 50 (1920X1080 25 FPS)"},
    {9, bmdModeHD1080i5994, "HD 1080i 59.94 (1920X1080 29.97 FPS)"},
    {10, bmdModeHD1080i6000, "HD 1080i 60 (1920X1080 30 FPS)"},
    {11, bmdModeHD720p50, "HD 720p 50 (1280X720 50 FPS)"},
    {12, bmdModeHD720p5994, "HD 720p 59.94 (1280X720 59.9401 FPS)"},
    {13, bmdModeHD720p60, "HD 720p 60 (1280X720 60 FPS)"},
    {14, bmdModeHD1080p50, "1080p50 (1920X1080 50 FPS)"},
    {15, bmdModeHD1080p5994, "1080p59.94 (1920X1080 59.9401 FPS)"},
    {16, bmdMode4K2160p25, "2160p25 (3840X2160 25 FPS)"},
    {17, bmdMode4K2160p2997, "2160p29.97 (3840X2160 29.97 FPS)"},
    {18, bmdMode2kDCI24, "1080p24 DCI 2K (2048X1080 24 FPS)"},
    {19, bmdMode2kDCI25, "1080p25 DCI 2K (2048X1080 25 FPS)"},
    {20, bmdMode4K2160p2398, "2160p23.98 (3840X2160 23.976 FPS)"},
    {21, bmdMode4K2160p24, "2160p24 (3840X2160 24 FPS)"},
    {22, bmdModeHD1080p6000, "1080p60 (1920X1080 60 FPS)"},
    {23, bmdMode2kDCI2398, "1080p23.98 DCI 2K (2048X1080 23.976 FPS)"},
    {24, bmdMode4K2160p30, "2160p30 (3840X2160 30 FPS)"},
    {25, bmdMode4kDCI2398, "2160p23.98 DCI 4K (4096X2160 23.976 FPS)"},
    {26, bmdMode4kDCI24, "2160p24 DCI 4K (4096X2160 24 FPS)"},
    {27, bmdMode4kDCI25, "2160p25 DCI 4K (4096X2160 25 FPS)"},
    {28, bmdMode4K2160p5994, "2160p5994 (3840X2160 5994 FPS)" },
    {29, bmdMode4K2160p60, "2160p60 (3840X2160 60 FPS)" },
    {30, bmdMode4K2160p50, "2160p50 (3840X2160 50 FPS)"}
};

enum _BMDVideoConnection {
    bmdVideoConnectionSDI                                        = 1 << 0,
    bmdVideoConnectionHDMI                                       = 1 << 1,
    bmdVideoConnectionOpticalSDI                                 = 1 << 2,
    bmdVideoConnectionComponent                                  = 1 << 3,
    bmdVideoConnectionComposite                                  = 1 << 4,
    bmdVideoConnectionSVideo                                     = 1 << 5
};
enum _BMDAudioConnection {
    bmdAudioConnectionEmbedded                                   = 1 << 0,
    bmdAudioConnectionAESEBU                                     = 1 << 1,
    bmdAudioConnectionAnalog                                     = 1 << 2,
    bmdAudioConnectionAnalogXLR                                  = 1 << 3,
    bmdAudioConnectionAnalogRCA                                  = 1 << 4,
    bmdAudioConnectionMicrophone                                 = 1 << 5,
    bmdAudioConnectionHeadphones                                 = 1 << 6
};

enum VideoConnection
{
    kSDI, "SDI"
    kHDMI,, "HDMI"
    kOpticalSDI, "OpticalSDI"
    kComponent, "Component"
    kComposite, "Composite"
    kSVideo, "SVideo"
};

enum AudioConnection
{
    kEmbedded, "Embedded"
    kAESEBU,, "AESEBU"
    kAnalog, "Analog"
    kAnalogXLR, "AnalogXLR"
    kAnalogRCA, "AnalogRCA"
    kMicrophone, "Microphone"
    kHeadphones, "Headphones"
};

Cameras Parameter:
{
    Length : n bits(0xFF 0x01)
    reserve : 8 bit
    index : 8 bit
    video_format:32bit  //BE
    video_connection: 8bit //video connection, it can be sdi or hdmi
    audio_connection:8bit  //audio connection, it can be embedded or aesebu or analag
}

#SDI,Embedded,9,1920,1080,29970


Common Json Parameter:
{
    Length:32bit
    reserve:32bit
    json string:nbit
}

#n bits(0xFF 0x01): the length principle 0xFF+0x01

**/
typedef struct SLibTvuMediaControlDataInternalProtocolCommon
{
    uint8_t u_version;
}libtvumedia_control_data_internal_pro_common_t;

typedef struct SLibTvuMediaControlDataInternalNode
{
    //uint8_t     u_len[0];
    uint8_t     u_reserver;
    //uint8_t     u_cmd_type[0];
    //uint8_t     u_param_offset[0];
    //uint8_t     u_params[0];
    //uint8_t        p_data[0];
}libtvumedia_control_data_internal_node_t;

typedef struct SLibTvuMediaControlInsertKFParamsInternalProtocol
{
    //uint8_t  u_len[0];
    uint8_t  u_reserver;
    uint8_t  u_program_index;
}libtvumedia_control_insert_key_frame_params_internal_pro_t;

typedef struct SLibTvuMediaControlDataInternalProtocol
{
    libtvumedia_control_data_internal_pro_common_t u_common;
    //uint8_t     u_counts[0];
    //libtvumedia_control_data_internal_node_t o_nodes[0];
}libtvumedia_control_data_internal_pro_t;

/* endif LIBSHM_MEDIA_TYPE_CONTROL_DATA protocol */

#pragma pack(pop)

class CLibTvuMediaControlDataInternalContext
{
public:
    CLibTvuMediaControlDataInternalContext();
    ~CLibTvuMediaControlDataInternalContext();
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
    int write(/*IN*/const libtvumedia_ctrlcmd_data_t *param, unsigned int counts, /*OUT*/const uint8_t **ppOut);
    int read(/*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len
             , /*OUT*/const libtvumedia_ctrlcmd_data_t **ppParams, /*OUT*/int *pCounts);
private:
    bool _validObject()
    {
        return _structSize == sizeof(CLibTvuMediaControlDataInternalContext);
    }
    int _writeBody(/*IN*/const libtvumedia_ctrlcmd_data_t *param);
    int _writeCmdInsertKeyFrame(/*IN*/const libtvumedia_ctrlcmd_insert_key_frame_params_t *param);
    int _writeCmdChangeBitrate(/*IN*/const libtvumedia_ctrlcmd_change_bitrate_params_t *param);
    int _writeCmdChangeResolution(/*IN*/const libtvumedia_ctrlcmd_change_resolution_params_t *param);
    int _writeCmdChangeProgramBitrate(/*IN*/const libtvumedia_ctrlcmd_change_prog_bitrate_params_t *param);
    int _writeCmdChangeAudioParams(/*IN*/const libtvumedia_ctrlcmd_change_audio_params_t *param);
    int _writeCmdChangeDelayParams(/*IN*/const libtvumedia_ctrlcmd_change_delay_params_t *param);
    int _writeCmdSendAudioVolumes(/*IN*/const libtvumedia_ctrlcmd_send_audio_volumes_params_t *param);
    int _writeCmdStartLive(const libtvumedia_ctrlcmd_start_live_params_t *param);
    int _writeCmdStopLive(const libtvumedia_ctrlcmd_stop_live_params_t *param);
    int _writeCmdGetCameras(const libtvumedia_ctrlcmd_cameras_params_t *param);
    int _writeCmdCommonJson(const libtvumedia_ctrlcmd_common_json_params_t *param);


    int _readBody(/*OUT*/libtvumedia_ctrlcmd_data_t *param, int left_len);
    int _readCmdInsertKeyFrame(/*OUT*/libtvumedia_ctrlcmd_insert_key_frame_params_t *param, int left_len);
    int _readCmdChangeBitrate(/*OUT*/ libtvumedia_ctrlcmd_change_bitrate_params_t *param, int left_len);
    int _readCmdChangeResolution(/*OUT*/libtvumedia_ctrlcmd_change_resolution_params_t *param, int left_len);
    int _readCmdChangeProgramBitrate(/*OUT*/libtvumedia_ctrlcmd_change_prog_bitrate_params_t *param, int left_len);
    int _readCmdChangeAudioParams(/*IN*/libtvumedia_ctrlcmd_change_audio_params_t *param, int left_len);
    int _readCmdChangeDelayParams(/*IN*/libtvumedia_ctrlcmd_change_delay_params_t *param, int left_len);
    int _readCmdSendAudioVolumes(/*IN*/libtvumedia_ctrlcmd_send_audio_volumes_params_t *param, int left_len);
    int _readCmdStartLive(/*IN*/libtvumedia_ctrlcmd_start_live_params_t *param, int left_len);
    int _readCmdStopLive(/*IN*/libtvumedia_ctrlcmd_stop_live_params_t *param, int left_len);
    int _readCmdGetCameras(/*IN*/libtvumedia_ctrlcmd_cameras_params_t *param, int left_len);
    int _readCmdCommonJson(/*IN*/libtvumedia_ctrlcmd_common_json_params_t *param, int left_len);
private:
    unsigned int    _structSize;
    tvushm::BufferController_t     _oBuffer;
    libtvumedia_ctrlcmd_data_t *_pParameterLst;
    uint32_t                    _nParameterLst;
};

class CLibShmmediaCtrlCmdWrapHandle
{
public:
    CLibShmmediaCtrlCmdWrapHandle()
    {
        hshm_ = NULL;
        hctrl_ = NULL;
    }

    virtual ~CLibShmmediaCtrlCmdWrapHandle()
    {
        destroy();
    }
    int create(const char *pshmname, uint32_t header_len
               , uint32_t item_count
               , uint64_t total_size);
    int open(const char *pshmname);
    int write(/*IN*/const libtvumedia_ctrlcmd_data_t *param, int counts);
    int read(/*OUT*/const libtvumedia_ctrlcmd_data_t **ppParams, /*OUT*/int *pCounts);
    int writeBin(/*IN*/const uint8_t *pbin, int nbin);
    int readBin(/*OUT*/const uint8_t **ppBin);
    void destroy();
public:
    libshm_media_handle_t hshm_;
    libtvumedia_control_handle_t hctrl_;
    std::string shmname_;
};

#endif // LIBSHM_DATA_PROTOCOL_INTERNAL_H
