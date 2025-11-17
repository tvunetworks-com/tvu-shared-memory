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

#include "libshm_media_raw_data_opt.h"
#include "libshm_media_internal.h"
#include "libshm_media_protocol_internal.h"


int LibShmMediaRawDataWrite(
      libshm_media_handle_t h
    , const libshmmedia_raw_data_param_t *pmi
)
{
    int ret = 0;
    return ret;
}

uint8_t *LibShmMediaRawDataApply(
      libshm_media_handle_t h
    , size_t len
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->ApplyRawData(len);
}

int LibShmMediaRawDataCommit(
      libshm_media_handle_t h
    , const uint8_t *comit_buf
    , size_t commit_len
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->CommitRawData(commit_len);
}

int LibShmMediaRawHeadWrite(
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

int LibShmMediaRawDataRead(
    libshm_media_handle_t         h
    , libshmmedia_raw_head_param_t   *pmh
    , libshmmedia_raw_data_param_t   *pmi
    , unsigned int timeout /* milli-seconds */
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->PollReadRawData(pmh, pmi, timeout);
}

int LibShmMediaHasReader(libshm_media_handle_t h, unsigned int timeout)
{
    int ret = 0;
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    ret = pctx->bHasReaders(timeout)?1:0;
    return ret;
}

int LibShmMediaItemApplyBuffer(
    libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , libshm_media_item_addr_layout_t *pLayout
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    int ret = 0;
    if (pctx) {
        ret = pctx->ApplyItemBuffer(pmi, pLayout);
    }
    return ret;
}

int LibShmMediaItemCommitBuffer(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    int ret = 0;
    if (pctx) {
        ret = pctx->CommitItemBuffer(pmh, pmi);
    }
    return ret;
}

int  LibShmMediaItemGetWriteBufferLayout(
      libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , uint8_t *pItemAddr
    , uint32_t itemLen
    , libshm_media_item_addr_layout_t *pLayout
)
{
    int  ret = 0;

    if (h)
    {
        bool b = LibShmMediaIsCreator(h);
        if (b)
        {
            uint32_t major_ver = LibShmMediaGetVersion(h);
            ret = libshmmediapro::getWriteItemBufferLayoutWithVer(pmi, pItemAddr, pLayout, major_ver);

            if (ret > 0 && (unsigned int)ret > itemLen)
            {
                DEBUG_WARN("write item buffer size %u < request length %d", itemLen, ret);
                return 0;
            }
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}

int LibShmMediaItemWriteBuffer(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr)
{
    int  ret = 0;
    CLibShmMediaCtx    *pctx = (CLibShmMediaCtx *)h;
    if (pctx && pctx->IsCreator())
    {
        libshm_media_item_param_t omi = *pmi;
        omi.u_copied_flags = 0xFFFFFFFF;
        uint32_t major_ver = LibShmMediaGetVersion(h);
        ret = libshmmediapro::writeItemBuffer(pmh, &omi, pItemAddr, major_ver);
    }

    return ret;
}




