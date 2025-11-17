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

#include "libshmmedia_encoding_protocol.h"
#include "libshm_util_common_internal.h"
#include <string.h>

#pragma pack(push, 1)
/* if LIBSHM_MEDIA_TYPE_ENCODING_DATA protocol */
typedef struct SLibShmMediaEncodingDataInternalProtocolCommon
{
    uint8_t u_version;
}libshmmedia_encoding_data_internal_pro_common_t;

typedef struct SLibShmMediaEncodingDataInternalProtocol
{
    libshmmedia_encoding_data_internal_pro_common_t u_common;
    /**
     *  u_codec_tag definition is at enum ETvuCodecTagFourCC of tvu_fourcc.h
     */
    uint32_t    u_codec_tag; // LE
    uint32_t    u_stream_index; // LE
    uint16_t    u_frame_index; //LE
    int64_t     i64_pts; // LE
    int64_t     i64_dts; // LE
    uint8_t     u_reserv[2];    /* must to be set 0 before using future */
    uint32_t    i_data_len;     /* len(p_data) */
    uint32_t    u_data_offset;
    //  .......
    //  char        p_data[0];
}libshmmedia_encoding_data_internal_pro_t;
/* endif LIBSHM_MEDIA_TYPE_ENCODING_DATA protocol */
#pragma pack(pop)

/* if Encoding data part */
uint32_t LibShmmediaEncodingDataGetBufferSize(uint32_t data_len)
{
    return sizeof(libshmmedia_encoding_data_internal_pro_t) + data_len;
}
uint32_t LibTvuMediaEncodingDataGetBufferSize(uint32_t data_len)
{
    return LibShmmediaEncodingDataGetBufferSize(data_len);
}

int LibShmmediaEncodingDataWrite(/*IN*/const libshmmedia_encoding_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size)
{
    int offset = 0;
    int total_size = 0;

    if (!buffer || !buffer_size)
    {
        return -1;
    }

    if((uint32_t)buffer_size < sizeof (libshmmedia_encoding_data_internal_pro_t) + pinfo->i_data)
    {
        return -1; //buffer size is not enough.
    }
    libshmmedia_encoding_data_internal_pro_t *ptvuencodingdataProV1 = (libshmmedia_encoding_data_internal_pro_t *)buffer;
    ptvuencodingdataProV1->u_common.u_version = kLibshmMediaEncodingDataProV1;
    offset += sizeof(libshmmedia_encoding_data_internal_pro_common_t);

    ptvuencodingdataProV1->u_codec_tag = pinfo->u_codec_tag;
    offset += sizeof(ptvuencodingdataProV1->u_codec_tag);

    ptvuencodingdataProV1->u_frame_index = pinfo->u_frame_index;
    offset += sizeof(ptvuencodingdataProV1->u_frame_index);

    ptvuencodingdataProV1->u_stream_index = pinfo->u_stream_index;
    offset += sizeof(ptvuencodingdataProV1->u_stream_index);

    ptvuencodingdataProV1->i64_dts = pinfo->i64_dts;
    ptvuencodingdataProV1->i64_pts = pinfo->i64_pts;
    offset += sizeof(ptvuencodingdataProV1->i64_pts) + sizeof(ptvuencodingdataProV1->i64_dts);

    ptvuencodingdataProV1->u_reserv[0] = 0;
    ptvuencodingdataProV1->u_reserv[1] = 0;
    offset += sizeof(ptvuencodingdataProV1->u_reserv);

    ptvuencodingdataProV1->i_data_len = pinfo->i_data;
    offset += sizeof(ptvuencodingdataProV1->i_data_len);

    offset += sizeof(ptvuencodingdataProV1->u_data_offset);
    ptvuencodingdataProV1->u_data_offset = offset;

    TVUUTIL_SAFE_MEMCPY(buffer + offset, pinfo->p_data, pinfo->i_data);
    total_size = offset + pinfo->i_data;
    return total_size;
}
int LibTvuMediaEncodingDataWrite(/*IN*/const libshmmedia_encoding_data_t *pinfo, /*OUT*/uint8_t *dest_buffer, /*IN*/uint32_t dest_buffer_size)
{
    return LibShmmediaEncodingDataWrite(pinfo, dest_buffer, dest_buffer_size);
}

int LibShmmediaEncodingDataRead(/*OUT*/libshmmedia_encoding_data_t *pinfo, /*IN*/ const uint8_t *buffer, /*IN*/const uint32_t buffer_len)
{
    libshmmedia_encoding_data_internal_pro_common_t *pCommon = (libshmmedia_encoding_data_internal_pro_common_t *) buffer;
    int ret = -1;

    if (!buffer || !buffer_len)
    {
        return ret;
    }

    switch(pCommon->u_version)
    {
        case kLibshmMediaEncodingDataProV1:
        {

            uint32_t offset = 0;
            libshmmedia_encoding_data_internal_pro_t * encodingDataPro = (libshmmedia_encoding_data_internal_pro_t *) buffer;
            offset += sizeof(libshmmedia_encoding_data_internal_pro_common_t);

            pinfo->u_codec_tag = encodingDataPro->u_codec_tag;
            offset += sizeof(encodingDataPro->u_codec_tag);

            uint32_t uStreamIndex = encodingDataPro->u_stream_index;
            pinfo->u_stream_index = uStreamIndex;
            offset += sizeof(encodingDataPro->u_stream_index);

            pinfo->u_frame_index = encodingDataPro->u_frame_index;
            offset += sizeof(encodingDataPro->u_frame_index);

            pinfo->i64_pts = encodingDataPro->i64_pts;
            pinfo->i64_dts = encodingDataPro->i64_dts;
            offset += sizeof(encodingDataPro->i64_pts) + sizeof(encodingDataPro->i64_dts);

            offset += sizeof(encodingDataPro->u_reserv);// i_resev[2]

            uint32_t data_len = pinfo->i_data = encodingDataPro->i_data_len;
            offset += sizeof(encodingDataPro->i_data_len);

            uint32_t data_offset = encodingDataPro->u_data_offset;
            offset += sizeof(encodingDataPro->u_data_offset);

            if (data_offset < offset)
            {
                ret = -1;
                break;
            }

            if (data_offset + data_len > buffer_len)
            {
                ret = -1;
                break;
            }

            pinfo->p_data = buffer + data_offset;
            ret = data_offset + data_len;
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
int LibTvuMediaEncodingDataRead(/*OUT*/libshmmedia_encoding_data_t *pinfo, /*IN*/ const uint8_t *buffer, /*IN*/const uint32_t buffer_len)
{
    return LibShmmediaEncodingDataRead(pinfo, buffer, buffer_len);
}
/* endif Encoding data part */
