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
#ifndef _LIBSHM_MEDIA_PROTOCOL_INTERNAL_H
#define _LIBSHM_MEDIA_PROTOCOL_INTERNAL_H    1

#include "sharememory_internal.h"
#include "libshm_media_struct.h"
#include "libshm_media_protocol.h"
#include "libshm_media_protocol_log_internal.h"
#include <stdint.h>
#include <string.h>

#define _LISHMMEDIA_MEM_ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

#if 0 /* un-used. */
typedef union ULibShmMediaInfo
{
    struct SVideoInfo
    {
        int32_t     duration;
        int32_t     scale;
        int32_t     width;
        int32_t     height;
        uint32_t    frame_type;
        uint32_t    pic_flag;
        uint32_t    interlace_flag;
    }v;

    struct SAudioInfo
    {
        int32_t     sample_rate;
        int32_t     depth;
        uint32_t    channels;
    }a;
}libshm_media_info_u_t;

typedef struct SLibShmMeidaDataInfo
{
    uint16_t            u_type;
    int                 i_len;
    uint32_t            u_rawFourCC;
    uint32_t            u_codecId;
    int64_t             i64_pts;
    int64_t             i64_dts;
    int64_t             i64_ctm;
    uint8_t             *p_data;
    libshm_media_info_u_t media_info;
}libshm_media_data_info_t;
#endif

typedef struct {

    uint32_t nKeyValueSize_;
    const uint8_t *pKeyValuePtr_;

}libshm_media_item_param_internal_t;

namespace libshmmediapro {

static inline uint32_t getAlignOffset(uint8_t *pstart, uint32_t offset)
{

    uint8_t *p_algin_start = (uint8_t *)_LISHMMEDIA_MEM_ALIGN((uintptr_t)(pstart + offset), 16);

    return (p_algin_start - pstart);
}

static inline uint32_t getItemParamDataLen(const libshm_media_item_param_t *pmiv)
{
    const libshm_media_item_param_v1_t *pvi = (const libshm_media_item_param_v1_t *)pmiv;
    return pvi->i_vLen + pvi->i_aLen + pvi->i_sLen + pvi->i_CCLen + pvi->i_timeCode + pvi->i_userDataLen;
}


int  alignShmItemLength(int rawLen);
int  preRequireItemHeadLength(uint32_t v);
int  preRequireMaxItemHeadLength(uint32_t v);
int  preRequireHeadLength(uint32_t v);
unsigned int getBufferLenFromItemBuffer(const uint8_t *pItem, uint32_t head_ver);
void setCloseFlag(uint8_t *pHead, bool bclose);
bool checkCloseFlag(const uint8_t *pHead);
int  initShmV2(uint8_t *phead, int flags);
int  initShmV3(uint8_t *phead, int flags);
int  initShmV4(uint8_t *phead, uint64_t head_len, int flags);

int  getReadItemBufferLayoutV12(/*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr);

int  getReadItemBufferLayoutV3(/*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr);

int  getReadItemBufferLayoutV4(/*OUT*/libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr);

int  getWriteItemBufferLayoutV4(/*IN*/const libshm_media_item_param_t *pmi, const libshm_media_item_param_internal_t &rii
    , /*IN*/uint8_t *pItemAddr, /*OUT*/libshm_media_item_addr_layout_t *playout);

int  getReadItemBufferLayoutWithVer(/*OUT*/libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr, uint32_t major_version);

int  getWriteItemBufferLayoutWithVer(/*IN*/const libshm_media_item_param_t *pmi
    , /*IN*/uint8_t *pItemAddr, /*OUT*/libshm_media_item_addr_layout_t *playout, uint32_t major_version);

int  getWriteItemBufferLayout(/*IN*/const libshm_media_item_param_t *pmi
    , /*IN*/uint8_t *pItemAddr, /*OUT*/libshm_media_item_addr_layout_t *playout);


int  writeItemBufferV4(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi, const libshm_media_item_param_internal_t &rii, uint8_t *pItemAddr);
int  writeItemBuffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi, uint8_t *pItemAddr, uint32_t item_ver);

int  readDataFromItemBuffer(libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr, uint32_t nItem);
int  readHeadFromItemBuffer(libshm_media_head_param_t *pmh, const uint8_t *pItemAddr, unsigned int nItem);
unsigned int getShmMediaHeadVersion(const uint8_t *headBuf);

uint32_t getItemHeadLengthV4();

uint32_t _headParamGetStructSize(const libshm_media_head_param_t &p);
uint32_t _headParamGetMinStructSize(const libshm_media_head_param_t &p1, const libshm_media_head_param_t &p2);
void _headParamLowCopy(libshm_media_head_param_t &dst, const libshm_media_head_param_t &src);
int _headParamLowCompare(const libshm_media_head_param_t &dst, const libshm_media_head_param_t &src);
const libshmmedia_audio_channel_layout_object_t *_headParamGetChannelLayout(const libshm_media_head_param_t &r);
}


#endif
