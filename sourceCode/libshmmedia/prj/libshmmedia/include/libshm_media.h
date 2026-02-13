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

#ifndef _LIBSHM_MEDIA_H
#define _LIBSHM_MEDIA_H   

#include "libshm_media_protocol.h"
#include "libshmmedia_common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

__EXTERN_C_BEGIN

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
 *      @item_length:
 *          every item size.
 *  Return:
 *      NULL, open failed. Or return the share memory handle.      
 */
_LIBSHMMEDIA_DLL_
libshm_media_handle_t LibShmMediaCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint32_t item_length
);

/**
 *  Functionality:
 *      used to create the share memory, or just open it if the share memory had existed.
 *      Same as LibShmMediaCreate but allows specifying POSIX shared-memory permission bits.
 *  Parameter:
 *      @pMemoryName:
 *          share memory entry name
 *      @header_len:
 *          the share memory head size, which would store the media head data.
 *      @item_count:
 *          how many counts of share memory item counts.
 *      @item_length:
 *          every item size.
 *      @mode:
 *          permission bits passed to shm_open (e.g. S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP).
 *  Return:
 *      NULL, open failed. Or return the share memory handle.
 */
_LIBSHMMEDIA_DLL_
libshm_media_handle_t LibShmMediaCreate2
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint32_t item_length
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
libshm_media_handle_t LibShmMediaOpen(
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
void LibShmMediaDestroy(libshm_media_handle_t h);

/**
 *  Functionality:
 *      LibShmMediaGetVersion/LibShmMediaGetHeadVersion are both used get the share memory head version number.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the share memory version number.
 */
_LIBSHMMEDIA_DLL_ 
uint32_t LibShmMediaGetVersion(libshm_media_handle_t h);
uint32_t LibShmMediaGetHeadVersion(libshm_media_handle_t h);
//void LibShmMediaSetCloseflag(libshm_media_handle_t h, int bclose);
//int LibShmMediaCheckCloseflag(libshm_media_handle_t h);

/**
 *  Functionality:
 *      get the item[index] writing/reading address of share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *      @index:
 *          the item index number.
 *  Warning:
 *      this would return the item's first data address, be careful to use it when the shm having video/audio mixing.
 *  Return:
 *      the item reading/writing address of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
uint8_t *LibShmMediaGetItemDataAddr(libshm_media_handle_t h, unsigned int index);

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
unsigned int LibShmMediaGetWriteIndex(libshm_media_handle_t h);

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
unsigned int LibShmMediaGetReadIndex(libshm_media_handle_t h);

/**
 *  Functionality:
 *      sync the reading index to writing index.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the synced reading index of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h);


/**
 *  Functionality:
 *      sync the reading index to assiged pos.
 *      used for reading caller.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *      @idx:
 *          seeking's reading index.
 *  Return:
 *      the synced reading index of the share memory.
 */
_LIBSHMMEDIA_DLL_
unsigned int LibShmMediaSeekReadIndex(libshm_media_handle_t h, uint32_t idx);


/**
 *  Functionality:
 *      sync the reading index to the ring start related to writing index.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the synced reading index of shm.
 */
_LIBSHMMEDIA_DLL_
unsigned int LibShmMediaSeekReadIndexToRingStart(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the head address point of the share memory.
 *  Parameter:
 *      @h:share memory handle.
 *  Return:
 *      the head address point of the share memory.
 */
_LIBSHMMEDIA_DLL_
const uint8_t * LibShmMediaGeHeadAddr(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the head version of the share memory.
 *  Parameter:
 *      @h:share memory handle.
 *  Return:
 *      the head version of the share memory.
 */
_LIBSHMMEDIA_DLL_
uint32_t LibShmMediaGeHeadVersion(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the head length of the share memory.
 *  Parameter:
 *      @h:share memory handle.
 *  Return:
 *      the head length of the share memory.
 */
_LIBSHMMEDIA_DLL_
unsigned int LibShmMediaGeHeadLength(libshm_media_handle_t h);

/**
 *  Functionality:
 *      used to get the item length of the share memory.
 *  Parameter:
 *      @h:
 *          share memory handle.
 *  Return:
 *      the item length of the share memory.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibShmMediaGetItemLength(libshm_media_handle_t h);

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
unsigned int LibShmMediaGetItemCounts(libshm_media_handle_t h);

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
unsigned int LibShmMediaGetItemOffset(libshm_media_handle_t h);

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
const char *LibShmMediaGetName(libshm_media_handle_t h);

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
int LibShmMediaIsCreator(libshm_media_handle_t h);

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
int LibShmMediaPollSendable(libshm_media_handle_t h, unsigned int timeout);

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
int LibShmMediaPollReadable(libshm_media_handle_t h, unsigned int timeout);

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
int LibShmMediaSendData(
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
int LibShmMediaSendDataWithFrequency1000(
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
 */
_LIBSHMMEDIA_DLL_ 
int LibShmMediaPollReadHead(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , unsigned int                timeout
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
int LibShmMediaPollReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
      , unsigned int                timeout
);

/**
 *  Functionality:
 *      Poll to read out media data from share memory, and put read index step if success.
 *  Parameter:
 *      @pmh : destination head information structure.
 *      @pmi : destination data information structure.
 *      @pext : destination extension data information structure.
 *      @timeout : milli-seconds unit, poll's timeout
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaPollReadDataV2(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
    , unsigned int                timeout
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
int LibShmMediaReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
);

/**
 *  Functionality:
 *      used to read out data, non-blocking mode, and put read index step if success.
 *  Parameter:
 *      it is special api of LibShmMediaPollReadData2 while timeout is 0, non-waiting.
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaReadDataV2(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
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
int LibShmMediaReadDataWithoutIndexStep(
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
void LibShmMediaReadIndexStep(
    libshm_media_handle_t         h
);

/**
 *  Functionality:
 *      to set shm read handle's reading index.
 *  Parameter:
 *      h[IN]       :   handle
 *      type[IN]    :
 *          'v' , according video pts to find
 *          'a' , according audio pts to find
 *          's' , according subtitle pts to find
 *      pts[IN]     :   wanted pts which was the value of the item current.
 *  Return:
 *      return the value of reading index of h.
 */
_LIBSHMMEDIA_DLL_ 
unsigned int LibShmMediaSetReadIndex(libshm_media_handle_t  h, char type, int64_t pts);

/**
 *  Functionality:
 *      to search out the matching item to the tvutimestamp.
 *  Parameter:
 *      h[IN]       :   handle
 *      tvutimestamp[IN]     :   tvutimestamp value from user setting.
 *      pmi[OUT]    : the item point for storing the item data.
 *  Return:
 *      false: did not get the matching item
 *      true:get the matching item, and the value would be saved to @pmi.
 */
_LIBSHMMEDIA_DLL_
bool LibShmMediaSearchItemWithTvutimestamp(libshm_media_handle_t  h, uint64_t tvutimestamp, libshm_media_item_param_t *pmi);

/**
 *  Functionality:
 *      to search out the matching item to the tvutimestamp.
 *  Parameter:
 *      h[IN]       :   handle
 *      tvutimestamp[IN]     :   tvutimestamp value from user setting.
 *      pmi[OUT]    : the item point for storing the item data.
 *  Return:
 *      false: did not get the matching item
 *      true:get the matching item, and the value would be saved to @pmi.
 */
_LIBSHMMEDIA_DLL_
bool LibShmMediaSearchItemWithTvutimestampV2(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , libshmmedia_extend_data_info_t *pext
    );

/**
 *  Functionality:
 *      to read out the matching item to the tvutimestamp.
 *  Parameter:
 *      h[IN]       :   handle
 *      tvutimestamp[IN]     :   tvutimestamp value from user setting.
  *      type[IN]    :
 *          'v' , according video pts to find
 *          'a' , according audio pts to find
 *          's' , according subtitle pts to find
  *         'd' , according user data timestamp to find
 *      pts [IN]    : the frame's pts.
 *      bFoundTvutimestamp[OUT] : whether to find the item of matching the tvutimestamp
 *      bFoundPts[OUT] : whether to find the item of matching the pts
 *      pmh[OUT]    : the media head point for storing the item data.
 *      pmi[OUT]    : the media item point for storing the item data.
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaReadItemWithTvutimestamp(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , char type
    , uint64_t pts
    , bool *bFoundTvutimestamp
    , bool *bFoundPts
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    );

/**
 *  Functionality:
 *      to read out the matching item to the tvutimestamp.
 *  Parameter:
 *      h[IN]       :   handle
 *      tvutimestamp[IN]     :   tvutimestamp value from user setting.
  *      type[IN]    :
 *          'v' , according video pts to find
 *          'a' , according audio pts to find
 *          's' , according subtitle pts to find
  *         'd' , according user data timestamp to find
 *      pts [IN]    : the frame's pts.
 *      bFoundTvutimestamp[OUT] : whether to find the item of matching the tvutimestamp
 *      bFoundPts[OUT] : whether to find the item of matching the pts
 *      pmh[OUT]    : the media head point for storing the item head.
 *      pmi[OUT]    : the media item point for storing the item data.
 *      pext[OUT]   : the media structure for storing the extension data.
 *  Return:
 *      0   -- means to wait & try again
 *      <0  -- means failure
 *      >0  -- means success
 */
_LIBSHMMEDIA_DLL_
int LibShmMediaReadItemWithTvutimestampV2(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , char type
    , uint64_t pts
    , bool *bFoundTvutimestamp
    , bool *bFoundPts
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , libshmmedia_extend_data_info_t *pext
    );


#if defined(TVU_LINUX)
/**
 *  Functionality:
 *      destroy the existing share memory.
 *  Return:
 *      0   : destroy successfully.
 *      <0  : failed.
**/
_LIBSHMMEDIA_DLL_
int
LibShmMediaRemoveShmidFromSystem(const char * pMemoryName);
#endif

__EXTERN_C_END

#endif
