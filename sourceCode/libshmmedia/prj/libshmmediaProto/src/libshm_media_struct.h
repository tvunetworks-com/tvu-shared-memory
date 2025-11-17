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
#ifndef _LIBSHM_MEDIA_STRUCT_H
#define _LIBSHM_MEDIA_STRUCT_H    

#include "shmhead.h"
#include "libshm_media_protocol.h"
#include <stdint.h>

#define LIBSHM_MEDIA_HEAD_V3_SUBVER_V0     0
#define LIBSHM_MEDIA_HEAD_V3_SUBVER_V1     1 // did not use current time, May 16th, 2022. And would not be used in future.
#define LIBSHM_MEDIA_HEAD_V3_SUB_VERSION   LIBSHM_MEDIA_HEAD_V3_SUBVER_V0

#define  LIBSHM_MEDIA_HEAD_VERSION_V1      MEMHEADER_V1
#define  LIBSHM_MEDIA_HEAD_VERSION_V2      MEMHEADER_V2
#define  LIBSHM_MEDIA_HEAD_VERSION_V3      (MEMHEADER_V3+LIBSHM_MEDIA_HEAD_V3_SUBVER_V0)
#define  LIBSHM_MEDIA_HEAD_VERSION_V4      (MEMHEADER_V4)

#define LIBSHMMEDIA_ITEM_READ_SHM_U64(v)         tvushm::_tvuutil_letoh64(v)
#define LIBSHMMEDIA_ITEM_READ_SHM_U32(v)         tvushm::_tvuutil_letoh32(v)
#define LIBSHMMEDIA_ITEM_READ_SHM_U16(v)         tvushm::_tvuutil_letoh16(v)
#define LIBSHMMEDIA_ITEM_READ_SHM_U8(v)          (v)

#define LIBSHMMEDIA_ITEM_WRITE_SHM_U64(v)        tvushm::_tvuutil_htole64(v)
#define LIBSHMMEDIA_ITEM_WRITE_SHM_U32(v)        tvushm::_tvuutil_htole32(v)
#define LIBSHMMEDIA_ITEM_WRITE_SHM_U16(v)        tvushm::_tvuutil_htole16(v)
#define LIBSHMMEDIA_ITEM_WRITE_SHM_U8(v)         (v)

/* header was not passed by RSHM, no need endian transfering */
#define LIBSHMMEDIA_HEAD_READ_SHM_U64(v)         (v)
#define LIBSHMMEDIA_HEAD_READ_SHM_U32(v)         (v)
#define LIBSHMMEDIA_HEAD_READ_SHM_U16(v)         (v)
#define LIBSHMMEDIA_HEAD_WRITE_SHM_U64(v)        (v)
#define LIBSHMMEDIA_HEAD_WRITE_SHM_U32(v)        (v)
#define LIBSHMMEDIA_HEAD_WRITE_SHM_U16(v)        (v)

typedef enum {
    kLibShmMediaItemVerInvalid = INVALID_SHM_HEAD_VERSION,
    kLibShmMediaItemVerV1 =  LIBSHM_MEDIA_HEAD_VERSION_V1,
    kLibShmMediaItemVerV2 =  LIBSHM_MEDIA_HEAD_VERSION_V2,
    kLibShmMediaItemVerV3 =  LIBSHM_MEDIA_HEAD_VERSION_V3,
    kLibShmMediaItemVerV4 =  LIBSHM_MEDIA_HEAD_VERSION_V4,
}libshmmedia_item_version_t;

#define LIBSHM_MEDIA_ITEM_CURRENT_VERSION   kLibShmMediaItemVerV4

#pragma pack(push, 1)

typedef struct {
    shm_construct_t head1;
    int close_flag;
    int init_flag;
    int vbr;      // kbps
    int sarw;     // 9999 codec sarw
    int sarh;     // 9999 codec sarh
    int srcw;     // 9999 codec width
    int srch;     // 9999 codec height
    int sdiw;     // player output YUV width
    int sdih;     // player output YUV heigh
}MEM_HEADER_V2, shm_media_head_info_v2_t;

typedef struct {
    unsigned int ver;
    unsigned int length;
    unsigned int videolen;
    unsigned int audiolen;
    unsigned int extralength;//extra data's length
    unsigned int videofourcc;
    unsigned int frametype;
    unsigned long long videocreatetime;
    unsigned long long videopts;
    unsigned long long videodts;
    int width;
    int height;
    int duration;
    int scale;
    unsigned int audiofourcc;
    unsigned long long audiocreatetime;
    unsigned long long audiopts;
    unsigned long long audiodts;
    int channels;
    int depth;
    int samplerate;
}MEM_ITEM_HEADER_V1, shm_media_item_info_v1_v2_t;

typedef struct SV3Common
{
    shm_construct_t head1;
    uint32_t        v3_sub_ver; // always 0, May 16th, 2022. would be not extended againin future.
    int32_t         close_flag;
    int32_t         init_flag;
}v3_common_t;

typedef struct 
{
    v3_common_t     v3;
    int32_t         vbr;      // kbps
    int32_t         sarw;     // 9999 codec sarw
    int32_t         sarh;     // 9999 codec sarh
    int32_t         srcw;     // 9999 codec width
    int32_t         srch;     // 9999 codec height
    int32_t         sdiw;     // player output YUV width
    int32_t         sdih;     // player output YUV heigh
}shm_media_head_info_v3_t;

typedef struct SLibshmMediaItemInfoV3 {
    uint32_t                length;
    uint32_t                videolen;
    uint32_t                audiolen;
    uint32_t                subtlen;
    uint32_t                cclen;//close caption length
    uint32_t                timecodelen; //timecode length
    uint32_t                datalen;//extra data's length
    uint32_t                frametype;
    uint32_t                interlace_flag;
    int64_t                 videoct;
    int64_t                 videopts;
    int64_t                 videodts;
    int32_t                 width;
    int32_t                 height;
    uint32_t                vfourcc;
    int32_t                 duration;
    int32_t                 scale;
    uint32_t                pic_flag;
    int64_t                 audioct;
    int64_t                 audiopts;
    int64_t                 audiodts;
    uint32_t                afourcc;
    int32_t                 channels;
    int32_t                 depth;
    int32_t                 samplerate;
    int64_t                 subtct;
    int64_t                 subtpts;
    int64_t                 subtdts;
    int64_t                 data_ct;
    uint32_t                data_type;
}shm_media_item_info_v3_t;


typedef struct
{
    shm_media_head_info_v3_t _v3h;
    uint64_t    _head_data_size;
    uint64_t    _item_data_size;
    uint64_t    _count_number;
    uint64_t    _total_data_size;
    char        _reserver[4];
}shm_media_head_info_v4_t;

typedef struct SLibshmMediaItemInfoV4 {
    shm_media_item_info_v3_t _v3i;
    uint32_t                _head_len;
    uint32_t                _v_data_offset;
    uint32_t                _a_data_offset;
    uint32_t                _s_data_offset;
    uint32_t                _cc_data_offset;
    uint32_t                _timecode_data_offset;
    uint32_t                _user_data_offset;
    char                    _reserved[4];
} shm_media_item_info_v40_t;

// not use now

typedef struct SLibshmMediaItemInfoV4Subv0 {
    shm_media_item_info_v3_t _v3i;
    uint32_t                _head_len;
    uint32_t                _v_data_offset;
    uint32_t                _a_data_offset;
    uint32_t                _s_data_offset;
    uint32_t                _cc_data_offset;
    uint32_t                _timecode_data_offset;
    uint32_t                _user_data_offset;
    uint8_t                 _item_version; // item version, from V40, the value is 0, subv0 would be 4
    uint8_t                 _reserver2;
    uint8_t                 _reserver3;
    uint8_t                 _reserver4;
}shm_media_item_info_v4_subv0_t;// equal to v40.

typedef struct SLibshmMediaItemInfoV4Subv0v0 {
    shm_media_item_info_v3_t _v3i;
    uint32_t                _head_len;
    uint32_t                _v_data_offset;
    uint32_t                _a_data_offset;
    uint32_t                _s_data_offset;
    uint32_t                _cc_data_offset;
    uint32_t                _timecode_data_offset;
    uint32_t                _user_data_offset;
    uint8_t                 _item_version; // item version, from V40, the value is 0, subv0 would be 4
    uint8_t                 _flag1_media_head_extension:1;
    uint8_t                 _flag1:7;
    uint8_t                 _reserver3;
    uint8_t                 _reserver4;
}shm_media_item_info_v4_subv0v0_t;// equal to v40.

typedef struct SLibshmMediaItemInfoV4Subv1 {
    shm_media_item_info_v4_subv0_t  _v4i;
    int32_t                         sarw;
    int32_t                         sarh;
    int32_t                         srcw;
    int32_t                         srch;
    uint8_t                         _reserver[8];
}shm_media_item_info_v4_subv1_t;

typedef struct SLibshmMediaItemInfoV4Subv1v0 {
    shm_media_item_info_v4_subv0v0_t  _v4i;
    int32_t                         sarw;
    int32_t                         sarh;
    int32_t                         srcw;
    int32_t                         srch;
    uint32_t                        _key_value_area_offset;
    uint32_t                        _key_value_area_len;
}shm_media_item_info_v4_subv1v0_t;

typedef shm_media_item_info_v4_subv1_t shm_media_item_info_v4_t;

#pragma pack(pop)

#endif
