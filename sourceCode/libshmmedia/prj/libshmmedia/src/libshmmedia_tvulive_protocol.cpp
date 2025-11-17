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
/*************************************************************************************
 * Description:
 *      used to accomplish libshmmedia_tvulive data protocol
 * Author:
 *      Lotus/TVU
 * History:
 *      Lotus Initialized it on April 25th 2022.
*************************************************************************************/

#include "libshmmedia_tvulive_protocol_internal.h"
#include "libshm_util_common_internal.h"
#include "libshm_util_endian.h"
#include "sharememory_internal.h"
#include <string.h>

#if defined (TVU_LINUX)
#include <arpa/inet.h>
#endif

#include <errno.h>

#define LIBSHMMEDIA_TVULIVE_DATA_PRO_CURRENT_VER      kLibshmMediaTvuliveDataProV2

static uint32_t LibTvuMediaTvuliveDataGetBufferSizeWithProVer(uint32_t data_len, enum ELibShmMediaTvuliveDataProtocol ver)
{
    uint32_t ret = 0;
    switch (ver){
        case kLibshmMediaTvuliveDataProV1:
        {
            ret = sizeof(libshmmedia_tvulive_data_internal_pro_v1_t) + data_len;
        }
        break;
        case kLibshmMediaTvuliveDataProV2:
        {
            ret = sizeof(libshmmedia_tvulive_data_internal_pro_v2_t) + data_len;
        }
        break;
        default:
        {

        }
        break;
    }

    return ret;
}

uint32_t LibTvuMediaTvulivePreferBufferSize(uint32_t data_len)
{
    return LibTvuMediaTvuliveDataGetBufferSizeWithProVer(data_len, LIBSHMMEDIA_TVULIVE_DATA_PRO_CURRENT_VER);
}

uint32_t LibTvuMediaTvulivePreferHeadSize()
{
    return LibTvuMediaTvulivePreferBufferSize(0);
}

static int _write_tvulive_v20(const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer)
{
    int offset = 0;
    unsigned int struct_size = sizeof(libshmmedia_tvulive_data_internal_pro_v20_t);
    libshmmedia_tvulive_data_internal_pro_v20_t *pTvulivedataProV2 = (libshmmedia_tvulive_data_internal_pro_v20_t *)buffer;

    pTvulivedataProV2->u_common.u_version = kLibshmMediaTvuliveDataProV2;
    offset += sizeof(libshmmedia_tvulivedata_pro_common_t);

    pTvulivedataProV2->u_type = pinfo->o_info.i_type & 0xFF;
    offset += sizeof(pTvulivedataProV2->u_type);

    pTvulivedataProV2->u_stream_index = htons(pinfo->o_info.u_stream_index);
    offset += sizeof(pTvulivedataProV2->u_stream_index);

    pTvulivedataProV2->u_program_index = htons(pinfo->o_info.u_program_index);
    offset += sizeof(pTvulivedataProV2->u_program_index);

    pTvulivedataProV2->u_frame_index = htons(pinfo->o_info.u_frame_index);
    offset += sizeof(pTvulivedataProV2->u_frame_index);

    memset(&pTvulivedataProV2->u_reserv_2, 0, sizeof(pTvulivedataProV2->u_reserv_2));
    offset += sizeof(pTvulivedataProV2->u_reserv_2);

    pTvulivedataProV2->i_data_len = htonl(pinfo->i_data);
    offset += sizeof(pTvulivedataProV2->i_data_len);

    pTvulivedataProV2->u_data_offset = htonl(struct_size);
    offset += sizeof(pTvulivedataProV2->u_data_offset);

    if ((unsigned int)offset != struct_size)
    {
        return -1;
    }

    if (pinfo->p_data)
    {
        TVUUTIL_SAFE_MEMCPY(buffer + offset,pinfo->p_data,pinfo->i_data);
        offset += pinfo->i_data;
    }
    return offset;
}

static int _write_tvulive_v21(const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer)
{
    int offset = 0;

    unsigned int struct_size = sizeof(libshmmedia_tvulive_data_internal_pro_v21_t);
    unsigned int data_offset = struct_size;
    libshmmedia_tvulive_data_internal_pro_v21_t *pV21 = (libshmmedia_tvulive_data_internal_pro_v21_t *)buffer;
    libshmmedia_tvulive_data_internal_pro_v20_t *pV20 = &pV21->v20_;

    {
        /* write V2.0 */
        libtvumedia_tvulive_data_t oinfo;
        oinfo = *pinfo;
        oinfo.p_data = 0;
        oinfo.i_data = 0;
        offset = _write_tvulive_v20(&oinfo, buffer);
        if (offset < 0)
        {
            return offset;
        }
    }


    pV20->i_data_len = htonl(pinfo->i_data);
    pV20->u_data_offset = htonl(data_offset);

    {
        pV21->u_reserve = 0;
        pV21->u_head_struct_size_ = htonl(struct_size);
        pV21->u_frame_timestamp_ms_ = tvushm::_tvuutil_hton64(pinfo->o_info.u_frame_timestamp_ms);
    }

    offset = data_offset;

    if (pinfo->p_data)
    {
        TVUUTIL_SAFE_MEMCPY(buffer + data_offset,pinfo->p_data,pinfo->i_data);
        offset += pinfo->i_data;
    }

    return offset;
}

static int _write_tvulive_v22(const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer)
{
    int offset = 0;

    unsigned int struct_size = sizeof(libshmmedia_tvulive_data_internal_pro_v22_t);
    unsigned int data_offset = struct_size;
    libshmmedia_tvulive_data_internal_pro_v22_t *pV22 = (libshmmedia_tvulive_data_internal_pro_v22_t *)buffer;
    libshmmedia_tvulive_data_internal_pro_v20_t *pV20 = &pV22->v20_;

    {
        /* write V2.0 */
        libtvumedia_tvulive_data_t oinfo;
        oinfo = *pinfo;
        oinfo.p_data = 0;
        oinfo.i_data = 0;
        offset = _write_tvulive_v20(&oinfo, buffer);
        if (offset < 0)
        {
            return offset;
        }
    }


    pV20->i_data_len = htonl(pinfo->i_data);
    pV20->u_data_offset = htonl(data_offset);

    {
        pV22->u_reserve = 0;
        pV22->u_head_struct_size_ = htonl(struct_size);
        pV22->u_frame_timestamp_ms_ = tvushm::_tvuutil_hton64(pinfo->o_info.u_frame_timestamp_ms);
        pV22->u_type_v2_ = htonl(pinfo->o_info.i_type);
    }

    offset = data_offset;

    if (pinfo->p_data)
    {
        TVUUTIL_SAFE_MEMCPY(buffer + data_offset,pinfo->p_data,pinfo->i_data);
        offset += pinfo->i_data;
    }

    return offset;
}

static int _write_tvulive_v23(const libtvumedia_tvulive_data_v2_t *pinfo, /*OUT*/uint8_t *buffer)
{
    int offset = 0;

    unsigned int struct_size = sizeof(libshmmedia_tvulive_data_internal_pro_v23_t);
    unsigned int data_offset = struct_size;
    libshmmedia_tvulive_data_internal_pro_v23_t *pV2 = (libshmmedia_tvulive_data_internal_pro_v23_t *)buffer;
    libshmmedia_tvulive_data_internal_pro_v20_t *pV20 = &pV2->v20_;

    {
        /* write V2.0 */
        libtvumedia_tvulive_data_t oinfo;
        oinfo = *pinfo;
        oinfo.p_data = 0;
        oinfo.i_data = 0;
        offset = _write_tvulive_v20(&oinfo, buffer);
        if (offset < 0)
        {
            return offset;
        }
    }


    pV20->i_data_len = htonl(pinfo->i_data);
    pV20->u_data_offset = htonl(data_offset);

    {
        pV2->u_reserve = 0;
        pV2->u_head_struct_size_ = htonl(struct_size);
        pV2->u_frame_timestamp_ms_ = tvushm::_tvuutil_hton64(pinfo->o_info.u_frame_timestamp_ms);
        pV2->u_type_v2_ = htonl(pinfo->o_info.i_type);
    }

    {
        // V2.3
        pV2->u_gop_poc = htonl(pinfo->o_info.ext.u_gop_poc);
    }

    offset = data_offset;

    if (pinfo->p_data)
    {
        TVUUTIL_SAFE_MEMCPY(buffer + data_offset,pinfo->p_data,pinfo->i_data);
        offset += pinfo->i_data;
    }

    return offset;
}

static int _read_tvulive_data_v20(libtvumedia_tvulive_data_t *pinfo, const uint8_t *buffer, uint32_t buffer_len)
{
    unsigned int offset = 0;
    int ret = -1;

    const libshmmedia_tvulive_data_internal_pro_v20_t* tvulivedataProV2 = (const libshmmedia_tvulive_data_internal_pro_v20_t*) buffer;
    offset += sizeof(libshmmedia_tvulivedata_internal_pro_common_t);

    pinfo->o_info.i_type = tvulivedataProV2->u_type;
    offset += sizeof(tvulivedataProV2->u_type); //i_type

    pinfo->o_info.u_stream_index = ntohs(tvulivedataProV2->u_stream_index);
    offset += sizeof(tvulivedataProV2->u_stream_index);

    pinfo->o_info.u_program_index = ntohs(tvulivedataProV2->u_program_index);
    offset += sizeof(tvulivedataProV2->u_program_index);

    pinfo->o_info.u_frame_index = ntohs(tvulivedataProV2->u_frame_index);
    offset += sizeof(tvulivedataProV2->u_frame_index);

    offset += sizeof(tvulivedataProV2->u_reserv_2);

    uint32_t data_len =
    pinfo->i_data = ntohl(tvulivedataProV2->i_data_len);
    offset += sizeof(tvulivedataProV2->i_data_len); //i_data_len;


    uint32_t data_offset =  ntohl(tvulivedataProV2->u_data_offset);
    offset += sizeof(tvulivedataProV2->u_data_offset); //i_data_len;

    if (data_offset < offset)
    {
        return ret;
    }

    if (data_offset + data_len > buffer_len)
    {
        return ret;
    }

    pinfo->p_data = buffer + data_offset;
    ret = data_offset+data_len;
    return ret;
}

static int _read_tvulive_data_v21(libtvumedia_tvulive_data_t *pinfo, const uint8_t *buffer, uint32_t buffer_len)
{
    int ret = _read_tvulive_data_v20(pinfo, buffer, buffer_len);

    if (ret < 0)
        return ret;

    const libshmmedia_tvulive_data_internal_pro_v21_t* pV21 = (const libshmmedia_tvulive_data_internal_pro_v21_t*) buffer;

    pinfo->o_info.u_frame_timestamp_ms = tvushm::_tvuutil_ntoh64(pV21->u_frame_timestamp_ms_);

    return ret;
}

static int _read_tvulive_data_v22(libtvumedia_tvulive_data_t *pinfo, const uint8_t *buffer, uint32_t buffer_len)
{
    int ret = _read_tvulive_data_v20(pinfo, buffer, buffer_len);

    if (ret < 0)
        return ret;

    const libshmmedia_tvulive_data_internal_pro_v22_t* pV22 = (const libshmmedia_tvulive_data_internal_pro_v22_t*) buffer;

    pinfo->o_info.u_frame_timestamp_ms = tvushm::_tvuutil_ntoh64(pV22->u_frame_timestamp_ms_);
    pinfo->o_info.i_type = ntohl(pV22->u_type_v2_);

    return ret;
}

static int _read_tvulive_data_v23(libtvumedia_tvulive_data_t *pinfo, const uint8_t *buffer, uint32_t buffer_len)
{
    int ret = _read_tvulive_data_v20(pinfo, buffer, buffer_len);

    if (ret < 0)
        return ret;

    const libshmmedia_tvulive_data_internal_pro_v23_t* pV2 = (const libshmmedia_tvulive_data_internal_pro_v23_t*) buffer;

    pinfo->o_info.u_frame_timestamp_ms = tvushm::_tvuutil_ntoh64(pV2->u_frame_timestamp_ms_);
    pinfo->o_info.i_type = ntohl(pV2->u_type_v2_);
    pinfo->o_info.ext.u_gop_poc = ntohl(pV2->u_gop_poc);

    return ret;
}

static int _read_tvulive_data_v23_with_struct_size(libtvumedia_tvulive_data_t *pinfo, const uint8_t *buffer, uint32_t buffer_len)
{
    if (pinfo->u_struct_size>=sizeof(libtvumedia_tvulive_data_v2_t))
    {
        return _read_tvulive_data_v23(pinfo, buffer, buffer_len);
    }
    else if (pinfo->u_struct_size>=sizeof(libtvumedia_tvulive_data_v1_t))
    {
        return _read_tvulive_data_v22(pinfo, buffer, buffer_len);
    }

    return _read_tvulive_data_v20(pinfo, buffer, buffer_len);
}

static int tvulive_write_buffer_v2(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size)
{
    int offset = 0;

    if (!buffer || !buffer_size)
    {
        return -1;
    }

    if (!pinfo)
    {
        return -EINVAL;
    }

    if((buffer_size < sizeof (libshmmedia_tvulive_data_internal_pro_v2_t) + pinfo->i_data)
            || pinfo->i_data < 0)
    {
        return -1; //buffer size is not enough.
    }

    if (sizeof(libshmmedia_tvulive_data_internal_pro_v2_t) == sizeof(libshmmedia_tvulive_data_internal_pro_v21_t))
    {
        offset = _write_tvulive_v21(pinfo, buffer);
    }
    else if (sizeof(libshmmedia_tvulive_data_internal_pro_v2_t) == sizeof(libshmmedia_tvulive_data_internal_pro_v22_t))
    {
        offset = _write_tvulive_v22(pinfo, buffer);
    }
    else if (sizeof(libshmmedia_tvulive_data_internal_pro_v2_t) == sizeof(libshmmedia_tvulive_data_internal_pro_v23_t))
    {
        size_t inputSize = pinfo->u_struct_size;
        if (!inputSize)
        {
            DEBUG_ERROR("made sure that the input struct size to be set.\n");
            return  -EINVAL;
        }
        if (inputSize >= sizeof(libtvumedia_tvulive_data_v2_t))
            offset = _write_tvulive_v23(pinfo, buffer);
    }

    return offset;
}

static int tvulive_write_data_v1(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size)
{
    int offset = 0;

    if (!buffer || !buffer_size)
    {
        return -1;
    }

    if((buffer_size < sizeof (libshmmedia_tvulive_data_internal_pro_v1_t) + pinfo->i_data)
            || pinfo->i_data < 0)
    {
        return -1; //buffer size is not enough.
    }

    libshmmedia_tvulive_data_internal_pro_v1_t *pTvulivedataProV1 = (libshmmedia_tvulive_data_internal_pro_v1_t *)buffer;

    pTvulivedataProV1->u_common.u_version = kLibshmMediaTvuliveDataProV1;
    offset += sizeof(libshmmedia_tvulivedata_pro_common_t);

    pTvulivedataProV1->i_type = pinfo->o_info.i_type & 0xFF;
    offset += sizeof(pTvulivedataProV1->i_type);

    pTvulivedataProV1->u_stream_index = pinfo->o_info.u_stream_index & 0xFF;
    offset += sizeof(pTvulivedataProV1->u_stream_index);

    pTvulivedataProV1->u_reserve1 = 0;
    offset += sizeof(pTvulivedataProV1->u_reserve1);

    pTvulivedataProV1->u_frame_index = pinfo->o_info.u_frame_index & 0xFFFF;
    offset += sizeof(pTvulivedataProV1->u_frame_index);

    pTvulivedataProV1->u_reserv_2[0] = 0;
    pTvulivedataProV1->u_reserv_2[1] = 0;
    offset += sizeof(pTvulivedataProV1->u_reserv_2);

    pTvulivedataProV1->i_total_len = pinfo->i_data+sizeof(libshmmedia_tvulive_data_internal_pro_v1_t);
    offset += sizeof(pTvulivedataProV1->i_total_len);

    TVUUTIL_SAFE_MEMCPY(buffer + sizeof(libshmmedia_tvulive_data_internal_pro_v1_t),pinfo->p_data,pinfo->i_data);
    offset += pinfo->i_data;
    return offset;
}

int LibTvuMediaTvuliveWriteData(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size)
{
    int ret = tvulive_write_buffer_v2(pinfo, buffer, buffer_size);
    return ret;
}

int LibTvuMediaTvuliveWriteHead(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size)
{
    int ret = -1;

    if (!pinfo)
    {
        ret = -EINVAL;
        return ret;
    }

    libtvumedia_tvulive_data_t oinfo = *pinfo;
    {
        oinfo.p_data = NULL;
    }

    ret = LibTvuMediaTvuliveWriteData(&oinfo, buffer, buffer_size);

    return ret;
}

static unsigned int sum_section_len(unsigned int counts, const libtvumedia_tvulive_section_pair_t *psec)
{
    unsigned int sum = 0;

    if (!counts || !psec)
    {
        return 0;
    }

    for (unsigned int i = 0; i < counts; i++)
    {
        sum += psec[i].i_section;
    }

    return sum;
}

static unsigned int write_section_to_buffer(unsigned int counts, const libtvumedia_tvulive_section_pair_t *psec, uint8_t *buffer, uint32_t nbuffer)
{
    unsigned int sum = 0;

    if (!counts || !psec)
    {
        return 0;
    }

    uint8_t *pdst = buffer;
    uint32_t left = nbuffer;

    for (unsigned int i = 0; i < counts; i++)
    {
        const uint8_t *psrc = psec[i].p_section;
        uint32_t nsrc = psec[i].i_section;
        if (nsrc > 0 && psrc && left >= nsrc)
        {
            memcpy(pdst, psrc, nsrc);
            left -= nsrc;
            pdst += nsrc;
            sum += nsrc;
        }
    }

    return sum;
}

static int LibTvuMediaTvuliveWriteSectionData(/*IN*/const libtvumedia_tvulive_data_sections_t *psec, int section_data_len, /*OUT*/uint8_t *dest_buffer, /*IN*/uint32_t dest_buffer_size)
{
    int ret = 0;
    uint8_t *pshmBuffer = dest_buffer;
    int nshmLeftBuffer = dest_buffer_size;
    unsigned int pre_head_len = LibTvuMediaTvulivePreferHeadSize();

    if (!section_data_len)
    {
        section_data_len = psec->u_sum_section_size;
        if (!section_data_len)
            section_data_len = sum_section_len(psec->u_section_counts, psec->p_sections);
    }

    if (!section_data_len)
    {
        ret = -EINVAL;
        DEBUG_ERROR("tvulive section parameters invalid, the pre tvulive len 0\n");
        return ret;
    }

    libtvumedia_tvulive_data_t oInfo;
    {
        memset(&oInfo, 0, sizeof(oInfo));
        if (psec->u_struct_size >= sizeof(libtvumedia_tvulive_data_sections_v2_t))
        {
            oInfo.u_struct_size = sizeof(libtvumedia_tvulive_data_v2_t);
        }

        oInfo.o_info = psec->o_info;
        oInfo.i_data = section_data_len;
        oInfo.p_data = NULL;
    }
    ret = LibTvuMediaTvuliveWriteHead(&oInfo, pshmBuffer, nshmLeftBuffer);

    if (ret <= 0)
    {
        DEBUG_ERROR("write tvulive protocol head failed, ret %d\n", ret);
        return ret;
    }
    else if (ret != (int)pre_head_len)
    {
        DEBUG_WARN("write head ret %d != pre head len %d\n", ret, pre_head_len);
        return 0;
    }

    pshmBuffer += ret;
    nshmLeftBuffer -= ret;

    if (nshmLeftBuffer < section_data_len)
    {
        DEBUG_WARN("writing tvulive data failed for lack of buffer."
                   "need%d:,left:%d"
                   "\n"
                   , section_data_len
                   , nshmLeftBuffer
                   );
        return 0;
    }

    ret = write_section_to_buffer(psec->u_section_counts, psec->p_sections, pshmBuffer, nshmLeftBuffer);
    if (ret != (int)section_data_len)
    {
        DEBUG_WARN("writing tvulive section data invalid, ignore."
                   "ret:%d,len:%d\n", ret, section_data_len);
        return 0;
    }

    ret = pre_head_len+section_data_len;
    return ret;
}

int LibTvuMediaTvuliveReadData(/*OUT*/libtvumedia_tvulive_data_t *pinfo, /*IN*/ const uint8_t *buffer, /*IN*/const uint32_t buffer_len)
{
    libshmmedia_tvulivedata_internal_pro_common_t *pCommon = (libshmmedia_tvulivedata_internal_pro_common_t *) buffer;
    int ret = -1;

    if (!buffer || !buffer_len || !pinfo)
    {
        return ret;
    }

    switch (pCommon->u_version) {
        case kLibshmMediaTvuliveDataProV1:
        {
            uint32_t offset = 0;
            libshmmedia_tvulive_data_internal_pro_v1_t* tvulivedataProV1 = (libshmmedia_tvulive_data_internal_pro_v1_t*) buffer;
            offset += sizeof(libshmmedia_tvulivedata_internal_pro_common_t);

            pinfo->o_info.i_type = tvulivedataProV1->i_type;
            offset += sizeof(tvulivedataProV1->i_type); //i_type

            pinfo->o_info.u_stream_index = tvulivedataProV1->u_stream_index;
            offset += sizeof(tvulivedataProV1->u_stream_index);

            offset += sizeof(tvulivedataProV1->u_reserve1);

            pinfo->o_info.u_frame_index = tvulivedataProV1->u_frame_index;
            offset += sizeof(tvulivedataProV1->u_frame_index);

            offset += sizeof(tvulivedataProV1->u_reserv_2);

            if (tvulivedataProV1->i_total_len <= sizeof(libshmmedia_tvulive_data_internal_pro_v1_t))
            {
                ret = -1;
                break;
            }

            uint32_t data_len =
            pinfo->i_data = tvulivedataProV1->i_total_len - sizeof(libshmmedia_tvulive_data_internal_pro_v1_t);
            offset += sizeof(tvulivedataProV1->i_total_len); //i_data_len;

            pinfo->p_data = buffer + offset;
            ret = offset+data_len;
        }
        break;
        case kLibshmMediaTvuliveDataProV2:
        {
            libshmmedia_tvulive_data_internal_pro_v20_t* tvulivedataProV20 = (libshmmedia_tvulive_data_internal_pro_v20_t*) buffer;
            uint32_t dataoffset = ntohl(tvulivedataProV20->u_data_offset);

            if (dataoffset < sizeof(libshmmedia_tvulive_data_internal_pro_v21_t))
            {
                ret = _read_tvulive_data_v20(pinfo, buffer, buffer_len);
            }
            else
            {
                libshmmedia_tvulive_data_internal_pro_v21_t* pV21 = (libshmmedia_tvulive_data_internal_pro_v21_t*) buffer;
                uint32_t struct_size = ntohl(pV21->u_head_struct_size_);

                if (struct_size >= sizeof(libshmmedia_tvulive_data_internal_pro_v23_t))
                {
                    ret = _read_tvulive_data_v23_with_struct_size(pinfo, buffer, buffer_len);
                }
                else if (struct_size >= sizeof(libshmmedia_tvulive_data_internal_pro_v22_t))
                {
                    ret = _read_tvulive_data_v22(pinfo, buffer, buffer_len);
                }
                else if (struct_size >= sizeof(libshmmedia_tvulive_data_internal_pro_v21_t))
                {
                    ret = _read_tvulive_data_v21(pinfo, buffer, buffer_len);
                }
            }
        }
        break;
        default:
        {
            ;
        }
        break;
    }

    return ret;
}

/* CLibShmmediaTvuliveWrapHandle functions --start */
int CLibShmmediaTvuliveWrapHandle::create(const char *pshmname, uint32_t header_len
                                , uint32_t item_count
                                , uint64_t total_size)
{
    libshm_media_handle_t hshm = LibViShmMediaCreate(pshmname, header_len, item_count, total_size);

    if (!hshm)
    {
        return -1;
    }

    hshm_ = hshm;
    shmname_ = pshmname;
    return 0;
}

int CLibShmmediaTvuliveWrapHandle::open(const char *pshmname)
{
    libshm_media_handle_t hshm = LibViShmMediaOpen(pshmname, NULL, NULL);

    if (!hshm)
    {
        return -1;
    }

    hshm_ = hshm;
    shmname_ = pshmname;
    return 0;
}

void CLibShmmediaTvuliveWrapHandle::destroy()
{
    if (hshm_)
    {
        LibViShmMediaDestroy(hshm_);
        hshm_ = NULL;
    }

    return;
}

int  CLibShmmediaTvuliveWrapHandle::write(const libtvumedia_tvulive_data_sections_t *p)
{
    int ret = -1;
    libshm_media_handle_t h = hshm_;

    if (h)
    {
        unsigned int pre_head_len = LibTvuMediaTvulivePreferHeadSize();
        unsigned int pre_tvulive_len = p->u_sum_section_size;
        unsigned int user_data_len = 0;
        uint8_t *pItemBuff = NULL;
        libshm_media_item_param_t omiv;
        {
            memset(&omiv, 0, sizeof(omiv));
        }

        libshm_media_item_param_t &omi = omiv;

        libshm_media_item_addr_layout_t bufferLayout;
        {
            memset(&bufferLayout, 0, sizeof(bufferLayout));
        }

        int sendableRet = LibViShmMediaPollSendable(h, 0);
        if (sendableRet <= 0)
        {
            return sendableRet;
        }

        if (!pre_tvulive_len)
        {
            pre_tvulive_len = sum_section_len(p->u_section_counts, p->p_sections);
        }

        if (!pre_tvulive_len)
        {
            ret = -EINVAL;
            DEBUG_ERROR("libshmmedia, tvulive section parameters invalid, the pre tvulive len 0\n");
            return ret;
        }

        user_data_len = pre_tvulive_len + pre_head_len;

        pItemBuff = LibViShmMediaItemApplyBuffer(h, user_data_len);

        if (!pItemBuff)
        {
            ret = -ENOMEM;
            DEBUG_ERROR("libshmmedia, item apply[u:%d] failed, ret %d\n", user_data_len, ret);
            return ret;
        }

        omi.i64_userDataCT = p->u_createTime;
        omi.i_userDataLen = user_data_len;
        omi.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;
        ret = LibViShmMediaItemPreGetWriteBufferLayout(h, &omiv, pItemBuff, &bufferLayout);

        if (!bufferLayout.p_userData || ret<= 0)
        {
            DEBUG_ERROR("libshmmedia, get item user data address failed\n");
            return 0;
        }

        uint8_t *pshmBuffer = bufferLayout.p_userData;
        uint32_t nshmLeftBuffer = user_data_len;

        ret  = LibTvuMediaTvuliveWriteSectionData(p, pre_tvulive_len, pshmBuffer, nshmLeftBuffer);
        if (ret <= 0)
        {
            DEBUG_ERROR("writing tvulive protocol section data failed."
                        "ret:%d\n", ret);
            return ret;
        }

        libshm_media_head_param_t omh;
        {
            memset(&omh, 0, sizeof(omh));
        }

        LibViShmMediaItemWriteBufferIgnoreInternalCopy(h, &omh, &omiv, pItemBuff);
        LibViShmMediaItemCommitBuffer(h, pItemBuff, user_data_len);
        ret = pre_head_len+pre_tvulive_len;
    }

    return ret;
}

int  CLibShmmediaTvuliveWrapHandle::writeWithFrequency1000(const libtvumedia_tvulive_data_sections_t *p)
{
    int64_t now = _libshm_get_sys_us64();
    if (now < _lastWritingSysTimeUs + 1000)
    {
        _libshm_common_usleep(1000);
    }

    int ret = write(p);

    if (ret > 0)
    {
        _lastWritingSysTimeUs = now;
    }

    return ret;
}

int  CLibShmmediaTvuliveWrapHandle::read(libtvumedia_tvulive_data_t *pInfo)
{
    int ret = -1;
    libshm_media_handle_t h = hshm_;

    if (h)
    {
        libshm_media_head_param_t omh;
        {
            memset(&omh, 0, sizeof(omh));
        }

        libshm_media_item_param_t omiv;
        {
            memset(&omiv, 0, sizeof(omiv));
        }

        ret = LibViShmMediaPollReadData(h, &omh, &omiv, 0);

        if (ret < 0)
        {
            DEBUG_ERROR("poll read shm data failed, ret %d\n", ret);
            return ret;
        }
        else if (ret == 0)
        {
            return ret;
        }

        const libshm_media_item_param_t &omi = omiv;

        if (omi.i_userDataLen <= 0 || !omi.p_userData)
        {
            DEBUG_WARN("read out the user data len [%d] or user data point NULL, invalide\n", omi.i_userDataLen);
            return 0;
        }

        if (omi.i_userDataType != LIBSHM_MEDIA_TYPE_TVULIVE_DATA)
        {
            DEBUG_WARN("read out the user data type[%d] invalide, not TVULIVE data\n", omi.i_userDataType);
            return 0;
        }

        if (!pInfo)
        {
            return 0;
        }

        pInfo->u_createTime = omi.i64_userDataCT;

        ret = LibTvuMediaTvuliveReadData(pInfo, omi.p_userData, omi.i_userDataLen);
    }

    return ret;
}

uint64_t CLibShmmediaTvuliveWrapHandle::getWriteIndex()
{
    uint64_t ret = -1;
    libshm_media_handle_t h = hshm_;
    if (h)
    {
        ret = LibViShmMediaGetWriteIndex(h);
    }
    return ret;
}

uint64_t CLibShmmediaTvuliveWrapHandle::getReadIndex()
{
    uint64_t ret = -1;
    libshm_media_handle_t h = hshm_;
    if (h)
    {
        ret = LibViShmMediaGetReadIndex(h);
    }
    return ret;
}

void CLibShmmediaTvuliveWrapHandle::seekReadIndex(uint64_t index)
{
    libshm_media_handle_t h = hshm_;
    if (h)
    {
        LibViShmMediaSeekReadIndex(h, index);
    }
    return;
}

void CLibShmmediaTvuliveWrapHandle::seekReadIndexToWriteIndex()
{
    libshm_media_handle_t h = hshm_;
    if (h)
    {
        LibViShmMediaSeekReadIndexToWriteIndex(h);
    }
    return;
}

struct TvuliveDataWrapUserDataCallback
{
    void *user_;
    libshmmedia_tvulive_wrap_handle_search_items_fn_t cb_;
};

static int _search_items_callback(void *userWrapCtx, const libshm_media_head_param_t */*pmh*/, const libshm_media_item_param_t*pmiv)
{
    const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
    struct TvuliveDataWrapUserDataCallback *pctx = (struct TvuliveDataWrapUserDataCallback *)userWrapCtx;
    libtvumedia_tvulive_data_t oinfo;
    {
        memset(&oinfo, 0, sizeof(libtvumedia_tvulive_data_t));
    }

    if (pmi->i_userDataLen <= 0 || !pmi->p_userData)
    {
        DEBUG_WARN("search items callback, data invalide." "l:%d, d:%p\n", pmi->i_userDataLen, pmi->p_userData);
        return 0;
    }

    if (pmi->i_userDataType != LIBSHM_MEDIA_TYPE_TVULIVE_DATA)
    {
        DEBUG_WARN("search items callback, not TVULIVE data, invalide." "dt:%d\n", pmi->i_userDataType);
        return 0;
    }

    int ret = 0;
    bool bsearched =false;
    {
        oinfo.u_createTime = pmi->i64_userDataCT;
        ret = LibTvuMediaTvuliveReadData(&oinfo, pmi->p_userData, pmi->i_userDataLen);

        if (ret > 0)
        {
            bsearched = pctx->cb_(pctx->user_, &oinfo);
        }
    }

    return bsearched?1:0;
}

int CLibShmmediaTvuliveWrapHandle::searchItems(void *user, libshmmedia_tvulive_wrap_handle_search_items_fn_t m)
{
    libshm_media_handle_t h = hshm_;
    int ret = 0;
    if (h)
    {
        struct TvuliveDataWrapUserDataCallback userWrapCtx;
        {
            memset(&userWrapCtx, 0, sizeof(userWrapCtx));
        }
        userWrapCtx.user_ = user;
        userWrapCtx.cb_ = m;
        ret = LibViShmMediaSearchItems(h, &userWrapCtx, _search_items_callback);
    }
    return ret;
}

/* CLibShmmediaTvuliveWrapHandle functions --end */


libshmmedia_tvulive_wrap_handle_t LibShmMediaTvuliveWrapHandleCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
)
{
    CLibShmmediaTvuliveWrapHandle *ph = NULL;

    if (!pMemoryName)
    {
        DEBUG_ERROR(
                    "libshmmediaTvuliveWrapHandleCreate failed for null shm name."
                    "\n");
        return NULL;
    }

    ph = new CLibShmmediaTvuliveWrapHandle();
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

libshmmedia_tvulive_wrap_handle_t LibShmMediaTvuliveWrapHandleOpen
(
    const char * pMemoryName
)
{
    CLibShmmediaTvuliveWrapHandle *ph = NULL;
    if (!pMemoryName)
    {
        DEBUG_ERROR(
                    "LibShmMediaTvuliveWrapHandleOpen failed for null shm name."
                    "\n");
        return NULL;
    }
    ph = new CLibShmmediaTvuliveWrapHandle();
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

void LibShmMediaTvuliveWrapHandleDestroy
(
    libshmmedia_tvulive_wrap_handle_t h
)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;

    if (ph)
    {
        delete  ph;
    }

    return;
}

int LibShmMediaTvuliveWrapHandleWrite(/*IN*/const libshmmedia_tvulive_wrap_handle_t h, const libtvumedia_tvulive_data_sections_t *p)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->write(p);
    }

    return ret;
}

int LibShmMediaTvuliveWrapHandleWriteWithFrequency1000(/*IN*/const libshmmedia_tvulive_wrap_handle_t h, const libtvumedia_tvulive_data_sections_t *p)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->writeWithFrequency1000(p);
    }

    return ret;
}

uint64_t LibShmMediaTvuliveWrapHandleGetWriteIndex(libshmmedia_tvulive_wrap_handle_t h)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    uint64_t ret = -1;

    if (ph)
    {
        ret = ph->getWriteIndex();
    }

    return ret;
}

int LibShmMediaTvuliveWrapHandleRead(/*IN*/const libshmmedia_tvulive_wrap_handle_t h
                                 , /*OUT*/libtvumedia_tvulive_data_t *pInfo)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    int ret = -1;

    if (ph)
    {
        ret = ph->read(pInfo);
    }

    return ret;
}

uint64_t LibShmMediaTvuliveWrapHandleGetReadIndex(libshmmedia_tvulive_wrap_handle_t h)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    uint64_t ret = -1;

    if (ph)
    {
        ret = ph->getReadIndex();
    }

    return ret;
}

void LibShmMediaTvuliveWrapHandleSeekReadIndex(libshmmedia_tvulive_wrap_handle_t h, uint64_t index)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;

    if (ph)
    {
        ph->seekReadIndex(index);
    }

    return;
}

void LibShmMediaTvuliveWrapHandleSeekReadIndexToWriteIndex(libshmmedia_tvulive_wrap_handle_t h)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;

    if (ph)
    {
        ph->seekReadIndexToWriteIndex();
    }

    return;
}

int LibShmMediaTvuliveWrapHandleSearchItems(/*IN*/const libshmmedia_tvulive_wrap_handle_t h
                                 , /*IN*/void *userCtx, /*IN*/libshmmedia_tvulive_wrap_handle_search_items_fn_t fn)
{
    CLibShmmediaTvuliveWrapHandle *ph = (CLibShmmediaTvuliveWrapHandle *)h;
    int ret = 0;

    if (ph)
    {
        ret = ph->searchItems(userCtx, fn);
    }

    return ret;
}

