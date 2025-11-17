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
#ifndef _SHARE_MEMORY_NEW_H
#define _SHARE_MEMORY_NEW_H

#include <stdint.h>
#include <stdlib.h>

#define SHM_FLAG_READ           1
#define SHM_FLAG_WRITE          2

#if defined(TVU_WINDOWS)
#include <Winsock2.h>
#include <windows.h>
#endif

#define MAX_SHARE_MEMROY_NAME   256
#define USE_POSIX_SHM 1

class CTvuBaseShareMemory
{
public:
    CTvuBaseShareMemory(void);
    ~CTvuBaseShareMemory(void);

    int InitCreateShm(uint32_t header_len, uint32_t item_count, uint32_t item_length);
    int InitOpenShm();
    uint8_t *CreateOrOpen(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, void (*clean_data)(uint8_t *, bool) = NULL);
    uint8_t *Open(const char * pMemoryName);
    int CloseMapFile();
    int CloseMapFileAndSleep();
    uint8_t *GetHeader() { return m_pHeader; }
    const char *GetName() { return m_memoryName; }
#if defined (TVU_LINUX)
    int     GetShmId(){return m_iShmId;}
    static int RemoveShmFromKernal(const char *shmname);
#endif
    uint32_t    GetHeadLen() { return m_uHeadLen; }
    uint32_t    GetWriteIndex();
    uint8_t     *GetWriteItemAddr();
    uint8_t     *GetItemAddrByIndex(uint32_t index);
    void        SetFlag(uint32_t flag);
    void        FinishWrite();
    void        FinishRead();
    bool        HasReaders(unsigned int timeout = 100 /* default 100ms */);

    /**
     *  > 0 : ready
     *  0   : waiting
     *  < 0 : EPERM, no privilege
     */
    int     Sendable();
    int     Readable(bool bClosed);
    bool    IsCreator();

    uint32_t    GetItemLength();
    uint32_t    GetItemCounts();
    uint32_t    GetItemOffset();
    uint32_t    GetShmVersion();
    int         GetShmFlag();
    uint32_t    GetReadIndex();
    void        SetReadIndex(uint32_t index);
    uint32_t    GetExtBuffLen();

    static uint32_t     GetShmVerBeforeCreate();
    static uint32_t     GetPreExtBuffSize();
private:
    uint32_t    m_uVersion;
    uint32_t    m_uHeadLen;
    uint32_t    m_uItemLen;
    uint32_t    m_uItemCounts;
    uint32_t    m_uExtBufLen;
#if defined TVU_WINDOWS || defined TVU_MINGW
    HANDLE m_hMapFile;//<the handle of memory-mapped file
#elif defined TVU_LINUX
#if defined(USE_POSIX_SHM)
    size_t  m_iShmSize;
#else
    key_t       m_iKey;
#endif
    int         m_iShmId;
#endif
    uint8_t *m_pHeader;
    char    m_memoryName[MAX_SHARE_MEMROY_NAME];
    uint32_t    m_uReadIndex;
    int     m_iFlags;
    int64_t    m_tmRemoveCheck;

#if defined (TVU_LINUX)
    uint8_t *_open(const char * pMemoryName, bool bForWriting = false);
    bool _isShmRemovedFromKernal();
#endif
    int     _readable(bool bClosed);
    inline
    void    _setReadTime();
};

#endif

