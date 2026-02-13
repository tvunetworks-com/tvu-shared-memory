/******************************************************************
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
 *  Description:
 *      libsharememory head files
 *
 *  History:
 *      May 13th, 2015, lotus initialized it
 *
*******************************************************************/

#ifndef _LIBSHM_MEDIA_VIRIABLE_ITEM_H
#define _LIBSHM_MEDIA_VIRIABLE_ITEM_H   
#include "libshm_media_protocol.h"
#include "libshmmedia_common.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
    #define __EXTERN_C_BEGIN extern "C" {
    #define __EXTERN_C_END }
#else
    #define  __EXTERN_C_BEGIN
    #define __EXTERN_C_END
#endif

__EXTERN_C_BEGIN

//#ifdef _LIBSHM_VARIABLE_ITEM_M

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
_LIBSHMMEDIA_DLL_
libshm_media_handle_t LibViShmMediaCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
);

/**
 *  Functionality:
 *      used to create the share memory with specified permission mode, or just open it if the share memory had existed.
 *  Parameter:
 *      @pMemoryName:
 *          share memory entry name
 *      @header_len:
 *          the share memory head size, which would store the media head data.
 *      @item_count:
 *          how many counts of share memory item counts.
 *      @total_size:
 *          total shm size.
 *      @mode:
 *          the permission mode for shm_open (e.g. S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP).
 *  Return:
 *      NULL, open failed. Or return the share memory handle.
 */
_LIBSHMMEDIA_DLL_
libshm_media_handle_t LibViShmMediaCreate2
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
    , mode_t mode
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
_LIBSHMMEDIA_DLL_
libshm_media_handle_t LibViShmMediaOpen(
    const char * pMemoryName
    , libshm_media_readcb_t cb
    , void *opaq
);


/**
 *  Functionality:
 *      destroy share memory handle context.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      void.
 */
_LIBSHMMEDIA_DLL_ 
void LibViShmMediaDestroy(libshm_media_handle_t h);

/**
 *  Functionality:
 *      get the share memory version number.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the share memory version number.
 */
_LIBSHMMEDIA_DLL_ 
uint32_t LibViShmMediaGetVersion(libshm_media_handle_t h);
void LibViShmMediaSetCloseflag(libshm_media_handle_t h, int bclose);
int LibViShmMediaCheckCloseflag(libshm_media_handle_t h);

/**
 *  Functionality:
 *      get the item[index] writing/reading address of share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *      @index:
 *          the item index number.
 *  Return:
 *      the item reading/writing address of the share memory.
 */
//_LIBSHMMEDIA_DLL_ 
//uint8_t *LibViShmMediaGetItemDataAddr(libshm_media_handle_t h, unsigned int index);

/**
 *  Functionality:
 *      get the current writing index of share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the current writing index of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
uint64_t LibViShmMediaGetWriteIndex(libshm_media_handle_t h);

/**
 *  Functionality:
 *      get the current reading index of share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the current reading index of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
uint64_t LibViShmMediaGetReadIndex(libshm_media_handle_t h);

/**
 *  Functionality:
 *      sync the reading index to writing index.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      0 means success.
 */
_LIBSHMMEDIA_DLL_
void LibViShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h);

/**
 *  Functionality:
 *      sync the reading index to 0.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      means success.
 */
_LIBSHMMEDIA_DLL_
void LibViShmMediaSeekReadIndexToZero(libshm_media_handle_t h);

/**
 *  Functionality:.
 *      used to put the reading postion of shm handle to @rindex.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *      @rindex: the postion of wanting to seek.
 *  Return:
 *      void
 */
void LibViShmMediaSeekReadIndex(libshm_media_handle_t h, uint64_t rindex);

/**
 *  Functionality:
 *      used to get the total payload size of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      variabe item did not have item length, so it would reture total paylaod size as LibViShmMediaGetTotalPayloadSize API.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibViShmMediaGetItemLength(libshm_media_handle_t h);
_LIBSHMMEDIA_DLL_
unsigned int LibViShmMediaGetTotalPayloadSize(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the item counts of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the item counts of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibViShmMediaGetItemCounts(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the first item offset of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the first item offset of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibViShmMediaGetHeadLen(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the first item offset of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the first item offset of the share memory.
 */
_LIBSHMMEDIA_DLL_
unsigned int LibViShmMediaGetItemOffset(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get name of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the name of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
const char *LibViShmMediaGetName(libshm_media_handle_t h);

/**
 *  Functionality:
 *      check whether the share memory handle is for writer or reader.
 *  Parameters:
 *      @h[IN]          : share memory handle.
 *  Return:
 *      0   :  not the creator
 *      !0  :  the creator
**/
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaIsCreator(libshm_media_handle_t h);

/**
 *  Functionality:
 *      Poll whether the share memory is sendable.
 *  Parameters:
 *      @h[IN]          : share memory handle.
 *      @timeout[IN]    : how many milli-seconds to poll.
 *  Return:
 *      0   :   not ready
 *      +   :   ready, there is data in.
 *      -   :   I/O error, need to destroy/create the handle again.
 */
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaPollSendable(libshm_media_handle_t h, uint32_t timeout);

/**
 *  Functionality:
 *      Poll whether the share memory is readable, it is to say whether there is data in.
 *  Parameters:
 *      @h[IN]          : share memory handle.
 *      @timeout[IN]    : how many milli-seconds to poll.
 *  Return:
 *      0   :   not ready
 *      +   :   ready, there is data in.
 *      -   :   I/O error, need to destroy/create the handle again.
 */
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaPollReadable(libshm_media_handle_t h, uint32_t timeout);

/**
 *  Functionality:
 *      used to write data to share memory.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmh[IN]    : inputting head information.
 *      @pmi[IN]    : inputting data information.
 *  Reutrn:
 *      0   :   not ready
 *      +   :   Send success, express writing size.
 *      -   :   I/O error
 */
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaSendData(
      libshm_media_handle_t h
    , const libshm_media_head_param_t *pmh
    , const libshm_media_item_param_t *pmi
);

/**
 *  Functionality:
 *      used to write data to share memory, with maximum frequency 1ms per one item.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmh[IN]    : inputting head information.
 *      @pmi[IN]    : inputting data information.
 *  Reutrn:
 *      0   :   not ready
 *      +   :   Send success, express writing size.
 *      -   :   I/O error
 */
_LIBSHMMEDIA_DLL_
int LibViShmMediaSendDataWithFrequency1000(
        libshm_media_handle_t h
      , const libshm_media_head_param_t *pmh
      , const libshm_media_item_param_t *pmi
  );

/**
 *  Functionality:
 *      poll to read out shm media head out, to parse media detail information.
 *  Parameter:
 *      @h : shm library handle.
 *      @pmh : shm head parameter structure.
 *      @timeout : milli-seconds unit, poll's timeout
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 *  History:
 *      May 16th, 2022, LLL hide it, variable shm not support poll read header for it would make reading item stepped,
 *      please use LibViShmMediaPollReadData to replace it.
 *      Sept 15th, 2022, LLL open it again for
 */

_LIBSHMMEDIA_DLL_
int LibViShmMediaPollReadHead(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , uint32_t                    timeout
);

/**
 *  Functionality:
 *      Poll to read out media data from share memory, and put read index step if success.
 *  Parameter:
 *      @pmh : destination head information structure.
 *      @pmi : destination data information structure.
 *      @timeout : milli-seconds unit, poll's timeout
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaPollReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
      , uint32_t                    timeout
);

/**
 *  Functionality:
 *      used to read out data, non-blocking mode, and put read index step if success.
 *  Parameter:
 *      it is special api of LibShmMediaPollReadData while timeout is 0, non-waiting.
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_ 
int LibViShmMediaReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
);


/**
 *  Functionality:
 *      used to read out data, non-blocking mode, and not put read-index step.
 *  Parameter:
 *      it is special api of LibShmMediaPollReadData while timeout is 0, non-waiting.
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_
int LibViShmMediaReadDataWithoutIndexStep(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
);

/**
 *  Functionality:
 *      put read index plus 1 step, it matches with LibShmMediaReadDataWithoutIndexStep API
 *  Parameter:
 *      h  -- the handle.
 *  Return:
 */
_LIBSHMMEDIA_DLL_
void LibViShmMediaReadIndexStep(
    libshm_media_handle_t         h
);


/**
 *  Functionality:
 *      used to search the wanted items.
 *  Parameter:
 *      @h, the handle.
 *      @userCtx, user context.
 *      @fn, item callback.
 *  Return:
 *      0 -- not searched.At this, the reading index would be just on the writing index.
 *      1 -- searched.At ths, the reading index would be just on the searched index.
**/
_LIBSHMMEDIA_DLL_
int  LibViShmMediaSearchItems(
      libshm_media_handle_t        h
      , void *userCtx
      , libshmmedia_item_checking_fn_t      fn
);

/**
 *  Functionality:
 *      destroy the existing share memory.
 *  Return:
 *      0   : destroy successfully.
 *      <0  : failed.
**/
_LIBSHMMEDIA_DLL_
int
LibViShmMediaRemoveShmFromSystem(const char * pMemoryName);

/**
 *  Functionality:
 *      used to get the item address points for user to write at its layer.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @nlen[IN]    : apply buffer length.
 *  Reutrn:
 *      NULL   :   failed
 *      others   :  buffer point
 */
_LIBSHMMEDIA_DLL_
uint8_t* LibViShmMediaItemApplyBuffer(
      libshm_media_handle_t h
    , unsigned int  nlen
);


/**
 *  Functionality:
 *      used to commit the buffer change.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pItemAddr[IN]  : item address point.
 *      @nlen[IN]    : commit buffer length.
 *  Reutrn:
 *      0   :   success
 *      <0   :  failed, not supported.
 */
_LIBSHMMEDIA_DLL_
int LibViShmMediaItemCommitBuffer(
    libshm_media_handle_t h,
    uint8_t *pItemAddr,
    unsigned int nlen
);


/**
 *  Functionality:
 *      used to commit the buffer change.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pItemAddr[IN]  : item address point.
 *      @nlen[IN]    : commit buffer length.
 *  Reutrn:
 *      0   :   not writable, need to wait
 *      <0  :   failed, not supported.
 *      >0  :   success that the written bytes to shm.
 */
int LibViShmMediaItemWriteBuffer(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr);

int LibViShmMediaItemWriteBufferIgnoreInternalCopy(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr);

/**
 *  Functionality:
 *      used to get buffer layout status for reading.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmi[OUT]    : item different length status.
 *      @pItemAddr[IN]  : item address point.
 *  Reutrn:
 *      >0   : reading the length
 *      =0   : possible priviledge issue.
 *      <0   : errors, as invalid handle
 */
_LIBSHMMEDIA_DLL_
int  LibViShmMediaItemPreGetReadBufferLayout(
      libshm_media_handle_t h
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , const uint8_t *pItemAddr
);

/**
 *  Functionality:
 *      used to get buffer layout status for user write its data directly.
 *  Parameters:
 *      @h[IN]      : share memory handle.
 *      @pmi[IN]    : item different length status.
 *      @pItemAddr[IN]  : item address point.
 *      @pLayout[OUT]   : return the layout status to the this struct.
 *  Reutrn:
 *      >0   :  possible pre-written length
 *      =0   :  possible privilege issue, or not support.
 *      <0   :  failed, possible handle invalid.
 */
_LIBSHMMEDIA_DLL_
int  LibViShmMediaItemPreGetWriteBufferLayout(
      libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , uint8_t *pItemAddr
    , libshm_media_item_addr_layout_t *pLayout
);


//#endif

__EXTERN_C_END

#endif
