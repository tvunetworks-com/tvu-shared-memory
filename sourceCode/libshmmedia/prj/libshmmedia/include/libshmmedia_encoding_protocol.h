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
 *      used to store libshmmedia encoding data protocol
 * Author:
 *      Lotus/TVU
 * History:
 *      Lotus Initialized it on April 25th 2022.
*************************************************************************************/
#ifndef LIBSHMMEDIA_ENCODING_DATA_PROTOCOL_H
#define LIBSHMMEDIA_ENCODING_DATA_PROTOCOL_H

#include <stdint.h>

#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
#endif

#else
#define _LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
#endif

/* if LIBSHM_MEDIA_TYPE_ENCODING_DATA structure */
enum ELibShmMediaEncodingDataProtocol
{
    kLibshmMediaEncodingDataProV1 = 1,
};

typedef struct SLibShmMediaEncodingData
{
    /**
     *  u_codec_tag definition is at enum ETvuCodecTagFourCC of tvu_fourcc.h
     */
    uint32_t    u_codec_tag;
    uint32_t    u_stream_index; /* the 32th bit is whether IDR frame, others 31bit are stream index */
    uint16_t    u_frame_index;
    int64_t     i64_pts;
    int64_t     i64_dts;
    uint32_t    i_data;     /* len(p_data) */
    const uint8_t *p_data;
    //  .......
    //  char        p_data[0];
}libshmmedia_encoding_data_t, libtvumedia_encoding_data_t;
/* endif LIBSHM_MEDIA_TYPE_ENCODING_DATA structure */

#ifdef __cplusplus
extern "C" {
#endif

/* if LIBSHM_MEDIA_TYPE_ENCODING_DATA apis */
/**
 *  Functionality:
 *      used to pre-get the buffer size for storing frames.
 *  Parameter:
 *      @data_len, original data length.
 *  Return:
 *      alway to be buffer minmum size.
**/
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
uint32_t LibShmmediaEncodingDataGetBufferSize(uint32_t data_len);
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
uint32_t LibTvuMediaEncodingDataGetBufferSize(uint32_t data_len);

/**
 *  Functionality:
 *      write @pinfo data to @dest_buffer.
 *  Parameter:
 *      @pinfo, encoding data ptr.
 *      @dest_buffer, destination buffer which used to store encoding data.
 *      @dest_buffer_size, destination buffer size.
 *  Return:
 *      < 0 : failed
 *      ==0 : write 0 bytes, invalid
 *      > 0 : write buffer length.
**/
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
int LibShmmediaEncodingDataWrite(/*IN*/const libshmmedia_encoding_data_t *pinfo, /*OUT*/uint8_t *dest_buffer, /*IN*/uint32_t dest_buffer_size);
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
int LibTvuMediaEncodingDataWrite(/*IN*/const libshmmedia_encoding_data_t *pinfo, /*OUT*/uint8_t *dest_buffer, /*IN*/uint32_t dest_buffer_size);

/**
 *  Functionality:
 *      parse out encoding data from @src_buffer.
 *  Parameter:
 *      @src_buffer  : shm's user data point
 *      @src_buffer_len : shm's user data len
 *  Return:
 *      < 0 : failed
 *      ==0 : read 0 bytes, invalide
 *      > 0 : read out buffer len
**/
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
int LibShmmediaEncodingDataRead(/*OUT*/libshmmedia_encoding_data_t *pinfo, /*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len);
_LIBSHMMEDIA_ENCODING_DATA_PRO_DLL_
int LibTvuMediaEncodingDataRead(/*OUT*/libshmmedia_encoding_data_t *pinfo, /*IN*/ const uint8_t *src_buffer, /*IN*/const uint32_t src_buffer_len);

/* endif LIBSHM_MEDIA_TYPE_ENCODING_DATA apis */
#ifdef __cplusplus
}
#endif

#endif // LIBSHMMEDIA_ENCODING_DATA_PROTOCOL_H
