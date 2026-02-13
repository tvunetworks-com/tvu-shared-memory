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
/*
Copyright 2005-2020, TVU networks. All rights reserved.
For internal members in TVU networks only.

Purpose: Foundation Library Header File
Modifiers: Tony Guo (tonyguo@tvunetworks.com)
--------------------------------------------------
History:
1. Tony Guo on May 15, 2020: Created
2. Lotus on July 26th 2022, transport shared memory out from libtvufoundatation.
His
*/

#pragma once
#include "TvuVaItemSharedMemory.h"
#include <stdint.h>
#include <sys/types.h>

namespace tvushm
{
    class SharedCompactRingBuffer
    {
    public:
        SharedCompactRingBuffer(void);
        ~SharedCompactRingBuffer(void);

        bool Open(const char*name);
        bool Create(const char*name,uint64_t fixedUserDataSize,uint64_t payloadBufferSize,uint64_t maxItemsNum);
        bool Create(const char*name,uint64_t fixedUserDataSize,uint64_t payloadBufferSize,uint64_t maxItemsNum,mode_t mode);

        bool GetInfo(uint64_t* fixedUserDataSizePtr=NULL,uint64_t* payloadBufferSizePtr=NULL,uint64_t* maxItemsNumPtr=NULL);

        void Close();
        void Destroy();

        bool Write(const void*buffer,size_t size);

        void* Apply(size_t maxSize);
        bool Commit(const void*buffer,size_t size);

        void* Read(size_t *sizePtr);
        void* ReadNoStep(size_t *sizePtr);
        /**
         *  Read directly, but not check writing status
        **/
        void *ReadByPos(size_t *sizePtr, uint64_t pos);
        uint64_t NextIndexStep(uint64_t pos)const;
        uint64_t PreviousIndexStep(uint64_t pos)const;

        void* GetFixedData(size_t *sizePtr);

        bool WriteFixedData(const void*buffer,size_t size,size_t offset);

        bool CommitFixedData(const void*buffer, size_t size);

        bool IsReadable(void);

        bool IsWriteable(void);

        uint64_t GetWriteIndex(void) const;

        uint64_t GetMaxItemNum()const;
        uint64_t GetPayloadSize()const;

        uint64_t GetReadIndex(void) const;

        bool SetReadIndex(uint64_t nextReadingIndex);

        // TODO, not used
        uint64_t SeekEarliestReadIndex(uint64_t windex)const;

        bool IsValid()const;

        bool IsActive();
        bool SetInactiveFlag();
        bool ClearInactiveFlag();

        bool IsRequested();
        bool SetRequestedFlag();
        bool ClearRequestedFlag();

    private:
        void  _previousReadingIndex();
        void* _getControlInfo();
        uint64_t _getMaxItemIndex()const;
        bool    _isValidShmData()const;
    private:
        SharedMemory _sm;
        uint64_t _nextReadingIndex;
    };
}
