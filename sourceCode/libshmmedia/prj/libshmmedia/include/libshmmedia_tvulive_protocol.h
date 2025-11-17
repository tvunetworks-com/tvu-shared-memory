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
 *      used to store libshmmedia_tvulive data protocol
 * Author:
 *      Lotus/TVU
 * History:
 *      Lotus Initialized it on April 25th 2022.
*************************************************************************************/

#ifndef LIBSHMMEDIA_TVULIVE_PROTOCOL_H
#define LIBSHMMEDIA_TVULIVE_PROTOCOL_H

#include <stdint.h>
#include "libtvu_media_fourcc.h"
#include "libshm_media_protocol.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_TVULIVE_PRO_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_TVULIVE_PRO_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_TVULIVE_PRO_DLL_
#endif

#else
#define _LIBSHMMEDIA_TVULIVE_PRO_DLL_
#endif
/*****************************************
 *  for extended data
 *  LIBSHM_MEDIA_TYPE_TVULIVE_DATA_V2
*****************************************/
#pragma pack(push, 1)

typedef struct SLibShmMediaTvuliveDataProtocolCommon
{
    uint8_t u_version;
}libshmmedia_tvulivedata_pro_common_t;

typedef struct SLibShmMediaTvuliveData_ProtocolV1
{
    libshmmedia_tvulivedata_pro_common_t u_common;
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
    uint32_t    i_len;
    //char        p_data[0];
}libshmmedia_tvulivedata_pro_v1_t;

#pragma pack(pop)


/* if LIBSHM_MEDIA_TYPE_TVULIVE_DATA structure */
enum ELibShmMediaTvuliveDataProtocol
{
    kLibshmMediaTvuliveDataProV1 = 1,
    kLibshmMediaTvuliveDataProV2 = 2,
};

/*******************************************************
 *
 * kLibShmMediaDataTypeRawAudioVolumeValue
 * All integer is Little Endian.
 * {
 *     "volume type"=>8bit, 00-unknow, 01-unsigned 8bit volume value, 02-signed 16bit volume value, , 03-24bit volume value, 04-32bit volume value, 0x05-percentage value
 *     "reserve" => 24bit
 *     32bit counts, LE
 *     [
 *         volume , 32 bit LE.
 *     ]
 * }
*******************************************************/

enum ELibShmMediaTvuliveDataType
{
    kLibShmMediaTvuliveDataType_VIDEO = 'v',
    kLibShmMediaDataTypeTvuliveVideo = kLibShmMediaTvuliveDataType_VIDEO,
    kLibShmMediaTvuliveDataType_AUDIO = 'a',
    kLibShmMediaDataTypeTvuliveAudio = kLibShmMediaTvuliveDataType_AUDIO,
    kLibShmMediaTvuliveDataType_HEADER = 'h',
    kLibShmMediaDataTypeTvuliveHead = kLibShmMediaTvuliveDataType_HEADER,
    kLibShmMediaDataTypeTvuliveMetadata = 'd',
    kLibShmMediaDataTypeTvuliveSubtile  = 's',
    kLibShmMediaDataTypeRawJpeg =  _TVU_LE_FOURCCTAG(0x00, 'j', 'p', 'g'), // to store jpeg image data
    kLibShmMediaDataTypeRawAudioVolumeValue = _TVU_LE_FOURCCTAG(0x00, 'a', 'v', 'v'), //to store audio volume value.
};

#define kLibTvuMediaTvuliveDataTypeVideo    kLibShmMediaTvuliveDataType_VIDEO
#define kLibTvuMediaTvuliveDataTypeAudio    kLibShmMediaTvuliveDataType_AUDIO
#define kLibTvuMediaTvuliveDataTypeHeader    kLibShmMediaTvuliveDataType_HEADER

typedef struct SLibShmMediaTvuliveInfo
{
    /**
     *  'v' express video type,
     *  'a' express audio type,
     *  'd' express meta data type,
     *  'h' express head,
     *  as ELibShmMediaTvuliveDataType definition.
    **/
    uint32_t        i_type;
    uint16_t        u_stream_index; /* the 16th bit is whether IDR frame, the other 15bits are stream index */
    uint16_t        u_program_index;
    uint16_t        u_frame_index;
    uint64_t        u_frame_timestamp_ms;
}libtvumedia_tvulive_info_t;

typedef struct SLibShmMediaTvuliveInfoExt2
{
    uint32_t        u_gop_poc;/* 31bit whether it exist, 30bit file end flag, 29bit gopend flag */
}libtvumedia_tvulive_info_ext2_t;

typedef libtvumedia_tvulive_info_ext2_t libtvumedia_tvulive_info_ext_t;

typedef struct SLibShmMediaTvuliveInfo2
{
    /**
     *  'v' express video type,
     *  'a' express audio type,
     *  'd' express meta data type,
     *  'h' express head,
     *  as ELibShmMediaTvuliveDataType definition.
    **/
    uint32_t        i_type;
    uint16_t        u_stream_index; /* the 16th bit is whether IDR frame, the other 15bits are stream index */
    uint16_t        u_program_index;
    uint16_t        u_frame_index;
    uint64_t        u_frame_timestamp_ms;
    libtvumedia_tvulive_info_ext2_t ext;
}libtvumedia_tvulive_info2_t;

typedef struct SLibShmMediaTvuliveDataV1
{
    libtvumedia_tvulive_info_t o_info;
    uint64_t        u_createTime;
    uint32_t        i_data;
    const uint8_t   *p_data; /* 1. for wirter, it is caller's buffer, 2. for reader, it is buffer point to shm's tvulive data */
}libtvumedia_tvulive_data_v1_t;

typedef struct SLibShmMediaTvuliveDataV2
{
    size_t          u_struct_size;
    uint64_t        u_createTime;
    uint32_t        i_data;
    const uint8_t   *p_data; /* 1. for wirter, it is caller's buffer, 2. for reader, it is buffer point to shm's tvulive data */
    libtvumedia_tvulive_info2_t o_info;
}libtvumedia_tvulive_data_v2_t;

typedef libtvumedia_tvulive_data_v2_t libtvumedia_tvulive_data_t;

typedef struct SLibShmMediaTvuliveSectionPair
{
    uint32_t    i_section;
    const uint8_t     *p_section;
}libtvumedia_tvulive_section_pair_t;

typedef struct SLibShmMediaTvuliveDataSectionsV1
{
    libtvumedia_tvulive_info_t o_info;
    uint64_t        u_createTime;
    uint32_t        u_sum_section_size; /* it has better to provide it, or the API would sum all section's length */
    uint32_t        u_section_counts;
    const libtvumedia_tvulive_section_pair_t   *p_sections;
}libtvumedia_tvulive_data_sections_v1_t;

typedef struct SLibShmMediaTvuliveDataSectionsV2
{
    size_t          u_struct_size;
    uint64_t        u_createTime;
    uint32_t        u_sum_section_size; /* it has better to provide it, or the API would sum all section's length */
    uint32_t        u_section_counts;
    const libtvumedia_tvulive_section_pair_t   *p_sections;
    libtvumedia_tvulive_info2_t o_info;
}libtvumedia_tvulive_data_sections_v2_t;

typedef  libtvumedia_tvulive_data_sections_v2_t libtvumedia_tvulive_data_sections_t;

/* endif LIBSHM_MEDIA_TYPE_TVULIVE_DATA structure */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Functionality:
 *      used to pre-get the buffer size for storing frames at default version, default is V2 now.
 *  Parameter:
 *      @data_len   : required data length.
 *  Return:
 *      alway to be buffer minmum size.
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
uint32_t LibTvuMediaTvulivePreferBufferSize(uint32_t data_len);

/**
 *  Functionality:
 *      used to pre-get the head size for storing protocol header at default version, default is V2 now.
 *  Parameter:
 *  Return:
 *      alway to be buffer minmum size.
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
uint32_t LibTvuMediaTvulivePreferHeadSize();

/**
 *  Functionality:
 *      write V2 tvulive data from @pinfo to @dest_buffer, default is V2 now.
 *  Parameter:
 *      @pinfo, tvulive data ptr.
 *      @dest_buffer, destination buffer which used to store tvulive protocol head and data.
 *      @dest_buffer_size, destination buffer size.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length.
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibTvuMediaTvuliveWriteData(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *dest_buffer, /*IN*/uint32_t dest_buffer_size);

/**
 *  Functionality:
 *      write tvulive protocol head from @pinfo to @dest_buffer, default is V2 protocol.
 *  Parameter:
 *      @pinfo, tvulive data ptr.
 *      @dest_buffer, destination buffer which used to store tvulive protocol head.
 *      @dest_buffer_size, destination buffer size.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length.
**/
int LibTvuMediaTvuliveWriteHead(/*IN*/const libtvumedia_tvulive_data_t *pinfo, /*OUT*/uint8_t *buffer, /*IN*/uint32_t buffer_size);

/**
 *  Functionality:
 *      parse out tvulive data from @src_buffer.
 *  Parameter:
 *      @pinfo, encoding data ptr.
 *      @src_buffer, src buffer ptr.
 *      @src_buffer_len, src buffer length.
 *  Return:
 *      < 0 : failed
 *      ==0 : read 0 bytes, invalide
 *      > 0 : read out buffer len
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibTvuMediaTvuliveReadData(/*OUT*/libtvumedia_tvulive_data_t *pinfo, /*OUT*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len);
/* endif LIBSHM_MEDIA_TYPE_TVULIVE_DATA apis */

typedef void *libshmmedia_tvulive_wrap_handle_t;
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
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
libshmmedia_tvulive_wrap_handle_t LibShmMediaTvuliveWrapHandleCreate
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
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
libshmmedia_tvulive_wrap_handle_t LibShmMediaTvuliveWrapHandleOpen
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
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
void LibShmMediaTvuliveWrapHandleDestroy
(
    libshmmedia_tvulive_wrap_handle_t h
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
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibShmMediaTvuliveWrapHandleWrite(/*IN*/const libshmmedia_tvulive_wrap_handle_t h, const libtvumedia_tvulive_data_sections_t *p);

/**
 *  Functionality:
 *      write @pcmd data to viriable shm, but with flow control, 1000 counts per 1 second.
 *  History:
 *      Why need this? For writer could flush the buffer at 1 millisecond, to cause shm buffer overflow.
 *  Parameter:
 *      @h , handle
 *      @pcmd, control command point.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length, equal control data length.
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibShmMediaTvuliveWrapHandleWriteWithFrequency1000(/*IN*/const libshmmedia_tvulive_wrap_handle_t h, const libtvumedia_tvulive_data_sections_t *p);

/**
 *  Functionality:
 *      used to get the write index of handle.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the writing index value.
 */
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
uint64_t LibShmMediaTvuliveWrapHandleGetWriteIndex(libshmmedia_tvulive_wrap_handle_t h);

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
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibShmMediaTvuliveWrapHandleRead(/*IN*/const libshmmedia_tvulive_wrap_handle_t h
                                 , /*OUT*/libtvumedia_tvulive_data_t *pInfo);

/**
 *  Functionality:
 *      used to get the read index of handle.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the reading index value.
 */
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
uint64_t LibShmMediaTvuliveWrapHandleGetReadIndex(libshmmedia_tvulive_wrap_handle_t h);

/**
 *  Functionality:
 *      sync the reading index to the @index pos.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *      @index: it would be seek for reading.
 *  Return:
 */
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
void LibShmMediaTvuliveWrapHandleSeekReadIndex(libshmmedia_tvulive_wrap_handle_t h, uint64_t index);

/**
 *  Functionality:
 *      sync the reading index to writing index.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 */
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
void LibShmMediaTvuliveWrapHandleSeekReadIndexToWriteIndex(libshmmedia_tvulive_wrap_handle_t h);


/**
 * Return:
 *  0--not searched, 1--searched
**/
typedef int (*libshmmedia_tvulive_wrap_handle_search_items_fn_t)(void *user, const libtvumedia_tvulive_data_t *);
/**
 *  Functionality:
 *      used to search shm's items.
 *  Parameter:
 *      @h , handle
 *      @userCtx, user context.
 *      @fn, item checking callback.
 *  Return:
 *      0 -- not searched.At this, the reading index would be just on the writing index.
 *      1 -- searched.At ths, the reading index would be just on the searched index.
**/
_LIBSHMMEDIA_TVULIVE_PRO_DLL_
int LibShmMediaTvuliveWrapHandleSearchItems(/*IN*/const libshmmedia_tvulive_wrap_handle_t h
                                 , /*IN*/void *userCtx, /*OUT*/libshmmedia_tvulive_wrap_handle_search_items_fn_t fn);
#ifdef __cplusplus
}
#endif
#endif // LIBSHMMEDIA_TVULIVE_PROTOCOL_H
