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

#define TVU_1LL                                                                         ((tvushm::uint64)1)
#define TVU_1KLL                                                                        ((tvushm::uint64)1000)
#define TVU_1MLL                                                                        ((tvushm::uint64)1000000)
#define TVU_1KMLL                                                                       (1000*(tvushm::uint64)1000000)
#define TVU_1GLL                                                                        (TVU_1KMLL)

#define TVU_USEC2MSEC(n)                                                                (((n)+500)/1000)
#define TVU_MSEC2USEC(n)                                                                ((n)*TVU_1KLL)

#define TVU_MSEC2SUSEC(n)                                                               ((n)*(tvu::int64)1000)

#define TVU_SEC2USEC(n)                                                                 ((n)*TVU_1MLL)
#define TVU_USEC2SEC(n)                                                                 (((n)+TVU_1MLL/2)/TVU_1MLL)

#define TVU_SEC2MSEC(n)                                                                 ((n)*1000)
#define TVU_MSEC2SEC(n)                                                                 (((n)+500)/1000)

#define TVU_MIN2USEC(n)                                                                 ((n)*TVU_1MLL*60)
#define TVU_USEC2MIN(n)                                                                 (((n)+TVU_1MLL/2)/(TVU_1MLL*60))

#define TVU_SEC2NSEC(n)                                                                 ((n)*TVU_1GLL)
#define TVU_NSEC2SEC(n)                                                                 (((n)+TVU_1GLL/2)/TVU_1GLL)

#define TVU_1M                                                                          1000000
#define TVU_1K                                                                          1000

#define TVU_TLS                                                                         TVU_TLS_DECL_SPEC

#define TVU_ZERO_MEMORY_UNSAFE(v) memset(&(v),0,sizeof(v))

#define _TVU_AUTO_NAME(prefix,suffix) prefix##suffix
#define _TVU_AUTO_NAME_REP(prefix,suffix) _TVU_AUTO_NAME(prefix,suffix)
#define TVU_AUTO_NAME(prefix) _TVU_AUTO_NAME_REP(prefix,__LINE__)

#define TVU_NO_DEFAULT_CONSTRUCTOR(class_name) \
    private:\
    class_name(void)

#define TVU_NO_COPY(class_name) \
    private:\
    class_name(const class_name&); \
    class_name&operator=(const class_name&)

#define TVU_NO_ASSIGN(class_name) \
    private:\
    class_name&operator=(const class_name&)

#define TVU_RETURN_VOID_IF(cond) do \
    { \
    if (cond)\
        {\
        return;\
        }\
    } while (0)

#define TVU_ASSERT(cond) \
    do\
    {\
        if(!(cond))\
        {\
            tvu::AssertFailedException e(__FILE__,__LINE__,TVU_FUNCTION_NAME,TVU_PRETTY_FUNCTION_NAME,#cond);\
            TVU_DEBUG_BREAK(); \
            e.Throw(); \
        }\
    }while(0)

#define TVU_ASSERT_EX(cond,message) \
    do\
    {\
        if(!(cond))\
        {\
            tvu::AssertFailedException e(__FILE__,__LINE__,TVU_FUNCTION_NAME,TVU_PRETTY_FUNCTION_NAME,#cond,message);\
            TVU_DEBUG_BREAK(); \
            e.Throw(); \
        }\
    }while(0)


#define TVU_NOT_IMPLEMENTED() \
do\
{\
    TVU_DEBUG_BREAK();\
    throw tvu::Exception("The method or operation is not implemented.");\
}while(0)

#define TVU_MOBILE_BACKGROUND_THREAD_LOOP_INTERVAL_MSEC                                    20

#define TVU_COUNT_OF_ARRAY(a)        (sizeof(a)/sizeof(a[0]))

//#define TVU_ENABLE_DEADLOCK_PREDICTOR

//#ifndef TVU_ENABLE_BENCHMARK_TEST
//#define TVU_ENABLE_BENCHMARK_TEST
//#endif
//
//#ifndef TVU_ENABLE_INSTANT_BENCHMARK_TEST
//#define TVU_ENABLE_INSTANT_BENCHMARK_TEST
//#endif


#ifdef __GNUC__
#define tvu_assume_aligned __builtin_assume_aligned
#else
#define tvu_assume_aligned(ptr,alignment) (ptr)
#endif
