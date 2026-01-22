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
Copyright 2005-2014, TVU networks. All rights reserved.
For internal members in TVU networks only.

Purpose: Foundation Library Implementation File
Modifiers: Tony Guo (tonyguo@tvunetworks.com)
History: Tony Guo on Feb 11, 2014: Created
*/

#include "TvuMemUtils.h"
#include "TvuTimeUtils.h"
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>
#include <time.h>

namespace tvushm
{
    MemUtils::MemUtils(void)
    {
    }

    MemUtils::~MemUtils(void)
    {
    }

    MemUtils::UntypedSafePtr::UntypedSafePtr(void*inMemoryPtr)
    {
        _memoryPtr=inMemoryPtr;
    }

    MemUtils::UntypedSafePtr::~UntypedSafePtr()
    {
        Free();
    }

    void MemUtils::UntypedSafePtr::Free()
    {
        if (_memoryPtr!=NULL)
        {
            free(_memoryPtr);
            _memoryPtr=NULL;
        }
    }

    void MemUtils::UntypedSafePtr::Reset(void*inMemoryPtr)
    {
        if (_memoryPtr!=NULL)
        {
            free(_memoryPtr);
        }
        _memoryPtr=inMemoryPtr;
    }

    void*MemUtils::UntypedSafePtr::Get()
    {
        return _memoryPtr;
    }

    void*MemUtils::UntypedSafePtr::Release()
    {
        if (_memoryPtr==NULL)
        {
            return NULL;
        }

        void*outMemoryPtr=_memoryPtr;
        _memoryPtr=NULL;
        return outMemoryPtr;
    }

    bool MemUtils::UntypedSafePtr::IsNull() const
    {
        return _memoryPtr==NULL;
    }

    bool MemUtils::UntypedSafePtr::IsNotNull() const
    {
        return _memoryPtr!=NULL;
    }

    void MemUtils::RandomFill(void*buffer,size32 size)
    {
        static unsigned int next = (unsigned int)(time(NULL)+TimeUtils::GetCurrentTimeInUsec());
        for (size32 i=0;i<size;i++)
        {
            next = next * 214013L + 2531011L;
            ((unsigned char*)buffer)[i]=(unsigned char)(next>>16);
        }
    }

    void MemUtils::Swap(void*buffer1,void*buffer2,size32 size)
    {
        byte*bytes1=(byte*)buffer1;
        byte*bytes2=(byte*)buffer2;
        for (size32 i=0;i<size;i++)
        {
            byte temp=bytes1[i];
            bytes1[i]=bytes2[i];
            bytes2[i]=temp;
        }
    }

    bool MemUtils::IsMemoryBeginWith(const void*buffer,size32 size,const void*subBuffer,size32 subBufferSize)
    {
        if (size<subBufferSize)
        {
            return false;
        }
        return memcmp(buffer,subBuffer,subBufferSize)==0;
    }

    bool MemUtils::IsMemoryBeginWith(const void*buffer,size32 size,const char*subStr)
    {
        return IsMemoryBeginWith(buffer,size,subStr,(size32)strlen(subStr));
    }

    int MemUtils::CompareMemory(const void*buffer1,size32 bufferSize1,const void*buffer2,size32 bufferSize2)
    {
        if (bufferSize1==bufferSize2)
        {
            return memcmp(buffer1,buffer2,bufferSize1);
        }
        else if (bufferSize1<bufferSize2)
        {
            int compareResult=memcmp(buffer1,buffer2,bufferSize1);
            if (compareResult<0)
            {
                return compareResult;
            }
            if (compareResult==0)
            {
                return -1;
            }
            return 1;
        }
        else
        {
            int compareResult=memcmp(buffer1,buffer2,bufferSize2);
            if (compareResult>0)
            {
                return compareResult;
            }
            if (compareResult==0)
            {
                return 1;
            }
            return -1;
        }
    }

    size32 MemUtils::AlignSize(size32 unalignedSize,size32 alignment)
    {
        if (alignment<=1)
        {
            return unalignedSize;
        }
        size32 reminderSize=unalignedSize%alignment;
        if (reminderSize==0)
        {
            return unalignedSize;
        }
        return unalignedSize+(alignment-reminderSize);
    }
}
