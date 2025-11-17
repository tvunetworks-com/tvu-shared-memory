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

#include "libshm_key_value.h"
#include "libshm_media_audio_track_channel_protocol.h"
#include "libshm_media_media_head_proto_internal.h"
#include "libshm_media_key_value_proto_internal.h"
#include "libshm_media_protocol_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include "libshm_util_common_internal.h"
#include <errno.h>
#include <stddef.h>

#define INIT_FINISH_STATE           0x12345678
#define WAIT_MS_NUM                 1000
#define V4_ITEM_ALIGN_SIZE  32

#ifdef TVU_LINUX
#    define attribute_deprecated __attribute__((deprecated))
#elif defined(TVU_WINDOWS)
#    define attribute_deprecated __declspec(deprecated)
#else
#    define attribute_deprecated
#endif


namespace libshmmediapro {

    static const char* _tvuutilTransBinaryToHexString(const uint8_t *data, int len, char hx_str[], int hx_str_len)
    {
        int ix = 0;

        for (int i = 0; i < len; i++)
        {
            if (i == 0)
            {
                snprintf(hx_str + ix, hx_str_len - ix, "%02hhx", data[i]);
            }
            else
            {
                snprintf(hx_str + ix, hx_str_len - ix, ",%02hhx", data[i]);
            }
            ix = strlen(hx_str);
        }

        return hx_str;
    }


    static inline uint32_t transfer_le_32(const uint8_t *p)
    {
        return (((uint32_t)p[0]) | (((uint32_t)p[1]) << 8) | (((uint32_t)p[2]) << 16) | (((uint32_t)p[3]) << 24));
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

    /* read out param head from shm head & item, deprecated api for shm head would not be used to save media data */
//    static attribute_deprecated
//    int _read_param_head_from_shm(libshm_media_head_param_t *pmh, const uint8_t *pItemAddr, const uint8_t *pHead, uint32_t version)
//    {
//        int         ret         = 1;
//        libshm_media_head_param_t  oh;
//        {
//            memset(&oh, 0, sizeof(oh));
//        }

//        switch (version)
//        {
//        case LIBSHM_MEDIA_HEAD_VERSION_V1:
//        case LIBSHM_MEDIA_HEAD_VERSION_V2:
//            {
//                const shm_media_item_info_v1_v2_t    *pi12= (shm_media_item_info_v1_v2_t *)pItemAddr;

//                if (version == LIBSHM_MEDIA_HEAD_VERSION_V2) {
//                    const shm_media_head_info_v2_t    *ph2    = (shm_media_head_info_v2_t *)pHead;
//                    copy_media_head_to_param(&oh, (void *)ph2, LIBSHM_MEDIA_HEAD_VERSION_V2);
//                }

//                (&oh)->i_dstw         = pi12->width;
//                (&oh)->i_dsth         = pi12->height;
//                (&oh)->u_videofourcc  = pi12->videofourcc;
//                (&oh)->i_duration     = pi12->duration;
//                (&oh)->i_scale        = pi12->scale;
//                (&oh)->u_audiofourcc  = pi12->audiofourcc;
//                (&oh)->i_channels     = pi12->channels;
//                (&oh)->i_depth        = pi12->depth;
//                (&oh)->i_samplerate   = pi12->samplerate;
//            }
//            break;
//        case LIBSHM_MEDIA_HEAD_VERSION_V3:
//        case LIBSHM_MEDIA_HEAD_VERSION_V4:
//            {
//                const shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;
//                const shm_media_head_info_v3_t    *ph3    = (shm_media_head_info_v3_t *)pHead;

//                copy_media_head_to_param(&oh, (void *)ph3, LIBSHM_MEDIA_HEAD_VERSION_V3);

//                (&oh)->i_dstw         = LIBSHMMEDIA_READ_SHM_U32(pi3->width);
//                (&oh)->i_dsth         = LIBSHMMEDIA_READ_SHM_U32(pi3->height);
//                (&oh)->u_videofourcc  = LIBSHMMEDIA_READ_SHM_U32(pi3->vfourcc);
//                (&oh)->i_duration     = LIBSHMMEDIA_READ_SHM_U32(pi3->duration);
//                (&oh)->i_scale        = LIBSHMMEDIA_READ_SHM_U32(pi3->scale);
//                (&oh)->u_audiofourcc  = LIBSHMMEDIA_READ_SHM_U32(pi3->afourcc);
//                (&oh)->i_channels     = LIBSHMMEDIA_READ_SHM_U32(pi3->channels);
//                (&oh)->i_depth        = LIBSHMMEDIA_READ_SHM_U32(pi3->depth);
//                (&oh)->i_samplerate   = LIBSHMMEDIA_READ_SHM_U32(pi3->samplerate);
//            }
//            break;
//        default:
//            {
//                ret   = -1;
//                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", version);
//            }
//            break;
//        }

//        if (ret >= 0)
//            *pmh    = oh;

//        return ret;
//    }

    static int _read_param_head_from_item_buffer(libshm_media_head_param_t *pmh, const uint8_t *pItemAddr, uint32_t version)
    {
        int         ret         = 1;
        libshm_media_head_param_t  oh;
        {
            LibShmMediaHeadParamInit(&oh, sizeof(libshm_media_head_param_t));
            oh.h_channel = libshmmediapro::_headParamGetChannelLayout(*pmh);
        }

        switch (version)
        {
        case kLibShmMediaItemVerV1:
        case kLibShmMediaItemVerV2:
            {
                const shm_media_item_info_v1_v2_t    *pi12= (shm_media_item_info_v1_v2_t *)pItemAddr;
                (&oh)->i_dstw         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->width);
                (&oh)->i_dsth         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->height);
                (&oh)->u_videofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->videofourcc);
                (&oh)->i_duration     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->duration);
                (&oh)->i_scale        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->scale);
                (&oh)->u_audiofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->audiofourcc);
                (&oh)->i_channels     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->channels);
                (&oh)->i_depth        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->depth);
                (&oh)->i_samplerate   = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->samplerate);
            }
            break;
        case kLibShmMediaItemVerV3:
            {
                const shm_media_item_info_v3_t    *pi3    = (shm_media_item_info_v3_t *)pItemAddr;

                (&oh)->i_dstw         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->width);
                (&oh)->i_dsth         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->height);
                (&oh)->u_videofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->vfourcc);
                (&oh)->i_duration     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->duration);
                (&oh)->i_scale        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->scale);
                (&oh)->u_audiofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->afourcc);
                (&oh)->i_channels     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->channels);
                (&oh)->i_depth        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->depth);
                (&oh)->i_samplerate   = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->samplerate);
            }
            break;
        case kLibShmMediaItemVerV4:
            {
                const shm_media_item_info_v3_t      *pi3    = (shm_media_item_info_v3_t *)pItemAddr;
                const shm_media_item_info_v40_t     *pi40   = (shm_media_item_info_v40_t *)pItemAddr;

                (&oh)->i_dstw         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->width);
                (&oh)->i_dsth         = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->height);
                (&oh)->u_videofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->vfourcc);
                (&oh)->i_duration     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->duration);
                (&oh)->i_scale        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->scale);
                (&oh)->u_audiofourcc  = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->afourcc);
                (&oh)->i_channels     = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->channels);
                (&oh)->i_depth        = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->depth);
                (&oh)->i_samplerate   = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->samplerate);

                if (LIBSHMMEDIA_ITEM_READ_SHM_U32(pi40->_head_len) >= sizeof(shm_media_item_info_v4_subv1_t))
                {
                    const shm_media_item_info_v4_subv1_t     *pi41 = (const shm_media_item_info_v4_subv1_t*)pi40;
                    (&oh)->i_sarw = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi41->sarw);
                    (&oh)->i_sarh = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi41->sarh);
                    (&oh)->i_srcw = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi41->srcw);
                    (&oh)->i_srch = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi41->srch);
                }
            }
            break;
        default:
            {
                ret   = -1;
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", version);
            }
            break;
        }

        if (ret >= 0 && pmh)
        {
            libshmmediapro::_headParamLowCopy(*pmh, oh);
        }

        return ret;
    }

    unsigned int getShmMediaHeadVersion(const uint8_t *headBuf)
    {
        shm_construct_t *pbase = (shm_construct_t *)headBuf;
        if (!pbase)
        {
            return INVALID_SHM_HEAD_VERSION;
        }

        return libshmhead_construct_get_version(pbase);
    }

    int  alignShmItemLength(int rawLen)
    {
        int     item_len = (rawLen + 0x400) >> 10 << 10;
        return item_len;
    }

    uint32_t getItemHeadLengthV4()
    {
        uint32_t len = sizeof(shm_media_item_info_v4_t);
        return len;
    }

    #define V4_ITEM_ALIGN_SIZE  32

    int preRequireItemHeadLength(uint32_t v)
    {
        int len = 0;
        switch (v)
        {
        case kLibShmMediaItemVerV1:
        case kLibShmMediaItemVerV2:
        {
            len = sizeof(shm_media_item_info_v1_v2_t);
        }
        break;
        case kLibShmMediaItemVerV3:
        {
            len = sizeof(shm_media_item_info_v3_t);
        }
        break;
        case kLibShmMediaItemVerV4:
        {
            len = sizeof(shm_media_item_info_v4_t) + V4_ITEM_ALIGN_SIZE /* align video/audio 16byts */;
        }
        break;
        default:
        {
            len = -1;
        }
        break;
        }
        return len;
    }

    int  preRequireMaxItemHeadLength(uint32_t v)
    {
        int len = preRequireItemHeadLength(v);
        return len;
    }

    static uint32_t GetPreExtBuffSize() { return sizeof(shm_construct_ext_t); }
    int preRequireHeadLength(uint32_t v)
    {
        int len = 0;
        switch (v)
        {
        case LIBSHM_MEDIA_HEAD_VERSION_V1:
        case LIBSHM_MEDIA_HEAD_VERSION_V2:
        {
            len = sizeof(shm_media_head_info_v2_t);
        }
        break;
        case LIBSHM_MEDIA_HEAD_VERSION_V3:
        {
            len = sizeof(shm_media_head_info_v3_t);
        }
        break;
        case LIBSHM_MEDIA_HEAD_VERSION_V4:
        {
            len = sizeof(shm_media_head_info_v4_t) + GetPreExtBuffSize();
        }
        break;
        default:
        {
            len = -1;
        }
        break;
        }
        return len;
    }

    bool checkCloseFlag(const uint8_t *pHead)
    {
        bool        bClose  = false;
        uint32_t    ver     = 0;
        if (!pHead)
            return false;

        ver = getShmMediaHeadVersion(pHead);

        switch (ver)
        {
        case INVALID_SHM_HEAD_VERSION:
            {
                bClose = true;
            }
            break;
        case LIBSHM_MEDIA_HEAD_VERSION_V1:
            {
                bClose = false;
            }
            break;
        case LIBSHM_MEDIA_HEAD_VERSION_V2:
            {
                shm_media_head_info_v2_t   *p2  = (shm_media_head_info_v2_t*)pHead;

                if (p2)
                    bClose  = (LIBSHMMEDIA_HEAD_READ_SHM_U32(p2->close_flag)) ? true : false;
            }
            break;
        case LIBSHM_MEDIA_HEAD_VERSION_V3:
        case LIBSHM_MEDIA_HEAD_VERSION_V4:
        default:
            {
                shm_media_head_info_v3_t    *p3 = (shm_media_head_info_v3_t*)pHead;

                if (p3)
                    bClose  = LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.close_flag) ? true : false;
            }
            break;
        }

        return bClose;
    }

    static unsigned int _getMapVer(uint8_t *pHead)
    {
        unsigned int main_v = INVALID_SHM_HEAD_VERSION;
        shm_construct_t             *p1 = (shm_construct_t *)pHead;
        shm_media_head_info_v3_t    *p3 = (shm_media_head_info_v3_t *)p1;

        main_v = libshmhead_construct_get_version(p1);

        if (main_v == INVALID_SHM_HEAD_VERSION)
            return main_v;
        if (main_v == LIBSHM_MEDIA_HEAD_VERSION_V1 || main_v == LIBSHM_MEDIA_HEAD_VERSION_V2)
            return main_v;

        unsigned int sub_v = LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.v3_sub_ver);

        return (main_v+sub_v);
    }

    void setCloseFlag(uint8_t *pHead, bool bclose)
    {
        if (!pHead)
            return;

        shm_construct_t             *p1 = (shm_construct_t *)pHead;
        shm_media_head_info_v2_t    *p2 = (shm_media_head_info_v2_t *)p1;
        shm_media_head_info_v3_t    *p3 = (shm_media_head_info_v3_t *)p1;
        int                         ver = 0;

        if (p1) {
            ver = _getMapVer(pHead);

            switch (ver)
            {
            case INVALID_SHM_HEAD_VERSION:
            case LIBSHM_MEDIA_HEAD_VERSION_V1:
            {

            }
            break;
            case LIBSHM_MEDIA_HEAD_VERSION_V2:
            {
                if (p2) {
                    uint32_t v = bclose ? 1 : 0;
                    p2->close_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(v);
                }
            }
            break;
            case LIBSHM_MEDIA_HEAD_VERSION_V3:
            case LIBSHM_MEDIA_HEAD_VERSION_V4:
            default:
            {
                if (p3)
                    p3->v3.close_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(bclose ? 1 : 0);
            }
            break;
            }
        }
    }

    int initShmV2(uint8_t *phead, int flags)
    {
        shm_media_head_info_v2_t    *p2 = (shm_media_head_info_v2_t *)phead;
        int                         nLoop = 0;
        int ret = 0;

        if (!phead)
            return -1;

        if (flags & SHM_FLAG_READ)
        {
            nLoop = 0;
            while (LIBSHMMEDIA_HEAD_READ_SHM_U32(p2->init_flag) != INIT_FINISH_STATE) {
                if (nLoop == WAIT_MS_NUM)
                {
                    DEBUG_SHMMEDIA_PROTO_ERROR("major ver[%2],can not get the correct init flag\n");
                    ret = -1;
                    break;
                }
                _libshm_common_msleep(1);
                nLoop++;
            }
        }
        else
        {
            p2->close_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(0);
            p2->init_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(INIT_FINISH_STATE);
        }
        DEBUG_SHMMEDIA_PROTO_INFO("major ver 2, close flag %d\n", LIBSHMMEDIA_HEAD_READ_SHM_U32(p2->close_flag));

        return ret;
    }

    int initShmV3(uint8_t *phead, int flags)
    {
        shm_media_head_info_v3_t    *p3 = NULL;
        int                         nLoop = 0;
        uint32_t                    v3_subver = 0;
        int                         ret = 0;

        if (!phead)
            return -1;

        p3 = (shm_media_head_info_v3_t *)phead;
        if (flags & SHM_FLAG_READ)
        {
            nLoop = 0;
            while (LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.init_flag) != INIT_FINISH_STATE) {
                if (nLoop == WAIT_MS_NUM)
                {
                    DEBUG_SHMMEDIA_PROTO_ERROR("major ver[3],can not get the correct init flag\n");
                    ret = -1;
                    break;
                }
                _libshm_common_msleep(1);
                nLoop++;
            }

            if (ret == 0)
            {
                switch (LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.v3_sub_ver))
                {
                case LIBSHM_MEDIA_HEAD_V3_SUBVER_V0:
                {
                    v3_subver = LIBSHM_MEDIA_HEAD_V3_SUBVER_V0;
                }
                break;
                case LIBSHM_MEDIA_HEAD_V3_SUBVER_V1:
                default:
                {
                    ret = -1;
                }
                break;
                }
            }

        }
        else
        {
            v3_subver = LIBSHM_MEDIA_HEAD_V3_SUB_VERSION;
            p3->v3.v3_sub_ver = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(LIBSHM_MEDIA_HEAD_V3_SUB_VERSION);
            p3->v3.close_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(0);
            p3->v3.init_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(INIT_FINISH_STATE);
        }

        DEBUG_SHMMEDIA_PROTO_INFO("major ver 3, sub ver %d, close flag %d\n"
            , LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.v3_sub_ver)
            , LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.close_flag));
        return ret;
    }

    int initShmV4(uint8_t *phead, uint64_t head_len, int flags)
    {
        shm_media_head_info_v4_t    *p = NULL;
        shm_media_head_info_v3_t    *p3 = NULL;
        int                         nLoop = 0;
        uint32_t                    v3_subver = 0;
        int                         ret = 0;

        if (!phead)
            return -1;

        p = (shm_media_head_info_v4_t *)phead;
        p3 = &p->_v3h;

        if (flags & SHM_FLAG_READ)
        {
            nLoop = 0;
            while (LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.init_flag) != INIT_FINISH_STATE) {
                if (nLoop == WAIT_MS_NUM)
                {
                    DEBUG_SHMMEDIA_PROTO_ERROR("major ver[3],can not get the correct init flag\n");
                    ret = -1;
                    break;
                }
                _libshm_common_msleep(1);
                nLoop++;
            }

            if (ret == 0)
            {
                switch (LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.v3_sub_ver))
                {
                case LIBSHM_MEDIA_HEAD_V3_SUBVER_V0:
                {
                    v3_subver = LIBSHM_MEDIA_HEAD_V3_SUBVER_V0;
                }
                break;
                case LIBSHM_MEDIA_HEAD_V3_SUBVER_V1:
                default:
                {
                    ret = -1;
                }
                break;
                }
            }

        }
        else
        {
            p->_head_data_size = LIBSHMMEDIA_HEAD_WRITE_SHM_U64(head_len);
            memset(&p->_reserver, 0, sizeof(p->_reserver));
            v3_subver = LIBSHM_MEDIA_HEAD_V3_SUB_VERSION;
            p3->v3.v3_sub_ver = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(LIBSHM_MEDIA_HEAD_V3_SUB_VERSION);
            p3->v3.close_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(0);
            p3->v3.init_flag = LIBSHMMEDIA_HEAD_WRITE_SHM_U32(INIT_FINISH_STATE);
        }

        DEBUG_SHMMEDIA_PROTO_INFO("major ver 4, sub ver %d, close flag %d, h4 size %ld\n"
            , LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.v3_sub_ver)
            , LIBSHMMEDIA_HEAD_READ_SHM_U32(p3->v3.close_flag), sizeof(shm_media_head_info_v4_t));
        return ret;
    }

    static bool _isV2Item(const uint8_t *pItemAddr, uint32_t nItem)
    {
        const shm_media_item_info_v1_v2_t     *pi12 = (const shm_media_item_info_v1_v2_t *)pItemAddr;

        if (pi12->length != nItem)
        {
            return false;
        }

        if (pi12->length != pi12->videolen + pi12->audiolen + pi12->extralength + sizeof(shm_media_item_info_v1_v2_t))
        {
            return false;
        }

        return true;
    }

    static bool _isV3Item(const uint8_t *pItemAddr, uint32_t nItem)
    {
        const shm_media_item_info_v3_t     *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;

        if (LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->length) != nItem)
        {
            return false;
        }

        uint32_t totallen   = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->length);
        uint32_t videolen    = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->videolen);
        uint32_t audiolen    = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->audiolen);
        uint32_t subtitlelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->subtlen);
        uint32_t cclen       = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->cclen);
        uint32_t timecodelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->timecodelen);
        uint32_t userdatalen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->datalen);

        if (totallen != videolen
                +audiolen
                +subtitlelen
                +cclen
                +timecodelen
                +userdatalen
                +sizeof(shm_media_item_info_v3_t))
        {
            return false;
        }

        return true;
    }

    static uint32_t _getItemVer4(const uint8_t *pItemAddr, uint32_t nItem)
    {
        uint32_t ver = kLibShmMediaItemVerInvalid;
        const shm_media_item_info_v3_t     *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
        uint32_t totallen   = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->length);
        uint32_t videolen    = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->videolen);
        uint32_t audiolen    = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->audiolen);
        uint32_t subtitlelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->subtlen);
        uint32_t cclen       = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->cclen);
        uint32_t timecodelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->timecodelen);
        uint32_t userdatalen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->datalen);
        uint32_t item_total_len =  videolen
                                    +audiolen
                                    +subtitlelen
                                    +cclen
                                    +timecodelen
                                    +userdatalen + sizeof(shm_media_item_info_v40_t);

        if (totallen != nItem)
        {
            return ver;
        }

        if (totallen < item_total_len)
        {
            return ver;
        }

         const shm_media_item_info_v4_subv0_t     *pi4 = (const shm_media_item_info_v4_subv0_t *)pItemAddr;
        if (LIBSHMMEDIA_ITEM_READ_SHM_U8(pi4->_item_version) > 0)
        {
            ver = LIBSHMMEDIA_ITEM_READ_SHM_U8(pi4->_item_version);
        }
        else
        {
            ver = kLibShmMediaItemVerV4;
        }

        return ver;
    }

    uint32_t getItemVerFromReadBuffer(const uint8_t *pItemAddr, uint32_t nItem)
    {
        uint32_t ver = kLibShmMediaItemVerInvalid;
        uint32_t first4 = transfer_le_32(pItemAddr);

        if (first4) /* ver>=3 the first 4 bytes is item buffer length */
        {
            ver = _getItemVer4(pItemAddr, nItem);
            if (ver != kLibShmMediaItemVerInvalid)
            {
                return ver;
            }
            if (_isV3Item(pItemAddr, nItem))
            {
                ver = kLibShmMediaItemVerV3;
                return  ver;
            }
            return ver;
        }
        else /* v2/v1, the first 4 bytes is version and const 0 */
        {
            if (_isV2Item(pItemAddr, nItem))
            {
                ver = kLibShmMediaItemVerV2;
                return ver;
            }
        }

        return ver;
    }

    int  getReadItemBufferLayoutV12(/*OUT*/libshm_media_item_param_t *pmiv, /*IN*/const uint8_t *pItemAddr)
    {
        libshm_media_item_param_v1_t *pmi = (libshm_media_item_param_v1_t *)pmiv;
        const shm_media_item_info_v1_v2_t     *pi12 = (const shm_media_item_info_v1_v2_t *)pItemAddr;
        int         item_head_len   = 0;
        int         item_v_offset   = 0;
        int         item_a_offset   = 0;
        int         item_e_offset   = 0;
        int         r_len = 0;

        int32_t videolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->videolen);
        int32_t audiolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->audiolen);
        int32_t extralen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->extralength);

        pmi->i_totalLen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->length);
        item_head_len = sizeof(shm_media_item_info_v1_v2_t);
        item_v_offset = item_head_len;
        item_a_offset = item_v_offset + videolen;
        item_e_offset = item_a_offset + audiolen;

        if (videolen > 0) {
            pmi->p_vData = pItemAddr + item_v_offset;
            pmi->i_vLen = videolen;
        }
        pmi->i64_vpts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->videopts);
        pmi->i64_vdts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->videodts);
        pmi->i64_vct = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->videocreatetime);
        pmi->u_frameType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi12->frametype);

        if (audiolen > 0) {
            pmi->p_aData = pItemAddr + item_a_offset;
            pmi->i_aLen = audiolen;
        }
        pmi->i64_apts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->audiopts);
        pmi->i64_adts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->audiodts);
        pmi->i64_act = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi12->audiocreatetime);

        if (extralen > 0) {
            pmi->p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
            pmi->i_CCLen = extralen;
        }
        r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_CCLen;
        return r_len;
    }

    int  getReadItemBufferLayoutV3(/*OUT*/libshm_media_item_param_t *pmiv, /*IN*/const uint8_t *pItemAddr)
    {
        int         r_len = 0;
        libshm_media_item_param_v1_t *pmi = (libshm_media_item_param_v1_t *)pmiv;
        const shm_media_item_info_v3_t    *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
        int         item_head_len = 0;
        int         item_v_offset = 0;
        int         item_a_offset = 0;
        int         item_s_offset = 0;
        int         item_e_offset = 0;
        int         item_timecode_offset = 0;
        int         item_data_offset = 0;

        int32_t videolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->videolen);
        int32_t audiolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->audiolen);
        int32_t subtitlelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->subtlen);
        int32_t cclen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->cclen);
        int32_t timecodelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->timecodelen);
        int32_t userdatalen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->datalen);

        pmi->i_totalLen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->length);
        item_head_len = sizeof(shm_media_item_info_v3_t);
        item_v_offset = item_head_len;
        item_a_offset = item_v_offset + videolen;
        item_s_offset = item_a_offset + audiolen;
        item_e_offset = item_s_offset + subtitlelen;
        item_timecode_offset = item_e_offset + cclen;
        item_data_offset = item_timecode_offset + timecodelen;

        pmi->u_picType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->pic_flag);

        if (videolen > 0) {
            pmi->p_vData = pItemAddr + item_v_offset;
            pmi->i_vLen = videolen;
        }
        pmi->i64_vpts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videopts);
        pmi->i64_vdts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videodts);
        pmi->i64_vct = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videoct);
        pmi->u_frameType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->frametype);
        pmi->i_interlaceFlag = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->interlace_flag);

        if (audiolen > 0) {
            pmi->p_aData = pItemAddr + item_a_offset;
            pmi->i_aLen = audiolen;
        }
        pmi->i64_apts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audiopts);
        pmi->i64_adts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audiodts);
        pmi->i64_act = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audioct);

        if (subtitlelen > 0) {
            pmi->p_sData = pItemAddr + item_s_offset;
            pmi->i_sLen = subtitlelen;
        }
        pmi->i64_spts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtpts);
        pmi->i64_sdts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtdts);
        pmi->i64_sct = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtct);

        if (cclen > 0) {
            pmi->p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
            pmi->i_CCLen = cclen;
        }

        if (timecodelen > 0) {
            pmi->p_timeCode = (const uint8_t *)(pItemAddr + item_timecode_offset);
            pmi->i_timeCode = timecodelen;
        }

        if (userdatalen> 0) {
            pmi->p_userData = (const uint8_t *)(pItemAddr + item_data_offset);
            pmi->i_userDataLen = userdatalen;
        }
        pmi->i64_userDataCT = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->data_ct);
        pmi->i_userDataType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->data_type);
        r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen;

        return r_len;
    }

    int  getReadItemBufferLayoutV4(/*OUT*/libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmiv, /*IN*/const uint8_t *pItemAddr)
    {
        int         r_len = 0;
        const shm_media_item_info_v3_t    *pi3 = (const shm_media_item_info_v3_t *)pItemAddr;
        const shm_media_item_info_v4_subv0_t    *pi4 = (const shm_media_item_info_v4_subv0_t *)pItemAddr;
        const shm_media_item_info_v4_subv0v0_t* pi400 = (const shm_media_item_info_v4_subv0v0_t *)pItemAddr;
        libshm_media_item_param_v1_t *pmi = (libshm_media_item_param_v1_t*)pmiv;
        int         item_head_len = 0;
        int         item_v_offset = 0;
        int         item_a_offset = 0;
        int         item_s_offset = 0;
        int         item_e_offset = 0;
        int         item_timecode_offset = 0;
        int         item_data_offset = 0;

        pmi->i_totalLen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->length);
        item_head_len = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_head_len);
        item_v_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_v_data_offset);
        item_a_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_a_data_offset);
        item_s_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_s_data_offset);
        item_e_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_cc_data_offset);
        item_timecode_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_timecode_data_offset);
        item_data_offset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_user_data_offset);

        pmi->u_picType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->pic_flag);

        int32_t videolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->videolen);
        int32_t audiolen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->audiolen);
        int32_t subtitlelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->subtlen);
        int32_t cclen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->cclen);
        int32_t timecodelen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->timecodelen);
        int32_t userdatalen = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->datalen);

        if (videolen > 0) {
            pmi->p_vData = pItemAddr + item_v_offset;
            pmi->i_vLen = videolen;
        }

        pmi->i64_vpts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videopts);
        pmi->i64_vdts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videodts);
        pmi->i64_vct = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->videoct);
        pmi->u_frameType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->frametype);
        pmi->i_interlaceFlag = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->interlace_flag);

        if (audiolen > 0) {
            pmi->p_aData = pItemAddr + item_a_offset;
            pmi->i_aLen = audiolen;
        }
        pmi->i64_apts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audiopts);
        pmi->i64_adts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audiodts);
        pmi->i64_act = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->audioct);

        if (subtitlelen > 0) {
            pmi->p_sData = pItemAddr + item_s_offset;
            pmi->i_sLen = subtitlelen;
        }
        pmi->i64_spts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtpts);
        pmi->i64_sdts = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtdts);
        pmi->i64_sct = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->subtct);

        if (cclen > 0) {
            pmi->p_CCData = (const uint8_t *)(pItemAddr + item_e_offset);
            pmi->i_CCLen = cclen;
        }

        if (timecodelen > 0) {
            pmi->p_timeCode = (const uint8_t *)(pItemAddr + item_timecode_offset);
            pmi->i_timeCode = timecodelen;
        }

        if (userdatalen > 0) {
            pmi->p_userData = (const uint8_t *)(pItemAddr + item_data_offset);
            pmi->i_userDataLen = userdatalen;
        }
        pmi->i64_userDataCT = LIBSHMMEDIA_ITEM_READ_SHM_U64(pi3->data_ct);
        pmi->i_userDataType = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi3->data_type);

        int nKeyValueArea = 0;
        if (
                pmh->u_reservePrivate >= sizeof(libshm_media_head_param_v2_t)\
                && pmh->h_channel
                && pi400->_flag1_media_head_extension
                && (LIBSHMMEDIA_ITEM_READ_SHM_U32(pi4->_head_len) >= sizeof(shm_media_item_info_v4_subv1_t)))
        {
            const shm_media_item_info_v4_subv1v0_t *pi410 = (const shm_media_item_info_v4_subv1v0_t *)pi400;
            uint32_t noffset = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi410->_key_value_area_offset);
            uint32_t n = LIBSHMMEDIA_ITEM_READ_SHM_U32(pi410->_key_value_area_len);
            const uint8_t* p = (const uint8_t *)(pItemAddr + noffset);
            nKeyValueArea = n;
            tvushm::keyValueProtoExtractFromBuffer((libshmmedia_audio_channel_layout_object_t *)pmh->h_channel, p, n);
        }

        r_len = pmi->i_vLen + pmi->i_aLen + pmi->i_sLen + pmi->i_CCLen + pmi->i_timeCode + pmi->i_userDataLen + nKeyValueArea;

        return r_len;
    }

    int  getWriteItemBufferLayoutV4(const libshm_media_item_param_t *pmiv, const libshm_media_item_param_internal_t &rii
        , uint8_t *pItemAddr, libshm_media_item_addr_layout_t *playout)
    {
        const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
        uint32_t    item_w_offset = 0;
        uint32_t    _offset = 0;
        int         ret = 0;
//        uint32_t    ignore_flag
//        bool        bIgnVideo = ignore_flag & 0x01;
//        bool        bIgnAudio = ignore_flag & 0x02;
//        bool        bIgnSubtitle = ignore_flag & 0x04;
//        bool        bIgnUserData = ignore_flag & 0x08;

        item_w_offset   += libshmmediapro::getItemHeadLengthV4();
        _offset = libshmmediapro::getAlignOffset(pItemAddr, item_w_offset);

        item_w_offset = _offset;

        playout->i_vOffset = _offset;

        if (pmi->i_vLen > 0)
        {
            //if (!bIgnVideo)
            {
                playout->p_vData = pItemAddr + playout->i_vOffset;
                item_w_offset += pmi->i_vLen;
            }
        }

        _offset = libshmmediapro::getAlignOffset(pItemAddr, item_w_offset);
        item_w_offset = _offset;

        playout->i_aOffset = _offset;
        if (pmi->i_aLen > 0)
        {
            //if (!bIgnAudio)
            {
                playout->p_aData = pItemAddr + playout->i_aOffset;
                item_w_offset += pmi->i_aLen;
            }
        }


        playout->i_sOffset = item_w_offset;
        if (pmi->i_sLen > 0) {
            //if (!bIgnSubtitle)
            {
                item_w_offset += pmi->i_sLen;
                playout->p_sData = pItemAddr+playout->i_sOffset;
            }
        }

        playout->i_ccOffset = item_w_offset;
        if (pmi->i_CCLen > 0)
        {
            //if (!bIgnUserData)
            {
                playout->p_CCData = pItemAddr + playout->i_ccOffset;
                item_w_offset += pmi->i_CCLen;
            }
        }

        playout->i_timecodeOffset = item_w_offset;
        if (pmi->i_timeCode > 0)
        {
            //if (!bIgnUserData)
            {
                playout->p_timeCode = pItemAddr + playout->i_timecodeOffset;
                item_w_offset += pmi->i_timeCode;
            }
        }

        playout->i_userOffset = item_w_offset;
        if (pmi->i_userDataLen > 0)
        {
            //if (!bIgnUserData)
            {
                playout->p_userData = pItemAddr + playout->i_userOffset;
                item_w_offset += pmi->i_userDataLen;
            }
        }

        playout->i_keyValueAreaOffset = item_w_offset;
        if (rii.nKeyValueSize_ > 0 && rii.pKeyValuePtr_)
        {
            {
                playout->p_keyValuePtr = pItemAddr + playout->i_keyValueAreaOffset;
                item_w_offset += rii.nKeyValueSize_;
            }
        }

        ret                 =
        playout->i_totalLen = item_w_offset;

        return ret;
    }

    int  getReadItemBufferLayoutWithVer(/*OUT*/libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr, uint32_t item_version)
    {
        int         ret         = 0;

        switch (item_version)
        {
        case kLibShmMediaItemVerV1:
        case kLibShmMediaItemVerV2:
            {
                ret = getReadItemBufferLayoutV12(pmi, pItemAddr);
            }
            break;
        case kLibShmMediaItemVerV3:
            {
                ret = getReadItemBufferLayoutV3(pmi, pItemAddr);
            }
            break;
        case kLibShmMediaItemVerV4:
            {
                ret = getReadItemBufferLayoutV4(pmh, pmi, pItemAddr);
            }
            break;
        default:
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", item_version);
            }
            break;
        }

        return ret;
    }

    unsigned int getBufferLenFromItemBuffer(const uint8_t *pItem, uint32_t head_ver)
    {
        uint32_t len = 0;
        if (head_ver >= LIBSHM_MEDIA_HEAD_VERSION_V3)
        {
            len = transfer_le_32(pItem);
        }
        else if (head_ver >= LIBSHM_MEDIA_HEAD_VERSION_V1)
        {
            len = transfer_le_32(&pItem[4]);
        }
        return len;
    }

    int readHeadFromItemBuffer(libshm_media_head_param_t *pmh, const uint8_t *pItemAddr, unsigned int nItem)
    {
        uint32_t ver = getItemVerFromReadBuffer(pItemAddr, nItem);
        int ret = _read_param_head_from_item_buffer(pmh, pItemAddr, ver);
        return ret;
    }

    int  parseItemBufferWithVer(libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr, uint32_t item_ver)
    {
        int ret = 0;
        ret = _read_param_head_from_item_buffer(pmh, pItemAddr, item_ver);
        if (ret < 0)
            return ret;
        ret = getReadItemBufferLayoutWithVer(pmh, pmi, pItemAddr, item_ver);
        return ret;
    }

    int  readDataFromItemBuffer(libshm_media_head_param_t *pmh, /*OUT*/libshm_media_item_param_t *pmi, /*IN*/const uint8_t *pItemAddr, uint32_t nItem)
    {
        uint32_t ver = getItemVerFromReadBuffer(pItemAddr, nItem);
        int ret = -1;

        if (ver == INVALID_SHM_HEAD_VERSION)
        {
            char sout[1024] = {0};
            _tvuutilTransBinaryToHexString(pItemAddr, 500, sout, sizeof(sout));
            DEBUG_ERROR_CR(
                        "shm item invalid version data."
                        "ptr:%p,len:%u,"
                        "cont:{%s}"
                        , pItemAddr, nItem
                        , sout
                        );
            return -EINVAL;
        }

        ret = parseItemBufferWithVer(pmh, pmi, pItemAddr, ver);

        return ret;
    }

    int  getWriteItemBufferLayoutWithVer(/*IN*/const libshm_media_item_param_t *pmi
        , /*IN*/uint8_t *pItemAddr, /*OUT*/libshm_media_item_addr_layout_t *playout, uint32_t major_version)
    {
        int         ret         = 0;

        switch (major_version)
        {
        case kLibShmMediaItemVerV1:
        case kLibShmMediaItemVerV2:
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", major_version);
            }
            break;
        case kLibShmMediaItemVerV3:
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", major_version);
            }
            break;
        case kLibShmMediaItemVerV4:
            {
                libshm_media_item_param_internal_t rii;
                {
                    memset(&rii, 0, sizeof(rii));
                }
                ret = libshmmediapro::getWriteItemBufferLayoutV4(pmi, rii, pItemAddr, playout);
            }
            break;
        default:
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", major_version);
            }
            break;
        }

        return ret;
    }

    int  applyWriteItemBufferLengthtWithVer(/*IN*/const libshm_media_item_param_t *pmi, uint32_t major_version)
    {
        int         ret         = 0;

        ret = preRequireItemHeadLength(major_version);
        if (ret < 0)
        {
            return 0;
        }
        ret += getItemParamDataLen(pmi);
        return ret;
    }

    #define NOT_EQUAL_DATA(d, s) ((s>0) && s!=d)

    #define COPY_DATA_WHILE_NOT_EQUAL(d, s) do {if((s>0) && s!=d) d=s;}while(0)

    #define INVALID_VIDEO_WIDTH(w) ((w)<=0)
    #define INVALID_VIDEO_HEIGHT(h) ((h)<=0)
    #define INVALID_VIDEO_FPS(d, s) (((d)<=0) || ((s)<=0) || ((d) > (s)))
    #define INVALID_AUDIO_DEPTH(d) (d <= 0)
    #define INVALID_AUDIO_SAMPLERATE(s) (s < 8000)
    #define INVALID_AUDIO_CHANNEL_LAYOUT(c) (c==0)

    int writeItemBufferV4(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmiv, const libshm_media_item_param_internal_t &rii, uint8_t *pItemAddr)
    {
        const libshm_media_item_param_v1_t *pmi = (const libshm_media_item_param_v1_t *)pmiv;
        bool        bvc = !(pmi->u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG);
        bool        bac = !(pmi->u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG);
        bool        bsc = !(pmi->u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG);
        bool        bec = !(pmi->u_copied_flags & LIBSHM_MEDIA_CLOSED_CAPTION_COPIED_FLAG);
        bool        buserc = !(pmi->u_copied_flags & LIBSHM_MEDIA_USER_DATA_COPIED_FLAG);

        shm_media_item_info_v4_subv0_t    *pi4 = NULL;
        int         item_head_len = 0;
        int         w_len = 0;
        int64_t     now = _libshm_get_sys_ms64();
        libshm_media_item_addr_layout_t olayout;
        {
            memset(&olayout, 0, sizeof(olayout));
        }

        libshmmediapro::getWriteItemBufferLayoutV4(pmiv, rii, pItemAddr, &olayout);

        item_head_len   = libshmmediapro::getItemHeadLengthV4();
        memset((void *)pItemAddr, 0, sizeof(shm_media_item_info_v4_t));
        pi4 = (shm_media_item_info_v4_subv0_t *)pItemAddr;

        shm_media_item_info_v3_t *pi3 = &pi4->_v3i;
        shm_media_item_info_v4_subv1_t *pi41 = (shm_media_item_info_v4_subv1_t *)pi4;

        pi4->_head_len = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(item_head_len);
        pi4->_item_version = LIBSHMMEDIA_ITEM_WRITE_SHM_U8(kLibShmMediaItemVerV4);

        {
            pi3->data_ct               = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_userDataCT);
            pi3->data_type             = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_userDataType);
            pi3->pic_flag              = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->u_picType);

            pi4->_v_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_vOffset);
            if (pmi->i_vLen > 0) {
                if (
                    INVALID_VIDEO_WIDTH(pmh->i_dstw)
                    || INVALID_VIDEO_HEIGHT(pmh->i_dsth)
                )
                {
                    DEBUG_SHMMEDIA_PROTO_WARN("video resolution [w %d, h %d] invalide\n", pmh->i_dstw, pmh->i_dsth);
                    w_len = -EINVAL;
                    return w_len;
                }

                if (
                    INVALID_VIDEO_FPS(pmh->i_duration, pmh->i_scale)
                )
                {
                    DEBUG_SHMMEDIA_PROTO_WARN("video [fps d %d, s %d] invalide\n", pmh->i_duration, pmh->i_scale);
                    w_len = -EINVAL;
                    return w_len;
                }

                pi3->frametype      = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->u_frameType);
                pi3->interlace_flag = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_interlaceFlag);
                pi3->videoct        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
                pi3->videopts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vpts);
                pi3->videodts       = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_vdts);
                pi3->vfourcc        = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->u_videofourcc);
                COPY_DATA_WHILE_NOT_EQUAL(pi3->width, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dstw));
                COPY_DATA_WHILE_NOT_EQUAL(pi3->height, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_dsth));
                COPY_DATA_WHILE_NOT_EQUAL(pi3->duration, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_duration));
                COPY_DATA_WHILE_NOT_EQUAL(pi3->scale, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_scale));

                COPY_DATA_WHILE_NOT_EQUAL(pi41->sarw, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_sarw));
                COPY_DATA_WHILE_NOT_EQUAL(pi41->sarh, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_sarh));
                COPY_DATA_WHILE_NOT_EQUAL(pi41->srcw, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_srcw));
                COPY_DATA_WHILE_NOT_EQUAL(pi41->srch, (int32_t)LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmh->i_srch));

                if (olayout.p_vData)
                {
                    pi3->videolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_vLen);
                    if (bvc && pmi->p_vData) {
                        memcpy((void *)olayout.p_vData, pmi->p_vData, pmi->i_vLen);
                    }
                }
            }

            pi4->_a_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_aOffset);
            if (pmi->i_aLen > 0) {
                if (
                    INVALID_AUDIO_CHANNEL_LAYOUT(pmh->i_channels)
                    || INVALID_AUDIO_DEPTH(pmh->i_depth)
                    || INVALID_AUDIO_SAMPLERATE(pmh->i_samplerate)
                )
                {
                    DEBUG_SHMMEDIA_PROTO_WARN("audio [depth %d, channel layout 0x%x, samplerate %d] invalide\n"
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

                if (olayout.p_aData)
                {
                    pi3->audiolen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_aLen);
                    if (bac && pmi->p_aData) {
                        memcpy((void *)olayout.p_aData, pmi->p_aData, pmi->i_aLen);
                    }

                    if (pmi->h_media_process)
                    {
                        pmi->h_media_process((uint8_t *)olayout.p_aData, pmi->i_aLen, pmi->p_opaq);
                    }
                }
            }

            pi4->_s_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_sOffset);
            if (pmi->i_sLen > 0) {
                pi3->subtct         = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(now);
                pi3->subtpts        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_spts);
                pi3->subtdts        = LIBSHMMEDIA_ITEM_WRITE_SHM_U64(pmi->i64_sdts);

                if (olayout.p_sData)
                {
                    pi3->subtlen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_sLen);
                    if (bsc && pmi->p_sData) {
                        TVUUTIL_SAFE_MEMCPY((void *)olayout.p_sData, pmi->p_sData, pmi->i_sLen);
                    }
                }
            }

            pi4->_cc_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_ccOffset);
            if (pmi->i_CCLen > 0)
            {
                if (olayout.p_CCData)
                {
                    pi3->cclen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_CCLen);
                    if (bec && pmi->p_CCData)
                    {
                        TVUUTIL_SAFE_MEMCPY((void *)olayout.p_CCData, pmi->p_CCData, pmi->i_CCLen);
                    }
                }
            }

            pi4->_timecode_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_timecodeOffset);
            if (pmi->i_timeCode > 0)
            {
                if (olayout.p_timeCode)
                {
                    pi3->timecodelen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_timeCode);
                    if (pmi->p_timeCode)
                        TVUUTIL_SAFE_MEMCPY((void *)olayout.p_timeCode, pmi->p_timeCode, pmi->i_timeCode);
                }
            }

            pi4->_user_data_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_userOffset);
            if (pmi->i_userDataLen > 0)
            {
                if (olayout.p_userData)
                {
                    pi3->datalen = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(pmi->i_userDataLen);
                    if (buserc && pmi->p_userData) {
                        TVUUTIL_SAFE_MEMCPY((void *)olayout.p_userData, pmi->p_userData, pmi->i_userDataLen);
                    }
                }
            }

            /* write key-value area */
            if (rii.nKeyValueSize_ && rii.pKeyValuePtr_)
            {
                shm_media_item_info_v4_subv0v0_t *pi400 = (shm_media_item_info_v4_subv0v0_t *)pi4;
                shm_media_item_info_v4_subv1v0_t *pi410 = (shm_media_item_info_v4_subv1v0_t *)pi4;
                pi400->_flag1_media_head_extension = 1;
                pi400->_flag1 = 0;
                pi410->_key_value_area_offset = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(olayout.i_keyValueAreaOffset);
                pi410->_key_value_area_len = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(rii.nKeyValueSize_);
                if (olayout.p_keyValuePtr)
                {
                    TVUUTIL_SAFE_MEMCPY((void *)olayout.p_keyValuePtr, rii.pKeyValuePtr_, rii.nKeyValueSize_);
                }
            }

            /* write the total item length for raw copy */
            w_len       = olayout.i_totalLen;
            pi3->length = LIBSHMMEDIA_ITEM_WRITE_SHM_U32(w_len);
        }
        return w_len;
    }

    int writeItemBuffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi, uint8_t *pItemAddr, uint32_t item_ver)
    {
        int         w_len       = 0;

        switch (item_ver)
        {
        case kLibShmMediaItemVerV3:
            {
                w_len   = -1;
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", item_ver);
            }
            break;
        case kLibShmMediaItemVerV4:
            {
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
                w_len = writeItemBufferV4(pmh, pmi, rii, pItemAddr);
            }
            break;
        default:
            {
                w_len   = -1;
                DEBUG_SHMMEDIA_PROTO_ERROR("unsupport version %d\n", item_ver);
            }
            break;
        }

        return w_len;
    }


    uint32_t _headParamGetStructSize(const libshm_media_head_param_t &p)
    {
        uint32_t strutSize = p.u_reservePrivate;

        if (strutSize >= sizeof(libshm_media_head_param_v2_t))
        {
            strutSize = sizeof(libshm_media_head_param_v2_t);
        }
        else
        {
            strutSize = sizeof(libshm_media_head_param_v1_t);
        }
        return strutSize;
    }

    uint32_t _headParamGetMinStructSize(const libshm_media_head_param_t &p1, const libshm_media_head_param_t &p2)
    {
        uint32_t strutSize1 = _headParamGetStructSize(p1);
        uint32_t strutSize2 = _headParamGetStructSize(p2);
        return TVUUTIL_MIN(strutSize1, strutSize2);
    }

    void _headParamLowCopy(libshm_media_head_param_t &dst, const libshm_media_head_param_t &src)
    {
        uint32_t strutSize = _headParamGetMinStructSize(dst, src);
        if (strutSize >= sizeof(libshm_media_head_param_v2_t))
        {
            libshm_media_head_param_v2_t *pdst = (libshm_media_head_param_v2_t *)&dst;
            const libshm_media_head_param_v2_t *psrc = (const libshm_media_head_param_v2_t *)&src;
            *pdst = *psrc;
        }
        else
        {
            libshm_media_head_param_v1_t *pdst = (libshm_media_head_param_v1_t *)&dst;
            const libshm_media_head_param_v1_t *psrc = (const libshm_media_head_param_v1_t *)&src;
            *pdst = *psrc;
        }
        return;
    }

    int _headParamLowCompare(const libshm_media_head_param_t &dst, const libshm_media_head_param_t &src)
    {
        int ret = 0;
        uint32_t strutSize = _headParamGetMinStructSize(dst, src);
        if (strutSize >= sizeof(libshm_media_head_param_v2_t))
        {
            ret = memcmp((void *)&dst, (void *)&src, sizeof(libshm_media_head_param_v2_t));
//            if (ret)
//            {
//                return ret;
//            }
//            ret = LibshmmediaAudioChannelLayoutCompare(dst.h_channel, src.h_channel);
        }
        else
        {
            ret = memcmp((void *)&dst, (void *)&src, sizeof(libshm_media_head_param_v1_t));
        }
        return ret;
    }
    const libshmmedia_audio_channel_layout_object_t *_headParamGetChannelLayout(const libshm_media_head_param_t &r)
    {
        if (r.u_reservePrivate >= sizeof(libshm_media_head_param_v2_t))
        {
            return r.h_channel;
        }
        return NULL;
    }

}

uint32_t LibShmMediaProGetItemParamDataLen(const libshm_media_item_param_t *pvi)
{
    return libshmmediapro::getItemParamDataLen(pvi);
}

unsigned int LibShmMediaProtoGetHeadVersion(const uint8_t *headBuf)
{
    return libshmmediapro::getShmMediaHeadVersion(headBuf);
}

int  LibShmMediaProtoReadItemBufferLayout(/*OUT*/libshm_media_head_param_t *pmh
                                     , /*OUT*/libshm_media_item_param_t *pmi
                                     , /*IN*/const uint8_t *pItemAddr, uint32_t nItem)
{
    return libshmmediapro::readDataFromItemBuffer(pmh, pmi, pItemAddr, nItem);
}

int  LibShmMediaProtoReadItemBufferLayoutWithHeadVer(/*OUT*/libshm_media_head_param_t *pmh
                                     , /*OUT*/libshm_media_item_param_t *pmi
                                     , /*IN*/const uint8_t *pItemAddr, uint32_t ver)
{
    return libshmmediapro::parseItemBufferWithVer(pmh, pmi, pItemAddr, ver);
}


int  LibShmMediaProtoGetWriteItemBufferLayout(/*IN*/const libshm_media_item_param_t *pmi
    , /*IN*/uint8_t *pItemAddr, uint32_t itemLen, /*OUT*/libshm_media_item_addr_layout_t *playout)
{
    int ret = libshmmediapro::getWriteItemBufferLayoutWithVer(pmi, pItemAddr, playout, LIBSHM_MEDIA_ITEM_CURRENT_VERSION);

    if (ret > 0 && (unsigned int)ret > itemLen)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("write item buffer size %u < request length %d", itemLen, ret);
        return -ENOMEM;
    }

    return ret;
}

int  LibShmMediaProtoRequireWriteItemBufferLength(/*IN*/const libshm_media_item_param_t *pmi)
{
    return libshmmediapro::applyWriteItemBufferLengthtWithVer(pmi, LIBSHM_MEDIA_ITEM_CURRENT_VERSION);;
}

int LibShmMediaProtoWriteItemBuffer(
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr)
{
    int ret = 0;
    libshm_media_item_param_t omi = *pmi;
    omi.u_copied_flags = 0xFFFFFFFF;
    ret = libshmmediapro::writeItemBuffer(pmh, &omi, pItemAddr, LIBSHM_MEDIA_ITEM_CURRENT_VERSION);
    return ret;
}

int LibShmMediaHeadParamInit(libshm_media_head_param_t *p, uint32_t structSize)
{
    if (!p)
    {
        return -1;
    }

    if (structSize >= sizeof(libshm_media_head_param_v2_t))
    {
        structSize = sizeof(libshm_media_head_param_v2_t);
    }
    else
    {
        structSize = sizeof(libshm_media_head_param_v1_t);
    }

    {
        p->u_reservePrivate = structSize;
        memset((uint8_t*)p+4, 0, structSize-4);
    }

    return 0;
}

void LibShmMediaHeadParamRelease(libshm_media_head_param_t *p)
{
    return;
}

int LibShmMediaItemParamInit(libshm_media_item_param_t *p, uint32_t structSize)
{
    if (!p)
    {
        return -1;
    }

    if (structSize >= sizeof(libshm_media_item_param_v1_t))
    {
        structSize = sizeof(libshm_media_item_param_v1_t);
    }
    else
    {
        structSize = sizeof(libshm_media_item_param_v1_t);
    }

    {
        p->u_reservePrivate = structSize;
        memset((uint8_t*)p+4, 0, structSize-4);
    }

    return 0;
}

void LibShmMediaItemParamRelease(libshm_media_item_param_t *p)
{
    return;
}

int LibShmMediaRawDataParamInit(libshmmedia_raw_data_param_t *p, uint32_t structSize)
{
    if (!p)
    {
        return -1;
    }

    if (
        structSize >= sizeof(libshmmedia_raw_data_param_v2_t)
    )
    {
        structSize = sizeof(libshmmedia_raw_data_param_v2_t);
        memset(p, 0, structSize);
        p->uReserverPrivate_ = structSize;
    }
    else
    {
        structSize = sizeof(libshmmedia_raw_data_param_v1_t);
        memset(p, 0, structSize);
    }

    return 0;
}

//void LibShmMediaRawDataParamRelease(libshmmedia_raw_data_param_t *p)
//{
//    return;
//}

int LibShmMediaRawHeadParamInit(libshmmedia_raw_head_param_t *p, uint32_t structSize)
{
    if (!p)
    {
        return -1;
    }

    if (
        structSize != sizeof(libshmmedia_raw_head_param_v2_t)
    )
    {
        return -1;
    }

    {
        memset(p, 0, structSize);
        p->uReservePrivate_ = structSize;
    }

    return 0;
}

//void LibShmMediaRawHeadParamRelease(libshmmedia_raw_head_param_t *p)
//{
//    return;
//}

