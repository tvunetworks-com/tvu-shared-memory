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
/******************************************************************************
 *  Description:
 *      used to variableitem shm rawdata APIs
 *  History:
 *      Lotus initialized on sept.16th 2022.
******************************************************************************/
#include "libshmmedia_variableitem_rawdata.h"
#include "libshm_media_variable_item_internal.h"

int LibViShmMediaRawDataWrite(
      libshm_media_handle_t h
    , const libshmmedia_raw_data_param_t *pmi
)
{
    int ret = 0;
    return ret;
}

uint8_t *LibViShmMediaRawDataApply(
      libshm_media_handle_t h
    , size_t len
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->applyRawData(len);
}


int LibViShmMediaRawDataCommit(
    libshm_media_handle_t h
    , const uint8_t *comit_buf
    , size_t comit_len
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->commitRawData(comit_buf, comit_len);
}

int LibViShmMediaRawHeadWrite(
      libshm_media_handle_t h
    , const libshmmedia_raw_head_param_t *pmh
)
{
    int ret = 0;
    if (!pmh)
      return ret;
    ret = pmh->uRawHead_;
    return ret;
}

int LibViShmMediaRawDataRead(
      libshm_media_handle_t         h
      , libshmmedia_raw_head_param_t   *pmh /* if head chage, would be evalued */
      , libshmmedia_raw_data_param_t   *pmi
      , unsigned int timeout /* milli-seconds */
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadRawData(pmh, pmi, timeout);
}

int LibViShmMediaHasReader(libshm_media_handle_t h, unsigned int timeout)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->bHasReaders(timeout);
}
