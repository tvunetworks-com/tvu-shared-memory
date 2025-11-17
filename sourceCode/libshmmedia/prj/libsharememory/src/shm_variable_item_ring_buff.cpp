/******************************************************
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
 *  File:
 *      sharememory.cpp
 *  CopyRight:
 *      tvu
 *  Description:
 *      Share Memory base class.
 *  History:
 *      In 2012, Eros Lin first initialize it.
 *      In April, 2014, Lotus, adding checking flag to
 *  solve dynamic share memory problem, version from 1 upgrade to 2.
******************************************************/

//#ifdef _LIBSHM_VARIABLE_ITEM_M

#if defined (TVU_MINGW)
#include <io.h>
#endif
#include <string.h>
#include <stdio.h>
#include "shmhead.h"
//#include "tvu_util.h"
#include "shm_variable_item_ring_buff.h"
#include "buildversion.h"

#if  _TVU_VIARIABLE_SHM_FEATURE_ENABLE
#include "TvuShmSharedCompactRingBuffer.h"
#endif

#define DEBUG_PRINTF(fmt, ...)      DEBUG_INFO(fmt, ##__VA_ARGS__)
#define WAIT_MS_NUM                 1000

#if !defined (TVU_WINDOWS) || defined(TVU_MINGW)
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#elif defined(TVU_WINDOWS)
#if (_MSC_VER == 1500)
#define snprintf(p, count, fmt, ...)    _snprintf(p, (count)-1, fmt, ##__VA_ARGS__)
#endif
#endif

#if _TVU_VIARIABLE_SHM_FEATURE_ENABLE

#define PREFIX_KEY_FILE "/dev/shm"

#define SHM_ENDIAN_CONVERT  0 /* shm head is only local, not part of RShm. */

#if SHM_ENDIAN_CONVERT
#define LIBSHMMEDIA_READ_SHM_U64(v)         _tvuutil_letoh64(v)
#define LIBSHMMEDIA_READ_SHM_U32(v)         _tvuutil_letoh32(v)
#define LIBSHMMEDIA_READ_SHM_U16(v)         _tvuutil_letoh16(v)

#define LIBSHMMEDIA_WRITE_SHM_U64(v)        _tvuutil_htole64(v)
#define LIBSHMMEDIA_WRITE_SHM_U32(v)        _tvuutil_htole32(v)
#define LIBSHMMEDIA_WRITE_SHM_U16(v)        _tvuutil_htole16(v)

#else
#define LIBSHMMEDIA_READ_SHM_U64(v)         (v)
#define LIBSHMMEDIA_READ_SHM_U32(v)         (v)
#define LIBSHMMEDIA_READ_SHM_U16(v)         (v)

#define LIBSHMMEDIA_WRITE_SHM_U64(v)        (v)
#define LIBSHMMEDIA_WRITE_SHM_U32(v)        (v)
#define LIBSHMMEDIA_WRITE_SHM_U16(v)        (v)
#endif

static inline bool  _is_shm_removed(const char *__name)
{
    struct stat os;
    char    skeyfile[1024]  = {0};

    snprintf(skeyfile, sizeof(skeyfile), "%s/%s", PREFIX_KEY_FILE, __name);
    int ret = stat(skeyfile, &os);

    return ( ret == 0 ) ? false : true;
}

uint64_t  CTvuVariableItemBaseShm::GetReadIndex()
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    return ptr->GetReadIndex();
}

uint64_t  CTvuVariableItemBaseShm::GetWriteIndex()
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    return ptr->GetWriteIndex();
}

void CTvuVariableItemBaseShm::SeekReadIndex2WriteIndex()
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    //uint64_t windex_1 = ptr->GetWriteIndex();
    ptr->SetReadIndex(-1);
}

void CTvuVariableItemBaseShm::SeekReadIndex2Zero()
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    ptr->SetReadIndex(0);
}

void CTvuVariableItemBaseShm::SeekReadIndex(uint64_t rindex)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    ptr->SetReadIndex(rindex);
}

uint8_t *CTvuVariableItemBaseShm::GetWriteItemAddr(size_t s)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    uint8_t *pret = NULL;
    if (ptr)
        pret = (uint8_t *)ptr->Apply(s);
    return pret;
}

bool CTvuVariableItemBaseShm::FinishWrite(const void *buff, size_t s)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;

    bool bRet = false;

    if (!(m_iFlags & SHM_FLAG_WRITE))
        return false;

    bRet = ptr->Commit(buff, s);

    return bRet;
}

uint8_t *CTvuVariableItemBaseShm::GetReadItemAddr(size_t *ps)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    uint8_t *pi = NULL;
    if (ptr)
    {
        pi = (uint8_t *)ptr->Read(ps);
    }
    return pi;
}

uint8_t *CTvuVariableItemBaseShm::GetReadItemAddrWithNoStep(size_t *ps)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    uint8_t *pi = NULL;
    if (ptr)
    {
        pi = (uint8_t *)ptr->ReadNoStep(ps);
    }
    return pi;
}

bool CTvuVariableItemBaseShm::FinishRead()
{
//    if (m_iFlags & SHM_FLAG_READ)
//        m_uReadIndex++;
    return true;
}

int CTvuVariableItemBaseShm::InitCreateShm(uint32_t header_len, uint32_t item_count, uint32_t shm_total_size)
{
    shm_construct_t   *p1 = (shm_construct_t*)m_pHeader;

    p1->version = LIBSHMMEDIA_WRITE_SHM_U32(INVALID_SHM_HEAD_VERSION);
    m_uItemCounts = item_count;
    p1->item_count = LIBSHMMEDIA_WRITE_SHM_U32(item_count);
    m_uShmTotalSize = shm_total_size;
    p1->item_length = LIBSHMMEDIA_WRITE_SHM_U32(shm_total_size);
    m_uHeadLen = header_len;
    p1->item_offset = LIBSHMMEDIA_WRITE_SHM_U32(header_len);
    p1->item_current = LIBSHMMEDIA_WRITE_SHM_U32(0);

#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    pext->ext_ver   = SHM_CONSTRUCT_EXT_VER;
    pext->reserve   = 0;
    m_uExtBufLen    =
    pext->ext_len   = sizeof(shm_construct_ext_t);
    memset(pext->a_reserve_, 0, sizeof(pext->a_reserve_));
    pext->item_current_64 = 0;
    pext->last_read_time_stamp = 0;
#endif

    m_uVersion = MEMHEADER_VARIABLE_ITEM_SHM_CURRENT_VERSION;
    p1->version = LIBSHMMEDIA_WRITE_SHM_U32(m_uVersion);

    return 0;
}

int CTvuVariableItemBaseShm::InitOpenShm()
{
    int             ret = 0;
    shm_construct_t *p1 = (shm_construct_t *)m_pHeader;
    int             nLoop = 0;
    tvushm::SharedCompactRingBuffer* ptr = ((tvushm::SharedCompactRingBuffer*)m_pRingShm);

    while (CHECK_INVALID_SHM_HEAD_VERSION(LIBSHMMEDIA_READ_SHM_U32(p1->version)))
    {
        if (nLoop == WAIT_MS_NUM)
        {
            DEBUG_ERROR("can not get valid shm version\n");
            ret = -1;
            break;
        }
        _libshm_common_msleep(1);
        nLoop++;
    }

    if (ret == 0)
    {
        m_uVersion = LIBSHMMEDIA_READ_SHM_U32(p1->version);
        m_uHeadLen = LIBSHMMEDIA_READ_SHM_U32(p1->item_offset);
        m_uShmTotalSize = LIBSHMMEDIA_READ_SHM_U32(p1->item_length);
        m_uItemCounts = LIBSHMMEDIA_READ_SHM_U32(p1->item_count);
        {
            ptr->SetReadIndex(ptr->GetWriteIndex());
        }
#if _SHM_HEAD_FEATURE_EXT_EABLE
        shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
        m_uExtBufLen = 0;
        switch(pext->ext_ver)
        {
            case kShmConstructExtVer1:
            {
                m_uExtBufLen = pext->ext_len;
            }
            break;
            default:
            {

            }
            break;
        }
#endif
    }

    return ret;
}

CTvuVariableItemBaseShm::CTvuVariableItemBaseShm(void)
    : m_uVersion(0)
    , m_uHeadLen(0)
    , m_uShmTotalSize(0)
    , m_uItemCounts(0)
{
    m_pHeader = NULL;
    memset(m_memoryName, 0, MAX_SHARE_MEMROY_NAME);
    m_iFlags = 0;
    _bForCreate = false;
    CreateRingShm();
    m_tmRemoveCheck = 0;
}

CTvuVariableItemBaseShm::~CTvuVariableItemBaseShm(void)
{
    FreeRingShm(_bForCreate);
}

uint8_t *CTvuVariableItemBaseShm::CreateOrOpen(const char *pMemoryName, const uint32_t header_len, const uint32_t item_count, const size_t shm_total_size)
{
    bool bshm = false;
    size_t head_size = 0;
    size_t isize = shm_total_size;

    _bForCreate = true;

    if (!m_pHeader)
    {
        strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME - 1);

        m_pHeader = _open(pMemoryName, true);

        if (m_pHeader)
        {
            DEBUG_PRINTF("Open shared memory of file[%s] successfully\n", pMemoryName);
            /* LL: open, should check init status */
            goto EXIT;
        }

        bshm = RingShmCreate(pMemoryName, header_len, isize, item_count);

        if (!bshm)
        {
            DEBUG_ERROR("Create SHM %s failed.\n", pMemoryName);
            m_pHeader = NULL;
            RingShmDestroy(_bForCreate);
            return NULL;
        }

        GetRingShmFixedData(&m_pHeader, &head_size);

        if (!m_pHeader || !head_size)
        {
            DEBUG_ERROR("Create SHM [%s], get head buffer failed.\n", pMemoryName);
            m_pHeader = NULL;
            RingShmDestroy(_bForCreate);
            return NULL;
        }

        uint64_t ringShmCount = getRingShmMaxItemsNum();
        uint64_t ringShmPayloadSize = getRingShmPayloadSize();

        if (InitCreateShm(header_len, ringShmCount, ringShmPayloadSize) < 0)
        {
            DEBUG_ERROR("create init %s failed\n", pMemoryName);
            RingShmDestroyAndSleep(_bForCreate);
            return NULL;
        }

        DEBUG_INFO("create vi shm success."
            "nm:%s, ver %d, head len : %d, counts : %d, total size : %d, build version{%s}, build version number %d\n"
            , m_memoryName
            , m_uVersion
            , m_uHeadLen
            , m_uItemCounts
            , m_uShmTotalSize
            , BUILD_VERSION
            , BUILD_VERSION_NUM
        );
    }

EXIT:
    m_iFlags = SHM_FLAG_WRITE;
    return m_pHeader;
}


bool CTvuVariableItemBaseShm::_isShmRemovedFromKernal()
{
    bool  bret = false;
#if defined (TVU_LINUX)
    int64_t now = _libshm_get_sys_ms64();

#define kShmRemovedSatusCheckTimeDuration   (1000LL)

    if (now - m_tmRemoveCheck > kShmRemovedSatusCheckTimeDuration)
    {
        bret = _is_shm_removed(m_memoryName);
        m_tmRemoveCheck = now;
    }
#endif
    return bret;
}

uint8_t *CTvuVariableItemBaseShm::_open(const char *pMemoryName, bool bForWriting)
{
    bool bopen = RingShmOpen(pMemoryName);
    size_t fixdatalen = 0;

    if (!bopen)
    {
        return NULL;
    }

    GetRingShmFixedData(&m_pHeader, &fixdatalen);

    if (!m_pHeader || !fixdatalen)
    {
        RingShmDestroy(bForWriting);
        return NULL;
    }

    if (bForWriting)
    {
        m_iFlags = SHM_FLAG_WRITE;
    }
    else
    {
        m_iFlags = SHM_FLAG_READ;
    }
    strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME - 1);
    DEBUG_PRINTF("Open SHM Success %s %p, build version{%s}, build version number %d\n", pMemoryName, m_pHeader, BUILD_VERSION, BUILD_VERSION_NUM);
    if (InitOpenShm() < 0) {
        DEBUG_ERROR("open init  %s failed\n", pMemoryName);
        RingShmDestroyAndSleep(_bForCreate);
    }

    return m_pHeader;
}

uint8_t *CTvuVariableItemBaseShm::Open(const char *pMemoryName)
{
    return _open(pMemoryName, false);
}

bool CTvuVariableItemBaseShm::RingShmOpen(const char *name)
{
    bool b = false;
    tvushm::SharedCompactRingBuffer* ptr = (tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (!ptr)
        return b;

    b = ptr->Open(name);
    return b;
}

bool CTvuVariableItemBaseShm::RingShmCreate(const char *pMemoryName, uint32_t header_len, uint32_t isize, uint32_t item_count)
{
    bool b = false;
    tvushm::SharedCompactRingBuffer* ptr = (tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (!ptr)
        return b;

    b = ptr->Create(pMemoryName, header_len, isize, item_count);
    return b;
}

int CTvuVariableItemBaseShm::RingShmDestroy(bool flagCreate)
{
    tvushm::SharedCompactRingBuffer* ptr = (tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (ptr)
    {
        DEBUG_INFO(
                    "deattached ring shm from process."
                    "name:%s"
                    "\n"
                    , m_memoryName
                    );
        if (flagCreate)
        {
            ptr->Destroy();
        }
        else
        {
            ptr->Close();
        }
    }
    return 0;
}

int CTvuVariableItemBaseShm::RingShmDestroyAndSleep(bool b)
{
    RingShmDestroy(b);

    /* 100ms, waiting other process close shm */
    _libshm_common_msleep(100);
    return 0;
}

int CTvuVariableItemBaseShm::CreateRingShm()
{
    m_pRingShm = new tvushm::SharedCompactRingBuffer();
    return 0;
}

void CTvuVariableItemBaseShm::GetRingShmFixedData(uint8_t **pp, size_t *plen)
{
    tvushm::SharedCompactRingBuffer* ptr = (tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (!ptr)
        return;

    *pp = (uint8_t *)ptr->GetFixedData(plen);
    return;
}

int CTvuVariableItemBaseShm::FreeRingShm(bool flagCreate)
{
    RingShmDestroy(flagCreate);
    if (m_pRingShm)
    {
        delete ((tvushm::SharedCompactRingBuffer*)m_pRingShm);
        m_pRingShm = NULL;
    }
    return 0;
}

bool CTvuVariableItemBaseShm::HasReaders(unsigned int timeout)
{
    bool bret = true;
#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    switch(pext->ext_ver)
    {
        case kShmConstructExtVer1:
        {
            uint64_t now = _libshm_get_sys_ms64();
            uint64_t time_stamp = pext->last_read_time_stamp;

            if ((int64_t)(now - time_stamp) > timeout || (time_stamp == 0) )
            {
                bret = false;
            }
        }
        break;
        default:
        {

        }
        break;
    }
#endif

    return bret;
}

int CTvuVariableItemBaseShm::Sendable()
{
    if (m_iFlags & SHM_FLAG_WRITE) {
        int ret = -1;

#ifdef TVU_LINUX
        {
            if (_isShmRemovedFromKernal())
            {
                DEBUG_ERROR_CR(
                            "sendable failed for the shm was removed."
                            "name:%s"
                            , m_memoryName);
                ret = -1;
                return ret;
            }
        }
#endif

        tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
        if (ptr)
        {
            if (!ptr->IsValid())
            {
                return -1;
            }
            ret = ptr->IsWriteable() ? 1 : 0;  // model regards reading faster then writing
        }
        return ret;
    }
    else {
        return -1;
    }
}

int CTvuVariableItemBaseShm::_readable()
{
    if (m_iFlags & SHM_FLAG_READ) {
        tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
        int ret = -1;
        if (ptr)
        {
            if (!ptr->IsValid())
            {
                return -1;
            }
            ret = ptr->IsReadable() ? 1 : 0;
        }
        return ret;
    }
    else {
        return -1;
    }
}

void CTvuVariableItemBaseShm::_setReadTime()
{
#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    switch(pext->ext_ver)
    {
        case kShmConstructExtVer1:
        {
            pext->last_read_time_stamp = _libshm_get_sys_ms64();
        }
        break;
        default:
        {

        }
        break;
    }
#endif
    return;
}

int CTvuVariableItemBaseShm::Readable()
{
    int ret = _readable();
#ifdef TVU_LINUX
    if (ret == 0)
    {
        if (_isShmRemovedFromKernal())
        {
            ret = -1;
        }
    }
#endif
    _setReadTime();
    return ret;
}

bool CTvuVariableItemBaseShm::IsCreator()
{
    return m_iFlags & SHM_FLAG_WRITE;
}



int CTvuVariableItemBaseShm::RemoveShmFromKernal(const char *shmname)
{
    tvushm::SharedCompactRingBuffer oshm;
    bool bopen = oshm.Open(shmname);

    if (bopen)
    {
        oshm.Destroy();
    }

    return 0;
}

void CTvuVariableItemBaseShm::GetInfo(uint64_t *pheadsize, uint64_t *ptotal_size, uint64_t *pmax_counts)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;

    if (ptr)
        ptr->GetInfo(pheadsize, ptotal_size, pmax_counts);
}

bool CTvuVariableItemBaseShm::SearchWholeItems(void *user, tvu_variableitem_base_shm_item_valid_determine_fn_t fn)
{
    tvushm::SharedCompactRingBuffer *ptr = (tvushm::SharedCompactRingBuffer *)m_pRingShm;
    bool bget = false;

    if (ptr)
    {
        uint64_t windex = ptr->GetWriteIndex();
        uint64_t searchCount = ptr->GetMaxItemNum();
        uint64_t searchIndex = ptr->PreviousIndexStep(windex);

        if (searchCount>=2)
        {
            searchCount -= 1;
        }

        for (uint64_t i = 0; i < searchCount; i++)
        {
            size_t buf_len = 0;
            void *pbuf = ptr->ReadByPos(&buf_len, searchIndex);

            if (pbuf && buf_len)
            {
                if (fn)
                {
                    int ret = fn(user, pbuf, buf_len, searchIndex);
                    if (ret < 0)
                    {
                        /* invalid data */
                        break;
                    }
                    else if (ret > 0)
                    {
                        bget = true;
                        break;
                    }
                }
            }
            else
            {
                break;
            }

            searchIndex = ptr->PreviousIndexStep(searchIndex);
        }

        if (bget)
        {
            ptr->SetReadIndex(searchIndex);
        }
        else
        {
            ptr->SetReadIndex(windex);
        }
    }

    return bget;
}

uint64_t CTvuVariableItemBaseShm::getRingShmPayloadSize()const
{
    uint64_t  ret = 0;
    const tvushm::SharedCompactRingBuffer* ptr = (const tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (!ptr)
        return ret;

    ret = ptr->GetPayloadSize();
    return ret;
}

uint64_t CTvuVariableItemBaseShm::getRingShmMaxItemsNum()const
{
    uint64_t  ret = 0;
    const tvushm::SharedCompactRingBuffer* ptr = (const tvushm::SharedCompactRingBuffer*)m_pRingShm;
    if (!ptr)
        return ret;

    ret = ptr->GetMaxItemNum();
    return ret;
}

#endif //_TVU_VIARIABLE_SHM_FEATURE_ENABLE
