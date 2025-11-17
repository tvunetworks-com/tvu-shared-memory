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
#ifndef LIBSHM_MEDIA_RAW_DATA_OPT_H
#define LIBSHM_MEDIA_RAW_DATA_OPT_H

#include "libshm_media.h"

__EXTERN_C_BEGIN
/* if LIBSHM_MEDIA_RAW_DATA_OPT apis */

_LIBSHMMEDIA_DLL_
int LibShmMediaRawDataWrite(
      libshm_media_handle_t h
    , const libshmmedia_raw_data_param_t *pmi
);

/**
 *  Functionality:
 *      used to preserver one buffer for storing data.
 *  Parameters:
 *      @h      : media handle
 *      @len    : the required length
 *  Return:
 *      the ptr of applied buffer, NULL means failed.
**/
_LIBSHMMEDIA_DLL_
uint8_t  *LibShmMediaRawDataApply(
      libshm_media_handle_t h
    , size_t len
);

/**
 *  Functionality:
 *      used to commit to write data.
 *  Parameters:
 *      @h      : media handle
 *      @comit_buf  : raw data buffer
 *      @commit_len    : It was not used at the current time.
 *  Return:
 *      the length of committed data.
**/
_LIBSHMMEDIA_DLL_
int LibShmMediaRawDataCommit(
      libshm_media_handle_t h
    , const uint8_t *comit_buf
    , size_t comit_len
);

/**
 *  Functionality:
 *      used to write head data, it was not useful at the current time.
 *  Parameters:
 *      @h      : media handle
 *      @pmh    : head data ptr.
 *  Return:
 *      the length of @pmh head data.
**/
_LIBSHMMEDIA_DLL_
int LibShmMediaRawHeadWrite(
      libshm_media_handle_t h
    , const libshmmedia_raw_head_param_t *pmh
);

/**
 *  Functionality:
 *      used to read out raw data.
 *  Parameters:
 *      @h      : media handle
 *      @pmh    : head data ptr, which used to store hea buffer ptr
 *      @pmi    : data ptr, which used to store data prt.
 *  Return:
 *      > 0, the length of data.
 *      < 0, error, need to destory @h, and re-create it.
 *      = 0, need to wait.
**/
_LIBSHMMEDIA_DLL_
int LibShmMediaRawDataRead(
      libshm_media_handle_t         h
      , libshmmedia_raw_head_param_t   *pmh /* if head chage, would be evalued */
      , libshmmedia_raw_data_param_t   *pmi
      , unsigned int timeout /* milli-seconds */
);


/**
 *  Functionality:
 *      used to read out raw data.
 *  Parameters:
 *      @h      : media handle
 *      @timeout  : the time threshold to determine whether there is reading action.
 *  Return:
 *      0 -- no reader
 *      others -- has reader
**/
_LIBSHMMEDIA_DLL_
int LibShmMediaHasReader(libshm_media_handle_t h, unsigned int timeout = 100/*milli-seconds*/);

/**
 *  Functionality:
 *      used to get the item address points for user to write at its layer,
 *  same to LibShmMediaRawDataApply.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmi[IN]    : request item status struct.
 *      @pLayout[OUT]    : item layout status for caller writing directly.
 *  Reutrn:
 *      >0   :   the real writing total length
 *      <=0   :  no memory for writing, or not support...
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaItemApplyBuffer(
    libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , libshm_media_item_addr_layout_t *pLayout
);

/**
 *  Functionality:
 *      used to commit the buffer change, same to LibShmMediaRawDataCommit
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @nlen[IN]    : commit buffer length.
 *  Reutrn:
 *      >0   :   the real committed total length.
 *      <=0  :  failed, not supported.
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaItemCommitBuffer(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
);

/**
 *  Functionality:
 *      used to get buffer layout status for user write its data directly.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmi[IN]    : item different length status.
 *      @pItemAddr[IN]  : item address point.
 *      @itemLen[IN]    : item buffer size.
 *      @pLayout[OUT]   : return the layout status to the this struct.
 *  Reutrn:
 *      >0   :  possible pre-written length,
 *              to make sure pItemAddr size was larger than this value before write the buffer.
 *      <=0   :  possible privilege issue, or not support.
 *      <0   :  failed, possible handle invalid.
 */
_LIBSHMMEDIA_DLL_
int  LibShmMediaItemGetWriteBufferLayout(
      libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , uint8_t *pItemAddr
    , uint32_t itemLen
    , libshm_media_item_addr_layout_t *pLayout
);

/**
 *  Functionality:
 *      used to write item head only, but not write data.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pItemAddr[IN]  : item address point.
 *      @nlen[IN]    : commit buffer length.
 *  Reutrn:
 *      0   :   not writable, need to wait
 *      <0  :   failed, not supported.
 *      >0  :   success that the written bytes to shm.
 */
int LibShmMediaItemWriteBuffer(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr);
/* endif LIBSHM_MEDIA_RAW_DATA_OPT apis */

__EXTERN_C_END


#endif // LIBSHM_MEDIA_RAW_DATA_OPT_H
