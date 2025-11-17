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

Purpose: Foundation Library Header File
Modifiers: Tony Guo (tonyguo@tvunetworks.com)
History: Tony Guo on Feb 11, 2014: Created
*/

#pragma once

#include "TvuShmCrossPlatformExtDefines.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace tvushm
{
    class MemUtils
    {
    public:
        class UntypedSafePtr
        {
        public:
            UntypedSafePtr(void*inMemoryPtr);
            ~UntypedSafePtr();

            void Free();
            void Reset(void*inMemoryPtr);
            void*Get();
            void*Release();
            bool IsNull() const;
            bool IsNotNull() const;
        private:
            void* _memoryPtr;
        };

        template<typename TypeName>class SafePtr:
            public UntypedSafePtr
        {
        public:
            SafePtr(void*inMemoryPtr)
                :UntypedSafePtr(inMemoryPtr)
            {
            }

            TypeName*Get()
            {
                return (TypeName*)UntypedSafePtr::Get();
            }

            TypeName*Release()
            {
                return (TypeName*)UntypedSafePtr::Release();
            }
        };
    public:
        static void RandomFill(void*buffer,size32 size);

        template<typename TypeName>static void RandomFill(TypeName&value)
        {
            RandomFill(&value,sizeof(TypeName));
        }

        template<typename TypeName> static int CompareMemory(const TypeName&data1,const TypeName&data2)
        {
            size_t bufferSize=sizeof(TypeName);
            return memcmp(&data1,&data2,bufferSize);
        }

        template<typename TypeName> static bool IsSameMemory(const TypeName&data1,const TypeName&data2)
        {
            return CompareMemory(data1,data2)==0;
        }

        template<typename TypeName> static bool IsMemoryLessThan(const TypeName&data1,const TypeName&data2)
        {
            return CompareMemory(data1,data2)<0;
        }

        static void Swap(void*buffer1,void*buffer2,size32 size);

        static bool IsMemoryBeginWith(const void*buffer,size32 size,const void*subBuffer,size32 subBufferSize);

        static bool IsMemoryBeginWith(const void*buffer,size32 size,const char*subStr);

        static int CompareMemory(const void*buffer1,size32 bufferSize1,const void*buffer2,size32 bufferSize2);

        //no lock for multi-thread, use at your own risk.
        //safe to use for non-POD type.
        //not safe to concurrent first use of POD type
        template<typename Type>
        static const Type&GetDefaultValue()
        {
            static Type defaultValue;
            return defaultValue;
        }

        template<typename Type>
        static Type&GetWriteableDefaultValue()
        {
            static Type defaultValue;
            return defaultValue;
        }

        template<typename Type>
        static Type&Initialize(Type&value)
        {
            value=GetDefaultValue<Type>();
            return value;
        }

        template<typename Type>
        static void InitializeZero(Type&value)
        {
            size_t valueSize=sizeof(Type);
            memset(&value,0,valueSize);
        }

        template<typename Type, size32 valuesNum>
        static void InitializeArray(Type(& values)[valuesNum])
        {
            const Type& defaultValue=GetDefaultValue<Type>();
            for (size32 i=0;i<valuesNum;i++)
            {
                values[i]=defaultValue;
            }
        }

        template<typename Type>
        static bool IsDefaultValue(const Type&value)
        {
            const Type& defaultValue=GetDefaultValue<Type>();
            return value == defaultValue;
        }

        template<typename Type>
        static bool IsDefaultMemory(const Type&value)
        {
            const Type& defaultValue=GetDefaultValue<Type>();
            return CompareMemory(&value,sizeof(Type),&defaultValue,sizeof(Type))==0;
        }

        static size32 AlignSize(size32 unalignedSize,size32 alignment=(size32)sizeof(size_t));

    private:

        MemUtils(void);
        ~MemUtils(void);
    };
}
