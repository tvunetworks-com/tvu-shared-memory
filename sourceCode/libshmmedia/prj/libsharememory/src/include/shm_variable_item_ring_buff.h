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
 *      sharememory.h
 *  CopyRight:
 *      tvu
 *  Description:
 *      Share Memory base class.
 *  History:
 *      In 2012, Eros Lin first initialize it.
 *      In April, 2014, Lotus, adding checking flag to
 *  solve dynamic share memory problem, version from 1 upgrade to 2.
******************************************************/
#ifndef _SHARE_MEMORY_VIRIABLE_ITEM_H
#define _SHARE_MEMORY_VIRIABLE_ITEM_H

#if defined (TVU_WINDOWS) || defined (TVU_MINGW)
#   include <Winsock2.h>
#   include "stdint.h"
#   include <sys/timeb.h>
#else
#   include <sys/ipc.h>
#   include <sys/shm.h>
#   include <stdint.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <unistd.h>
#   include <string>
#endif

//#include "sysdef.h"
#include "shmhead.h"
#include "sharememory_internal.h"

//#ifdef _LIBSHM_VARIABLE_ITEM_M

#if defined (TVU_LINUX) || defined(TVU_WINDOWS)
#define _TVU_VIARIABLE_SHM_FEATURE_ENABLE   1
#endif

#if _TVU_VIARIABLE_SHM_FEATURE_ENABLE

//#include "TvuShmSharedCompactRingBuffer.h"

typedef int (*tvu_variableitem_base_shm_item_valid_determine_fn_t)(void *ctx, const void *, size_t, uint64_t pos);

class CTvuVariableItemBaseShm
{
private:
    uint32_t    m_uVersion;
    uint32_t    m_uHeadLen;
    uint32_t    m_uShmTotalSize;
    uint32_t    m_uItemCounts;
    uint32_t    m_uExtBufLen;

    uint8_t     *m_pHeader;
    char        m_memoryName[MAX_SHARE_MEMROY_NAME];
    int         m_iFlags;
    bool        _bForCreate;
    void        *m_pRingShm;
    int64_t     m_tmRemoveCheck;

    uint8_t *_open(const char * pMemoryName, bool bForWriting = false);
    int     _readable();
    void    _setReadTime();
    bool    _isShmRemovedFromKernal();

public:
    CTvuVariableItemBaseShm(void);
    virtual ~CTvuVariableItemBaseShm(void);

    int InitCreateShm(uint32_t header_len, uint32_t item_count, uint32_t item_length);
    int InitOpenShm();
    uint8_t *CreateOrOpen(const char * pMemoryName, const uint32_t header_len, const uint32_t item_count, const size_t item_length);
    uint8_t *CreateOrOpen(const char * pMemoryName, const uint32_t header_len, const uint32_t item_count, const size_t item_length, mode_t mode);
    uint8_t *Open(const char * pMemoryName);
    bool RingShmOpen(const char *name);
    bool RingShmCreate(const char *pMemoryName, uint32_t header_len, uint32_t isize, uint32_t item_count);
    bool RingShmCreate(const char *pMemoryName, uint32_t header_len, uint32_t isize, uint32_t item_count, mode_t mode);
    int RingShmDestroy(bool flagCreate);
    int RingShmDestroyAndSleep(bool flagCreate);
    int FreeRingShm(bool flagCreate);
    int CreateRingShm();
    void GetRingShmFixedData(uint8_t **pp, size_t *plen);
    uint8_t *GetHeader() { return m_pHeader; }
    const char *GetName() { return m_memoryName; }
    static int RemoveShmFromKernal(const char *shmname);

    bool HasReaders(unsigned int timeout);
    /**
    *  > 0 : ready
    *  0   : waiting
    *  < 0 : EPERM, no privilege
    */
    int     Sendable();
    int     Readable();
    bool    IsCreator();

    void GetInfo(uint64_t *, uint64_t *, uint64_t *);

    uint32_t    GetItemCounts()
    {
        return m_uItemCounts;
    }

    uint32_t    GetHeadLen()
    {
        return m_uHeadLen;
    }

    uint32_t    GetShmVersion()
    {
        return m_uVersion;
    }

    static
        uint32_t    GetShmVerBeforeCreate()
    {
        return MEMHEADER_VARIABLE_ITEM_SHM_CURRENT_VERSION;
    }

    int     GetShmFlag()
    {
        return  m_iFlags;
    }

    uint64_t  GetReadIndex();

    uint64_t  GetWriteIndex();

    void SeekReadIndex2WriteIndex();
    void SeekReadIndex2Zero();
    void SeekReadIndex(uint64_t rindex);

    uint8_t *GetWriteItemAddr(size_t s);

    bool FinishWrite(const void *buff, size_t s);
    uint8_t *GetReadItemAddr(size_t *ps);
    uint8_t *GetReadItemAddrWithNoStep(size_t *ps);
    bool FinishRead();

    bool SearchWholeItems(void *, tvu_variableitem_base_shm_item_valid_determine_fn_t fn);
private:
    uint64_t getRingShmPayloadSize()const;
    uint64_t getRingShmMaxItemsNum()const;
};

#endif //_TVU_VIARIABLE_SHM_FEATURE_ENABLE

#endif

