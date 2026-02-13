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
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "libshm_media_internal.h"
#include "libshm_media_protocol_internal.h"
#include "libshm_media_key_value_proto_internal.h"
#include "libshm_time_internal.h"
#include "TvuLog.h"
#include "libshm_tvu_timestamp.h"

#define NOT_EQUAL_DATA(d, s) ((s>0) && s!=d)

#define COPY_DATA_WHILE_NOT_EQUAL(d, s) do {if((s>0) && s!=d) d=s;}while(0)

#define INVALID_VIDEO_WIDTH(w) ((w)<=0)
#define INVALID_VIDEO_HEIGHT(h) ((h)<=0)
#define INVALID_VIDEO_FPS(d, s) (((d)<=0) || ((s)<=0) || ((d) > (s)))
#define INVALID_AUDIO_DEPTH(d) (d <= 0)
#define INVALID_AUDIO_SAMPLERATE(s) (s < 8000)
#define INVALID_AUDIO_CHANNEL_LAYOUT(c) (c==0)
#ifdef TVU_WINDOWS
#pragma comment(lib,"ws2_32.lib")
#endif

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

static int CheckShmStatus(CTvuBaseShareMemory  *psh, uint32_t  *ver)
{
    uint32_t    major_ver   = 0;
    uint32_t    v3_subver   = 0;
    int         flags       = 0;
    int         ret         = 0;

    major_ver       = psh->GetShmVersion();
    flags           = psh->GetShmFlag();

    switch (major_ver)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
        {
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        //{
        //    shm_media_head_info_v2_t    *p2     = (shm_media_head_info_v2_t *)psh->GetHeader();
        //    int                         nLoop   = 0;
        //    if (flags & SHM_FLAG_READ) 
        //    {
        //        nLoop   = 0;
        //        while(p2->init_flag != INIT_FINISH_STATE) {
        //            if (nLoop == WAIT_MS_NUM)
        //            {
        //                DEBUG_ERROR("ver[%d],can not get the correct init flag\n", major_ver);
        //                ret     = -1;
        //                break;
        //            }
        //            _libshm_common_msleep(1);
        //            nLoop++;
        //        }
        //    }
        //    else
        //    {
        //        p2->close_flag  = 0;
        //        p2->init_flag   = INIT_FINISH_STATE;
        //    }
        //    DEBUG_INFO("major ver 2, close flag %d\n", p2->close_flag);
        //}
        {
            ret = libshmmediapro::initShmV2(psh->GetHeader(), flags);
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        //{
        //    shm_media_head_info_v3_t    *p3     = (shm_media_head_info_v3_t *)psh->GetHeader();
        //    int                         nLoop   = 0;
        //    if (flags & SHM_FLAG_READ) 
        //    {
        //        nLoop   = 0;
        //        while(p3->v3.init_flag != INIT_FINISH_STATE) {
        //            if (nLoop == WAIT_MS_NUM)
        //            {
        //                DEBUG_ERROR("major ver[3],can not get the correct init flag\n");
        //                ret     = -1;
        //                break;
        //            }
        //            _libshm_common_msleep(1);
        //            nLoop++;
        //        }

        //        if (ret == 0)
        //        {
        //            switch (p3->v3.v3_sub_ver)
        //            {
        //            case LIBSHM_V3_SUBVER_V0:
        //                {
        //                    v3_subver   = LIBSHM_V3_SUBVER_V0;
        //                }
        //                break;
        //            case LIBSHM_V3_SUBVER_V1:
        //            default:
        //                {
        //                    ret = -1;
        //                }
        //                break;
        //            }
        //        }

        //    }
        //    else
        //    {
        //        v3_subver           = 
        //        p3->v3.v3_sub_ver   = LIBSHM_V3_SUB_VERSION;
        //        p3->v3.close_flag   = 0;
        //        p3->v3.init_flag    = INIT_FINISH_STATE;
        //    }

        //    DEBUG_INFO("major ver 3, sub ver %d, close flag %d\n"
        //        , p3->v3.v3_sub_ver
        //        , p3->v3.close_flag);
        //}
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

int CLibShmMediaCtx::CreateShmEntry(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length)
{
    return CreateShmEntry(pMemoryName, header_len, item_count, item_length, S_IRUSR | S_IWUSR);
}

int CLibShmMediaCtx::CreateShmEntry(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, mode_t mode)
{
    CTvuBaseShareMemory    *pshm   = NULL;
    uint32_t        ver     = 0;
    uint32_t        item_total_length   = 0;
    uint32_t    desire_major_v         = 0;
    uint32_t    item_head_len   = 0;
    uint32_t    min_head_len = 0;
    uint32_t    nReserverKeyValueLen = 1024;

    pshm    = new CTvuBaseShareMemory();

    if (!pshm)
    {
        DEBUG_ERROR("new CShareMemory failed\n");
        goto FAILED;
    }

    desire_major_v         = CTvuBaseShareMemory::GetShmVerBeforeCreate();

    item_head_len = libshmmediapro::preRequireMaxItemHeadLength(desire_major_v);

    if (item_head_len <= 0)
    {
        DEBUG_ERROR("sharemeory[name=>%s] item head len %d invalide\n"
            , pMemoryName, item_head_len);
        goto FAILED;
    }

    item_total_length     = libshmmediapro::alignShmItemLength(item_length + item_head_len + nReserverKeyValueLen);

    min_head_len = libshmmediapro::preRequireHeadLength(desire_major_v);
    if (min_head_len < 0)
    {
        DEBUG_ERROR("sharemeory[name=>%s] mini head len %d invalide\n"
            , pMemoryName, min_head_len);
        goto FAILED;
    }

    if (header_len < min_head_len)
    {
        header_len = min_head_len+16;
    }
    header_len = _LISHMMEDIA_MEM_ALIGN(header_len, 16);//make sure multiple by 16

    if (!pshm->CreateOrOpen(pMemoryName, header_len, item_count, item_total_length, mode, &libshmmediapro::setCloseFlag))
    {
        DEBUG_ERROR("sharemeory[name=>%s, head len=>%d, "
            "item count=>%d, item len=>%d] create failed\n"
            , pMemoryName, header_len, item_count, item_total_length);
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

    if (item_total_length > pshm->GetItemLength())
    {
        DEBUG_ERROR("desired item length %d, > real item length %d\n", item_total_length, pshm->GetItemLength());
        goto FAILED;
    }

    if (item_count > pshm->GetItemCounts())
    {
        DEBUG_ERROR("desired item counts %d, > real item counts %d\n", item_count, pshm->GetItemCounts());
        goto FAILED;
    }

    DEBUG_INFO("wanted[%u, %u], real[%u, %u], item head len %d, head data size %u, head len %u\n"
        , item_total_length, item_count, pshm->GetItemLength(), pshm->GetItemCounts(), item_head_len, min_head_len, header_len);

    if (CheckShmStatus(pshm, &ver) != 0) {
        goto FAILED;
    }

    DEBUG_INFO("sharemeory[name=>%s] create success, item head info=>{V3=>%lu, V4_0=>%lu, V4_1=>%lu, V4=>%lu}\n", pMemoryName
               , sizeof(shm_media_item_info_v3_t), sizeof(shm_media_item_info_v40_t)
               , sizeof(shm_media_item_info_v4_subv1_t), sizeof(shm_media_item_info_v4_t));

    m_uVersion      = ver;
    m_uItemVer      = LIBSHM_MEDIA_ITEM_CURRENT_VERSION;
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

int CLibShmMediaCtx::OpenShmEntry(const char * pMemoryName, libshm_media_readcb_t cb, void *opaq)
{
    CTvuBaseShareMemory    *pshm   = NULL;
    uint32_t        ver     = 0;

    pshm    = new CTvuBaseShareMemory();

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

    DEBUG_INFO("sharemeory[name=>%s] open success, item head info=>{V3=>%lu, V4_0=>%lu, V4_1=>%lu, V4=>%lu}\n", pMemoryName
               , sizeof(shm_media_item_info_v3_t), sizeof(shm_media_item_info_v40_t)
               , sizeof(shm_media_item_info_v4_subv1_t), sizeof(shm_media_item_info_v4_t));
    m_uVersion      = ver;
    m_uItemVer      = LIBSHM_MEDIA_ITEM_CURRENT_VERSION;
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

void CLibShmMediaCtx::SetCloseFlag(bool bclose)
{
    if (!m_pShmObj)
        return;

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    libshmmediapro::setCloseFlag(m_pShmObj->GetHeader(), bclose);

#else
    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
        {

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_head_info_v2_t    *p2 = (shm_media_head_info_v2_t*)m_pShmObj->GetHeader();

            if (p2)
                p2->close_flag       = bclose ? 1 : 0;
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_head_info_v3_t    *p3 = (shm_media_head_info_v3_t*)m_pShmObj->GetHeader();

            if (p3)
                p3->v3.close_flag   = bclose ? 1 : 0;
        }
        break;
    default:
        break;
    }
#endif

}

bool CLibShmMediaCtx::CheckCloseFlag()
{
#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    return libshmmediapro::checkCloseFlag(m_pShmObj->GetHeader());
#else
    bool                bClose  = false;

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
        {

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_head_info_v2_t   *p2  = (shm_media_head_info_v2_t*)m_pShmObj->GetHeader();

            if (p2)
                bClose  = p2->close_flag ? true : false;
        }
        break;
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

uint8_t *CLibShmMediaCtx::GetItemDataAddr(uint32_t index)
{
    uint8_t         *pItem          = m_pShmObj->GetItemAddrByIndex(index);
    int             item_head_len   = 0;
    uint8_t         *pret           = NULL;

    if (!m_pShmObj->IsCreator())
    {
        return NULL;
    }

    switch(m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            item_head_len   = sizeof(shm_media_item_info_v1_v2_t);
            pret            = pItem + item_head_len;
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            item_head_len   = sizeof(shm_media_item_info_v3_t);
            pret            = pItem + item_head_len;
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            uint32_t ihead_len = libshmmediapro::getItemHeadLengthV4();
            item_head_len = libshmmediapro::getAlignOffset(pItem, ihead_len);
            pret          = pItem + item_head_len;
        }
        break;
    default:
        {
            pret = NULL;
        }
        break;
    }

    return  pret;
}

static void copy_media_head_from_param(void *h, const libshm_media_head_param_t *p, uint32_t ver)
{

    switch(ver) 
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
        {

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_head_info_v2_t    *h2 =   (shm_media_head_info_v2_t *)h;
            COPY_DATA_WHILE_NOT_EQUAL(h2->vbr, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_vbr));
            COPY_DATA_WHILE_NOT_EQUAL(h2->sarw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_sarw));
            COPY_DATA_WHILE_NOT_EQUAL(h2->sarh, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_sarh));
            COPY_DATA_WHILE_NOT_EQUAL(h2->srcw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_srcw));
            COPY_DATA_WHILE_NOT_EQUAL(h2->srch, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_srch));
            COPY_DATA_WHILE_NOT_EQUAL(h2->sdiw, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_dstw));
            COPY_DATA_WHILE_NOT_EQUAL(h2->sdih, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_dsth));
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_head_info_v3_t    *h3 = (shm_media_head_info_v3_t *)h;
            COPY_DATA_WHILE_NOT_EQUAL(h3->vbr, LIBSHMMEDIA_HEAD_WRITE_SHM_U32(p->i_vbr));
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
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
        {

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_head_info_v2_t    *h2 =   (shm_media_head_info_v2_t *)h;
            p->i_vbr        = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->vbr);
            p->i_sarw       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->sarw);
            p->i_sarh       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->sarh);
            p->i_srcw       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->srcw);
            p->i_srch       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->srch);
            p->i_dstw       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->sdiw);
            p->i_dsth       = LIBSHMMEDIA_HEAD_READ_SHM_U32(h2->sdih);
        }
        break;
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

//#define MULTIPLE_PROCESS_ATOMIC_OPT_TEST    1

int CLibShmMediaCtx::FinishWrite()
{
    m_pShmObj->FinishWrite();
#ifdef MULTIPLE_PROCESS_ATOMIC_OPT_TEST
    uint8_t *ph = m_pShmObj->GetHeader();
    uint64_t flag = *(uint64_t *)(ph + sizeof(shm_media_head_info_v4_t));

    //DEBUG_ERROR("uintptr %p, flag 0x%llx, size v4 %ld\n", (uintptr_t)(ph+sizeof(shm_media_head_info_v4_t)), flag, sizeof(shm_media_head_info_v4_t));
    if ((flag&0x00000000FFFFFFFF) && (flag&0xFFFFFFFF00000000))
    {
        DEBUG_ERROR("error, duplicated writing at the same time, uintptr %p, flag 0x%llx, size v4 %ld\n"
                    , (uintptr_t)(ph+sizeof(shm_media_head_info_v4_t)), flag, sizeof(shm_media_head_info_v4_t));
    }
#endif
    return 0;
}

int CLibShmMediaCtx::SendHead(const libshm_media_head_param_t *pmh)
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

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE

int CLibShmMediaCtx::_sendV4Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmiv, uint8_t *pItemAddr, uint32_t item_len)
{
    unsigned int size = 0;
    int         w_len = 0;
    if (!pmh || !pmiv)
    {
        DEBUG_ERROR("invalid input prt");
        return -1;
    }

    const libshmmedia_audio_channel_layout_object_t *hChannel = libshmmediapro::_headParamGetChannelLayout(*pmh);

    tvushm::BufferController_t tmpBuf;
    int  nout = 0;
    const uint8_t* pout = NULL;
    if (!(tvushm::keyValueProtoAppendToBuffer(tmpBuf, hChannel) <= 0))
    {
        pout = tmpBuf.GetOrigPtr();
        nout = tmpBuf.GetBufLength();
    }

    const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;

    size    = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen + nout;

    if (size <= 0)
    {
        DEBUG_ERROR("shm[%s] require "
            "size %u {v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]} invalid, ignore!!!\n"
            , m_pShmObj->GetName()
            , size
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
        );
        return 0;
    }

    size += libshmmediapro::preRequireItemHeadLength(kLibShmMediaItemVerV4);

    if (size > item_len)
    {
        DEBUG_ERROR("shm[%s] item length [%u] "
            "< size %u{v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d] +h[%u]}\n"
            , m_pShmObj->GetName()
            , item_len
            , size
            , pmi->i_vLen
            , pmi->i_aLen
            , pmi->i_sLen
            , pmi->i_CCLen
            , pmi->i_timeCode
            , pmi->i_userDataLen
            , nout
        );
        return 0;
    }

    if (!pItemAddr)
    {
        DEBUG_ERROR("shm[%s] required\n item address is null\n"
            , m_pShmObj->GetName()
        );
        return 0;
    }

    libshm_media_item_param_internal_t rii;
    {
        memset(&rii, 0, sizeof(rii));
        rii.nKeyValueSize_ = nout;
        rii.pKeyValuePtr_ = pout;
    }

    {
        w_len = libshmmediapro::writeItemBufferV4(pmh, pmiv, rii, pItemAddr);
    }

    FinishWrite();
    return w_len;
}

#else
int CLibShmMediaCtx::_sendV4Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi, uint8_t *pItemAddr, uint32_t item_len)
{
    bool        bvc = !(pmi->u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG);
    bool        bac = !(pmi->u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG);
    bool        bsc = !(pmi->u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG);
    bool        bec = !(pmi->u_copied_flags & LIBSHM_MEDIA_EXT_COPIED_FLAG);

    shm_media_item_info_v4_t    *pi4 = NULL;
    int         item_head_len = 0;
    int         item_w_offset = 0;
    uint64_t    size = 0;
    int         w_len = 0;
    int64_t     now = _tvuutil_get_sys_ms64();


    item_head_len = sizeof(shm_media_item_info_v4_t);

    item_w_offset += item_head_len;

    size = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

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

	if (size > item_len)
	{
		DEBUG_ERROR("shm[%s] item length [%u] "
			"< {h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]}\n"
			, m_pShmObj->GetName()
			, item_len
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

    memset((void *)pItemAddr, 0, sizeof(shm_media_item_info_v4_t));
    pi4 = (shm_media_item_info_v4_t *)pItemAddr;

    shm_media_item_info_v3_t *pi3 = &pi4->_v3i;

    pi4->_head_len = item_head_len;

    {
        uint32_t _offset = 0;
        pi3->data_ct = pmi->i64_userDataCT;
        pi3->data_type = pmi->i_userDataType;
        pi3->pic_flag = pmi->u_picType;

        _offset = libshmmediapro::getAlignOffset(pItemAddr, item_w_offset);
        item_w_offset = _offset;
        pi4->_v_data_offset = _offset;
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

            pi3->frametype = pmi->u_frameType;
            pi3->interlace_flag = pmi->i_interlaceFlag;
            pi3->videoct = now;
            pi3->videopts = pmi->i64_vpts;
            pi3->videodts = pmi->i64_vdts;
            pi3->vfourcc = pmh->u_videofourcc;
            COPY_DATA_WHILE_NOT_EQUAL(pi3->width, pmh->i_dstw);
            COPY_DATA_WHILE_NOT_EQUAL(pi3->height, pmh->i_dsth);
            COPY_DATA_WHILE_NOT_EQUAL(pi3->duration, pmh->i_duration);
            COPY_DATA_WHILE_NOT_EQUAL(pi3->scale, pmh->i_scale);

            pi3->videolen = pmi->i_vLen;
            if (bvc) {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_vData, pmi->i_vLen);
            }
            item_w_offset += pmi->i_vLen;
        }

        _offset = libshmmediapro::getAlignOffset(pItemAddr, item_w_offset);
        item_w_offset = _offset;
        pi4->_a_data_offset = _offset;
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
            pi3->audioct = now;
            pi3->audiopts = pmi->i64_apts;
            pi3->audiodts = pmi->i64_adts;
            pi3->afourcc = pmh->u_audiofourcc;
            pi3->channels = pmh->i_channels;
            pi3->depth = pmh->i_depth;
            pi3->samplerate = pmh->i_samplerate;
            {
                pi3->audiolen = pmi->i_aLen;
                if (bac) {
                    _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_aData, pmi->i_aLen);
                }

                if (pmi->h_media_process)
                {
                    pmi->h_media_process((uint8_t *)(pItemAddr + item_w_offset), pmi->i_aLen, pmi->p_opaq);
                }
                item_w_offset += pmi->i_aLen;
            }
        }

        pi4->_s_data_offset = item_w_offset;
        if (pmi->i_sLen > 0) {
            pi3->subtct = now;
            pi3->subtpts = pmi->i64_spts;
            pi3->subtdts = pmi->i64_sdts;

            pi3->subtlen = pmi->i_sLen;
            if (bsc) {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_sData, pmi->i_sLen);
            }
            item_w_offset += pmi->i_sLen;
        }

        pi4->_cc_data_offset = item_w_offset;
        if (pmi->i_CCLen > 0)
        {
            pi3->cclen = pmi->i_CCLen;
            if (bec)
            {
                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_CCData, pmi->i_CCLen);
            }
            item_w_offset += pmi->i_CCLen;
        }

        pi4->_timecode_data_offset = item_w_offset;
        if (pmi->i_timeCode > 0)
        {
            pi3->timecodelen = pmi->i_timeCode;
            _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_timeCode, pmi->i_timeCode);
            item_w_offset += pmi->i_timeCode;
        }

        pi4->_user_data_offset = item_w_offset;
        if (pmi->i_userDataLen > 0)
        {
            pi3->datalen = pmi->i_userDataLen;
            _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_userData, pmi->i_userDataLen);
            item_w_offset += pmi->i_userDataLen;
        }

        pi3->length = item_w_offset;
        w_len = item_w_offset;
        FinishWrite();
    }
    return w_len;
}
#endif

int CLibShmMediaCtx::SendData(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmiv)
{
    const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
    uint8_t     *pItemAddr  = m_pShmObj->GetWriteItemAddr();
    uint32_t    item_len    = m_pShmObj->GetItemLength();
    int         item_head_len   = 0;
    int         item_w_offset   = 0;
    bool        bvc         = !(pmi->u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG);
    bool        bac         = !(pmi->u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG);
    bool        bsc         = !(pmi->u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG);
    bool        bec         = !(pmi->u_copied_flags & LIBSHM_MEDIA_EXT_COPIED_FLAG);
    unsigned int size       = 0;
    int         w_len       = 0;
    int64_t     now         = 0;

    now = _libshm_get_sys_ms64();

    SendHead(pmh);

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_item_info_v1_v2_t    *pi12 = (shm_media_item_info_v1_v2_t *)pItemAddr;
            memset((void *)pItemAddr, 0, sizeof(shm_media_item_info_v1_v2_t));

            item_head_len = sizeof(shm_media_item_info_v1_v2_t);
            item_w_offset = item_head_len;

            size = pmi->i_vLen + pmi->i_aLen + pmi->i_CCLen;
            if (size > 0)
            {
                if (size + item_head_len > item_len)
                {
                    DEBUG_ERROR("shm[%s] item length [%u] "
                        "< h[%d] + v[%d] + a[%d] + e[%d]\n"
                        , m_pShmObj->GetName()
                        , item_len
                        , item_head_len
                        , pmi->i_vLen
                        , pmi->i_aLen
                        , pmi->i_CCLen
                    );
                    w_len = -1;
                }
                else
                {
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
                        pi12->videofourcc = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->u_videofourcc);
                        pi12->frametype = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->u_frameType);
                        pi12->videocreatetime = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
                        pi12->videopts = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vpts);
                        pi12->videodts = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vdts);
                        {
                            uint32_t dstw = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dstw);
                            uint32_t dsth = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dsth);
                            uint32_t duration = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_duration);
                            uint32_t scale = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_scale);
                            COPY_DATA_WHILE_NOT_EQUAL(pi12->width, (int)dstw);
                            COPY_DATA_WHILE_NOT_EQUAL(pi12->height, (int)dsth);
                            COPY_DATA_WHILE_NOT_EQUAL(pi12->duration, (int)duration);
                            COPY_DATA_WHILE_NOT_EQUAL(pi12->scale, (int)scale);
                        }

                        pi12->videolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_vLen);
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
                        pi12->audiofourcc = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->u_audiofourcc);
                        pi12->audiocreatetime = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
                        pi12->audiopts = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_apts);
                        pi12->audiodts = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_adts);
                        pi12->channels = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_channels);
                        pi12->depth = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_depth);
                        pi12->samplerate = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_samplerate);
                        {
                            pi12->audiolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_aLen);
                            if (bac) {
                                _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_aData, pmi->i_aLen);
                            }

                            if (pmi->h_media_process)
                            {
                                pmi->h_media_process((uint8_t *)(pItemAddr + item_w_offset), pmi->i_aLen, pmi->p_opaq);
                            }
                            item_w_offset += pmi->i_aLen;
                        }
                    }

                    if (pmi->i_CCLen > 0)
                    {
                        if (bec)
                        {
                            _libshm_memcpy((void *)(pItemAddr + item_w_offset), pmi->p_CCData, pmi->i_CCLen);
                        }
                        pi12->extralength = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_CCLen);
                        item_w_offset += pmi->i_CCLen;
                    }

                    pi12->length = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(item_w_offset);
                    w_len = item_w_offset;
                    FinishWrite();
                }
            }
            else
            {
                DEBUG_WARN("shm write size [%d] invalid\n", size);
                w_len = size;
            }
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;
            memset((void *)pItemAddr, 0, sizeof(shm_media_item_info_v3_t));

            item_head_len   = sizeof(shm_media_item_info_v3_t);
            item_w_offset   = item_head_len;

            size    = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;
            if (size > 0)
            {
                if (size + item_head_len > item_len)
                {
                    DEBUG_ERROR("shm[%s] item length [%u] "
                        "< h[%d] + v[%d] + a[%d] + s[%d] + e[%d] + t[%d] + u[%d]\n"
                        , m_pShmObj->GetName()
                        , item_len
                        , item_head_len
                        , pmi->i_vLen
                        , pmi->i_aLen
                        , pmi->i_sLen
                        , pmi->i_CCLen
                        , pmi->i_timeCode
                        , pmi->i_userDataLen
                    );
                    w_len = -1;
                }
                else
                {
                    //pi3->length                = sizeof(shm_media_item_info_v3_t) + size;
                    //pi3->videolen              = pmi->i_vLen;
                    //pi3->audiolen              = pmi->i_aLen;
                    //pi3->subtlen               = pmi->i_sLen;
                    //pi3->cclen                 = pmi->i_CCLen;
                    //pi3->timecodelen           = pmi->i_timeCode;
                    //pi3->datalen               = pmi->i_userDataLen;
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
                        {
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
                    FinishWrite();
                }
            }
            else
            {
                DEBUG_WARN("shm write size [%d] invalid\n", size);
                w_len = size;
            }
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            w_len = _sendV4Data(pmh, pmiv, pItemAddr, item_len);
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

int CLibShmMediaCtx::SendDataWithFrequency1000(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi)
{
    int64_t now     = _libshm_get_sys_ms64();

#if !defined(TVU_WINDOWS)
/**
  * TODO, we need optimize the flow control logic.
  * Why windows hide it, for some windows pc, socket sleep 1 could have 15-16ms in fact,
  * then it would trigger the write shm slowly.
**/
    if (now == m_i64LastSendSysTime)
    {
        _libshm_common_usleep(1000);
        now = _libshm_get_sys_ms64();
    }
    m_i64LastSendSysTime = now;
#endif //TVU_WINDOWS

    return SendData(pmh, pmi);
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
    uint32_t size = libshmmediapro::_headParamGetMinStructSize(*from, *to);
    if (memcmp(from, to, size)) {
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

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
/**
 *  Return:
 *      < 0 : failed,
 *      0   : wait,
 *      > 0 : success
 */
int CLibShmMediaCtx::PollReadHead(libshm_media_head_param_t *pmh, unsigned int timeout)
{
    int         ret         = 1;

    ret = PollReadable(timeout);

    if (ret < 0) {
        DEBUG_ERROR("poll read timeout %d, ret %d\n", timeout, ret);
    }
    if (ret <= 0) {
        return ret;
    }

    uint32_t    r_index     = m_pShmObj->GetReadIndex();
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(r_index);

    unsigned int item_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);

    ret = libshmmediapro::readHeadFromItemBuffer(pmh, pItemAddr, item_len);

    return ret;
}

int CLibShmMediaCtx::PollReadDataWithoutIndexStep(
    libshm_media_head_param_t *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
    , unsigned int timeout)
{
    uint32_t    read_index  = m_pShmObj->GetReadIndex();
    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    ret = ReadItemData(pmh, pmi, pext, read_index);

    if (ret < 0)
    {
        DEBUG_WARN_CR("parse item data failed."
                    "ret:%d", ret);
        ret = 0;
    }
    return ret;
}

int CLibShmMediaCtx::PollReadDateWithTvutimestamp(
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi
    , libshmmedia_extend_data_info_t *pext
    , bool *bFoundTvutimestamp
    , bool *bFoundPts
    , uint64_t tvutimestamp
    , char type, uint64_t pts
    )
{
    if (!pmh || !pmi || !bFoundTvutimestamp || !bFoundPts)
    {
        return -1;
    }

    int ret     = PollReadable(0);

    if (ret <= 0) {
        return ret;
    }

    ResultRecorder firstItem;
    ResultRecorder matchintItem;
    bool bGotMatchingItem = SearchTheFirstMatchingItemWithTvutimestamp(tvutimestamp, matchintItem);
    *bFoundTvutimestamp = bGotMatchingItem;

    if (bGotMatchingItem)
    {
        DEBUG_INFO_CR(
            "It found the item data matching tvutimestamp."
            "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
            , GetName()
            , matchintItem.itemIdx
            , tvutimestamp
            , matchintItem.GetItem().GetTvutimestamp()
            , LibshmutilTvutimestampMinusWithMS(matchintItem.GetItem().GetTvutimestamp(), tvutimestamp)
            );
        ret = matchintItem.GetItem().GetReadingRet();
        *pmh = matchintItem.GetItem().curHead_;
        *pmi = matchintItem.GetItem().curItem_;
        if (pext)
        {
            *pext  = matchintItem.GetItem().curExt_;
        }
        SetRIndex(matchintItem.itemIdx+1);
        return ret;
    }

    bGotMatchingItem = SearchTheFirstMatchingItemWithPts(pts, matchintItem);
    *bFoundPts = bGotMatchingItem;
    if (bGotMatchingItem)
    {
        DEBUG_INFO_CR(
            "It found the item data matching pts."
            "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
            , GetName()
            , matchintItem.itemIdx
            , pts
            , matchintItem.GetItem().GetPts()
            , (matchintItem.GetItem().GetPts() - pts)
            );
        ret = matchintItem.GetItem().GetReadingRet();
        *pmh = matchintItem.GetItem().curHead_;
        *pmi = matchintItem.GetItem().curItem_;
        if (pext)
        {
            *pext  = matchintItem.GetItem().curExt_;
        }
        SetRIndex(matchintItem.itemIdx+1);
        return ret;
    }

    //if (!bGotMatchingItem)
    {
        ret = _readOutFirstItemInfor(firstItem);
        if (ret <= 0)
        {
            DEBUG_ERROR_CR("It was impossible that it did not get the first item data here."
                "ret:%d"
                , ret
            );
            return ret;
        }

        DEBUG_WARN_CR("It did not find matching item data, just responsed the first item."
            "ret:%d"
            ",idx:%u"
            , ret
            , firstItem.itemIdx
        );
        *pmh = firstItem.GetItem().curHead_;
        *pmi = firstItem.GetItem().curItem_;
        if (pext)
        {
            *pext  = firstItem.GetItem().curExt_;
        }
        SetRIndex(firstItem.itemIdx+1);
    }

    return ret;
}

int CLibShmMediaCtx::ReadItemData(
    libshm_media_head_param_t *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
    , unsigned int rindex)
{
    uint32_t    read_index  = rindex;
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(read_index);
    int         r_len       = -1;

    if (!pmh)
    {
        DEBUG_ERROR("media head ptr was null, invalid");
        return -1;
    }

    if (!pmi)
    {
        DEBUG_ERROR("media item ptr was null, invalid");
        return -1;
    }

    libshm_media_item_param_t   oipv;
    {
        memset(&oipv, 0, sizeof(oipv));
    }

    libshm_media_head_param_t   ohp;
    {
        LibShmMediaHeadParamInit(&ohp, sizeof(ohp));
        ohp.h_channel = libshmmediapro::_headParamGetChannelLayout(*pmh);
    }

    unsigned int buffer_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);

    r_len = libshmmediapro::readDataFromItemBuffer(&ohp, &oipv, pItemAddr, buffer_len);

    libshm_media_item_param_t &oip = oipv;

    if (r_len > 0)
    {
        oip.u_read_index = read_index;
        if (libshmmediapro::_headParamLowCompare(*pmh, ohp))
        {
            libshmmediapro::_headParamLowCopy(*pmh, ohp);
            //return -1;
        }

        *pmi = oipv;
        if (m_fnReadCb) {
            int ret   = m_fnReadCb(m_pOpaq, &oipv);
            if (ret < 0) {
                DEBUG_ERROR("read callback return %d invalid", ret);
            }
        }

        if (pext)
        {
            if (oipv.i_userDataLen > 0 && oipv.p_userData)
            {
                LibShmMeidaParseExtendDataV2(pext, oipv.p_userData, oipv.i_userDataLen);
            }

        }
    }

    return r_len;
}

int CLibShmMediaCtx::ReadItemData2(
    unsigned int rindex
    , libshm_media_head_param_t &mh
    , libshm_media_item_param_t &mi
    , libshmmedia_extend_data_info_t &ext)
{
    uint32_t    read_index  = rindex;
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(read_index);
    int         r_len       = -1;

    unsigned int buffer_len = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);

    r_len = libshmmediapro::readDataFromItemBuffer(&mh, &mi, pItemAddr, buffer_len);

    if (r_len > 0 && mi.i_userDataLen > 0 && mi.p_userData)
    {
        LibShmMeidaParseExtendDataV2(&ext, mi.p_userData, mi.i_userDataLen);
    }

    return r_len;
}

#else
/**
 *  Return:
 *      < 0 : failed,
 *      0   : wait,
 *      > 0 : success
 */
int CLibShmMediaCtx::PollReadHead(libshm_media_head_param_t *pmh, bool bCheckReadable, unsigned int timeout)
{
    uint8_t     *pHead      = m_pShmObj->GetHeader();
    uint32_t    r_index     = m_pShmObj->GetReadIndex();
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(r_index);
    int         ret         = 1;
    libshm_media_head_param_t  oh  = {0};

    if (bCheckReadable) {
        ret = PollReadable(timeout);

        if (ret < 0) {
            DEBUG_ERROR("poll read timeout %d\n", timeout);
            return ret;
        }
    }

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_item_info_v1_v2_t    *pi12= (shm_media_item_info_v1_v2_t *)pItemAddr;

            if (m_uVersion == LIBSHM_MEDIA_HEAD_VERSION_V2) {
                shm_media_head_info_v2_t    *ph2    = (shm_media_head_info_v2_t *)pHead;
                copy_media_head_to_param(&oh, (void *)ph2, LIBSHM_MEDIA_HEAD_VERSION_V2);
            }

            //if (!pi12->width) {
            //    DEBUG_ERROR("%dx%d\n", pi12->width, pi12->height);
            //}
            (&oh)->i_dstw         = pi12->width;
            (&oh)->i_dsth         = pi12->height;
            (&oh)->u_videofourcc  = pi12->videofourcc;
            (&oh)->i_duration     = pi12->duration;
            (&oh)->i_scale        = pi12->scale;
            (&oh)->u_audiofourcc  = pi12->audiofourcc;
            (&oh)->i_channels     = pi12->channels;
            (&oh)->i_depth        = pi12->depth;
            (&oh)->i_samplerate   = pi12->samplerate;
        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;
            shm_media_head_info_v3_t    *ph3    = (shm_media_head_info_v3_t *)pHead;

            copy_media_head_to_param(&oh, (void *)ph3, LIBSHM_MEDIA_HEAD_VERSION_V3);

            (&oh)->i_dstw         = pi3->width;
            (&oh)->i_dsth         = pi3->height;
            (&oh)->u_videofourcc  = pi3->vfourcc;
            (&oh)->i_duration     = pi3->duration;
            (&oh)->i_scale        = pi3->scale;
            (&oh)->u_audiofourcc  = pi3->afourcc;
            (&oh)->i_channels     = pi3->channels;
            (&oh)->i_depth        = pi3->depth;
            (&oh)->i_samplerate   = pi3->samplerate;
        }
        break;
    default:
        {
            ret   = -1;
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }

    if (ret >= 0)
        *pmh    = oh;

    return ret;
}

int CLibShmMediaCtx::_readV4Data(libshm_media_item_param_t   *pmi, const uint32_t rindex, const uint8_t *pItemAddr)
{
    int         r_len = -1;
    const shm_media_item_info_v3_t    *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
    const shm_media_item_info_v4_t    *pi4 = (const shm_media_item_info_v4_t *)pItemAddr;
    int         item_v_offset = 0;
    int         item_a_offset = 0;
    int         item_s_offset = 0;
    int         item_e_offset = 0;
    int         item_timecode_offset = 0;
    int         item_data_offset = 0;

    pmi->i_totalLen = pi3->length;
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

    pmi->u_read_index = rindex;
    r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

    return r_len;
}

int CLibShmMediaCtx::PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, unsigned int timeout)
{
    uint32_t    read_index  = m_pShmObj->GetReadIndex();
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(read_index);
    int         item_head_len   = 0;
    int         item_v_offset   = 0;
    int         item_a_offset   = 0;
    int         item_s_offset   = 0;
    int         item_e_offset   = 0;
    int         item_timecode_offset   = 0;
    int         item_data_offset = 0;
    int         r_len       = -1;
    libshm_media_item_param_t   oip = {0};
    libshm_media_head_param_t   ohp = {0};

    int         ret     = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

    if (PollReadHead(&ohp, false, timeout) <= 0) {
        return -1;
    }

    if (_media_head_cmp(pmh, &ohp)) {
        *pmh    = ohp;
        //return -1;
    }

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_item_info_v1_v2_t     *pi12 = (shm_media_item_info_v1_v2_t *)pItemAddr;

            oip.i_totalLen = pi12->length;
            item_head_len = sizeof(shm_media_item_info_v1_v2_t);
            item_v_offset = item_head_len;
            item_a_offset = item_v_offset + pi12->videolen;
            item_e_offset = item_a_offset + pi12->audiolen;

            if (pi12->videolen > 0) {
                oip.p_vData = pItemAddr + item_v_offset;
                oip.i_vLen = pi12->videolen;
            }
            oip.i64_vpts = pi12->videopts;
            oip.i64_vdts = pi12->videodts;
            oip.i64_vct = pi12->videocreatetime;
            oip.u_frameType = pi12->frametype;

            if (pi12->audiolen > 0) {
                oip.p_aData = pItemAddr + item_a_offset;
                oip.i_aLen = pi12->audiolen;
            }
            oip.i64_apts = pi12->audiopts;
            oip.i64_adts = pi12->audiodts;
            oip.i64_act = pi12->audiocreatetime;

            if (pi12->extralength > 0) {
                oip.p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
                oip.i_CCLen = pi12->extralength;
            }

            if (m_fnReadCb) {
                r_len = m_fnReadCb(m_pOpaq, &oip);
                if (r_len < 0) {
                    DEBUG_ERROR("read callback return %d invalid");
                }
            }

            oip.u_read_index = read_index;
            r_len = oip.i_vLen + oip.i_aLen + oip.i_CCLen;

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;

            oip.i_totalLen  = pi3->length;
            item_head_len   = sizeof(shm_media_item_info_v3_t);
            item_v_offset   = item_head_len;
            item_a_offset   = item_v_offset + pi3->videolen;
            item_s_offset   = item_a_offset + pi3->audiolen;
            item_e_offset   = item_s_offset + pi3->subtlen;
            item_timecode_offset = item_e_offset + pi3->cclen;
            item_data_offset = item_timecode_offset + pi3->timecodelen;

            oip.u_picType = pi3->pic_flag;
            if (pi3->videolen > 0) {
                oip.p_vData = pItemAddr + item_v_offset;
                oip.i_vLen = pi3->videolen;
            }
            oip.i64_vpts = pi3->videopts;
            oip.i64_vdts = pi3->videodts;
            oip.i64_vct = pi3->videoct;
            oip.u_frameType = pi3->frametype;
            oip.i_interlaceFlag = pi3->interlace_flag;

            if (pi3->audiolen > 0) {
                oip.p_aData = pItemAddr + item_a_offset;
                oip.i_aLen = pi3->audiolen;
            }
            oip.i64_apts = pi3->audiopts;
            oip.i64_adts = pi3->audiodts;
            oip.i64_act = pi3->audioct;

            if (pi3->subtlen > 0) {
                oip.p_sData = pItemAddr + item_s_offset;
                oip.i_sLen = pi3->subtlen;
            }
            oip.i64_spts = pi3->subtpts;
            oip.i64_sdts = pi3->subtdts;
            oip.i64_sct = pi3->subtct;

            if (pi3->cclen > 0) {
                oip.p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
                oip.i_CCLen = pi3->cclen;
            }

            if (pi3->timecodelen > 0) {
                oip.p_timeCode = (const uint8_t *)(pItemAddr + item_timecode_offset);
                oip.i_timeCode = pi3->timecodelen;
            }

            if (pi3->datalen > 0) {
                oip.p_userData = (const uint8_t *)(pItemAddr + item_data_offset);
                oip.i_userDataLen = pi3->datalen;
            }
            oip.i64_userDataCT = pi3->data_ct;
            oip.i_userDataType = pi3->data_type;

            if (m_fnReadCb) {
                r_len   = m_fnReadCb(m_pOpaq, &oip);
                if (r_len < 0) {
                    DEBUG_ERROR("read callback return %d invalid");
                }
            }

            oip.u_read_index    = read_index;
            r_len   = oip.i_vLen + oip.i_aLen + oip.i_sLen + oip.i_CCLen + oip.i_timeCode + oip.i_userDataLen;
        }
        break;
        case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            r_len = _readV4Data(&oip, read_index, pItemAddr);
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

int CLibShmMediaCtx::FinishRead()
{
    m_pShmObj->FinishRead();

#ifdef MULTIPLE_PROCESS_ATOMIC_OPT_TEST
    uint8_t *ph = m_pShmObj->GetHeader();
    uint64_t    *pi = (uint64_t *)(ph + sizeof(shm_media_head_info_v4_t));
    static char *penv = getenv("TEST_ODD");
    static uint32_t index = 1;

    if(penv)
    {
        *pi = ((uint64_t)index << 32);
    }
    else
    {
        *pi = index;
    }

    index++;
#endif
    return 0;
}

int CLibShmMediaCtx::PollReadData(
    libshm_media_head_param_t *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
    , unsigned int timeout)
{
    int ret = PollReadDataWithoutIndexStep(pmh, pmi, pext, timeout);

    if (ret > 0)
    {
        FinishRead();
    }
    return ret;
}

uint32_t CLibShmMediaCtx::SetReadIndex(char type, int64_t pts)
{
    uint32_t    windex  = GetWIndex();
    uint32_t    counts  = GetItemCounts();
    uint32_t    rindex  = windex >= counts ? (windex - counts + 1) : 0;
    SetRIndex(rindex);
    
    for (; (int)(windex - rindex) > 0; rindex++)
    {
        libshm_media_head_param_t   oh  = {0};
        libshm_media_item_param_t   opv  = {0};
        int ret = ReadItemData(&oh, &opv, NULL, rindex);
        int64_t c_pts   = 0;

        if (ret <= 0) {
            rindex  = windex;
            break;
        }

        libshm_media_item_param_t &op = opv;

        c_pts = LibShmMediaItemParamGetPts(&op, type);

        if (c_pts <= 0) {
            rindex  = windex;
            break;
        }

        if (c_pts >= pts) {
            //rindex  = (rindex > 0) ? (rindex - 1) : 0;
            break;
        }
    }

    SetRIndex(rindex);
    return rindex;
}

static bool isSameSign(int a, int b) {
    return (a ^ b) >= 0;
}

int CLibShmMediaCtx::_readOutItemInfor(
    uint32_t rindex
    ,libshm_media_head_param_t &oh
    ,libshm_media_item_param_t &op
    ,libshmmedia_extend_data_info_t &ext
    ,bool &gotTvutimestamp
    ,uint64_t &tvutimestamp
    )
{
    gotTvutimestamp = false;
    const int ret = ReadItemData2(rindex, oh, op, ext);
    if (ret <= 0)
    {
        DEBUG_ERROR_CR(
            "It can not get the item data."
            "nm:%s,inx:%u"
            , GetName()
            , rindex
            );
        return ret;
    }

    if (!ext.i_timecode_fps_index
     || !ext.p_timecode_fps_index
     || (ext.i_timecode_fps_index != sizeof(uint64_t))
     )
    {
        DEBUG_WARN_CR(
            "the shm did not have tvutimestamp."
            "nm:%s,inx:%u"
            , GetName()
            , rindex
            );
        return ret;
    }

    uint64_t fpsTimecode = LIBSHMMEDIA_ITEM_READ_SHM_U64(*(uint64_t*)ext.p_timecode_fps_index);
    if (!LibshmutilTvutimestampValid(fpsTimecode))
    {
        DEBUG_WARN_CR(
            "the shm did not get the valid tvutimestamp."
            "nm:%s,inx:%u,tm:0x%" PRIx64 ""
            , GetName()
            , rindex
            , fpsTimecode
            );
        return ret;
    }

    gotTvutimestamp = true;
    tvutimestamp = fpsTimecode;
    return ret;
}

const tvushm::ItemInfo&
CLibShmMediaCtx::_readOutItemInfor(uint32_t rindex)
{
    uint32_t counts = GetItemCounts();
    uint32_t itemIdx = rindex % counts;

    if (_itemNodes.capacity() <counts)
    {
        _itemNodes.resize(counts);
    }

    tvushm::ItemInfo &item = _itemNodes[itemIdx];

    if (item.IsReadingSuccess(rindex))
    {
        return item;
    }

    item.Init();

    libshm_media_head_param_t &oh = item.curHead_;
    libshm_media_item_param_t &op = item.curItem_;
    libshmmedia_extend_data_info_t &ext = item.curExt_;

    const int ret = ReadItemData2(rindex, oh, op, ext);
    item.readRet_ = ret;
    item.itemIdex_ = rindex;

    if (ret <= 0)
    {
        DEBUG_ERROR_CR(
            "It can not get the item data."
            "nm:%s,inx:%u"
            , GetName()
            , rindex
            );
        return item;
    }

    item.pts_ = LibShmMediaItemParamGetPts(&op, 0);

    if (!ext.bGotTvutimestamp)
    {
        DEBUG_WARN_CR(
            "the shm did not have tvutimestamp."
            "nm:%s,inx:%u"
            , GetName()
            , rindex
            );
        return item;
    }

    item.bGotTvutimestamp_ = ext.bGotTvutimestamp;
    item.tvutimestamp_ = ext.u64Tvutimestamp;
    return item;
}

int CLibShmMediaCtx::_readOutItemInfor(uint32_t rindex, ResultRecorder &rec)
{
    const tvushm::ItemInfo &item = _readOutItemInfor(rindex);
    const int ret = item.GetReadingRet();
    rec.itemIdx = rindex;
    rec.pItem = &item;
    return ret;
}

int CLibShmMediaCtx::_readOutFirstItemInfor(ResultRecorder &rec)
{
    uint32_t rindex = GetRIndex();
    return _readOutItemInfor(rindex, rec);
}

/**
 * return the reading status.
 **/
int CLibShmMediaCtx::_cmpMatchingTvutimestamp(
    uint32_t rindex
    , const uint64_t &tvutimestamp
    , ResultRecorder &rec
    )
{
    const int ret = _readOutItemInfor(rindex, rec);
    const tvushm::ItemInfo &item = *(rec.pItem);

    if (!item.IsReadingSuccess(rindex))
    {
        return ret;
    }

    if (item.HasGottenTvutimestamp(rindex))
    {
        rec.cmpRet = LibshmutilTvutimestampCompare(item.GetTvutimestamp(), tvutimestamp);
    }

    return ret;
}

static int _cmpUint64(uint64_t v1, uint64_t v2)
{
    int64_t d = v1 - v2;
    if (d > 0)
    {
        return 1;
    }
    else if (d < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int CLibShmMediaCtx::_cmpMatchingPts(
    uint32_t rindex
    , const uint64_t &pts
    , ResultRecorder &rec
    )
{
    const int ret = _readOutItemInfor(rindex, rec);
    const tvushm::ItemInfo &item = *(rec.pItem);

    if (!item.IsReadingSuccess(rindex))
    {
        return ret;
    }

    if (item.HasGottenPts(rindex))
    {
        rec.cmpRet = _cmpUint64(item.GetPts(), pts);
    }

    return ret;
}

bool CLibShmMediaCtx::SearchItemWithTvutimestamp(
    const uint64_t &tvutimestamp
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , libshmmedia_extend_data_info_t *pext
    )
{
    uint32_t    windex  = GetWIndex();
    //uint32_t    counts  = GetItemCounts();
    uint32_t    rindex  = GetRIndex();//windex >= counts ? (windex - counts + 1) : 0;
    int32_t     gapCnt  = windex - rindex;

    if (!LibshmutilTvutimestampValid(tvutimestamp))
    {
        DEBUG_WARN_CR(
            "invalid tvutimestamp input."
            "nm:%s,tm:0x%" PRIx64 ""
            , GetName()
            , tvutimestamp
            );
        return false;
    }

    if (gapCnt <= 0)
    {
        return false;
    }

    bool bGotMatching = false;
    uint32_t matchingIdx = 0;
    uint64_t matchingTvustamp = 0;

    int64_t beginTime = _libshm_get_sys_ms64();

    do {
        uint32_t startInx = rindex;
        uint32_t endInx = windex - 1;

        // in the rage
        uint32_t left = startInx;
        uint32_t right = endInx ;

        while ((int32_t)(left - right) <= 0) {
            uint32_t mid = left + (right - left) / 2;
            ResultRecorder result;
            _cmpMatchingTvutimestamp(mid, tvutimestamp, result);
            const tvushm::ItemInfo &item = *(result.pItem);
            if (!item.IsReadingSuccess(mid))
            {
                DEBUG_ERROR_CR(
                    "It can not open the item data for searching tvutimestamp."
                    "nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
                    , GetName()
                    , mid
                    , tvutimestamp
                    );
                break;
            }

            if (!item.HasGottenTvutimestamp(mid))
            {
                DEBUG_WARN_CR(
                    "It did not get the tvutimestamp from the item data for searching tvutimestamp."
                    "nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
                    , GetName()
                    , mid
                    , tvutimestamp
                    );
                break;
            }

            if (result.cmpRet == 0) {
                matchingTvustamp = item.GetTvutimestamp();
                matchingIdx = mid;
                if (pmh)
                    *pmh = result.GetItem().curHead_;
                if (pmi)
                    *pmi = result.GetItem().curItem_;
                if (pext)
                    *pext = result.GetItem().curExt_;
                bGotMatching = true;
                break;
            } else if (result.cmpRet < 0) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    } while(0);

    int64_t diffTime = _libshm_get_sys_ms64() - beginTime;
    if (bGotMatching)
    {
        if (diffTime <= 5)
        {
            DEBUG_INFO_CR(
                "It found the item data for searching tvutimestamp."
                "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
                ",tm:%" PRId64 ""
                , GetName()
                , matchingIdx
                , tvutimestamp
                , matchingTvustamp
                , LibshmutilTvutimestampMinusWithMS(matchingTvustamp, tvutimestamp)
                , diffTime
            );
        }
        else
        {
            DEBUG_WARN_CR(
                "It found the item data for searching tvutimestamp."
                "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
                ",tm:%" PRId64 ""
                , GetName()
                , matchingIdx
                , tvutimestamp
                , matchingTvustamp
                , LibshmutilTvutimestampMinusWithMS(matchingTvustamp, tvutimestamp)
                , diffTime
            );
        }
    }
    else
    {
        DEBUG_WARN_CR(
            "It did not find the item data for searching tvutimestamp."
            "nm:%s,expect:0x%" PRIx64 ""
            ",tm:%" PRId64 ""
            , GetName()
            , tvutimestamp
            , diffTime
        );
    }

    return bGotMatching;
}

// template <typename TimeValidFun, typename cmpFun
//          , typename GetTimeVal_t
//          , typename HasGotTimeVal_t
//          >
// bool CLibShmMediaCtx::TemplateSearchFirstItemMatching(
//     const uint64_t &tvutimestamp, ResultRecorder &matchingItem
//     , TimeValidFun fnTimeValid, cmpFun fnCmp
//     , GetTimeVal_t fnGetTime
//     , HasGotTimeVal_t fnHasGotTime
//     )
bool CLibShmMediaCtx::SearchFirstItemMatching(
    const uint64_t &tvutimestamp
    , ResultRecorder &matchingItem, FnTimestampValid_t fnTimeValid, FnCmpFetchingItem_t fnCmp
    , FnGetItemTimeVal_t fnGetTime
    , FnHasGottenTimeVal_t fnHasGotTime
    , FnTimestampValMinus_t fnMinus
    , const char *module
)
{
    uint32_t    windex  = GetWIndex();
    //uint32_t    counts  = GetItemCounts();
    uint32_t    rindex  = GetRIndex();//windex >= counts ? (windex - counts + 1) : 0;
    int32_t     gapCnt  = windex - rindex;

    if (gapCnt <= 0)
    {
        DEBUG_WARN_CR(
            "no items were ready for reading."
            "at:%s,nm:%s,tm:0x%" PRIx64 ""
            , module
            , GetName()
            , tvutimestamp
            );
        return false;
    }

    if (!fnTimeValid(tvutimestamp))
    {
        DEBUG_WARN_CR(
            "invalid tvutimestamp input."
            "at:%s,nm:%s,tm:0x%" PRIx64 ""
            , module
            , GetName()
            , tvutimestamp
            );
        return false;
    }

    bool bGotMatching = false;
    uint64_t matchingTvustamp = 0;
    uint64_t minTvustamp = 0;
    uint64_t maxTvustamp = 0;

    int64_t beginTime = _libshm_get_sys_ms64();

    do {
        uint32_t startInx = rindex;
        uint32_t endInx = windex - 1;

        ResultRecorder lastResult;
        {
            fnCmp(startInx, tvutimestamp, lastResult);
            const tvushm::ItemInfo &item = *(lastResult.pItem);
            if (!fnHasGotTime(&item, startInx))
            {
                break;
            }

            uint64_t t0 = fnGetTime(&item);
            minTvustamp = t0;
            if (lastResult.cmpRet == 0) /* found */
            {
                matchingTvustamp = t0;
                matchingItem = lastResult;
                bGotMatching = true;
                break;
            }
            else if (lastResult.cmpRet > 0) /* out of range */
            {
                DEBUG_WARN_CR(
                    "the 1st item tvutimestamp was larger than request."
                    "at:%s, nm:%s,idx:%u,v1:0x%" PRIx64 ",v2:0x%" PRIx64 ",-:%" PRId64 "ms"
                    , module
                    , GetName()
                    , startInx
                    , tvutimestamp
                    , t0
                    , fnMinus(t0, tvutimestamp)
                    );
                break;
            }
        }

        if (endInx == startInx) /* only one item */
        {
            break;
        }

        if ((int)(endInx - startInx) < 0)
        {
            DEBUG_ERROR_CR(
                "It was impossible that end index is less than start index."
                "at:%s,nm:%s,eidx:%u,sidx:%u"
                , module
                , GetName()
                , endInx
                , startInx
                );
            break;
        }

        ResultRecorder lastResult2;
        //if (endInx != startInx)
        {
            fnCmp(endInx, tvutimestamp, lastResult2);
            const tvushm::ItemInfo &item = *(lastResult2.pItem);
            if (!fnHasGotTime(&item,endInx))
            {
                break;
            }

            uint64_t t0 = fnGetTime(&item);
            maxTvustamp = t0;
            if (lastResult2.cmpRet == 0) /* found */
            {
                matchingTvustamp = t0;
                matchingItem = lastResult2;
                bGotMatching = true;
                break;
            }
            else if (lastResult2.cmpRet < 0) /* out of range */
            {
                DEBUG_WARN_CR(
                    "the last item tvutimestamp was smaller than request."
                    "at:%s,nm:%s,idx:%u,v1:0x%" PRIx64 ",v2:0x%" PRIx64 ",-:%" PRId64 "ms"
                    , module
                    , GetName()
                    , endInx
                    , tvutimestamp
                    , t0
                    , fnMinus(t0, tvutimestamp)
                    );
                break;
            }
        }

        if (endInx - startInx <= 1)
        {
            matchingTvustamp = fnGetTime(lastResult2.pItem);
            matchingItem = lastResult2;
            bGotMatching = true;
            break;
        }

        // in the rage
        uint32_t left = startInx;
        uint32_t right = endInx ;

        while ((int32_t)(left - right) <= 0) {
            uint32_t mid = left + (right - left) / 2;
            ResultRecorder result;
            if (mid == startInx)
            {
                result = lastResult;
            }
            else if (mid == endInx)
            {
                result = lastResult2;
            }
            else
            {
                fnCmp(mid, tvutimestamp, result);
            }

            const tvushm::ItemInfo &item = *(result.pItem);
            if (!item.IsReadingSuccess(mid))
            {
                DEBUG_ERROR_CR(
                    "It can not open the item data for searching tvutimestamp."
                    "at:%s,nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
                    , module
                    , GetName()
                    , mid
                    , tvutimestamp
                    );
                break;
            }

            if (!fnHasGotTime(&item,mid))
            {
                DEBUG_WARN_CR(
                    "It did not get the tvutimestamp from the item data for searching tvutimestamp."
                    "at:%s,nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
                    , module
                    , GetName()
                    , mid
                    , tvutimestamp
                    );
                break;
            }

            if (result.cmpRet == 0) {
                matchingTvustamp = fnGetTime(&item);
                matchingItem = result;
                bGotMatching = true;
                break;
            } else if (result.cmpRet < 0) {
                left = mid + 1;
            } else {
                matchingTvustamp = fnGetTime(&item);
                matchingItem = result;
                bGotMatching = true;
                right = mid - 1;
            }
        }

        if (!bGotMatching)
        {
            DEBUG_ERROR_CR(
                "It was impossible that did not get matching item here."
                "at:%s,nm:%s,expect:0x%" PRIx64 ",min:0x%" PRIx64 ",max:0x%" PRIx64 ",-:%" PRId64 "ms"
                , module
                , GetName()
                , tvutimestamp
                , minTvustamp
                , maxTvustamp
                , fnMinus(maxTvustamp, minTvustamp)
                );
        }
    } while(0);

    int64_t diffTime = _libshm_get_sys_ms64() - beginTime;
    if (bGotMatching)
    {
        if (diffTime <= 5)
        {
            DEBUG_INFO_CR(
                "It found the item data for searching timestamp."
                "at:%s,nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
                ",tm:%" PRId64 ""
                , module
                , GetName()
                , matchingItem.itemIdx
                , tvutimestamp
                , matchingTvustamp
                , fnMinus(matchingTvustamp, tvutimestamp)
                , diffTime
                );
        }
        else
        {
            DEBUG_WARN_CR(
                "It found the item data for searching timestamp."
                "at:%s,nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
                ",tm:%" PRId64 ""
                , module
                , GetName()
                , matchingItem.itemIdx
                , tvutimestamp
                , matchingTvustamp
                , fnMinus(matchingTvustamp, tvutimestamp)
                , diffTime
                );
        }
    }
    else
    {
        DEBUG_WARN_CR(
            "It did not find the item data for searching tvutimestamp."
            "at:%s,nm:%s,expect:0x%" PRIx64 ",min:0x%" PRIx64 ", max:0x%" PRIx64 ""
            ",tm:%" PRId64 ""
            , module
            , GetName()
            , tvutimestamp
            , minTvustamp, maxTvustamp
            , diffTime
            );
    }

    return bGotMatching;
}

bool CLibShmMediaCtx::SearchTheFirstMatchingItemWithTvutimestamp(
    const uint64_t &tvutimestamp
    , ResultRecorder &matchingItem
    )
{
    FnTimestampValid_t fnPtsValid =
        std::bind(LibshmutilTvutimestampValid, std::placeholders::_1);
    FnCmpFetchingItem_t fnCmp =
        std::bind(&CLibShmMediaCtx::_cmpMatchingTvutimestamp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FnGetItemTimeVal_t fnGetTime =
        std::bind(&tvushm::ItemInfo::GetTvutimestamp, std::placeholders::_1);
    FnHasGottenTimeVal_t fnGottenTime =
        std::bind(&tvushm::ItemInfo::HasGottenTvutimestamp, std::placeholders::_1, std::placeholders::_2);
    FnTimestampValMinus_t fnMinus = std::bind(&LibshmutilTvutimestampMinusWithMS, std::placeholders::_1, std::placeholders::_2);
    return SearchFirstItemMatching(tvutimestamp, matchingItem, fnPtsValid, fnCmp, fnGetTime, fnGottenTime, fnMinus, "tvutimestamp");
    // uint32_t    windex  = GetWIndex();
    // //uint32_t    counts  = GetItemCounts();
    // uint32_t    rindex  = GetRIndex();//windex >= counts ? (windex - counts + 1) : 0;
    // int32_t     gapCnt  = windex - rindex;
    // int         ret = 0;

    // if (gapCnt <= 0)
    // {
    //    DEBUG_WARN_CR(
    //         "no items were ready for reading."
    //         "nm:%s,tm:0x%" PRIx64 ""
    //         , GetName()
    //         , tvutimestamp
    //         );
    //     return false;
    // }

    // if (!LibshmutilTvutimestampValid(tvutimestamp))
    // {
    //     DEBUG_WARN_CR(
    //         "invalid tvutimestamp input."
    //         "nm:%s,tm:0x%" PRIx64 ""
    //         , GetName()
    //         , tvutimestamp
    //         );
    //     return false;
    // }

    // bool bGotMatching = false;
    // uint64_t matchingTvustamp = 0;
    // uint64_t minTvustamp = 0;
    // uint64_t maxTvustamp = 0;

    // int64_t beginTime = _libshm_get_sys_ms64();

    // do {
    //     uint32_t startInx = rindex;
    //     uint32_t endInx = windex - 1;

    //     ResultRecorder lastResult;
    //     {
    //         _cmpMatchingTvutimestamp(startInx, tvutimestamp, lastResult);
    //         const tvushm::ItemInfo &item = *(lastResult.pItem);
    //         if (!item.HasGottenTvutimestamp(startInx))
    //         {
    //             break;
    //         }

    //         uint64_t t0 = item.GetTvutimestamp();
    //         minTvustamp = t0;
    //         if (lastResult.cmpRet == 0) /* found */
    //         {
    //             matchingTvustamp = t0;
    //             matchingItem = lastResult;
    //             bGotMatching = true;
    //             break;
    //         }
    //         else if (lastResult.cmpRet > 0) /* out of range */
    //         {
    //             DEBUG_WARN_CR(
    //                 "the 1st item tvutimestamp was larger than request."
    //                 "nm:%s,idx:%u,v1:0x%" PRIx64 ",v2:0x%" PRIx64 ",-:%" PRId64 "ms"
    //                 , GetName()
    //                 , startInx
    //                 , tvutimestamp
    //                 , t0
    //                 , LibshmutilTvutimestampMinusWithMS(t0, tvutimestamp)
    //                 );
    //             break;
    //         }
    //     }

    //     if (endInx == startInx) /* only one item */
    //     {
    //         break;
    //     }
        
    //     if ((int)(endInx - startInx) < 0)
    //     {
    //         DEBUG_ERROR_CR(
    //             "It was impossible that end index is less than start index."
    //             "nm:%s,eidx:%u,sidx:%u"
    //             , GetName()
    //             , endInx
    //             , startInx
    //             );
    //         break;
    //     }

    //     ResultRecorder lastResult2;
    //     //if (endInx != startInx)
    //     {
    //         _cmpMatchingTvutimestamp(endInx, tvutimestamp, lastResult2);
    //         const tvushm::ItemInfo &item = *(lastResult2.pItem);
    //         if (!item.HasGottenTvutimestamp(endInx))
    //         {
    //             break;
    //         }

    //         maxTvustamp = item.GetTvutimestamp();
    //         if (lastResult2.cmpRet == 0) /* found */
    //         {
    //             matchingTvustamp = item.GetTvutimestamp();
    //             matchingItem = lastResult2;
    //             bGotMatching = true;
    //             break;
    //         }
    //         else if (lastResult2.cmpRet < 0) /* out of range */
    //         {
    //             DEBUG_WARN_CR(
    //                 "the last item tvutimestamp was smaller than request."
    //                 "nm:%s,idx:%u,v1:0x%" PRIx64 ",v2:0x%" PRIx64 ",-:%" PRId64 "ms"
    //                 , GetName()
    //                 , endInx
    //                 , tvutimestamp
    //                 , item.GetTvutimestamp()
    //                 , LibshmutilTvutimestampMinusWithMS(item.GetTvutimestamp(), tvutimestamp)
    //                 );
    //             break;
    //         }
    //     }

    //     if (endInx - startInx <= 1)
    //     {
    //         matchingTvustamp = lastResult2.pItem->GetTvutimestamp();
    //         matchingItem = lastResult2;
    //         bGotMatching = true;
    //         break;
    //     }

    //     // in the rage
    //     uint32_t left = startInx;
    //     uint32_t right = endInx ;

    //     while ((int32_t)(left - right) <= 0) {
    //         uint32_t mid = left + (right - left) / 2;
    //         ResultRecorder result;
    //         if (mid == startInx)
    //         {
    //             result = lastResult;
    //         }
    //         else if (mid == endInx)
    //         {
    //             result = lastResult2;
    //         }
    //         else
    //         {
    //             _cmpMatchingTvutimestamp(mid, tvutimestamp, result);
    //         }

    //         const tvushm::ItemInfo &item = *(result.pItem);
    //         if (!item.IsReadingSuccess(mid))
    //         {
    //             DEBUG_ERROR_CR(
    //                 "It can not open the item data for searching tvutimestamp."
    //                 "nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
    //                 , GetName()
    //                 , mid
    //                 , tvutimestamp
    //                 );
    //             break;
    //         }

    //         if (!item.HasGottenTvutimestamp(mid))
    //         {
    //             DEBUG_WARN_CR(
    //                 "It did not get the tvutimestamp from the item data for searching tvutimestamp."
    //                 "nm:%s,idx:%u,v1:0x%" PRIx64 "ms"
    //                 , GetName()
    //                 , mid
    //                 , tvutimestamp
    //                 );
    //             break;
    //         }

    //         if (result.cmpRet == 0) {
    //             matchingTvustamp = item.GetTvutimestamp();
    //             matchingItem = result;
    //             bGotMatching = true;
    //             break;
    //         } else if (result.cmpRet < 0) {
    //             left = mid + 1;
    //         } else {
    //             matchingTvustamp = item.GetTvutimestamp();
    //             matchingItem = result;
    //             bGotMatching = true;
    //             right = mid - 1;
    //         }
    //     }

    //     if (!bGotMatching)
    //     {
    //         DEBUG_ERROR_CR(
    //             "It was impossible that did not get matching item here."
    //             "nm:%s,expect:0x%" PRIx64 ",min:0x%" PRIx64 ",max:0x%" PRIx64 ",-:%" PRId64 "ms"
    //             , GetName()
    //             , tvutimestamp
    //             , minTvustamp
    //             , maxTvustamp
    //             , LibshmutilTvutimestampMinusWithMS(maxTvustamp, minTvustamp)
    //         );
    //     }
    // } while(0);

    // int64_t diffTime = _libshm_get_sys_ms64() - beginTime;
    // if (bGotMatching)
    // {
    //     if (diffTime <= 5)
    //     {
    //         DEBUG_INFO_CR(
    //             "It found the item data for searching tvutimestamp."
    //             "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
    //             ",tm:%" PRId64 ""
    //             , GetName()
    //             , matchingItem.itemIdx
    //             , tvutimestamp
    //             , matchingTvustamp
    //             , LibshmutilTvutimestampMinusWithMS(matchingTvustamp, tvutimestamp)
    //             , diffTime
    //         );
    //     }
    //     else
    //     {
    //         DEBUG_WARN_CR(
    //             "It found the item data for searching tvutimestamp."
    //             "nm:%s,idx:%u,expect:0x%" PRIx64 ",find:0x%" PRIx64 ",-:%" PRId64 "ms"
    //             ",tm:%" PRId64 ""
    //             , GetName()
    //             , matchingItem.itemIdx
    //             , tvutimestamp
    //             , matchingTvustamp
    //             , LibshmutilTvutimestampMinusWithMS(matchingTvustamp, tvutimestamp)
    //             , diffTime
    //         );
    //     }
    // }
    // else
    // {
    //     DEBUG_WARN_CR(
    //         "It did not find the item data for searching tvutimestamp."
    //         "nm:%s,expect:0x%" PRIx64 ",min:0x%" PRIx64 ", max:0x%" PRIx64 ""
    //         ",tm:%" PRId64 ""
    //         , GetName()
    //         , tvutimestamp
    //         , minTvustamp, maxTvustamp
    //         , diffTime
    //     );
    // }

    // return bGotMatching;
}

static bool _isPtsValid(const uint64_t &pts)
{
    return true;
}

static int64_t _uint64Minus(uint64_t v1, uint64_t v2)
{
    return (int64_t)(v1-v2);
}

bool CLibShmMediaCtx::SearchTheFirstMatchingItemWithPts(
    const uint64_t &pts
    , ResultRecorder &matchingItem
    )
{
    FnTimestampValid_t fnPtsValid =
        std::bind(&_isPtsValid, std::placeholders::_1);
    FnCmpFetchingItem_t fnCmp =
        std::bind(&CLibShmMediaCtx::_cmpMatchingPts, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FnGetItemTimeVal_t fnGetTime =
        std::bind(&tvushm::ItemInfo::GetPts, std::placeholders::_1);
    FnHasGottenTimeVal_t fnGottenTime =
        std::bind(&tvushm::ItemInfo::HasGottenPts, std::placeholders::_1, std::placeholders::_2);
    FnTimestampValMinus_t fnMinus = std::bind(&_uint64Minus, std::placeholders::_1, std::placeholders::_2);
    return SearchFirstItemMatching(pts, matchingItem, fnPtsValid, fnCmp, fnGetTime, fnGottenTime, fnMinus, "pts");
}

#if defined(TVU_LINUX)
int CLibShmMediaCtx::RemoveShm(const char *shmname)
{
    return CTvuBaseShareMemory::RemoveShmFromKernal(shmname);
}
#endif

int CLibShmMediaCtx::PollReadRawData(libshmmedia_raw_head_param_t   *pmh, libshmmedia_raw_data_param_t   *pmi, unsigned int timeout)
{
    uint32_t    read_index  = m_pShmObj->GetReadIndex();
    uint8_t     *pItemAddr  = m_pShmObj->GetItemAddrByIndex(read_index);

    int         ret         = 0;
    libshmmedia_raw_data_param_t   oip;
    {
        memset((void*)&oip, 0, sizeof(libshmmedia_raw_data_param_t));
    }

    /*  head does not need to transfer, disable it out */
#if 0
    uint8_t     *pHeadAddr  = m_pShmObj->GetHeader();
    uint32_t    head_len    = m_pShmObj->GetHeadLen();
    pmh->pRawHead_ = pHeadAddr;
    pmh->uRawHead_ = head_len;
#endif

    ret  = PollReadable(timeout);

    if (ret <= 0) {
        return ret;
    }

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    ret = libshmmediapro::getBufferLenFromItemBuffer(pItemAddr, m_uVersion);
    if (ret > 0)
    {
        oip.uRawData_ = ret;
        oip.pRawData_ = pItemAddr;
    }
#else

    switch (m_uVersion)
    {
    case LIBSHM_MEDIA_HEAD_VERSION_V1:
    case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            shm_media_item_info_v1_v2_t     *pi12 = (shm_media_item_info_v1_v2_t *)pItemAddr;

            oip.uRawData_ = pi12->length;
            oip.pRawData_ = pItemAddr;
            ret = pi12->length;

        }
        break;
    case LIBSHM_MEDIA_HEAD_VERSION_V3:
    case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;

            oip.uRawData_  = pi3->length;
            oip.pRawData_ = pItemAddr;
            ret   = pi3->length;
        }
        break;
    default:
        {
            ret   = -1;
            DEBUG_ERROR("unsupport version %d\n", m_uVersion);
        }
        break;
    }
#endif

    if (ret > 0)
    {
        FinishRead();
    }

    *pmi    = oip;
    return ret;
}

int CLibShmMediaCtx::ApplyRawData(libshmmedia_raw_data_param_t   *pmi)
{
    if (!IsCreator())
    {
        return 0;
    }

    uint8_t     *pItemAddr  = m_pShmObj->GetWriteItemAddr();
    int32_t     item_len    = (int32_t)m_pShmObj->GetItemLength();

    if (!pmi)
    {
        return -1;
    }

    pmi->pRawData_ = pItemAddr;
    pmi->uRawData_ = item_len;

    return item_len;
}

uint8_t *CLibShmMediaCtx::ApplyRawData(size_t len)
{
    if (!IsCreator())
    {
        return NULL;
    }
    uint8_t     *pItemAddr  = m_pShmObj->GetWriteItemAddr();
    uint32_t     item_len    = m_pShmObj->GetItemLength();
    uint8_t     *pret = NULL;

    if (item_len >= len)
    {
        pret = pItemAddr;
    }

    return pret;
}

int CLibShmMediaCtx::ApplyItemBuffer(const libshm_media_item_param_t *pmi, libshm_media_item_addr_layout_t *pLayout)
{
    if (!IsCreator())
    {
        return 0;
    }

    uint8_t     *pItemAddr  = m_pShmObj->GetWriteItemAddr();
    uint32_t     item_len    = m_pShmObj->GetItemLength();

    uint32_t    item_ver = GetItemVersion();
    int ret = 0;
    ret = libshmmediapro::getWriteItemBufferLayoutWithVer(pmi, pItemAddr, pLayout, item_ver);
    if (ret <= 0)
    {
        DEBUG_WARN("getWriteItemBufferLayout ret %d\n", ret);
        return ret;
    }

    if ((uint32_t)ret > item_len)
    {
        DEBUG_WARN("item length %u < request length %d\n", item_len, ret);
        return -ENOMEM;
    }

    return ret;
}

int CLibShmMediaCtx::CommitRawData(size_t commit_len)
{
    FinishWrite();
    return commit_len;
}

int CLibShmMediaCtx::CommitItemBuffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi)
{
    if (!IsCreator())
    {
        return 0;
    }
    int         ret         = 0;
    uint8_t     *pItemAddr  = m_pShmObj->GetWriteItemAddr();
    uint32_t    item_ver   = GetItemVersion();
    libshm_media_item_param_t omi = *pmi;

    {
        libshm_media_item_param_t &v0 = omi;
        v0.u_copied_flags = 0xFFFFFFFF;
    }

    ret = libshmmediapro::writeItemBuffer(pmh, &omi, pItemAddr, item_ver);

    if (ret <= 0)
    {
        DEBUG_ERROR("commit item head data failed, ret %d", ret);
        return -1;
    }

    CommitRawData(ret);

    return ret;
}

void
LibShmMediaSetCloseflag(libshm_media_handle_t h, int bclose)
{
    CLibShmMediaCtx *pctx       = (CLibShmMediaCtx *)h;
    pctx->SetCloseFlag(bclose ? true : false);
    return;
}

int LibShmMediaCheckCloseflag(libshm_media_handle_t h)
{
    CLibShmMediaCtx *pctx   = (CLibShmMediaCtx *)h;
    bool            bClose  = pctx->CheckCloseFlag();
    return bClose ? 1 : 0;
}

libshm_media_handle_t
LibShmMediaCreate
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint32_t item_length
)
{
    return LibShmMediaCreate2(pMemoryName, header_len, item_count, item_length, S_IRUSR | S_IWUSR);
}

libshm_media_handle_t
LibShmMediaCreate2
(
    const char * pMemoryName
    , uint32_t header_len
    , uint32_t item_count
    , uint32_t item_length
    , mode_t mode
)
{
    CLibShmMediaCtx             *pctx   = NULL;
    libshm_media_handle_t    h       = NULL;

    pctx    = new CLibShmMediaCtx();

    if (!pctx)
    {
        DEBUG_ERROR("malloc media shm context failed\n");
        goto FAILED;
    }

    if (pctx->CreateShmEntry(pMemoryName, header_len, item_count, item_length, mode) < 0) {
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

#if defined(TVU_LINUX)
int 
LibShmMediaRemoveShmidFromSystem
(
    const char * pMemoryName
)
{
    libshm_media_handle_t h = LibShmMediaOpen(pMemoryName, NULL, NULL);
    if (h)
    {
        LibShmMediaSetCloseflag(h, 1); /* it must set close flag before removing */
        LibShmMediaDestroy(h);
    }
    return CLibShmMediaCtx::RemoveShm(pMemoryName);
}
#endif

libshm_media_handle_t
LibShmMediaOpen
(
    const char * pMemoryName
    , libshm_media_readcb_t cb
    , void *opaq
)
{
    CLibShmMediaCtx             *pctx   = NULL;
    libshm_media_handle_t      h       = NULL;

    pctx    = new CLibShmMediaCtx();

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
LibShmMediaDestroy(libshm_media_handle_t h)
{
#if defined TVU_LINUX
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
    if (LibShmMediaIsCreator(h)) {
        LibShmMediaSetCloseflag(h, 1);
    }
#endif

    CLibShmMediaCtx *pctx       = (CLibShmMediaCtx *)h;

    if (pctx)
        delete pctx;

    return;
}

uint32_t LibShmMediaGetHeadVersion(libshm_media_handle_t h)
{
    CLibShmMediaCtx *pctx       = (CLibShmMediaCtx *)h;
    uint32_t        vers        = 0;

    if (pctx)
        vers    = pctx->GetVersion();

    return vers;
}

uint32_t LibShmMediaGetVersion(libshm_media_handle_t h)
{
    return LibShmMediaGetHeadVersion(h);
}

uint8_t *LibShmMediaGetItemDataAddr(libshm_media_handle_t h, unsigned int index)
{
    CLibShmMediaCtx *pctx   = (CLibShmMediaCtx *)h;

    return pctx->GetItemDataAddr(index);
}

unsigned int LibShmMediaGetWriteIndex(libshm_media_handle_t h)
{
    CLibShmMediaCtx *pctx   = (CLibShmMediaCtx *)h;
    return  pctx->GetWIndex();
}

unsigned int LibShmMediaGetReadIndex(
      libshm_media_handle_t         h
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetRIndex();
}

unsigned int LibShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    unsigned int wIndex = pctx->GetWIndex();
    pctx->SetRIndex(wIndex);
    return wIndex;
}

unsigned int LibShmMediaSeekReadIndex(libshm_media_handle_t h, uint32_t pos)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    pctx->SetRIndex(pos);
    return pos;
}

unsigned int LibShmMediaSeekReadIndexToRingStart(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    unsigned int wIndex = pctx->GetWIndex();
    uint32_t nCount = pctx->GetItemCounts();
    uint32_t startIndex = (wIndex>=nCount)?(wIndex-nCount+1):0;
    pctx->SetRIndex(startIndex);
    return startIndex;
}

unsigned int LibShmMediaSetReadIndex(libshm_media_handle_t  h, char type, int64_t pts)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->SetReadIndex(type, pts);
}

bool LibShmMediaSearchItemWithTvutimestamp(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , libshm_media_item_param_t *pmi)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    if (!pctx || !pmi)
    {
        return false;
    }
    return pctx->SearchItemWithTvutimestamp(tvutimestamp, NULL, pmi, NULL);
}

bool LibShmMediaSearchItemWithTvutimestampV2(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    , libshmmedia_extend_data_info_t *pext
    )
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    if (!pctx)
    {
        return false;
    }
    return pctx->SearchItemWithTvutimestamp(tvutimestamp, pmh, pmi, pext);
}

int LibShmMediaReadItemWithTvutimestamp(
    libshm_media_handle_t  h
    , uint64_t tvutimestamp
    , char type
    , uint64_t pts
    , bool *bFoundTvutimestamp
    , bool *bFoundPts
    , libshm_media_head_param_t *pmh
    , libshm_media_item_param_t *pmi
    )
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    if (!pctx)
    {
        return -1;
    }
    return pctx->PollReadDateWithTvutimestamp(pmh, pmi, NULL, bFoundTvutimestamp, bFoundPts, tvutimestamp, type, pts);
}

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
    )
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    if (!pctx)
    {
        return -1;
    }
    return pctx->PollReadDateWithTvutimestamp(pmh, pmi, pext, bFoundTvutimestamp, bFoundPts, tvutimestamp, type, pts);
}

const uint8_t * LibShmMediaGeHeadAddr(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetHeadAddr();
}

uint32_t LibShmMediaGeHeadVersion(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetHeadVer();
}

unsigned int LibShmMediaGeHeadLength(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetHeadLen();
}

unsigned int LibShmMediaGetItemLength(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetItemLen();
}

unsigned int LibShmMediaGetItemCounts(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetItemCounts();
}

unsigned int LibShmMediaGetItemOffset(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetItemOffset();
}

const char *LibShmMediaGetName(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->GetName();
}

/**
 *  Return:
 *      0   :   not the creator
 *      !0  :  the creator
**/
int LibShmMediaIsCreator(libshm_media_handle_t h)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
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
int LibShmMediaPollSendable(libshm_media_handle_t h, unsigned int timeout)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    //DEBUG_INFO("\n");
    return pctx->PollSendable(timeout);
}

int LibShmMediaPollReadable(libshm_media_handle_t h, unsigned int timeout)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->PollReadable(timeout);
}

int LibShmMediaSendData(
      libshm_media_handle_t h
      , const libshm_media_head_param_t *pmh
      , const libshm_media_item_param_t *pmi
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    //DEBUG_INFO("\n");
    return pctx->SendData(pmh, pmi);
}

int LibShmMediaSendDataWithFrequency1000(
      libshm_media_handle_t h
      , const libshm_media_head_param_t *pmh
      , const libshm_media_item_param_t *pmi
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    //DEBUG_INFO("\n");
    return pctx->SendDataWithFrequency1000(pmh, pmi);
}

int LibShmMediaPollReadHead(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , unsigned int                 timeout
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
#ifdef    _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    return pctx->PollReadHead(pmh, timeout);
#else
    return pctx->PollReadHead(pmh, true, timeout);
#endif
}

int LibShmMediaPollReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
      , unsigned int                timeout
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->PollReadData(pmh, pmi, NULL, timeout);
}

int LibShmMediaPollReadDataV2(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
    , libshmmedia_extend_data_info_t *pext
    , unsigned int                timeout
    )
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->PollReadData(pmh, pmi, pext, timeout);
}

int LibShmMediaReadData(
      libshm_media_handle_t         h
      , libshm_media_head_param_t   *pmh
      , libshm_media_item_param_t   *pmi
)
{
    CLibShmMediaCtx    *pctx    = (CLibShmMediaCtx *)h;
    return pctx->PollReadData(pmh, pmi, NULL, 0);
}

int LibShmMediaReadDataWithoutIndexStep(
    libshm_media_handle_t         h
    , libshm_media_head_param_t   *pmh
    , libshm_media_item_param_t   *pmi
)
{
    CLibShmMediaCtx    *pctx = (CLibShmMediaCtx *)h;
    return pctx->PollReadDataWithoutIndexStep(pmh, pmi, NULL, 0);
}

void LibShmMediaReadIndexStep(
    libshm_media_handle_t         h
)
{
    CLibShmMediaCtx    *pctx = (CLibShmMediaCtx *)h;
    pctx->FinishRead();
    return;
}

static int(*_gfnLibshmmedia)(int , const char *, va_list ap) = NULL;
static int _fncallback(int level, const char *fmt, ...)
{
    int ret = 0;
    if (!_gfnLibshmmedia)
    {
        return 0;
    }

    va_list ap;
    va_start(ap, fmt);
    {
        ret = _gfnLibshmmedia(level, fmt, ap);
    }
    va_end(ap);
    return ret;
}

void LibShmMediaSetLogCallback(int(*cb)(int , const char *, va_list ap))
{
    _gfnLibshmmedia = cb;
    libsharememory_set_log_cabllback_internal_v2(cb);
    LibShmMediaProtoSetLogCbInternalV2(cb);

    tvushm::Log::SetLogCallback(_fncallback);
    tvushm::Log::SetLogTag("vishm");
}

void LibShmMediaSetLogCb(int(*cb)(int , const char *, ...))
{
    libsharememory_set_log_cabllback_internal(cb);
    LibShmMediaProtoSetLogCbInternal(cb);
    tvushm::Log::SetLogCallback(cb);
    tvushm::Log::SetLogTag("vishm");
    return;
}


