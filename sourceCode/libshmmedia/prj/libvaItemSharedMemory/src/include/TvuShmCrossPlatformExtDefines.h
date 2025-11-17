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

#include <stdint.h>

namespace tvushm
{
#if defined(TVU_WINDOWS)

    typedef unsigned char byte;
    typedef __int8  int8;
    typedef __int16 int16;
    typedef __int32 int32;
    typedef __int64 int64;

    typedef unsigned __int8  uint8;
    typedef unsigned __int16 uint16;
    typedef unsigned __int32 uint32;

#ifndef _TVU_UINT64_DEFINED
#define _TVU_UINT64_DEFINED
    typedef unsigned __int64 uint64;
#endif

    #if defined(_WIN64)
        #define TVU_WORDSIZE	64
    #else
        #define TVU_WORDSIZE	32
    #endif

#define TVU_W64 __w64

    typedef int64 longlong;
    typedef uint64 ulonglong;


#elif defined(TVU_ANDROID) || defined(TVU_LINUX) || defined(TVU_MINI) || defined(TVU_MAC) || defined(TVU_IOS)
    typedef uint8_t byte;
    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;

#ifndef _TVU_UINT64_DEFINED
#define _TVU_UINT64_DEFINED
    typedef uint64_t uint64;
#endif

    typedef int8_t int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;

    #if defined(__WORDSIZE)
    #define TVU_WORDSIZE __WORDSIZE
    #elif defined(__SIZEOF_POINTER__)
    #define TVU_WORDSIZE (__SIZEOF_POINTER__*8)
    #else
    #error can not determine word size
    #endif

#define TVU_W64


    typedef long long longlong;
    typedef unsigned long long ulonglong;

#else
    #error Platform not supported
#endif

    typedef int32 ErrorCode;

    typedef unsigned int uint;

    typedef uint32 size32;
    typedef uint64 size64;

#if (TVU_WORDSIZE==64)
    typedef uint64 uintptr;
#else
    typedef uint32 uintptr;
#endif

}
