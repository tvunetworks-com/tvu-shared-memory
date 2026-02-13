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
/******************************************************
 *  File:
 *      libsharememory.cpp
 *  CopyRight:
 *      tvu
 *  Description:
 *      libsharememory library codes.
 *  History:
 *      In 2015, Lotus adding for shielding Version extension.
******************************************************/

#include "libshm_media_variable_item_internal.h"
#include "libshm_media_protocol_internal.h"
#include "libshm_media_key_value_proto_internal.h"
#include "libshm_time_internal.h"
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>

#if _TVU_VIARIABLE_SHM_FEATURE_ENABLE

#define _LIBSHM_MEDIA_VARIABLE_SHM_DEPRECATED_APIS 1

#define INIT_FINISH_STATE           0x12345678
#define WAIT_MS_NUM                 1000

#define NOT_EQUAL_DATA(d, s) ((s>0) && s!=d)

#define COPY_DATA_WHILE_NOT_EQUAL(d, s) do {if((s>0) && s!=d) d=s;}while(0)

#define INVALID_VIDEO_WIDTH(w) ((w)<=0)
#define INVALID_VIDEO_HEIGHT(h) ((h)<=0)
#define INVALID_VIDEO_FPS(d, s) (((d)<=0) || ((s)<=0) || ((d) > (s)))
#define INVALID_AUDIO_DEPTH(d) (d <= 0)
#define INVALID_AUDIO_SAMPLERATE(s) (s < 8000)
#define INVALID_AUDIO_CHANNEL_LAYOUT(c) (c==0)

#define V4_ITEM_ALIGN_SIZE (32) // v4 need align audio/video data to 16.

static void  *_libshm_memcpy(void *dst, const void *src, size_t len)
{
#if 1
    TVUUTIL_SAFE_MEMCPY(dst, src, len);
#else
    int     max_bix     = 12;
    int     copy_unit   = 1 << max_bix;
    int     copy_mask   = copy_unit - 1;
    int     couts       = len >> max_bix;
    int     left        = len & copy_mask;
    int     i           = 0;

    for (; i < couts; i++)
    {
        memcpy((void *)((uint8_t *)dst + i * copy_unit), (void *)((uint8_t *)src + i * copy_unit), copy_unit);
    }

    if (left > 0)
        memcpy((void *)((uint8_t *)dst + couts * copy_unit), (void *)((uint8_t *)src + couts * copy_unit), left);
#endif
    return dst;
}

static int CheckShmStatus(CTvuVariableItemBaseShm  *psh, uint32_t  *ver)
{
    uint32_t    major_ver   = 0;
    uint32_t    v3_subver   = LIBSHM_MEDIA_HEAD_V3_SUB_VERSION;
    int         flags       = 0;
    int         ret         = 0;

    major_ver       = psh->GetShmVersion();
    flags           = psh->GetShmFlag();

    switch (major_ver)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    {
        ret = libshmmediapro::initShmV3(psh->GetHeader(), flags);
    }
    break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
    {
        ret = libshmmediapro::initShmV4(psh->GetHeader(), psh->GetHeadLen(), flags);
    }
    break;
    default:
        {
            DEBUG_ERROR("can not support major version [%d]\n", major_ver);
            ret     = -1;
        }
        break;
    }

    if (!ret) {
        *ver    = major_ver + v3_subver;
    }

    return ret;
}

int CTvuVariableItemRingShmCtx::CreateShmEntry(
    const char * pMemoryName
    , const uint32_t _header_len
    , const uint32_t item_count
    , const uint64_t total_size
)
{
    return CreateShmEntry(pMemoryName, _header_len, item_count, total_size, S_IRUSR | S_IWUSR);
}

int CTvuVariableItemRingShmCtx::CreateShmEntry(
    const char * pMemoryName
    , const uint32_t _header_len
    , const uint32_t item_count
    , const uint64_t total_size
    , mode_t mode
)
{
    CTvuVariableItemBaseShm    *pshm   = NULL;
    uint32_t        ver     = 0;
    uint64_t        shm_total_size   = 0;
    uint32_t    desire_major_v         = 0;
    uint32_t    item_head_len   = 0;
    uint64_t real_head_len = 0, real_total_len = 0, real_counts = 0;
    uint32_t min_head_len = 0;
    uint32_t header_len = _header_len;

    pshm    = new CTvuVariableItemBaseShm();

    if (!pshm)
    {
        DEBUG_ERROR("new CShareMemory failed\n");
        goto FAILED;
    }

    desire_major_v         = CTvuVariableItemBaseShm::GetShmVerBeforeCreate();

    item_head_len = libshmmediapro::preRequireMaxItemHeadLength(desire_major_v);
    if (item_head_len < 0)
    {
        DEBUG_ERROR("unsupport major version %d\n", desire_major_v);
        goto FAILED;
    }

    shm_total_size     = libshmmediapro::alignShmItemLength(total_size + (uint64_t)item_head_len*(uint64_t)item_count);

    min_head_len = libshmmediapro::preRequireHeadLength(desire_major_v);

    if (header_len < min_head_len)
    {
        header_len = min_head_len + 16;
    }
    header_len = _LISHMMEDIA_MEM_ALIGN(header_len, 16);/* multiple by 16 */

    if (!pshm->CreateOrOpen(pMemoryName, header_len, item_count, shm_total_size, mode))
    {
        DEBUG_ERROR("vi shm creating failed.name=>%s, head len=>%d, "
            "item count=>%d, total size=>%d\n"
            , pMemoryName, header_len, item_count, shm_total_size);
        goto FAILED;
    }

    if (desire_major_v != pshm->GetShmVersion())
    {
        /** 
         *  desired major version should match real shm version, 
         *  or should close this share memory.
         *  If server just force the desired major version to shm
         *  client would perhaps also use the old major version, 
         *  so keeping the old major version was better method.
         *
         */
        DEBUG_ERROR("desired major version %d, not matched real version %d\n", desire_major_v, pshm->GetShmVersion());
        goto FAILED;
    }

    pshm->GetInfo(&real_head_len, &real_total_len, &real_counts);

    if (shm_total_size > real_total_len)
    {
        DEBUG_ERROR("desired shm size %d, > real shm size %" PRId64 "\n", shm_total_size, real_total_len);
        goto FAILED;
    }

    if (item_count > real_counts)
    {
        DEBUG_ERROR("desired item counts %d, > real item counts %" PRId64 "\n", item_count, real_counts);
        goto FAILED;
    }

    DEBUG_INFO("wanted[%u, %u], real[%" PRId64 ", %" PRId64 "]\n"
        , shm_total_size, item_count, real_total_len, real_counts);

    if (CheckShmStatus(pshm, &ver) != 0) {
        goto FAILED;
    }

    m_uVersion      = ver;
    m_pShmObj       = pshm;
    return  0;

FAILED:

    if (pshm) 
    {
        libshmmediapro::setCloseFlag(pshm->GetHeader(), true);
        delete pshm;
        pshm    = NULL;
    }

    return -1;
}

int CTvuVariableItemRingShmCtx::OpenShmEntry(const char * pMemoryName, libshm_media_readcb_t cb, void *opaq)
{
    CTvuVariableItemBaseShm    *pshm   = NULL;
    uint32_t        ver     = 0;

    pshm    = new CTvuVariableItemBaseShm();

    if (!pshm)
    {
        DEBUG_ERROR("new CShareMemory failed\n");
        goto FAILED;
    }

    if (!pshm->Open(pMemoryName))
    {
        DEBUG_ERROR("sharemeory[name=>%s] open failed\n", pMemoryName);
        goto FAILED;
    }

    if (CheckShmStatus(pshm, &ver) != 0) {
        goto FAILED;
    }

    m_uVersion      = ver;
    m_pShmObj       = pshm;
    m_fnReadCb      = cb;
    m_pOpaq         = opaq;
    return  0;
FAILED:
    if (pshm) {
        delete pshm;
        pshm    = NULL;
    }

    return -1;
}

void CTvuVariableItemRingShmCtx::SetCloseFlag(bool bclose)
{
    if (!m_pShmObj)
        return;
    libshmmediapro::setCloseFlag(m_pShmObj->GetHeader(), bclose);
    return;
}

bool CTvuVariableItemRingShmCtx::CheckCloseFlag()
{
#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    return libshmmediapro::checkCloseFlag(m_pShmObj->GetHeader());
#else
    bool                bClose  = false;

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_head_info_v3_t    *p3 = (shm_media_head_info_v3_t*)m_pShmObj->GetHeader();

            if (p3)
                bClose  = p3->v3.close_flag ? true : false;
        }
        break;
    default:
        break;
    }

    return bClose;
#endif
}

uint8_t *CTvuVariableItemRingShmCtx::GetItemDataAddr(uint32_t index)
{
    return NULL;
}

static void copy_media_head_from_param(void *h, const libshm_media_head_param_t *p, uint32_t ver)
{

    switch(ver) 
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_head_info_v3_t    *h3 = (shm_media_head_info_v3_t *)h;
            COPY_DATA_WHILE_NOT_EQUAL(h3->vbr,  LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_vbr));
            COPY_DATA_WHILE_NOT_EQUAL(h3->sarw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_sarw));
            COPY_DATA_WHILE_NOT_EQUAL(h3->sarh, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_sarh));
            COPY_DATA_WHILE_NOT_EQUAL(h3->srcw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_srcw));
            COPY_DATA_WHILE_NOT_EQUAL(h3->srch, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_srch));
            COPY_DATA_WHILE_NOT_EQUAL(h3->sdiw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_dstw));
            COPY_DATA_WHILE_NOT_EQUAL(h3->sdih, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_dsth));
        }
        break;
    default:
        break;
    }

    return;
}

static void copy_media_head_to_param(libshm_media_head_param_t *p, void *h, uint32_t ver)
{

    switch(ver) 
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_head_info_v3_t    *h3 = (shm_media_head_info_v3_t *)h;
            p->i_vbr            = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->vbr);
            p->i_sarw           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->sarw);
            p->i_sarh           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->sarh);
            p->i_srcw           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->srcw);
            p->i_srch           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->srch);
            p->i_dstw           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->sdiw);
            p->i_dsth           = LIBSHMMEDIA_HEAD_READ_SHM_U32(h3->sdih);
        }
        break;
    default:
        break;
    }

    return;
}

static int _cmp_shm_header(libshm_media_head_param_t *porg, const libshm_media_head_param_t *pcur)
{
    if (NOT_EQUAL_DATA(porg->i_sarw, pcur->i_sarw)
        || NOT_EQUAL_DATA(porg->i_sarh, pcur->i_sarh)
        || NOT_EQUAL_DATA(porg->i_srcw, pcur->i_srcw)
        || NOT_EQUAL_DATA(porg->i_srch, pcur->i_srch)
        || NOT_EQUAL_DATA(porg->i_dstw, pcur->i_dstw)
        || NOT_EQUAL_DATA(porg->i_dsth, pcur->i_dsth)
        || NOT_EQUAL_DATA(porg->i_duration, pcur->i_duration)
        || NOT_EQUAL_DATA(porg->i_scale, pcur->i_scale)
        || NOT_EQUAL_DATA(porg->u_videofourcc, pcur->u_videofourcc)
        || NOT_EQUAL_DATA(porg->u_audiofourcc, pcur->u_audiofourcc)
        || NOT_EQUAL_DATA(porg->i_depth, pcur->i_depth)
        || NOT_EQUAL_DATA(porg->i_channels, pcur->i_channels)
        || NOT_EQUAL_DATA(porg->i_samplerate, pcur->i_samplerate)
    )
    {
        return 1;
    }

    return 0;
}

static int _copy_shm_header(libshm_media_head_param_t *porg, const libshm_media_head_param_t *pcur)
{
    int ret = 0;
    if (_cmp_shm_header(porg, pcur))
    {
        DEBUG_WARN("media head changed:\n");
        DEBUG_WARN(
            "from{vbr[%d], sar[%dx%d]"
            ", source[%dx%d], dest[%dx%d], vfourcc[%08x], fps[%d, %d]"
            ", afourcc[%08x], channels[%08x], depth[%d], samplerate[%d]}\n"
            , porg->i_vbr
            , porg->i_sarw
            , porg->i_sarh
            , porg->i_srcw
            , porg->i_srch
            , porg->i_dstw
            , porg->i_dsth
            , porg->u_videofourcc
            , porg->i_duration
            , porg->i_scale
            , porg->u_audiofourcc
            , porg->i_channels
            , porg->i_depth
            , porg->i_samplerate
        );
        DEBUG_WARN(
            "to=>{vbr[%d], sar[%dx%d]"
            ", source[%dx%d], dest[%dx%d], vfourcc[%08x], fps[%d, %d]"
            ", afourcc[%08x], channels[%08x], depth[%d], samplerate[%d]}\n"
            , pcur->i_vbr
            , pcur->i_sarw
            , pcur->i_sarh
            , pcur->i_srcw
            , pcur->i_srch
            , pcur->i_dstw
            , pcur->i_dsth
            , pcur->u_videofourcc
            , pcur->i_duration
            , pcur->i_scale
            , pcur->u_audiofourcc
            , pcur->i_channels
            , pcur->i_depth
            , pcur->i_samplerate
        );

        COPY_DATA_WHILE_NOT_EQUAL(porg->i_sarw, pcur->i_sarw);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_sarh, pcur->i_sarh);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_srcw, pcur->i_srcw);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_srch, pcur->i_srch);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_dstw, pcur->i_dstw);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_dsth, pcur->i_dsth);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_duration, pcur->i_duration);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_scale, pcur->i_scale);
        COPY_DATA_WHILE_NOT_EQUAL(porg->u_videofourcc, pcur->u_videofourcc);
        COPY_DATA_WHILE_NOT_EQUAL(porg->u_audiofourcc, pcur->u_audiofourcc);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_depth, pcur->i_depth);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_channels, pcur->i_channels);
        COPY_DATA_WHILE_NOT_EQUAL(porg->i_samplerate, pcur->i_samplerate);

        ret = 1;
    
    }
    return ret;
}

int CTvuVariableItemRingShmCtx::SendHead(const libshm_media_head_param_t *pmh)
{
    uint8_t     *pHead      = m_pShmObj->GetHeader();

    if (!m_bGotMediaHead)
    {
        m_oMediaHead    = *pmh;
        copy_media_head_from_param(pHead, pmh, m_uVersion);
        m_bGotMediaHead = true;
    }
    else 
    {
        if (_copy_shm_header(&m_oMediaHead, pmh))
        {
            copy_media_head_from_param(pHead, &m_oMediaHead, m_uVersion);
        }
    }

    return 1;
}

int CTvuVariableItemRingShmCtx::_sendV3Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmiv)
{
    const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
    bool        bvc = !(pmi->u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG);
    bool        bac = !(pmi->u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG);
    bool        bsc = !(pmi->u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG);
    bool        bec = !(pmi->u_copied_flags & LIBSHM_MEDIA_EXT_COPIED_FLAG);

    shm_media_item_info_v3_t    *pi3 = NULL; 
    int         item_head_len = 0;
    uint8_t     *pItemAddr = NULL;
    int         item_w_offset = 0;
    int         size = 0;
    int         w_len = 0;
    int64_t     now = _libshm_get_sys_ms64();


    item_head_len   = sizeof(shm_media_item_info_v3_t);

    item_w_offset   = item_head_len;

    size    = item_head_len + pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

    if (size <= 0)
    {
        DEBUG_ERROR("shm[%s] require "
            "size {h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]} invalid, ignore!!!\n"
            , m_pShmObj->GetName()
            , item_head_len
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
        );
        return 0;
    }

    pItemAddr = m_pShmObj->GetWriteItemAddr(size);

    if (!pItemAddr)
    {
        DEBUG_ERROR("shm[%s] require "
            "size {h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]} failed, can not write\n"
            , m_pShmObj->GetName()
            , item_head_len
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
        );
        return 0;
    }

    memset((void *)pItemAddr, 0, sizeof(shm_media_item_info_v3_t));
    pi3 = (shm_media_item_info_v3_t *)pItemAddr;

    {
        pi3->data_ct               = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_userDataCT);
        pi3->data_type             = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_userDataType);
        pi3->pic_flag              = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->u_picType);

        if (pmi->i_vLen > 0) {
            if (
                INVALID_VIDEO_WIDTH(pmh->i_dstw)
                || INVALID_VIDEO_HEIGHT(pmh->i_dsth)
            )
            {
                DEBUG_WARN("video resolution [w %d, h %d] invalide\n", pmh->i_dstw, pmh->i_dsth);
                w_len = -EINVAL;
                return w_len;
            }

            if (
                INVALID_VIDEO_FPS(pmh->i_duration, pmh->i_scale)
            )
            {
                DEBUG_WARN("video [fps d %d, s %d] invalide\n", pmh->i_duration, pmh->i_scale);
                w_len = -EINVAL;
                return w_len;
            }

            pi3->frametype      = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->u_frameType);
            pi3->interlace_flag = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_interlaceFlag);
            pi3->videoct        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
            pi3->videopts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vpts);
            pi3->videodts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vdts);
            pi3->vfourcc        = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->u_videofourcc);
            {
                uint32_t dstw = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dstw);
                uint32_t dsth = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dsth);
                uint32_t duration = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_duration);
                uint32_t scale = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_scale);
                COPY_DATA_WHILE_NOT_EQUAL(pi3->width, (int32_t)dstw);
                COPY_DATA_WHILE_NOT_EQUAL(pi3->height, (int32_t)dsth);
                COPY_DATA_WHILE_NOT_EQUAL(pi3->duration, (int32_t)duration);
                COPY_DATA_WHILE_NOT_EQUAL(pi3->scale, (int32_t)scale);
            }

            pi3->videolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_vLen);
            if (bvc) {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_vData, pmi->i_vLen);
            }
            item_w_offset += pmi->i_vLen;
        }

        if (pmi->i_aLen > 0) {
            if (
                INVALID_AUDIO_CHANNEL_LAYOUT(pmh->i_channels)
                || INVALID_AUDIO_DEPTH(pmh->i_depth)
                || INVALID_AUDIO_SAMPLERATE(pmh->i_samplerate)
            )
            {
                DEBUG_WARN("audio [depth %d, channel layout 0x%x, samplerate %d] invalide\n"
                    , pmh->i_depth, pmh->i_channels, pmh->i_samplerate);
                w_len = -EINVAL;
                return w_len;
            }
            pi3->audioct        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
            pi3->audiopts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_apts);
            pi3->audiodts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_adts);
            pi3->afourcc        = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->u_audiofourcc);
            pi3->channels       = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_channels);
            pi3->depth          = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_depth);
            pi3->samplerate     = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_samplerate);

            pi3->audiolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_aLen);
            if (bac) {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_aData, pmi->i_aLen);
            }

            if (pmi->h_media_process)
            {
                pmi->h_media_process((uint8_t *)(pItemAddr + item_w_offset), pmi->i_aLen, pmi->p_opaq);
            }
            item_w_offset += pmi->i_aLen;
        }

        if (pmi->i_sLen > 0) {
            pi3->subtct         = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
            pi3->subtpts        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_spts);
            pi3->subtdts        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_sdts);

            pi3->subtlen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_sLen);
            if (bsc) {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_sData, pmi->i_sLen);
            }
            item_w_offset += pmi->i_sLen;
        }

        if (pmi->i_CCLen > 0)
        {
            pi3->cclen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_CCLen);
            if (bec)
            {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_CCData, pmi->i_CCLen);
            }
            item_w_offset += pmi->i_CCLen;
        }

        if (pmi->i_timeCode > 0)
        {
            pi3->timecodelen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_timeCode);
            _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_timeCode, pmi->i_timeCode);
            item_w_offset += pmi->i_timeCode;
        }

        if (pmi->i_userDataLen > 0)
        {
            pi3->datalen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_userDataLen);
            _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_userData, pmi->i_userDataLen);
            item_w_offset += pmi->i_userDataLen;
        }

        pi3->length = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(item_w_offset);
        w_len = item_w_offset;
        if (!m_pShmObj->FinishWrite((const void *)pItemAddr, size))
        {
            w_len = 0;
            DEBUG_WARN("commit failed, address 0x%lx, size %u\n", (unsigned long)pItemAddr, size);
        }
    }
    return w_len;
}

int CTvuVariableItemRingShmCtx::_sendV4Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmiv)
{
    const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
    int         item_head_len = 0;
    uint8_t     *pItemAddr = NULL;
    int         size = 0;
    int         w_len = 0;

    const libshmmedia_audio_channel_layout_object_t *hChannel = libshmmediapro::_headParamGetChannelLayout(*pmh);
    tvushm::BufferController_t tmpBuf;
    int  nout = 0;
    const uint8_t* pout = NULL;
    tvushm::keyValueProtoAppendToBuffer(tmpBuf, hChannel);
    pout = tmpBuf.GetOrigPtr();
    nout = tmpBuf.GetBufLength();
    libshm_media_item_param_internal_t rii;
    {
        memset(&rii, 0, sizeof(rii));
        rii.nKeyValueSize_ = nout;
        rii.pKeyValuePtr_ = pout;
    }

    item_head_len   = sizeof(shm_media_item_info_v4_t);
    size    = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen + nout;

    if (size <= 0)
    {
        DEBUG_ERROR("shm[%s] require "
            "size {h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]} invalid, ignore!!!\n"
            , m_pShmObj->GetName()
            , item_head_len
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
        );
        return 0;
    }

    size += libshmmediapro::preRequireItemHeadLength(LIBSHM_MEDIA_HEAD_VERSION_V4);

    pItemAddr = m_pShmObj->GetWriteItemAddr(size);

    if (!pItemAddr)
    {
        DEBUG_ERROR("shm[%s] require "
            "size {h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]} failed, can not write\n"
            , m_pShmObj->GetName()
            , item_head_len
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
        );
        return 0;
    }

    w_len = _writeV4Buffer(pmh, pmiv, rii, pItemAddr);

    bool bcommit = m_pShmObj->FinishWrite((const void *)pItemAddr, size);

    int ret = bcommit?w_len:0;

    return ret;
}

int CTvuVariableItemRingShmCtx::_writeV4Buffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi
                                               , const libshm_media_item_param_internal_t &rii, uint8_t *pItemAddr)
{
    return libshmmediapro::writeItemBufferV4(pmh, pmi, rii, pItemAddr);
}

uint8_t *CTvuVariableItemRingShmCtx::_applyV4Buffer(unsigned int nlen)
{
    int         item_head_len = 0;
    uint8_t     *pItemAddr = NULL;
    int         size = nlen;

    item_head_len   = sizeof(shm_media_item_info_v4_t);

    size += libshmmediapro::preRequireItemHeadLength(LIBSHM_MEDIA_HEAD_VERSION_V4);

    pItemAddr = m_pShmObj->GetWriteItemAddr(size);

    return pItemAddr;
}

bool CTvuVariableItemRingShmCtx::_commitV4Buffer(uint8_t *pItemAddr, unsigned int nlen)
{
    int size = nlen;

    size += libshmmediapro::preRequireItemHeadLength(LIBSHM_MEDIA_HEAD_VERSION_V4);

    return m_pShmObj->FinishWrite((const void *)pItemAddr, size);
}

int CTvuVariableItemRingShmCtx::SendData(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi)
{
    int         w_len       = 0;

    SendHead(pmh);

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            w_len = _sendV3Data(pmh, pmi);
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            w_len = _sendV4Data(pmh, pmi);
        }
        break;
    default:
        {
            w_len   = -1;
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }

    return w_len;
}

int CTvuVariableItemRingShmCtx::SendDataWithFrequency1000(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi)
{
#if !defined(TVU_WINDOWS)
    {
        int64_t     now         = 0;
        now     = _libshm_get_sys_ms64();

        if (now == m_i64LastSendSysTime)
        {
            _libshm_common_usleep(1000);
            now = _libshm_get_sys_ms64();
        }

        m_i64LastSendSysTime = now;
    }
#endif

    return SendData(pmh, pmi);
}

uint8_t *CTvuVariableItemRingShmCtx::applyBuffer(unsigned int nlen)
{
    uint8_t *buf = NULL;

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            buf = _applyV4Buffer(nlen);
        }
        break;
    default:
        {
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }

    return buf;
}

bool CTvuVariableItemRingShmCtx::commitBuffer(uint8_t *pItemAddr, unsigned int size)
{
    bool bret = false;

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            bret = _commitV4Buffer(pItemAddr, size);
        }
        break;
    default:
        {
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }

    return bret;
}

int CTvuVariableItemRingShmCtx::PollReadRawData(libshmmedia_raw_head_param_t *pmh, libshmmedia_raw_data_param_t *pmi, unsigned int timeout)
{
    size_t      itemsize = 0;
    uint8_t     *pItemAddr = NULL;

    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    pItemAddr = m_pShmObj->GetReadItemAddr(&itemsize);

    if (!pItemAddr || !itemsize)
    {
        ret = 0;
        return ret;
    }

    pmi->pRawData_ = pItemAddr;
    pmi->uRawData_ = itemsize;
    ret = itemsize;
    return ret;
}

uint8_t *CTvuVariableItemRingShmCtx::applyRawData(size_t len)
{
    if (!IsCreator())
    {
        return NULL;
    }

    return m_pShmObj->GetWriteItemAddr(len);;
}

int CTvuVariableItemRingShmCtx::commitRawData(const uint8_t *buf, size_t s)
{
    bool b = m_pShmObj->FinishWrite((const void *)buf, s);
    return b?s:0;
}

/**
 *  Return:
 *      0   : equal
 *      >,< : not equal
 */
static inline
int _media_head_cmp(libshm_media_head_param_t *from, libshm_media_head_param_t *to)
{
    int     ret     = 0;
    if (memcmp(from, to, sizeof(libshm_media_head_param_t))) {
        //DEBUG_ERROR("media shm header changed:\n");
        //DEBUG_ERROR(
        //    "from{vbr[%d], sar[%dx%d]"
        //    ", source[%dx%d], dest[%dx%d], vfourcc[%08x], fps[%d, %d]"
        //    ", afourcc[%08x], channels[%08x], depth[%d], samplerate[%d]}\n"
        //    , from->i_vbr
        //    , from->i_sarw
        //    , from->i_sarh
        //    , from->i_srcw
        //    , from->i_srch
        //    , from->i_dstw
        //    , from->i_dsth
        //    , from->i_videofourcc
        //    , from->i_duration
        //    , from->i_scale
        //    , from->i_audiofourcc
        //    , from->i_channels
        //    , from->i_depth
        //    , from->i_samplerate

        //);
        //DEBUG_ERROR(
        //    "to=>{vbr[%d], sar[%dx%d]"
        //    ", source[%dx%d], dest[%dx%d], vfourcc[%08x], fps[%d, %d]"
        //    ", afourcc[%08x], channels[%08x], depth[%d], samplerate[%d]}\n"
        //    , to->i_vbr
        //    , to->i_sarw
        //    , to->i_sarh
        //    , to->i_srcw
        //    , to->i_srch
        //    , to->i_dstw
        //    , to->i_dsth
        //    , to->i_videofourcc
        //    , to->i_duration
        //    , to->i_scale
        //    , to->i_audiofourcc
        //    , to->i_channels
        //    , to->i_depth
        //    , to->i_samplerate
        //);
        ret     = -1;
    }
    return ret;
}

/**
 *  Return:
 *      < 0 : failed,
 *      0   : wait,
 *      > 0 : success
 */
int CTvuVariableItemRingShmCtx::PollReadHead(libshm_media_head_param_t *pmh, uint32_t timeout)
{
    size_t          itemsize = 0;
    uint8_t         *pItemAddr = NULL;
    libshm_media_item_param_t   oip;
    {
        memset((void*)&oip, 0, sizeof(libshm_media_item_param_t));
    }
    libshm_media_head_param_t   ohp;
    {
        memset((void*)&ohp, 0, sizeof(libshm_media_head_param_t));
    }
    uint64_t    read_index = 0;

    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    pItemAddr = m_pShmObj->GetReadItemAddrWithNoStep(&itemsize);
    read_index = m_pShmObj->GetReadIndex();

    if (!pItemAddr || !itemsize)
    {
        DEBUG_ERROR("get read item address failed\n");
        return 0;
    }

    unsigned int item_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);
    ret = libshmmediapro::readHeadFromItemBuffer(pmh, pItemAddr, item_len);
    return ret;
}

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE

int CTvuVariableItemRingShmCtx::PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, uint32_t timeout)
{
    size_t         itemsize = 0;
    uint8_t     *pItemAddr = NULL;
    int         r_len       = -1;
    libshm_media_item_param_t   oip;
    {
        memset((void*)&oip, 0, sizeof(libshm_media_item_param_t));
    }
    libshm_media_head_param_t   ohp;
    {
        memset((void*)&ohp, 0, sizeof(libshm_media_head_param_t));
    }
    uint64_t    read_index = 0;

    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    read_index = m_pShmObj->GetReadIndex();
    pItemAddr = m_pShmObj->GetReadItemAddr(&itemsize);

    if (!pItemAddr || !itemsize)
    {
        DEBUG_WARN("get read item address failed\n");
        return 0;
    }

    unsigned int buffer_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);

    r_len = libshmmediapro::readDataFromItemBuffer(&ohp, &oip, pItemAddr, buffer_len);

    if (r_len > 0)
    {
        oip.u_read_index = read_index;
        if (_media_head_cmp(pmh, &ohp))
        {
            *pmh    = ohp;
            //return -1;
        }

        *pmi = oip;
        if (m_fnReadCb) {
            ret   = m_fnReadCb(m_pOpaq, &oip);
            if (ret < 0) {
                DEBUG_ERROR("read callback return %d invalid", ret);
            }
        }
    }

    return r_len;
}

#else
int CTvuVariableItemRingShmCtx::_readV3Data(libshm_media_item_param_t   *pmi, const uint8_t *pItemAddr)
{
    int         r_len = -1;
    const shm_media_item_info_v3_t    *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
    int         item_head_len = 0;
    int         item_v_offset = 0;
    int         item_a_offset = 0;
    int         item_s_offset = 0;
    int         item_e_offset = 0;
    int         item_timecode_offset = 0;
    int         item_data_offset = 0;

    pmi->i_totalLen = pi3->length;
    item_head_len = sizeof(shm_media_item_info_v3_t);
    item_v_offset = item_head_len;
    item_a_offset = item_v_offset + pi3->videolen;
    item_s_offset = item_a_offset + pi3->audiolen;
    item_e_offset = item_s_offset + pi3->subtlen;
    item_timecode_offset = item_e_offset + pi3->cclen;
    item_data_offset = item_timecode_offset + pi3->timecodelen;

    pmi->u_picType = pi3->pic_flag;
    if (pi3->videolen > 0) {
        pmi->p_vData = pItemAddr + item_v_offset;
        pmi->i_vLen = pi3->videolen;
    }
    pmi->i64_vpts = pi3->videopts;
    pmi->i64_vdts = pi3->videodts;
    pmi->i64_vct = pi3->videoct;
    pmi->u_frameType = pi3->frametype;
    pmi->i_interlaceFlag = pi3->interlace_flag;

    if (pi3->audiolen > 0) {
        pmi->p_aData = pItemAddr + item_a_offset;
        pmi->i_aLen = pi3->audiolen;
    }
    pmi->i64_apts = pi3->audiopts;
    pmi->i64_adts = pi3->audiodts;
    pmi->i64_act = pi3->audioct;

    if (pi3->subtlen > 0) {
        pmi->p_sData = pItemAddr + item_s_offset;
        pmi->i_sLen = pi3->subtlen;
    }
    pmi->i64_spts = pi3->subtpts;
    pmi->i64_sdts = pi3->subtdts;
    pmi->i64_sct = pi3->subtct;

    if (pi3->cclen > 0) {
        pmi->p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
        pmi->i_CCLen = pi3->cclen;
    }

    if (pi3->timecodelen > 0) {
        pmi->p_timeCode = (const uint8_t *)(pItemAddr + item_timecode_offset);
        pmi->i_timeCode = pi3->timecodelen;
    }

    if (pi3->datalen > 0) {
        pmi->p_userData = (const uint8_t *)(pItemAddr + item_data_offset);
        pmi->i_userDataLen = pi3->datalen;
    }
    pmi->i64_userDataCT = pi3->data_ct;
    pmi->i_userDataType = pi3->data_type;

    if (m_fnReadCb) {
        r_len = m_fnReadCb(m_pOpaq, pmi);
        if (r_len < 0) {
            DEBUG_ERROR("read callback return %d invalid");
        }
    }

    pmi->u_read_index = 0;
    r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

    return r_len;
}

int CTvuVariableItemRingShmCtx::_readV4Data(libshm_media_item_param_t   *pmi, const uint8_t *pItemAddr)
{
    int         r_len = -1;
    const shm_media_item_info_v3_t    *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
    const shm_media_item_info_v4_t    *pi4 = (const shm_media_item_info_v4_t *)pItemAddr;
    int         item_head_len = 0;
    int         item_v_offset = 0;
    int         item_a_offset = 0;
    int         item_s_offset = 0;
    int         item_e_offset = 0;
    int         item_timecode_offset = 0;
    int         item_data_offset = 0;

    pmi->i_totalLen = pi3->length;
    item_head_len = pi4->_head_len;
    item_v_offset = pi4->_v_data_offset;
    item_a_offset = pi4->_a_data_offset;
    item_s_offset = pi4->_s_data_offset;
    item_e_offset = pi4->_cc_data_offset;
    item_timecode_offset = pi4->_timecode_data_offset;
    item_data_offset = pi4->_user_data_offset;

    pmi->u_picType = pi3->pic_flag;
    if (pi3->videolen > 0) {
        pmi->p_vData = pItemAddr + item_v_offset;
        pmi->i_vLen = pi3->videolen;
    }
    pmi->i64_vpts = pi3->videopts;
    pmi->i64_vdts = pi3->videodts;
    pmi->i64_vct = pi3->videoct;
    pmi->u_frameType = pi3->frametype;
    pmi->i_interlaceFlag = pi3->interlace_flag;

    if (pi3->audiolen > 0) {
        pmi->p_aData = pItemAddr + item_a_offset;
        pmi->i_aLen = pi3->audiolen;
    }
    pmi->i64_apts = pi3->audiopts;
    pmi->i64_adts = pi3->audiodts;
    pmi->i64_act = pi3->audioct;

    if (pi3->subtlen > 0) {
        pmi->p_sData = pItemAddr + item_s_offset;
        pmi->i_sLen = pi3->subtlen;
    }
    pmi->i64_spts = pi3->subtpts;
    pmi->i64_sdts = pi3->subtdts;
    pmi->i64_sct = pi3->subtct;

    if (pi3->cclen > 0) {
        pmi->p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
        pmi->i_CCLen = pi3->cclen;
    }

    if (pi3->timecodelen > 0) {
        pmi->p_timeCode = (const uint8_t *)(pItemAddr + item_timecode_offset);
        pmi->i_timeCode = pi3->timecodelen;
    }

    if (pi3->datalen > 0) {
        pmi->p_userData = (const uint8_t *)(pItemAddr + item_data_offset);
        pmi->i_userDataLen = pi3->datalen;
    }
    pmi->i64_userDataCT = pi3->data_ct;
    pmi->i_userDataType = pi3->data_type;

    if (m_fnReadCb) {
        r_len = m_fnReadCb(m_pOpaq, pmi);
        if (r_len < 0) {
            DEBUG_ERROR("read callback return %d invalid");
        }
    }

    pmi->u_read_index = 0;
    r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

    return r_len;
}

int CTvuVariableItemRingShmCtx::PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, uint32_t timeout)
{
    size_t         itemsize = 0;
    uint8_t     *pItemAddr = NULL;
    int         r_len       = -1;
    libshm_media_item_param_t   oip;
    {
        memset((void*)&oip, 0, sizeof(libshm_media_item_param_t));
    }
    libshm_media_head_param_t   ohp;
    {
        memset((void*)&ohp, 0, sizeof(libshm_media_head_param_t));
    }
    uint64_t    read_index = 0;

    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    pItemAddr = m_pShmObj->GetReadItemAddr(&itemsize);
    read_index = m_pShmObj->GetReadIndex();

    if (!pItemAddr || !itemsize)
    {
        DEBUG_ERROR("get read item address failed\n");
        return 0;
    }

    if (PollReadHead(&ohp, pItemAddr, false, timeout) <= 0) {
        return -1;
    }

    if (_media_head_cmp(pmh, &ohp)) {
        *pmh    = ohp;
        //return -1;
    }

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            r_len = _readV3Data(&oip, pItemAddr);
            oip.u_read_index = read_index;
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            r_len = _readV4Data(&oip, pItemAddr);
            oip.u_read_index = read_index;
        }
        break;
    default:
        {
            r_len   = -1;
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }

    *pmi    = oip;
    return r_len;
}

#endif

int CTvuVariableItemRingShmCtx::ReadIndexStep()
{
    m_pShmObj->FinishRead();
    return 0;
}

int CTvuVariableItemRingShmCtx::PollReadData(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, uint32_t timeout)
{
    int ret = PollReadDataWithoutIndexStep(pmh, pmi, timeout);

    if (ret > 0)
    {
        ReadIndexStep();
    }
    return ret;
}

int CTvuVariableItemRingShmCtx::IsTheSearchingItem(const void *buf, size_t nbuf, uint64_t pos, void *user, libshmmedia_item_checking_fn_t fn)
{
    size_t         itemsize = nbuf;
    const uint8_t  *pItemAddr = (const uint8_t *)buf;
    int         r_len       = -1;
    libshm_media_item_param_t   oip;
    {
        memset((void*)&oip, 0, sizeof(libshm_media_item_param_t));
    }
    libshm_media_head_param_t   ohp;
    {
        memset((void*)&ohp, 0, sizeof(libshm_media_head_param_t));
    }


    if (!pItemAddr || !itemsize)
    {
        return -1;
    }

    unsigned int buffer_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);

    if (buffer_len > nbuf)
    {
        return -1;
    }

    r_len = libshmmediapro::readDataFromItemBuffer(&ohp, &oip, pItemAddr, buffer_len);

    if (r_len <= 0)
    {
        return r_len;
    }

    bool bsearching = false;

    if (r_len > 0)
    {
        /* checking whether it was just the searching. */
        oip.u_read_index = pos;
        bsearching =  !!fn(user, &ohp, &oip);
    }

    return bsearching?r_len:0;
}

struct _LocalCallBackContext
{
    void *userCtx;
    CTvuVariableItemRingShmCtx *shmCtx;
    libshmmedia_item_checking_fn_t fnChecking;
};

static int _item_valid_checking(void *ctx, const void *buf, size_t n, uint64_t pos)
{
    struct _LocalCallBackContext *ptr = (struct _LocalCallBackContext*)ctx;
    int ret = ptr->shmCtx->IsTheSearchingItem(buf, n, pos, ptr->userCtx, ptr->fnChecking);
    return ret;
}

bool CTvuVariableItemRingShmCtx::SearchItems(void *user, libshmmedia_item_checking_fn_t ch)
{
    tvu_variableitem_base_shm_item_valid_determine_fn_t fn = _item_valid_checking;
    struct _LocalCallBackContext obj;
    {
        obj.userCtx = user;
        obj.fnChecking = ch;
        obj.shmCtx = this;
    }
    return m_pShmObj->SearchWholeItems(&obj, fn);
}

void CTvuVariableItemRingShmCtx::SeekReadIndexToWriteIndex()
{
    CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
    if (pshm)
        pshm->SeekReadIndex2WriteIndex();
    return;
}

void CTvuVariableItemRingShmCtx::SeekReadIndexToZero()
{
    CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
    if (pshm)
        pshm->SeekReadIndex2Zero();
    return;
}

void CTvuVariableItemRingShmCtx::SeekReadIndex(uint64_t rindex)
{
    CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
    if (pshm)
        pshm->SeekReadIndex(rindex);
    return;
}

int CTvuVariableItemRingShmCtx::RemoveShm(const char *shmname)
{
    return CTvuVariableItemBaseShm::RemoveShmFromKernal(shmname);
}

void
LibViShmMediaSetCloseflag(libshm_media_handle_t h, int bclose)
{
    CTvuVariableItemRingShmCtx *pctx       = (CTvuVariableItemRingShmCtx *)h;
    pctx->SetCloseFlag(bclose ? true : false);
    return;
}

int LibViShmMediaCheckCloseflag(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx *pctx   = (CTvuVariableItemRingShmCtx *)h;
    bool            bClose  = pctx->CheckCloseFlag();
    return bClose ? 1 : 0;
}

libshm_media_handle_t
LibViShmMediaCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
)
{
    return LibViShmMediaCreate2(pMemoryName, header_len, item_count, total_size, S_IRUSR | S_IWUSR);
}

libshm_media_handle_t
LibViShmMediaCreate2
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint64_t total_size
    , mode_t mode
)
{
    CTvuVariableItemRingShmCtx             *pctx   = NULL;
    libshm_media_handle_t    h       = NULL;

    pctx    = new CTvuVariableItemRingShmCtx();

    if (!pctx)
    {
        DEBUG_ERROR("malloc media shm context failed\n");
        goto FAILED;
    }

    if (pctx->CreateShmEntry(pMemoryName, header_len, item_count, total_size, mode) < 0) {
        goto FAILED;
    }

    h   = (libshm_media_handle_t)pctx;
    return h;

FAILED:
    if (pctx) {
        delete pctx;
        pctx    = NULL;
    }

    return NULL;
}

int 
LibViShmMediaRemoveShmFromSystem
(
    const char * pMemoryName
)
{
    libshm_media_handle_t h = LibViShmMediaOpen(pMemoryName, NULL, NULL);
    if (h)
    {
        LibViShmMediaSetCloseflag(h, 1); /* it must set close flag before removing */
        LibViShmMediaDestroy(h);
    }
    return CTvuVariableItemRingShmCtx::RemoveShm(pMemoryName);
}

libshm_media_handle_t
LibViShmMediaOpen
(
    const char * pMemoryName
    , libshm_media_readcb_t cb
    , void *opaq
)
{
    CTvuVariableItemRingShmCtx             *pctx   = NULL;
    libshm_media_handle_t      h       = NULL;

    pctx    = new CTvuVariableItemRingShmCtx();

    if (!pctx)
    {
        DEBUG_ERROR("malloc media shm context failed\n");
        goto FAILED;
    }

    if (pctx->OpenShmEntry(pMemoryName, cb, opaq) < 0) {
        goto FAILED;
    }

    h   = (libshm_media_handle_t)pctx;
    return h;

FAILED:
    if (pctx) {
        delete pctx;
        pctx    = NULL;
    }

    return NULL;
}

void
LibViShmMediaDestroy(libshm_media_handle_t h)
{
/**
 *  LLL, Dec 25th 2015.
 *  Why need this currently?
 *  Linux platform can not accomplish that 
 *  the share-memory would be recycled automatically 
 *  while all processes which were using the share-memory.
 *  So the process who create the share-memory
 *  should remove the share-memory from system manually, then
 *  It should tell other reading processes to close the share-memory,
 *  for the share-memory would not be used again.
 *  
 */
    if (LibViShmMediaIsCreator(h)) {
        LibViShmMediaSetCloseflag(h, 1);
    }

    CTvuVariableItemRingShmCtx *pctx       = (CTvuVariableItemRingShmCtx *)h;

    if (pctx)
        delete pctx;

    return;
}

void LibViShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx)
    {
        pctx->SeekReadIndexToWriteIndex();
    }
    return;
}

void LibViShmMediaSeekReadIndexToZero(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx)
    {
        pctx->SeekReadIndexToZero();
    }
    return;
}

void LibViShmMediaSeekReadIndex(libshm_media_handle_t h, uint64_t rindex)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx)
    {
        pctx->SeekReadIndex(rindex);
    }
    return;
}

uint32_t LibViShmMediaGetVersion(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx *pctx       = (CTvuVariableItemRingShmCtx *)h;
    uint32_t        vers        = 0;

    if (pctx)
        vers    = pctx->GetVersion();

    return vers;
}

uint8_t *LibViShmMediaGetItemDataAddr(libshm_media_handle_t h, unsigned int index)
{
    CTvuVariableItemRingShmCtx *pctx   = (CTvuVariableItemRingShmCtx *)h;

    return pctx->GetItemDataAddr(index);
}

unsigned int LibViShmMediaGetItemLength(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetShmTotalSize();
}

unsigned int LibViShmMediaGetTotalPayloadSize(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetShmTotalSize();
}

unsigned int LibViShmMediaGetItemCounts(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetItemCounts();
}

unsigned int LibViShmMediaGetHeadLen(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetHeadLen();
}

unsigned int LibViShmMediaGetItemOffset(libshm_media_handle_t h)
{
    return LibViShmMediaGetHeadLen(h);
}

const char *LibViShmMediaGetName(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetName();
}

uint64_t LibViShmMediaGetWriteIndex(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx *pctx = (CTvuVariableItemRingShmCtx *)h;
    return  pctx->GetWIndex();
}

uint64_t LibViShmMediaGetReadIndex(
    libshm_media_handle_t         h
)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    return pctx->GetRIndex();
}

/**
 *  Return:
 *      0   :   not the creator
 *      !0  :  the creator
**/
int LibViShmMediaIsCreator(libshm_media_handle_t h)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->IsCreator() ? 1 : 0;
}

/**
 *  Parameter:
 *      timeout : currently, it was no use
 *  Return:
 *      0   :   not ready
 *      +   :   ready
 *      -   :   I/O error
**/
int LibViShmMediaPollSendable(libshm_media_handle_t h, uint32_t timeout)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    //DEBUG_INFO("\n");
    return pctx->PollSendable(timeout);
}

int LibViShmMediaPollReadable(libshm_media_handle_t h, uint32_t timeout)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadable(timeout);
}

int LibViShmMediaSendData(
      libshm_media_handle_t h
      , const libshm_media_head_param_t *pmh
      , const libshm_media_item_param_t *pmi
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    //DEBUG_INFO("\n");
    return pctx->SendData(pmh, pmi);
}

int LibViShmMediaSendDataWithFrequency1000(
        libshm_media_handle_t h
      , const libshm_media_head_param_t *pmh
      , const libshm_media_item_param_t *pmi
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->SendDataWithFrequency1000(pmh, pmi);
}

int LibViShmMediaPollReadHead(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , uint32_t                    timeout
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadHead(pmh, timeout);
}

int LibViShmMediaPollReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
      , uint32_t                    timeout
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadData(pmh, pmi, timeout);
}

int LibViShmMediaReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
)
{
    CTvuVariableItemRingShmCtx    *pctx    = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadData(pmh, pmi, 0);
}

int LibViShmMediaReadDataWithoutIndexStep(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    return pctx->PollReadDataWithoutIndexStep(pmh, pmi, 0);
}

void LibViShmMediaReadIndexStep(
    libshm_media_handle_t         h
)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    pctx->ReadIndexStep();
    return;
}

int LibViShmMediaSearchItems(
      libshm_media_handle_t        h
      , void *userCtx
      , libshmmedia_item_checking_fn_t      fn
)
{
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    bool bget = pctx->SearchItems(userCtx, fn);
    return bget?1:0;
}

uint8_t* LibViShmMediaItemApplyBuffer(
      libshm_media_handle_t h
    , unsigned int  nlen
)
{
    uint8_t *buf = NULL;
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx && pctx->IsCreator())
    {
        buf = pctx->applyBuffer(nlen);
    }

    return buf;
}

int LibViShmMediaItemCommitBuffer(
    libshm_media_handle_t h,
    uint8_t *pItemAddr,
    unsigned int nlen
)
{
    int  ret = -1;
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx && pctx->IsCreator())
    {
        bool b = pctx->commitBuffer(pItemAddr, nlen);
        ret = b?0:-1;
    }

    return ret;
}

int LibViShmMediaItemWriteBuffer(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr)
{
    int  ret = 0;
    CTvuVariableItemRingShmCtx    *pctx = (CTvuVariableItemRingShmCtx *)h;
    if (pctx && pctx->IsCreator())
    {
        uint32_t major_ver = LibViShmMediaGetVersion(h);
        ret = libshmmediapro::writeItemBuffer(pmh, pmi, pItemAddr, major_ver);
    }

    return ret;
}

int LibViShmMediaItemWriteBufferIgnoreInternalCopy(
        libshm_media_handle_t h,
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr)
{
    libshm_media_item_param_t omi = *pmi;

    {
        omi.u_copied_flags = 0xFFFFFFFF;
    }
    return LibViShmMediaItemWriteBuffer(h, pmh, &omi, pItemAddr);
}

int  LibViShmMediaItemPreGetReadBufferLayout(
      libshm_media_handle_t h
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , const uint8_t *pItemAddr
)
{
    int  ret = -1;

    if (h)
    {
        bool b = LibViShmMediaIsCreator(h);
        if (!b)
        {
            uint32_t major_ver = LibViShmMediaGetVersion(h);
            ret = libshmmediapro::getReadItemBufferLayoutWithVer(pmh, pmi, pItemAddr, major_ver);
        }
    }

    return ret;
}

int  LibViShmMediaItemPreGetWriteBufferLayout(
      libshm_media_handle_t h
    , const libshm_media_item_param_t *pmi
    , uint8_t *pItemAddr
    , libshm_media_item_addr_layout_t *pLayout
)
{
    int  ret = -1;

    if (h)
    {
        bool b = LibViShmMediaIsCreator(h);
        if (b)
        {
            uint32_t major_ver = LibViShmMediaGetVersion(h);
            ret = libshmmediapro::getWriteItemBufferLayoutWithVer(pmi, pItemAddr, pLayout, major_ver);
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}

#endif

