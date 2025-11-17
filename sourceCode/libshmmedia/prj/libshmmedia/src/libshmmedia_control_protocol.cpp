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
 *    libshm_data_protocol.cpp
 *  Description:
 *    used to accomplish data protocol
 * Author:
 *    Lotus/TVU initialize on June 24th 2021
******************************************************************************/

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "libshmmedia_control_protocol.h"
#include "libshmmedia_control_protocol_internal.h"
#include "sharememory_internal.h"

static void _trans_raw_data_to_hex_string(const uint8_t *data, int len, char hx_str[], int hx_str_len)
{
    int ix = 0;

    for(int i = 0; i < len; i++)
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

    return;
}

/* if LIBSHM_MEDIA_TYPE_CONTROL_DATA structure */

CLibTvuMediaControlDataInternalContext::CLibTvuMediaControlDataInternalContext()
{
    _structSize = sizeof(CLibTvuMediaControlDataInternalContext);
    tvushm::BufferCtrlInit(&_oBuffer);
    _pParameterLst = NULL;
    _nParameterLst = 0;
}

CLibTvuMediaControlDataInternalContext::~CLibTvuMediaControlDataInternalContext()
{
    _structSize = 0;
    tvushm::BufferCtrlRelease(&_oBuffer);
}

static inline int get_length_from_buffer(tvushm::BufferController_t *p, uint32_t &len)
{
    uint8_t temp = 0;
    int offset = 0;
    len = 0;
    do
    {
        temp = tvushm::BufferCtrlR8(p);
        len += temp;
        offset++;
    }while (temp == 0xFF);

    return offset;
}

static inline int set_length_to_buffer(tvushm::BufferController_t *p, uint32_t len)
{
    uint8_t temp = 0;
    int offset = 0;
    do
    {
        bool bout = false;
        if (len >= 0xFF)
        {
            temp = 0xFF;
        }
        else
        {
            bout = true;
            temp = len & 0xFF;
        }
        FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, temp));
        offset++;
        len -= temp;

        if (bout)
        {
            break;
        }
    }while (1);

    return offset;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdInsertKeyFrame(/*IN*/const libtvumedia_ctrlcmd_insert_key_frame_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t param_len_pos = tvushm::BufferCtrlTellCurPos(p);
    int cur_pos = 0;

    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_program_index));
    offset++;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);

    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdChangeBitrate(/*IN*/const libtvumedia_ctrlcmd_change_bitrate_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t param_len_pos = tvushm::BufferCtrlTellCurPos(p);

    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_vbitrate));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_abitrate));
    offset+=4;

    param_len = offset;

    int cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdChangeResolution(/*IN*/const libtvumedia_ctrlcmd_change_resolution_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t param_len_pos = tvushm::BufferCtrlTellCurPos(p);

    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe16(p, param->u_width));
    offset += 2;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe16(p, param->u_height));
    offset += 2;

    param_len = offset;

    int cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdChangeProgramBitrate(/*IN*/const libtvumedia_ctrlcmd_change_prog_bitrate_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_programIndex));
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_vbitrate));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_abitrate));
    offset+=4;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdChangeAudioParams(/*IN*/const libtvumedia_ctrlcmd_change_audio_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_audio_only));
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_aac_profile));
    offset+=1;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe64(p, param->u_tracks_layout));
    offset+=8;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdChangeDelayParams(/*IN*/const libtvumedia_ctrlcmd_change_delay_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_delay_ms));
    offset+=4;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdSendAudioVolumes(/*IN*/const libtvumedia_ctrlcmd_send_audio_volumes_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_counts));
    offset+=1;

    for (int i = 0; i < param->u_counts; i++)
    {
        FAILED_PUSH_DATA(tvushm::BufferCtrlWBe16(p, param->u_volume[i]));
        offset+=2;
    }

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdStartLive(const libtvumedia_ctrlcmd_start_live_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_program_index));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->b_vbr));
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_video_bit_rate));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_audio_bit_rate));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_video_codec_fourcc));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_audio_codec_fourcc));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe64(p, param->u_audio_tracks_layout));
    offset+=8;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdStopLive(const libtvumedia_ctrlcmd_stop_live_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_program_index));
    offset+=4;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdGetCameras(const libtvumedia_ctrlcmd_cameras_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;
    param_len_size = set_length_to_buffer(p, param_len);

    if (param_len_size < 0)
    {
        ret = param_len_size;
        return ret;
    }

    offset += param_len_size;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_index));
    offset+=1;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_video_connection));
    offset+=1;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, param->u_audio_connection));
    offset+=1;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_video_fmt));
    offset+=4;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    ret = set_length_to_buffer(p, param_len);
    if (ret != param_len_size)
    {
        DEBUG_ERROR("unsupoort dynamical length currently\n");
        return -1;
    }
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeCmdCommonJson(const libtvumedia_ctrlcmd_common_json_params_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int32_t param_len_pos = 0;
    int cur_pos =  tvushm::BufferCtrlTellCurPos(p);

    param_len_pos = cur_pos;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, 0));
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, 0)); //reserve
    offset+=4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlPushData(p, (const uint8_t *)param->p_json, param->u_len));
    offset += param->u_len;

    param_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, param_len_pos, SEEK_SET);
    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param_len));
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    ret = offset;
    return ret;
}

int CLibTvuMediaControlDataInternalContext::_writeBody(/*IN*/const libtvumedia_ctrlcmd_data_t *param)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t len_pos = tvushm::BufferCtrlTellCurPos(p);
    uint32_t cmd_len = 0;
    int cur_pos = 0;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, 0));
    offset += 4;

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, 0)); //reserve
    offset++;

    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, param->u_command_type));
    offset += 4;

    ret = set_length_to_buffer(p, offset+1);
    if (ret != 1)
    {
        DEBUG_ERROR("unsupoort dynamical offset currently\n");
        return ret;
    }
    offset += ret;

#if 0
    /* test code for checking reading whether can support. */
    ret = set_length_to_buffer(p, offset+5);
    if (ret != 1)
    {
        DEBUG_ERROR("unsupoort dynamical offset currently\n");
        return ret;
    }
    tvushm::BufferCtrlReadSkip(p, 5-ret);
    offset += 5;
#endif

    switch (param->u_command_type)
    {
        case kLibTvuMediaCtrlCmdInsertKF:
        {
            ret = _writeCmdInsertKeyFrame(&param->o_params.o_insertKF);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeBitRate:
        {
            ret = _writeCmdChangeBitrate(&param->o_params.o_changeBitrate);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeResolution:
        {
            ret = _writeCmdChangeResolution(&param->o_params.o_changeResolution);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeProgramBitRate:
        {
            ret = _writeCmdChangeProgramBitrate(&param->o_params.o_progBitrate);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeAudioParams:
        {
            ret = _writeCmdChangeAudioParams(&param->o_params.o_audio);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeDelayParams:
        {
            ret = _writeCmdChangeDelayParams(&param->o_params.o_delay);
        }
        break;
        case kLibTvuMediaCtrlCmdSendAudioVolumeParams:
        {
            ret = _writeCmdSendAudioVolumes(&param->o_params.o_audioVolume);
        }
        break;
        case kLibTvuMediaCtrlCmdStartProgLiveParams:
        {
            ret = _writeCmdStartLive(&param->o_params.o_startLive);
        }
        break;
        case kLibTvuMediaCtrlCmdStopProgLiveParams:
        {
            ret = _writeCmdStopLive(&param->o_params.o_stopLive);
        }
        break;
        case kLibTvuMediaCtrlCmdCamerasParams:
        {
            ret = _writeCmdGetCameras(&param->o_params.o_cameras);
        }
        break;
        case kLibTvuMediaCtrlCmdCommonJsonParams:
        {
            ret = _writeCmdCommonJson(&param->o_params.o_json);
        }
        break;
        default:
        {
            ret = -1;
        }
        break;
    }

    if (ret  < 0)
    {
        return ret;
    }

    offset += ret;
    cmd_len = offset;

    cur_pos = tvushm::BufferCtrlTellCurPos(p);
    tvushm::BufferCtrlSeek(p, len_pos, SEEK_SET);
    FAILED_PUSH_DATA(tvushm::BufferCtrlWBe32(p, cmd_len));
    tvushm::BufferCtrlSeek(p, cur_pos, SEEK_SET);

    return offset;
}

int CLibTvuMediaControlDataInternalContext::write(/*IN*/const libtvumedia_ctrlcmd_data_t *param, unsigned int counts, /*OUT*/const uint8_t **ppOut)
{
    int ret = -1;

    if (!_validObject())
    {
        return -1;
    }

    tvushm::BufferController_t *p = &_oBuffer;
    uint8_t version = kLibTvuMediaControlDataProV1;
    uint32_t offset = 0;
    tvushm::BufferCtrlSeek(p, 0, SEEK_SET);

    FAILED_PUSH_DATA(tvushm::BufferCtrlW8(p, version));
    offset++;
    ret = set_length_to_buffer(p, counts);
    if (ret < 0)
    {
        return ret;
    }
    offset += ret;

    for (unsigned int i = 0; i < counts; i++)
    {
        ret = _writeBody(&param[i]);
        if (ret < 0)
        {
            return ret;
        }

        offset += ret;
//        {
//            char str[1024] = {0};
//            _trans_raw_data_to_hex_string(p->p_buff, p->i_current, str, 1024);
//            DEBUG_ERROR("index, %d, offset %u, ret %d, icurrent %d, str[%s]\n", i, offset, ret, p->i_current, str);
//        }
    }

    if (ret < 0)
    {
        return ret;
    }

    *ppOut = BufferCtrlGetOrigPtr(&_oBuffer);

    return offset;
}


int CLibTvuMediaControlDataInternalContext::_readCmdInsertKeyFrame(/*OUT*/libtvumedia_ctrlcmd_insert_key_frame_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u,<  param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_program_index = tvushm::BufferCtrlR8(p);
    offset++;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;
    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdChangeBitrate(/*OUT*/libtvumedia_ctrlcmd_change_bitrate_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u  < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_vbitrate = tvushm::BufferCtrlRBe32(p);
    offset+=4;
    param->u_abitrate = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdChangeResolution(/*OUT*/libtvumedia_ctrlcmd_change_resolution_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u < not equal param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_width = tvushm::BufferCtrlRBe16(p);
    offset+=2;
    param->u_height = tvushm::BufferCtrlRBe16(p);
    offset+=2;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdChangeProgramBitrate(/*OUT*/libtvumedia_ctrlcmd_change_prog_bitrate_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_programIndex = tvushm::BufferCtrlR8(p);
    offset+=1;
    param->u_vbitrate = tvushm::BufferCtrlRBe32(p);
    offset+=4;
    param->u_abitrate = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdChangeAudioParams(/*IN*/libtvumedia_ctrlcmd_change_audio_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_audio_only = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_aac_profile = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_tracks_layout = tvushm::BufferCtrlRBe64(p);
    offset+=8;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdChangeDelayParams(/*IN*/libtvumedia_ctrlcmd_change_delay_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_delay_ms = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdSendAudioVolumes(/*IN*/libtvumedia_ctrlcmd_send_audio_volumes_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_counts = tvushm::BufferCtrlR8(p);
    offset+=1;

    if (param->u_counts > sizeof(param->u_volume)/sizeof(param->u_volume[0]))
    {
        DEBUG_WARN("volumes counts %hhu > 32, why???\n", param->u_counts);
        param->u_counts = sizeof(param->u_volume)/sizeof(param->u_volume[0]);
    }

    for (int i = 0; i < param->u_counts; i++)
    {
        param->u_volume[i] = tvushm::BufferCtrlRBe16(p);
        offset+=2;
    }

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdStartLive(/*IN*/libtvumedia_ctrlcmd_start_live_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_program_index = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    param->b_vbr = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_video_bit_rate = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    param->u_audio_bit_rate = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    param->u_video_codec_fourcc = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    param->u_audio_codec_fourcc = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    param->u_audio_tracks_layout = tvushm::BufferCtrlRBe64(p);
    offset+=8;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdStopLive(/*IN*/libtvumedia_ctrlcmd_stop_live_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_program_index = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdGetCameras(/*IN*/libtvumedia_ctrlcmd_cameras_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    int param_len_size = 0;
    uint32_t buffer_len = left_len;

    param_len_size = get_length_from_buffer(p, param_len);
    offset += param_len_size;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("buffer left size %u, < param len %u, invalid\n", buffer_len, param_len);
        return -1;
    }

    tvushm::BufferCtrlReadSkip(p, 1); //reserve
    offset++;

    param->u_index = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_video_connection = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_audio_connection = tvushm::BufferCtrlR8(p);
    offset+=1;

    param->u_video_fmt = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readCmdCommonJson(/*IN*/libtvumedia_ctrlcmd_common_json_params_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t param_len = 0;
    uint32_t buffer_len = left_len;

    param_len = tvushm::BufferCtrlRBe32(p);
    offset+=4;

    tvushm::BufferCtrlReadSkip(p, 4); //reserve
    offset+=4;

    if (param_len > buffer_len)
    {
        DEBUG_ERROR("param buffer len %u extend buffer len %u\n", param_len, buffer_len);
        return -1;
    }

    if (offset > param_len)
    {
        DEBUG_ERROR("buffer offset %u extend param len %u\n", offset, param_len);
        return -1;
    }

    param->u_len = param_len - offset;
    param->p_json = (char *)tvushm::BufferCtrlGetCurPtr(p);

    if (param_len > offset)
    {
        tvushm::BufferCtrlReadSkip(p, param_len-offset);
    }

    ret = param_len;

    return ret;
}

int CLibTvuMediaControlDataInternalContext::_readBody(/*OUT*/libtvumedia_ctrlcmd_data_t *param, int left_len)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint32_t offset = 0;
    uint32_t cmd_len = 0;
    uint32_t cmd_type = -1;
    int cur_pos = tvushm::BufferCtrlTellCurPos(p);
    uint32_t data_offset = 0;

    cmd_len = tvushm::BufferCtrlRBe32(p);
    offset += 4;

    tvushm::BufferCtrlReadSkip(p, 1); //reserver
    offset++;

    cmd_type = tvushm::BufferCtrlRBe32(p);
    offset += 4;

    if (offset >= (uint32_t)left_len)
    {
        DEBUG_ERROR("data invalid after get command type\n");
        return -1;
    }

    ret = get_length_from_buffer(p, data_offset);

    tvushm::BufferCtrlSeek(p, cur_pos+data_offset, SEEK_SET);
    offset = data_offset;

    param->u_command_type = cmd_type;

    switch (cmd_type)
    {
        case kLibTvuMediaCtrlCmdInsertKF:
        {
            ret = _readCmdInsertKeyFrame(&param->o_params.o_insertKF, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeBitRate:
        {
            ret = _readCmdChangeBitrate(&param->o_params.o_changeBitrate, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeResolution:
        {
            ret = _readCmdChangeResolution(&param->o_params.o_changeResolution, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeProgramBitRate:
        {
            ret = _readCmdChangeProgramBitrate(&param->o_params.o_progBitrate, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeAudioParams:
        {
            ret = _readCmdChangeAudioParams(&param->o_params.o_audio, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdChangeDelayParams:
        {
            ret = _readCmdChangeDelayParams(&param->o_params.o_delay, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdSendAudioVolumeParams:
        {
            ret = _readCmdSendAudioVolumes(&param->o_params.o_audioVolume, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdStartProgLiveParams:
        {
            ret = _readCmdStartLive(&param->o_params.o_startLive, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdStopProgLiveParams:
        {
            ret = _readCmdStopLive(&param->o_params.o_stopLive, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdCamerasParams:
        {
            ret = _readCmdGetCameras(&param->o_params.o_cameras, left_len-offset);
        }
        break;
        case kLibTvuMediaCtrlCmdCommonJsonParams:
        {
            ret = _readCmdCommonJson(&param->o_params.o_json, left_len-offset);
        }
        break;
        default:
        {
            ret = -1;
        }
        break;
    }

    if (ret  < 0)
    {
        return ret;
    }

    offset += ret;

    return offset;
}

int CLibTvuMediaControlDataInternalContext::read(/*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len
         , /*OUT*/const libtvumedia_ctrlcmd_data_t **ppParams, /*OUT*/int *pCounts)
{
    int ret = -1;
    tvushm::BufferController_t *p = &_oBuffer;
    uint8_t version = 0;
    int offset = 0;
    uint32_t counts = 0;
    int nbuflen = 0;

    if (!src_buffer || !src_buffer_len)
    {
        DEBUG_ERROR("invalid data input\n");
        return -1;
    }

//    {
//        char hex[1024] = {0};
//        _trans_raw_data_to_hex_string(src_buffer, (int)src_buffer_len, hex, 1024);
//        printf("%s\n", hex);
//    }

    tvushm::BufferCtrlSeek(p, 0, SEEK_SET);
    if ((ret = tvushm::BufferCtrlPushData(p, src_buffer, src_buffer_len)) < 0)
    {
        DEBUG_ERROR("pushing data failed\n");
        return -1;
    }

    nbuflen = tvushm::BufferCtrlTellCurPos(p);

    tvushm::BufferCtrlSeek(p, 0, SEEK_SET);

    version = tvushm::BufferCtrlR8(p);
    offset++;

    switch(version)
    {
        case kLibTvuMediaControlDataProV1:
        {
            ret = get_length_from_buffer(p, counts);
            offset += ret;

            if (_nParameterLst < counts)
            {
                _pParameterLst = (libtvumedia_ctrlcmd_data_t *)realloc(_pParameterLst, counts*sizeof(libtvumedia_ctrlcmd_data_t));
                if (!_pParameterLst)
                {
                    DEBUG_ERROR("alloc paramete list failed\n");
                    return -1;
                }
                _nParameterLst = counts;
            }

            for (unsigned int i = 0; i < counts; i++)
            {
                ret = _readBody(&_pParameterLst[i], nbuflen-offset);
                if (ret < 0)
                {
                    return ret;
                }
                offset += ret;
            }
        }
        break;
        default:
        {
            ret = -1;
            return ret;
        }
        break;
    }

    *pCounts = counts;
    *ppParams = _pParameterLst;
    return offset;
}

libtvumedia_control_handle_t LibTvuMediaControlHandleCreate()
{
    CLibTvuMediaControlDataInternalContext *ph = new CLibTvuMediaControlDataInternalContext();
    return (libtvumedia_control_handle_t*)ph;
}

void LibTvuMediaControlHandleDestory(libtvumedia_control_handle_t h)
{
    CLibTvuMediaControlDataInternalContext *ph = (CLibTvuMediaControlDataInternalContext *)h;

    if (ph)
    {
        delete  ph;
    }

    return;
}

int LibTvuMediaControlHandleWrite(/*IN*/const libtvumedia_control_handle_t h, /*IN*/const libtvumedia_ctrlcmd_data_t *param, int counts, /*OUT*/const uint8_t **ppOut)
{
    CLibTvuMediaControlDataInternalContext *ph = (CLibTvuMediaControlDataInternalContext *)h;
    int ret = -1;

    if (ph)
    {
        if (param->u_structSize >= sizeof(libtvumedia_ctrlcmd_data_v1_t))
        {
            ret = ph->write(param, counts, ppOut);
        }
        else
        {
            DEBUG_WARN("param's struct size %u invalid\n", param->u_structSize);
            ret = -1;
        }
    }
    return ret;
}

int LibTvuMediaControlHandleRead(/*OUT*/libtvumedia_control_handle_t h, /*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len
                                 , /*OUT*/const libtvumedia_ctrlcmd_data_t **ppParams, /*OUT*/int *pCounts)
{
    CLibTvuMediaControlDataInternalContext *ph = (CLibTvuMediaControlDataInternalContext *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->read(src_buffer, src_buffer_len, ppParams, pCounts);
    }
    return ret;
}

/* endif LIBSHM_MEDIA_TYPE_CONTROL_DATA structure */

void CLibShmmediaCtrlCmdWrapHandle::destroy()
{
    if (hshm_)
    {
        LibViShmMediaDestroy(hshm_);
        hshm_ = NULL;
    }

    if (hctrl_)
    {
        LibTvuMediaControlHandleDestory(hctrl_);
        hctrl_ = NULL;
    }
    return;
}

int CLibShmmediaCtrlCmdWrapHandle::write(/*IN*/const libtvumedia_ctrlcmd_data_t *param, int counts)
{
    if (!hctrl_)
    {
        return -1;
    }

    const  uint8_t *pbuf = NULL;
    int ret =  LibTvuMediaControlHandleWrite(hctrl_, param, counts, &pbuf);

    if (ret < 0)
    {
        return ret;
    }

    int nbuf = ret;

    ret = -1;
    if (hshm_)
    {
        ret = LibViShmMediaPollSendable(hshm_, 1);

        if (ret <= 0)
        {
            return -1;
        }
        {
            libshm_media_head_param_t oh;
            libshm_media_item_param_t oiv;

            memset(&oh, 0, sizeof(libshm_media_head_param_t));
            memset(&oiv, 0, sizeof(libshm_media_item_param_t));

            libshm_media_item_param_v1_t *pv0 = (libshm_media_item_param_v1_t *) &oiv;
            libshm_media_item_param_v1_t &oi = *pv0;
            {
                oi.u_reservePrivate = sizeof(libshm_media_item_param_t);
            }
            oi.i_userDataType = LIBSHM_MEDIA_TYPE_CONTROL_DATA;
            oi.i_userDataLen = nbuf;
            oi.p_userData = pbuf;

//            {
//                char str[1024] = {0};
//                _trans_raw_data_to_hex_string(oi.p_userData, oi.i_userDataLen, str, 1024);
//                printf("%s\n", str);
//            }

            ret = LibViShmMediaSendData(hshm_, &oh, &oiv);
            if (ret <= 0)
            {
                return -1;
            }
        }
    }

    ret = nbuf;
    return ret;
}

int CLibShmmediaCtrlCmdWrapHandle::read(/*OUT*/const libtvumedia_ctrlcmd_data_t **ppParams, /*OUT*/int *pCounts)
{
    int ret = -1;
    if (!hshm_)
    {
        hshm_ = LibViShmMediaOpen(shmname_.c_str(), NULL, NULL);
        if (!hshm_)
        {
            DEBUG_WARN("shm[%s] re-open failed, need to wait\n", shmname_.c_str());
            return -1;
        }
    }
    libshm_media_head_param_t oh;
    libshm_media_item_param_t oiv;
    memset(&oh, 0, sizeof(libshm_media_head_param_t));
    memset(&oiv, 0, sizeof(libshm_media_item_param_t));

    libshm_media_item_param_v1_t *pv0 = (libshm_media_item_param_v1_t *) &oiv;
    libshm_media_item_param_v1_t &oi = *pv0;
    {
        oi.u_reservePrivate = sizeof(libshm_media_item_param_t);
    }

    ret = LibViShmMediaPollReadData(hshm_, &oh, &oiv, 1000);

    if (ret < 0)
    {
        DEBUG_WARN("shm[%s] reading failed, ret %d, need to re-open\n", shmname_.c_str(), ret);
        LibViShmMediaDestroy(hshm_);
        hshm_ = NULL;
        return 0;
    }
    else if (ret == 0)
    {
        return ret;
    }

    const uint8_t *src_data = NULL;
    uint32_t src_len;

    if (oi.i_userDataType != LIBSHM_MEDIA_TYPE_CONTROL_DATA)
    {
        DEBUG_ERROR("data type was not control data type, ignore\n");
        return 0;
    }

    src_data = oi.p_userData;
    src_len = oi.i_userDataLen;
    ret = LibTvuMediaControlHandleRead(hctrl_, src_data, src_len, ppParams, pCounts);

    if (ret <= 0)
    {
        return 0;
    }

    return ret;
}

int CLibShmmediaCtrlCmdWrapHandle::writeBin(/*IN*/const uint8_t *pbin, int nbin)
{
    if (!hctrl_)
    {
        return -1;
    }

    int ret = -1;

    if (hshm_)
    {
        ret = LibViShmMediaPollSendable(hshm_, 1);

        if (ret <= 0)
        {
            return -1;
        }
        {
            libshm_media_head_param_t oh;
            libshm_media_item_param_t oiv;
            memset(&oh, 0, sizeof(libshm_media_head_param_t));
            memset(&oiv, 0, sizeof(libshm_media_item_param_t));
            libshm_media_item_param_v1_t *pv0 = (libshm_media_item_param_v1_t *) &oiv;
            libshm_media_item_param_v1_t &oi = *pv0;
            {
                oi.u_reservePrivate = sizeof(libshm_media_item_param_t);
            }

            oi.i_userDataType = LIBSHM_MEDIA_TYPE_CONTROL_DATA;
            oi.i_userDataLen = nbin;
            oi.p_userData = pbin;
            ret = LibViShmMediaSendData(hshm_, &oh, &oiv);
            if (ret <= 0)
            {
                return -1;
            }
        }
    }

    if (ret > 0)
    {
        ret = nbin;
    }

    return ret;
}

int CLibShmmediaCtrlCmdWrapHandle::readBin(/*OUT*/const uint8_t **ppBin)
{
    int ret = -1;
    if (!hshm_)
    {
        hshm_ = LibViShmMediaOpen(shmname_.c_str(), NULL, NULL);
        if (!hshm_)
        {
            DEBUG_WARN("shm[%s] re-open failed, need to wait\n", shmname_.c_str());
            return -1;
        }
    }
    libshm_media_head_param_t oh;
    libshm_media_item_param_t oiv;
    memset(&oh, 0, sizeof(libshm_media_head_param_t));
    memset(&oiv, 0, sizeof(libshm_media_item_param_t));

    libshm_media_item_param_v1_t *pv0 = (libshm_media_item_param_v1_t *) &oiv;
    libshm_media_item_param_v1_t &oi = *pv0;
    {
        oi.u_reservePrivate = sizeof(libshm_media_item_param_t);
    }

    ret = LibViShmMediaPollReadData(hshm_, &oh, &oiv, 1000);

    if (ret < 0)
    {
        DEBUG_WARN("shm[%s] reading failed, ret %d, need to re-open\n", shmname_.c_str(), ret);
        LibViShmMediaDestroy(hshm_);
        hshm_ = NULL;
        return 0;
    }
    else if (ret == 0)
    {
        return ret;
    }

    if (oi.i_userDataType != LIBSHM_MEDIA_TYPE_CONTROL_DATA)
    {
        DEBUG_ERROR("data type was not control data type, ignore\n");
        return 0;
    }

    *ppBin = oi.p_userData;
    ret = oi.i_userDataLen;

    return ret;
}

int CLibShmmediaCtrlCmdWrapHandle::create(const char *pshmname, uint32_t header_len
           , uint32_t item_count
           , uint64_t total_size)
{
    libshm_media_handle_t hshm = LibViShmMediaCreate(pshmname, header_len, item_count, total_size);

    if (!hshm)
    {
        return -1;
    }

    libtvumedia_control_handle_t hctrl = LibTvuMediaControlHandleCreate();

    if (!hctrl)
    {
        LibViShmMediaDestroy(hshm);
        return -1;
    }

    hshm_ = hshm;
    hctrl_ = hctrl;
    shmname_ = pshmname;
    return 0;
}

int CLibShmmediaCtrlCmdWrapHandle::open(const char *pshmname)
{
    libshm_media_handle_t hshm = LibViShmMediaOpen(pshmname, NULL, NULL);

    if (!hshm)
    {
        return -1;
    }

    libtvumedia_control_handle_t hctrl = LibTvuMediaControlHandleCreate();

    if (!hctrl)
    {
        LibViShmMediaDestroy(hshm);
        return -1;
    }

    hshm_ = hshm;
    hctrl_ = hctrl;
    shmname_ = pshmname;
    return 0;
}

libshmmedia_ctrlcmd_wrap_handle_t LibShmMediaCtrlCmdWrapHandleCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = NULL;

    ph = new CLibShmmediaCtrlCmdWrapHandle();
    if (!ph)
    {
        return  NULL;
    }

    int ret = ph->create(pMemoryName, header_len, item_count, total_size);
    if (ret < 0)
    {
        delete  ph;
        return NULL;
    }

    return ph;
}

libshmmedia_ctrlcmd_wrap_handle_t LibShmMediaCtrlCmdWrapHandleOpen
(
    const char * pMemoryName
)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = NULL;

    ph = new CLibShmmediaCtrlCmdWrapHandle();
    if (!ph)
    {
        return  NULL;
    }

    int ret = ph->open(pMemoryName);
    if (ret < 0)
    {
        delete  ph;
        return NULL;
    }

    return ph;
}

void LibShmMediaCtrlCmdWrapHandleDestroy
(
    libshmmedia_ctrlcmd_wrap_handle_t h
)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = (CLibShmmediaCtrlCmdWrapHandle *)h;
    if (ph)
    {
        delete ph;
    }

    return;
}

int LibShmMediaCtrlCmdWrapHandleWrite(/*IN*/const libshmmedia_ctrlcmd_wrap_handle_t h, const libtvumedia_ctrlcmd_data_t *param, int counts)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = (CLibShmmediaCtrlCmdWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->write(param, counts);
    }

    return ret;
}

int LibShmMediaCtrlCmdWrapHandleRead(/*OUT*/libshmmedia_ctrlcmd_wrap_handle_t h
                                 , /*OUT*/const libtvumedia_ctrlcmd_data_t **pp_param, /*OUT*/int *pcounts)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = (CLibShmmediaCtrlCmdWrapHandle *)h;
    int ret = -1;
    if (ph)
    {
        ret = ph->read(pp_param, pcounts);
    }

    return ret;
}

int LibShmMediaCtrlCmdWrapHandleWriteBinary(/*IN*/const libshmmedia_ctrlcmd_wrap_handle_t h, const uint8_t *pbin, int nbin)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = (CLibShmmediaCtrlCmdWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->writeBin(pbin, nbin);
    }

    return ret;
}

int LibShmMediaCtrlCmdWrapHandleReadBinary(/*OUT*/libshmmedia_ctrlcmd_wrap_handle_t h
                                 , /*OUT*/const uint8_t **ppBin)
{
    CLibShmmediaCtrlCmdWrapHandle *ph = (CLibShmmediaCtrlCmdWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->readBin(ppBin);
    }

    return ret;
}
