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
#ifndef LIBSHMMEDIA_TVULIVE_PROTOCOL_INTERNAL_H
#define LIBSHMMEDIA_TVULIVE_PROTOCOL_INTERNAL_H

#include "libshmmedia_tvulive_protocol.h"
#include "libshm_media_variable_item.h"
#include <string>
#include <stdint.h>

#pragma pack(push, 1)
/*****************************************
 *  for extended data
 *  LIBSHM_MEDIA_TYPE_TVULIVE_DATA_V2
*****************************************/
/* if LIBSHM_MEDIA_TYPE_TVULIVE_DATA protocol */

typedef struct SLibShmMediaTvuliveDataInternalProtocolCommon
{
    uint8_t u_version;
}libshmmedia_tvulivedata_internal_pro_common_t;

typedef struct SLibShmMediaTvuliveDataInternal_ProtocolV1
{
    libshmmedia_tvulivedata_internal_pro_common_t u_common;
    /**
     *  'v' express video type,
     *  'a' express audio,
     *  'h' express head
     */
    int8_t      i_type;
    uint8_t     u_stream_index;
    uint8_t     u_reserve1;
    uint16_t    u_frame_index;
    uint8_t     u_reserv_2[2];
    uint32_t    i_total_len;
    //char        p_data[0];
}libshmmedia_tvulive_data_internal_pro_v1_t;

typedef struct SLibShmMediaTvuliveDataInternal_ProtocolV20
{
    libshmmedia_tvulivedata_internal_pro_common_t u_common;
    /**
     *  'v' express video type,
     *  'a' express audio type
     *  'h' express head
     */
    uint8_t     u_type;
    uint16_t    u_stream_index; /* BE */
    uint16_t    u_program_index; /* BE */
    uint16_t    u_frame_index; /*BE*/
    uint8_t     u_reserv_2[2];  /* must to be set 0 before using future */
    uint32_t    i_data_len;     /* BE len(p_data) */
    uint32_t    u_data_offset; /*BE*/
#if 0//EXTEND_DATA
{
    programid, u8
}
#endif
    // ... ...
    //char        p_data[0];
}libshmmedia_tvulive_data_internal_pro_v20_t; /* 18Bytes */


typedef struct SLibShmMediaTvuliveDataInternal_ProtocolV21
{
    libshmmedia_tvulive_data_internal_pro_v20_t v20_;

    // V2.1 parrt
    uint16_t    u_reserve;
    uint32_t    u_head_struct_size_; /* BE, can be used to determine which head protocol to use */
    uint64_t    u_frame_timestamp_ms_;

    // ... ...
    //char        p_data[0];
}libshmmedia_tvulive_data_internal_pro_v21_t; /* 32bytes */

typedef struct SLibShmMediaTvuliveDataInternal_ProtocolV22
{
    libshmmedia_tvulive_data_internal_pro_v20_t v20_;

    // V2.1 part
    uint16_t    u_reserve;
    uint32_t    u_head_struct_size_; /* BE, can be used to determine which head protocol to use */
    uint64_t    u_frame_timestamp_ms_;

    // V2.2 part
    uint32_t    u_type_v2_; /* here, it would replace u_type(8bit) */
    // ... ...
    //char        p_data[0];
}libshmmedia_tvulive_data_internal_pro_v22_t; /* 36bytes */

typedef struct SLibShmMediaTvuliveDataInternal_ProtocolV23
{
    libshmmedia_tvulive_data_internal_pro_v20_t v20_;

    // V2.1 part
    uint16_t    u_reserve;
    uint32_t    u_head_struct_size_; /* BE, can be used to determine which head protocol to use */
    uint64_t    u_frame_timestamp_ms_;

    // V2.2 part
    uint32_t    u_type_v2_; /* here, it would replace u_type(8bit) */

    // V2.3 part
    uint32_t    u_gop_poc;/*  31bit whether it exist, 30bit file end flag, 29bit gopend flag */
    // ... ...
    //char        p_data[0];
}libshmmedia_tvulive_data_internal_pro_v23_t; /* 36bytes */

typedef libshmmedia_tvulive_data_internal_pro_v23_t libshmmedia_tvulive_data_internal_pro_v2_t;


/* endif LIBSHM_MEDIA_TYPE_TVULIVE_DATA protocol */

#pragma pack(pop)



class CLibShmmediaTvuliveWrapHandle
{
public:
    CLibShmmediaTvuliveWrapHandle()
    {
        hshm_ = NULL;
        _lastWritingSysTimeUs = 0;
    }

    virtual ~CLibShmmediaTvuliveWrapHandle()
    {
        destroy();
    }
    int create(const char *pshmname, uint32_t header_len
               , uint32_t item_count
               , uint64_t total_size);
    int  open(const char *pshmname);
    void destroy();
    int  write(const libtvumedia_tvulive_data_sections_t *p);
    int  writeWithFrequency1000(const libtvumedia_tvulive_data_sections_t *p);
    int  read(libtvumedia_tvulive_data_t *pInfo);
    uint64_t getWriteIndex();
    uint64_t getReadIndex();
    void seekReadIndex(uint64_t index);
    void seekReadIndexToWriteIndex();
    int searchItems(void *userCtx, libshmmedia_tvulive_wrap_handle_search_items_fn_t m);
public:
    libshm_media_handle_t hshm_;
    std::string shmname_;
private:
    int64_t _lastWritingSysTimeUs;
};

#endif // LIBSHMMEDIA_TVULIVE_PROTOCOL_INTERNAL_H
