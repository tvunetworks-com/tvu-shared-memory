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
#ifndef LibViShmMedia_VARIABLEITEM_RAWDATA_H
#define LibViShmMedia_VARIABLEITEM_RAWDATA_H

#include "libshmmedia.h"

#ifdef __cplusplus
    #define __EXTERN_C_BEGIN extern "C" {
    #define __EXTERN_C_END }
#else
    #define  __EXTERN_C_BEGIN
    #define __EXTERN_C_END
#endif

__EXTERN_C_BEGIN

_LIBSHMMEDIA_DLL_
int LibViShmMediaRawDataWrite(
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
uint8_t  *LibViShmMediaRawDataApply(
      libshm_media_handle_t h
    , size_t len
);

/**
 *  Functionality:
 *      used to commit to write data.
 *  Parameters:
 *      @h      : media handle
 *      @commit_len    : It was not used at the current time.
 *  Return:
 *      >0:the length of committed data.
 *      <=0:failed commit
**/
_LIBSHMMEDIA_DLL_
int LibViShmMediaRawDataCommit(
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
int LibViShmMediaRawHeadWrite(
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
int LibViShmMediaRawDataRead(
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
int LibViShmMediaHasReader(libshm_media_handle_t h, unsigned int timeout = 100/*milli-seconds*/);

__EXTERN_C_END

#endif // LibViShmMedia_VARIABLEITEM_RAWDATA_H
