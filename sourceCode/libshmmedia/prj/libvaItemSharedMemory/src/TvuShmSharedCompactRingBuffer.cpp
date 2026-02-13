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
------------------------------------------------------------------
1. History: Tony Guo on May 15, 2020: Created
2. History: Lotus on July 26th 2022, transport shared memory out from libtvufoundatation.
*/
//#include "TvuMemDbg.h"
#include "TvuShmSharedCompactRingBuffer.h"
#include "TvuCrossPlatformDefines.h"
#include <sys/stat.h>
#include "TvuMemUtils.h"
#include "TvuLog.h"
#include "TvuTimeUtils.h"
#include "TvuFormatUtils.h"
#include "TvuCommonDefines.h"
#include <string.h>
#include <algorithm>

//WARNINGS: DON'T CHANGE BELOW LINE OF MAGIC CODE
#define TVU_SHARED_COMPACT_RING_BUFFER_MAGIC_CODE        "TVUCRB@20200518"

namespace tvushm
{
#pragma pack (push,1)
    class SharedCompactRingBufferImpl
    {
    public:
        //this structure is architecture independent. 32 bit process and 64 bit process can
        //share the same memory;
        struct ControlDataV1
        {
            char magicCode[16];
            uint64 controlDataSize;
            uint64 totalSharedMemorySize;
            uint64 fixedUserDataOffset;
            uint64 fixedUserDataSize;
            uint64 indexDataOffset;
            uint64 indexDataSize;
            uint64 maxItemsNum;
            uint64 maxItemIndex;
            uint64 indexItemSize;
            uint64 actualIndexItemSize;
            uint64 payloadDataOffset;
            uint64 payloadDataSize;
            uint64 nextFreeItemIndex;
            uint64 lastFilledItemIndex;
            uint64 nextFreeItemPayloadOffset;
            uint64 itemIndexChecksum; //==nextFreeItemIndex+lastFilledItemIndex;
        };

        struct ControlDataV2:ControlDataV1
        {
            //flag 0~7: for creator/writer
            byte inactiveFlag:1; //need to close and reopen;
            byte reservedFlag1:1;
            byte reservedFlag2:1;
            byte reservedFlag3:1;
            byte reservedFlag4:1;
            byte reservedFlag5:1;
            byte reservedFlag6:1;
            byte reservedFlag7:1;
            //flag 8~15: for reader
            byte reservedFlag8:1;
            byte reservedFlag9:1;
            byte reservedFlag10:1;
            byte reservedFlag11:1;
            byte reservedFlag12:1;
            byte reservedFlag13:1;
            byte reservedFlag14:1;
            byte reservedFlag15:1;
            //flag: 16~23: reader/writer shared;
            byte requestedFlag:1;
            byte reservedFlag17:1;
            byte reservedFlag18:1;
            byte reservedFlag19:1;
            byte reservedFlag20:1;
            byte reservedFlag21:1;
            byte reservedFlag22:1;
            byte reservedFlag23:1;
            //TODO, flag: 24~31: versionminus1
            byte reservedFlag24:1;
            byte reservedFlag25:1;
            byte reservedFlag26:1;
            byte reservedFlag27:1;
            byte reservedFlag28:1;
            byte reservedFlag29:1;
            byte reservedFlag30:1;
            byte reservedFlag31:1;
            byte reservedFlag32:1;
            byte reservedFlag33:1;
            byte reservedFlag34:1;
            byte reservedFlag35:1;
            byte reservedFlag36:1;
            byte reservedFlag37:1;
            byte reservedFlag38:1;
            byte reservedFlag39:1;
            byte reservedFlag40:1;
            byte reservedFlag41:1;
            byte reservedFlag42:1;
            byte reservedFlag43:1;
            byte reservedFlag44:1;
            byte reservedFlag45:1;
            byte reservedFlag46:1;
            byte reservedFlag47:1;
            byte reservedFlag48:1;
            byte reservedFlag49:1;
            byte reservedFlag50:1;
            byte reservedFlag51:1;
            byte reservedFlag52:1;
            byte reservedFlag53:1;
            byte reservedFlag54:1;
            byte reservedFlag55:1;
            byte reservedFlag56:1;
            byte reservedFlag57:1;
            byte reservedFlag58:1;
            byte reservedFlag59:1;
            byte reservedFlag60:1;
            byte reservedFlag61:1;
            byte reservedFlag62:1;
            byte reservedFlag63:1;
            uint64 reserved1;
            uint64 reserved2;
            uint64 reserved3;
            uint64 reserved4;
            uint64 reserved5;
            uint64 reserved6;
            uint64 reserved7;
        };

        typedef struct ControlDataV1 MinimumControlData;
        typedef struct ControlDataV2 MaximumControlData;

        struct IndexItemV1
        {
            uint64 index;
            uint64 offset;
            uint64 size;
            uint64 checksum; //index+offset+size;
        };

        typedef IndexItemV1 IndexItem;

        enum ControlMasks
        {
            InactiveControlMask=0x1
        };
    };
#pragma pack(pop)

    SharedCompactRingBuffer::SharedCompactRingBuffer(void)
    {
        _nextReadingIndex=(uint64)(-1);
    }

    SharedCompactRingBuffer::~SharedCompactRingBuffer(void)
    {
    }

    bool SharedCompactRingBuffer::Open(const char*name)
    {
        if (!_sm.Open(name))
        {
            return false;
        }

        if (!_isValidShmData())
        {
            _sm.Close();
            return false;
        }

        return true;
    }

    void SharedCompactRingBuffer::Close()
    {
        _sm.Close();
        _nextReadingIndex=(uint64)(-1);
    }

    void SharedCompactRingBuffer::Destroy()
    {
        _sm.Destroy();
        _nextReadingIndex=(uint64)(-1);
    }

    bool SharedCompactRingBuffer::Create(const char*name,uint64_t fixedUserDataSize,uint64_t payloadBufferSize,uint64_t maxItemsNum)
    {
        return Create(name, fixedUserDataSize, payloadBufferSize, maxItemsNum, S_IRUSR | S_IWUSR);
    }

    bool SharedCompactRingBuffer::Create(const char*name,uint64_t fixedUserDataSize,uint64_t payloadBufferSize,uint64_t maxItemsNum,mode_t mode)
    {
        //estimate required shared memory size.
        if (maxItemsNum<=1 || payloadBufferSize==0)
        {
            //bad parameter;
            return false;
        }

        size_t alignment=sizeof(uint64);
        size_t controlDataSize=sizeof(SharedCompactRingBufferImpl::MaximumControlData);
        size_t alignedControlDataSize=controlDataSize;
        if ((alignedControlDataSize % alignment)!=0)
        {
            alignedControlDataSize+=alignment-(alignedControlDataSize % alignment);
        }

        uint64 alignedFixedUserDataSize=fixedUserDataSize;
        if ((alignedFixedUserDataSize %alignment)!=0)
        {
            alignedFixedUserDataSize+=alignment-(alignedFixedUserDataSize % alignment);
        }

        size_t indexItemSize=sizeof(SharedCompactRingBufferImpl::IndexItem);
        size_t alignedIndexItemSize=indexItemSize;
        if ((alignedIndexItemSize %alignment)!=0)
        {
            alignedIndexItemSize+=alignment-(alignedIndexItemSize % alignment);
        }

        uint64 indexDataSize=maxItemsNum*alignedIndexItemSize;
        uint64 alignedIndexDataSize=indexDataSize;
        if ((alignedIndexDataSize %alignment)!=0)
        {
            alignedIndexDataSize+=alignment-(alignedIndexDataSize % alignment);
        }

        uint64 expectedSharedMemorySize=controlDataSize+
            alignedFixedUserDataSize+
            alignedIndexDataSize+
            payloadBufferSize;

        bool isNew=false;
        if (!_sm.Create(name,expectedSharedMemorySize,isNew,mode))
        {
            //failed to create shared memory;
            TVU_TAGGED_HEARTBEAT_WARN(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("create shm failed.")
                        <<"name:"<<name
                        <<"size:"<<expectedSharedMemorySize
                        );
            return false;
        }

        if (isNew)
        {
            //initialize control area;
            size_t shmSize = _sm.GetSize();
            if (expectedSharedMemorySize != shmSize)
            {
                TVU_TAGGED_HEARTBEAT_WARN(
                            "virsmw",1,Log::GetDefaultLog(),
                            Formatter("create shm, invalid shm size. ")
                            <<"expect:"<<expectedSharedMemorySize
                            <<"size:"<<shmSize
                            );
                _sm.Destroy();
                return false;
            }

            SharedCompactRingBufferImpl::MaximumControlData& controlData=
                *reinterpret_cast<SharedCompactRingBufferImpl::MaximumControlData*>(_sm.GetBytes());
            MemUtils::InitializeZero(controlData);
            const char magicCode[]=TVU_SHARED_COMPACT_RING_BUFFER_MAGIC_CODE;
            tvu_memcpy_s(controlData.magicCode,sizeof(controlData.magicCode),
                magicCode,sizeof(magicCode));

            controlData.controlDataSize=controlDataSize;
            controlData.totalSharedMemorySize=expectedSharedMemorySize;

            controlData.fixedUserDataOffset=alignedControlDataSize;
            controlData.fixedUserDataSize=fixedUserDataSize;

            controlData.indexDataOffset=alignedControlDataSize+alignedFixedUserDataSize;
            controlData.indexDataSize=indexDataSize;
            controlData.indexItemSize=alignedIndexItemSize;
            controlData.actualIndexItemSize=indexItemSize;

            controlData.payloadDataOffset=alignedControlDataSize+alignedFixedUserDataSize+alignedIndexDataSize;
            controlData.payloadDataSize=payloadBufferSize;
            controlData.maxItemsNum=maxItemsNum;
            controlData.maxItemIndex=maxItemsNum*2;

            if (!_isValidShmData())
            {
                TVU_TAGGED_HEARTBEAT_ERROR(
                            "virsmw",1,Log::GetDefaultLog(),
                            Formatter("create shm new, invalid shm data which could had dirty data. ")
                            <<"name:"<<name
                            );
                _sm.Destroy();
                return false;
            }
        }
        else
        {
            if (!_isValidShmData())
            {
                TVU_TAGGED_HEARTBEAT_ERROR(
                            "virsmw",1,Log::GetDefaultLog(),
                            Formatter("create shm existing, invalid shm data which could had dirty data. ")
                            <<"name:"<<name
                            );
                _sm.Destroy(); /* here need to destroy but not close, for making sure next successfull creation. */
                return false;
            }
            size_t shmSize = _sm.GetSize();
            const SharedCompactRingBufferImpl::MaximumControlData& controlData=
                *reinterpret_cast<const SharedCompactRingBufferImpl::MaximumControlData*>(_sm.GetBytes());
            if (shmSize < expectedSharedMemorySize
                    || controlData.maxItemsNum < maxItemsNum
                    || controlData.payloadDataSize < payloadBufferSize
                    )
            {
                TVU_TAGGED_HEARTBEAT_WARN(
                            "virsmw",1,Log::GetDefaultLog(),
                            Formatter("create shm existing, payload size or item num were less. ")
                            <<"size:{expect:"<<expectedSharedMemorySize
                            <<",real:"<<shmSize<<"}"
                            <<",payload:{expect:"<<payloadBufferSize
                            <<",real:"<<controlData.payloadDataSize << "}"
                            <<",itemnum:{expect:"<<maxItemsNum
                            <<",real:"<<controlData.maxItemsNum << "}"
                            );
            }
        }

        return true;
    }

    bool SharedCompactRingBuffer::Write(const void*buffer,size_t size)
    {
#if 0
        if (buffer==NULL || size==0)
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.maxItemsNum<=1)
        {
            return false;
        }

        if (controlData.maxItemIndex<=1)
        {
            return false;
        }

        if (size>controlData.payloadDataSize)
        {
            return false;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV1))
        {
            return false;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return false;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return false;
        }

        uint64 freeItemIndex=controlData.nextFreeItemIndex;
        uint64 freeItemPayloadOffset=controlData.nextFreeItemPayloadOffset;

        if (freeItemIndex>=controlData.maxItemIndex)
        {
            freeItemIndex=0;
        }

        //align to machine word;
        if (freeItemPayloadOffset<controlData.payloadDataOffset)
        {
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        size_t alignment=sizeof(uint64);
        if ((freeItemPayloadOffset %alignment)!=0)
        {
            freeItemPayloadOffset+=alignment-(freeItemPayloadOffset % alignment);
        }

        if (freeItemPayloadOffset+size>=controlData.payloadDataOffset+controlData.payloadDataSize)
        {
            //unwind to first;
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        tvu_memcpy_s(smBytes+freeItemPayloadOffset,size,buffer,size);

        uint64 nextFreeItemIndex=(freeItemIndex+1)%(controlData.maxItemIndex);
        SharedCompactRingBufferImpl::IndexItem&freeItem=
            *reinterpret_cast<SharedCompactRingBufferImpl::IndexItem*>(smBytes+
            controlData.indexDataOffset+
            controlData.indexItemSize*(freeItemIndex%controlData.maxItemsNum));

        controlData.nextFreeItemIndex=nextFreeItemIndex;
        freeItem.offset=freeItemPayloadOffset;
        freeItem.size=size;
        freeItem.index=freeItemIndex;
        freeItem.checksum=(freeItemIndex+freeItemPayloadOffset+size);
        //update the index;
        controlData.lastFilledItemIndex=freeItemIndex;
        controlData.nextFreeItemPayloadOffset=freeItemPayloadOffset+size;
        controlData.itemIndexChecksum=freeItemIndex+nextFreeItemIndex;
        return true;
#endif
        void *dest = Apply(size);
        if (!dest)
        {
            return false;
        }

        tvu_memcpy_s(dest, size, buffer, size);

        return Commit(dest, size);
    }

    void* SharedCompactRingBuffer::Apply(size_t maxSize)
    {
        if (maxSize==0)
        {
            return NULL;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return NULL;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.maxItemsNum<=1)
        {
            return NULL;
        }

        if (controlData.maxItemIndex<=1)
        {
            return NULL;
        }

        if (maxSize>controlData.payloadDataSize)
        {
            return NULL;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return NULL;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return NULL;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return NULL;
        }

        uint64 freeItemPayloadOffset=controlData.nextFreeItemPayloadOffset;

        //align to machine word;
        if (freeItemPayloadOffset<controlData.payloadDataOffset)
        {
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        size_t alignment=sizeof(uint64);
        if ((freeItemPayloadOffset %alignment)!=0)
        {
            freeItemPayloadOffset+=alignment-(freeItemPayloadOffset % alignment);
        }

        if (freeItemPayloadOffset+maxSize>=controlData.payloadDataOffset+controlData.payloadDataSize)
        {
            //LLL, clear the writing before.
            if (controlData.payloadDataOffset+controlData.payloadDataSize > freeItemPayloadOffset)
            {
                memset(smBytes+freeItemPayloadOffset, 0, controlData.payloadDataOffset+controlData.payloadDataSize-freeItemPayloadOffset);
            }
            //unwind to first;
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        byte*nextFreeBytes=smBytes+freeItemPayloadOffset;
        return nextFreeBytes;
    }

    bool SharedCompactRingBuffer::Commit(const void*buffer,size_t size)
    {
        if (buffer==NULL || size==0)
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.maxItemsNum<=1)
        {
            return false;
        }

        if (controlData.maxItemIndex<=1)
        {
            return false;
        }

        if (size>controlData.payloadDataSize)
        {
            return false;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return false;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return false;
        }

        uint64 freeItemIndex=controlData.nextFreeItemIndex;
        uint64 freeItemPayloadOffset=controlData.nextFreeItemPayloadOffset;

        if (freeItemIndex>=controlData.maxItemIndex)
        {
            freeItemIndex=0;
        }

        //align to machine word;
        if (freeItemPayloadOffset<controlData.payloadDataOffset)
        {
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        size_t alignment=sizeof(uint64);
        if ((freeItemPayloadOffset % alignment)!=0)
        {
            freeItemPayloadOffset+=alignment-(freeItemPayloadOffset % alignment);
        }

        if (freeItemPayloadOffset+size>=controlData.payloadDataOffset+controlData.payloadDataSize)
        {
            //unwind to first;
            freeItemPayloadOffset=controlData.payloadDataOffset;
        }

        const byte*bufferBytes=static_cast<const byte*>(buffer);
        if (bufferBytes<smBytes)
        {
            return false;
        }

        size_t bufferOffset=bufferBytes-smBytes;
        if (bufferOffset<controlData.payloadDataOffset)
        {
            return false;
        }

        if (bufferOffset!=freeItemPayloadOffset &&
            bufferOffset!=controlData.payloadDataOffset)
        {
            return false;
        }

        uint64 nextFreeItemIndex=(freeItemIndex+1)%(controlData.maxItemIndex);

        SharedCompactRingBufferImpl::IndexItem&freeItem=
            *reinterpret_cast<SharedCompactRingBufferImpl::IndexItem*>(smBytes+
            controlData.indexDataOffset+
            controlData.indexItemSize*(freeItemIndex%controlData.maxItemsNum));

        freeItem.offset=bufferOffset;
        freeItem.size=size;
        freeItem.index=freeItemIndex;
        freeItem.checksum=freeItemIndex+bufferOffset+size;
        //update the index;
        controlData.nextFreeItemPayloadOffset=bufferOffset+size;
        controlData.lastFilledItemIndex=freeItemIndex;
        controlData.itemIndexChecksum=nextFreeItemIndex+freeItemIndex;
        controlData.nextFreeItemIndex=nextFreeItemIndex;
        return true;
    }

    void* SharedCompactRingBuffer::Read(size_t *sizePtr)
    {
        if (sizePtr==NULL)
        {
            return NULL;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return NULL;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return NULL;
        }

        //if (controlData.controlDataSize>=sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        //{
        //    SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
        //        *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);
        //    controlDataV2.requestedFlag=1;
        //}

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return NULL;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return NULL;
        }

        if (controlData.maxItemsNum<=1)
        {
            return NULL;
        }

        if (controlData.maxItemIndex<=1)
        {
            return NULL;
        }

        uint64 nextFreeItemIndex=controlData.nextFreeItemIndex;
        uint64 lastFilledItemIndex=controlData.lastFilledItemIndex;
        uint64 itemIndexChecksum=controlData.itemIndexChecksum;

        if (nextFreeItemIndex==0 && lastFilledItemIndex==0)
        {
            //never enqueued.
            return NULL;
        }

        uint64 position=_nextReadingIndex;
        if (itemIndexChecksum!=nextFreeItemIndex+lastFilledItemIndex)
        {
            TVU_TAGGED_HEARTBEAT_WARN("virsmw",1,Log::GetDefaultLog(),Formatter("unexpected control checksum. ")
                <<"write index: "<<nextFreeItemIndex
                <<", last written index: "<<lastFilledItemIndex
                <<", unexpected checksum: "<<itemIndexChecksum
                <<", expected checksum: "<<nextFreeItemIndex+lastFilledItemIndex
                <<", next reading index:" << position
            ); /* control data was writing. */
            return NULL;
        }

        if (position>=controlData.maxItemIndex)
        {
            position=lastFilledItemIndex;
        }
        else if (position==nextFreeItemIndex)
        {
            return NULL;
        }

        SharedCompactRingBufferImpl::IndexItem&expectedItem=
            *reinterpret_cast<SharedCompactRingBufferImpl::IndexItem*>(smBytes+
            controlData.indexDataOffset+
            controlData.indexItemSize*(position%controlData.maxItemsNum));

        uint64 index=expectedItem.index;
        uint64 offset=expectedItem.offset;
        uint64 size=expectedItem.size;
        uint64 checksum=expectedItem.checksum;

        if (checksum!=(index+offset+size))
        {
            TVU_TAGGED_HEARTBEAT_WARN("virsmw",1,Log::GetDefaultLog(),Formatter("unexpected index checksum.")
                <<" read index: "<<position
                <<", write index: "<<nextFreeItemIndex
                <<", item index: "<<index
                <<", read offset: "<<offset
                <<", read size: "<<size
                <<", item checksum: "<<checksum
                <<", calculated checksum: "<<(index+offset+size)
                <<", pointer: " << (smBytes+(size_t)offset)
                );
            return NULL;
        }

        if (index!=position)
        {
            TVU_TAGGED_HEARTBEAT_WARN("virsmw",1,Log::GetDefaultLog(),Formatter("unexpected item index.")
                <<" read index: "<<position
                <<", write index: "<<nextFreeItemIndex
                <<", unexpected item index: "<<index
                <<", read offset: "<<offset
                <<", read size: "<<size
                <<", pointer: " << (smBytes+(size_t)offset)
                );
            return NULL;
        }

        uint64 nextPosition=(position+1)%controlData.maxItemIndex;

        if (offset<controlData.payloadDataOffset ||
            offset>=controlData.totalSharedMemorySize)
        {
            _nextReadingIndex=nextPosition;
            return NULL;
        }

        if (offset+size>controlData.totalSharedMemorySize)
        {
            _nextReadingIndex=nextPosition;
            return NULL;
        }

        *sizePtr=(size_t)size;
        byte*buffer=smBytes+(size_t)offset;
        _nextReadingIndex=nextPosition;

        TVU_TAGGED_DEBUG("virsmd",Log::GetDefaultLog(),Formatter("read item.")
            <<" read index: "<<position
            <<", write index: "<<nextFreeItemIndex
            <<", pointer: "<< buffer
            <<", size: "<<size
            <<", content: "//<<FormatUtils::FormatHexAndAscii(buffer,32,(size32)size)
            );

        return static_cast<void*>(buffer);
    }

    void *SharedCompactRingBuffer::ReadByPos(size_t *sizePtr, uint64_t position)
    {
        if (sizePtr==NULL)
        {
            return NULL;
        }

        if (!IsValid())
        {
            return NULL;
        }

        byte*smBytes=_sm.GetBytes();

        const SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);
        uint64 nextFreeItemIndex=controlData.nextFreeItemIndex;

        SharedCompactRingBufferImpl::IndexItem&expectedItem=
            *reinterpret_cast<SharedCompactRingBufferImpl::IndexItem*>(smBytes+
            controlData.indexDataOffset+
            controlData.indexItemSize*(position%controlData.maxItemsNum));

        uint64 index=expectedItem.index;
        uint64 offset=expectedItem.offset;
        uint64 size=expectedItem.size;
        uint64 checksum=expectedItem.checksum;

        if (checksum!=(index+offset+size))
        {
            TVU_TAGGED_HEARTBEAT_WARN("virsmw",1,Log::GetDefaultLog(),Formatter("unexpected index checksum.")
                <<" read index: "<<position
                <<", write index: "<<nextFreeItemIndex
                <<", item index: "<<index
                <<", read offset: "<<offset
                <<", read size: "<<size
                <<", item checksum: "<<checksum
                <<", calculated checksum: "<<(index+offset+size)
                <<", pointer: " << (smBytes+(size_t)offset)
                );
            return NULL;
        }

        if (index!=position)
        {
            TVU_TAGGED_HEARTBEAT_WARN("virsmw",1,Log::GetDefaultLog(),Formatter("unexpected item index.")
                <<" read index: "<<position
                <<", write index: "<<nextFreeItemIndex
                <<", unexpected item index: "<<index
                <<", read offset: "<<offset
                <<", read size: "<<size
                <<", pointer: " << (smBytes+(size_t)offset)
                );
            return NULL;
        }

        if (offset<controlData.payloadDataOffset ||
            offset>=controlData.totalSharedMemorySize)
        {
            return NULL;
        }

        if (offset+size>controlData.totalSharedMemorySize)
        {
            return NULL;
        }

        *sizePtr=(size_t)size;
        byte*buffer=smBytes+(size_t)offset;

        TVU_TAGGED_DEBUG("virsmd",Log::GetDefaultLog(),Formatter("read item.")
            <<" read index: "<<position
            <<", write index: "<<nextFreeItemIndex
            <<", pointer: "<< buffer
            <<", size: "<<size
            <<", content: "//<<FormatUtils::FormatHexAndAscii(buffer,32,(size32)size)
            );

        return static_cast<void*>(buffer);
    }

    uint64_t SharedCompactRingBuffer::NextIndexStep(uint64_t position)const
    {
        uint64 maxItemIndex = _getMaxItemIndex();
        uint64 nextPosition=(position+1)%maxItemIndex;
        return nextPosition;
    }

    uint64_t SharedCompactRingBuffer::PreviousIndexStep(uint64_t position)const
    {
        uint64 maxItemIndex = _getMaxItemIndex();
        uint64 prePosition = 0;
        if (position == 0)
        {
            prePosition = maxItemIndex-1;
        }
        else
        {
            prePosition = position-1;
        }
        return prePosition;
    }

    void *SharedCompactRingBuffer::_getControlInfo()
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return NULL;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return NULL;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return NULL;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return NULL;
        }

        if (controlData.maxItemsNum<=1)
        {
            return NULL;
        }

        if (controlData.maxItemIndex<=1)
        {
            return NULL;
        }

        return smBytes;
    }

    void  SharedCompactRingBuffer::_previousReadingIndex()
    {
        byte*smBytes = (byte*)_getControlInfo();

        if (!smBytes)
            return;

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        uint64 position=_nextReadingIndex;
        uint64 previousPos = position;
        if (position==0)
        {
            previousPos=controlData.maxItemIndex-1;
        }
        else if (position < controlData.maxItemIndex)
        {
            previousPos = position - 1;;
        }

        _nextReadingIndex = previousPos;
        return;
    }

    void* SharedCompactRingBuffer::ReadNoStep(size_t *sizePtr)
    {
        void *buf = Read(sizePtr);

        if (!buf)
        {
            return NULL;
        }

        _previousReadingIndex();
        return buf;
    }

    bool SharedCompactRingBuffer::SetReadIndex(uint64_t nextReadingIndex)
    {
        //0: first;
        //-1: last;
        _nextReadingIndex=nextReadingIndex;
        return true;
    }

    uint64_t SharedCompactRingBuffer::GetWriteIndex(void) const
    {
        const byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return (uint64)(-1);
        }

        const SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<const SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return (uint64)(-1);
        }

        return controlData.nextFreeItemIndex;
    }

    uint64_t SharedCompactRingBuffer::GetMaxItemNum()const
    {
        const byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return 0;
        }

        const SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<const SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return 0;
        }

        return controlData.maxItemsNum;
    }

    uint64_t SharedCompactRingBuffer::GetPayloadSize()const
    {
        const byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return 0;
        }

        const SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<const SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return 0;
        }

        return controlData.payloadDataSize;
    }

    uint64_t SharedCompactRingBuffer::_getMaxItemIndex()const
    {
        const byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return 0;
        }

        const SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<const SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return 0;
        }

        return controlData.maxItemIndex;
    }

    uint64_t SharedCompactRingBuffer::GetReadIndex(void) const
    {
        if (_nextReadingIndex==-1)
        {
            return GetWriteIndex();
        }
        return _nextReadingIndex;
    }

    uint64_t SharedCompactRingBuffer::SeekEarliestReadIndex(uint64_t windex)const
    {
        uint64_t searchIndex = 0;
        uint64 maxIndex = _getMaxItemIndex();
        uint64 maxCounts = GetMaxItemNum();

        searchIndex = NextIndexStep(windex);
        if (searchIndex >= maxIndex)
        {
            return 0;
        }

        if (searchIndex>=maxCounts)
        {
            searchIndex -= maxCounts;
        }
        else
        {
            searchIndex += maxCounts;
        }
        return searchIndex;
    }

    void* SharedCompactRingBuffer::GetFixedData(size_t *sizePtr)
    {
        if (sizePtr==NULL)
        {
            return NULL;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return NULL;
        }

        if (!IsValid())
        {
            return NULL;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        *sizePtr=(size_t)controlData.fixedUserDataSize;
        byte*buffer=smBytes+(size_t)controlData.fixedUserDataOffset;
        return static_cast<void*>(buffer);
    }

    bool SharedCompactRingBuffer::CommitFixedData(const void*buffer, size_t size)
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.fixedUserDataOffset<controlData.controlDataSize)
        {
            return false;
        }

        if (controlData.fixedUserDataOffset+controlData.fixedUserDataSize>controlData.indexDataOffset)
        {
            return false;
        }

        if (controlData.fixedUserDataOffset+controlData.fixedUserDataSize>controlData.totalSharedMemorySize)
        {
            return false;
        }

        if ((const byte*)buffer<smBytes+(size_t)controlData.fixedUserDataOffset ||
            ((const byte*)buffer)+size>smBytes+(size_t)(controlData.fixedUserDataOffset)+controlData.fixedUserDataSize)
        {
            return false;
        }

        return true;
    }

    bool SharedCompactRingBuffer::WriteFixedData(const void*buffer,size_t size,size_t offset)
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.fixedUserDataOffset<controlData.controlDataSize)
        {
            return false;
        }

        if (controlData.fixedUserDataOffset+controlData.fixedUserDataSize>controlData.indexDataOffset)
        {
            return false;
        }

        if (controlData.fixedUserDataOffset+controlData.fixedUserDataSize>controlData.totalSharedMemorySize)
        {
            return false;
        }

        if (offset+size>(size_t)(controlData.fixedUserDataSize))
        {
            return false;
        }

        tvu_memcpy_s(smBytes+offset+controlData.fixedUserDataOffset,size,buffer,size);
        return true;
    }

    bool SharedCompactRingBuffer::IsReadable(void)
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            return false;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            return false;
        }

        uint64 itemIndexChecksum=controlData.itemIndexChecksum;
        uint64 lastFilledItemIndex=controlData.lastFilledItemIndex;
        uint64 nextFreeItemIndex=controlData.nextFreeItemIndex;

        if (nextFreeItemIndex==0 && lastFilledItemIndex==0)
        {
            return false;
        }

        if (itemIndexChecksum!=lastFilledItemIndex+nextFreeItemIndex)
        {
            return false;
        }

        if (_nextReadingIndex==nextFreeItemIndex)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool SharedCompactRingBuffer::IsWriteable(void)
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            TVU_HEARTBEAT_MULTIPLE_ERROR(1,10,Log::GetDefaultLog(), Formatter("shm writable failed. shm buffer is null"));
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            TVU_HEARTBEAT_MULTIPLE_ERROR(1,10,Log::GetDefaultLog(), Formatter("shm writable failed.control.datasize ")
                 << controlData.controlDataSize << "invalid"
                 );
            return false;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            TVU_HEARTBEAT_MULTIPLE_ERROR(1,10,Log::GetDefaultLog(), Formatter("shm writable failed.control.indexItemSize ")
                 << controlData.indexItemSize << " < control.actualIndexItemSize " << controlData.actualIndexItemSize
                 );
            return false;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            TVU_HEARTBEAT_MULTIPLE_ERROR(1,10,Log::GetDefaultLog(), Formatter("shm writable failed.control.actualIndexItemSize ")
                 << controlData.actualIndexItemSize << " < " << sizeof(SharedCompactRingBufferImpl::IndexItemV1)
                );
            return false;
        }

        return true;
    }

    bool SharedCompactRingBuffer::GetInfo(
        uint64_t* fixedUserDataSizePtr,uint64_t* payloadBufferSizePtr,uint64_t* maxItemsNumPtr)
    {
        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (fixedUserDataSizePtr!=NULL)
        {
            *fixedUserDataSizePtr=controlData.fixedUserDataSize;
        }

        if (payloadBufferSizePtr!=NULL)
        {
            *payloadBufferSizePtr=controlData.payloadDataSize;
        }

        if (maxItemsNumPtr!=NULL)
        {
            *maxItemsNumPtr=controlData.maxItemsNum;
        }

        return true;
    }


    bool SharedCompactRingBuffer::IsValid()const
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        return _isValidShmData();
    }

    bool SharedCompactRingBuffer::IsActive()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return true;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        if (controlDataV2.inactiveFlag==0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool SharedCompactRingBuffer::SetInactiveFlag()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return false;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        controlDataV2.inactiveFlag=1;
        return true;
    }

    bool SharedCompactRingBuffer::ClearInactiveFlag()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return false;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        controlDataV2.inactiveFlag=0;
        return true;
    }

    bool SharedCompactRingBuffer::IsRequested()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            return false;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return false;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        return controlDataV2.requestedFlag!=0;
    }

    bool SharedCompactRingBuffer::SetRequestedFlag()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return false;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        controlDataV2.requestedFlag=1;
        return true;
    }

    bool SharedCompactRingBuffer::ClearRequestedFlag()
    {
        if (!_sm.IsValid())
        {
            return false;
        }

        byte*smBytes=_sm.GetBytes();
        if (smBytes==NULL)
        {
            return false;
        }

        SharedCompactRingBufferImpl::MinimumControlData&controlData=
            *reinterpret_cast<SharedCompactRingBufferImpl::MinimumControlData*>(smBytes);

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::ControlDataV2))
        {
            return false;
        }

        SharedCompactRingBufferImpl::ControlDataV2&controlDataV2=
            *reinterpret_cast<SharedCompactRingBufferImpl::ControlDataV2*>(smBytes);

        controlDataV2.requestedFlag=0;
        return true;
    }

    bool SharedCompactRingBuffer::_isValidShmData()const
    {
        if (!_sm.GetBytes())
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm validation declare, invalid shm bytes.")
                        );
            return false;
        }
        const SharedCompactRingBufferImpl::MaximumControlData& controlData=
            *reinterpret_cast<const SharedCompactRingBufferImpl::MaximumControlData*>(_sm.GetBytes());

        size_t shmSize = _sm.GetSize();

        const char magicCode[]=TVU_SHARED_COMPACT_RING_BUFFER_MAGIC_CODE;

        if (memcmp(controlData.magicCode, magicCode, std::min(sizeof(magicCode),sizeof(controlData.magicCode))))
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("existing shm had dirty data as magic code invalid.")
                        <<"mem value: "<<controlData.magicCode<<","
                        );
            return false;
        }

        if (controlData.totalSharedMemorySize != shmSize)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("existing shm had dirty data as invalid data length.")
                        <<"total:"<<controlData.totalSharedMemorySize<<","
                        <<"shmsize:"<<shmSize
                        );
            return false;
        }

        if (controlData.controlDataSize<sizeof(SharedCompactRingBufferImpl::MinimumControlData))
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, invalid control data size. ")
                        <<"control size:"<<controlData.controlDataSize<<","
                        );
            return false;
        }

        if (controlData.controlDataSize > controlData.fixedUserDataOffset)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, invalid fixed User Data offset. ")
                        <<"fixed off:"<<controlData.fixedUserDataOffset<<","
                        <<"control size:"<<controlData.controlDataSize<<","
                        );
            return false;
        }

        if (controlData.fixedUserDataOffset+controlData.fixedUserDataSize>controlData.indexDataOffset)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, invalid Index Data offset. ")
                        <<"index off:"<<controlData.indexDataOffset<<","
                        <<"fixed off:"<<controlData.fixedUserDataOffset<<","
                        <<"fixed size:"<<controlData.fixedUserDataSize<<","
                        );
            return false;
        }

        if (controlData.indexDataOffset+controlData.indexDataSize > controlData.payloadDataOffset)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, invalid Payload Data offset. ")
                        <<"payload off:"<<controlData.payloadDataOffset<<","
                        <<"index off:"<<controlData.indexDataOffset<<","
                        <<"index size:"<<controlData.indexDataSize<<","
                        );
            return false;
        }

        if (controlData.payloadDataOffset + controlData.payloadDataSize > controlData.totalSharedMemorySize)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, invalid Total Data offset. ")
                        <<"total off:"<<controlData.totalSharedMemorySize<<","
                        <<"payload off:"<<controlData.payloadDataOffset<<","
                        <<"payload size:"<<controlData.payloadDataSize<<","
                        );
            return false;
        }

        if (controlData.actualIndexItemSize<sizeof(SharedCompactRingBufferImpl::IndexItemV1))
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, actual index item size invalid. ")
                        <<"size:"<<controlData.actualIndexItemSize<<","
                        );
            return false;
        }

        if (controlData.indexItemSize<controlData.actualIndexItemSize)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, index item size invalid. ")
                        <<"size:"<<controlData.indexItemSize<<","
                        <<"actual:"<<controlData.actualIndexItemSize<<","
                        );
            return false;
        }

        if (controlData.maxItemsNum<=1)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, max items number invalid. ")
                        <<"num:"<<controlData.maxItemsNum<<","
                        );
            return false;
        }

        if (controlData.maxItemIndex<=1)
        {
            TVU_TAGGED_HEARTBEAT_ERROR(
                        "virsmw",1,Log::GetDefaultLog(),
                        Formatter("shm constrol data, max item index invalid. ")
                        <<"idx:"<<controlData.maxItemIndex<<","
                        );
            return false;
        }
        return true;
    }

}
