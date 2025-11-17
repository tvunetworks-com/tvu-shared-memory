/******************************************************
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
 *********************************************************
 *  File:
 *      shmhead.h
 *  CopyRight:
 *      tvu
 *  Description:
 *      Share Memory structure
 *  History:
 *      In 2012, Eros Lin first initialize it.
 *      In April, 2014, Lotus, adding checking flag to 
 *  solve dynamic share memory problem, version from 1 upgrade to 2.
******************************************************/
#ifndef SHMHEAD_H__
#define SHMHEAD_H__

#include <stdint.h>

#define _SHM_HEAD_FEATURE_EXT_EABLE  1

#pragma pack(push, 1)
#define INVALID_SHM_HEAD_VERSION                0
#define CHECK_INVALID_SHM_HEAD_VERSION(ver)     (ver == INVALID_SHM_HEAD_VERSION)
#define MEMHEADER_V1    1
/**
 *  lotus adding.
**/
#define MEMHEADER_V2    2
#define MEMHEADER_V3    3
#define MEMHEADER_V4    4

#define MEMHEADER_V5    5 /* Add it at Jan 26th, 2021, for future use. At this, we need start to use shm_construct_5_t */

//#if defined(TVU_WINDOWS)
#define MEMHEADER_CURRENT_VERSION   MEMHEADER_V4

#define MEMHEADER_VARIABLE_ITEM_SHM_CURRENT_VERSION   MEMHEADER_V4
//#else
//#define MEMHEADER_CURRENT_VERSION   MEMHEADER_V2
//#endif

/*
the correct position should be 
pHeader + item_offset + (item_current % item_count)*item_length
*/
typedef struct {
    unsigned int version;
    unsigned int item_count;//total count of items
    unsigned int item_length;//each item's length
    unsigned int item_current;//the item number of current write, 0 to 4g,
    unsigned int item_offset;//first item offset bytes of the entire share memory
}MEM_HEADER_V1, shm_construct_t;

#define SHM_MEDIA_HEAD_INFO_V4_OFFSET   96 /* sizeof(shm_media_head_info_v4_t) */

#define SHM_CONSTRUCT_EXT_LEN   sizeof(shm_construct_ext_t)

enum {
    kShmConstructExtVer0 = 0,
    kShmConstructExtVer1 = 1,
};
#define SHM_CONSTRUCT_EXT_VER   kShmConstructExtVer1

typedef struct {
    //shm_construct_t item;
    uint8_t ext_ver;
    uint8_t reserve;
    uint16_t ext_len;
    uint8_t  a_reserve_[4];
    uint64_t item_current_64;//item_current 64bit,
    uint64_t last_read_time_stamp;
} shm_construct_ext_t;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

unsigned int libshmhead_construct_get_version(const shm_construct_t *pshm);

#ifdef __cplusplus
}
#endif

#endif
