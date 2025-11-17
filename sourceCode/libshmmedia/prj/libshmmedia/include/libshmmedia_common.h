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
#ifndef LIBSHMMEDIA_COMMON_H
#define LIBSHMMEDIA_COMMON_H

#include "libshm_media_protocol.h"
#include <stdarg.h>

#ifdef __cplusplus
    #define __EXTERN_C_BEGIN extern "C" {
    #define __EXTERN_C_END }
#else
    #define  __EXTERN_C_BEGIN
    #define __EXTERN_C_END
#endif

#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_DLL_
#endif

#else
#define _LIBSHMMEDIA_DLL_ __attribute__((visibility("default")))
#endif

#if !defined (attribute_deprecated)
#if defined(TVU_LINUX)
#    define attribute_deprecated __attribute__((deprecated))
#elif defined(TVU_WINDOWS)
#    define attribute_deprecated __declspec(deprecated)
#else
#    define attribute_deprecated
#endif
#endif


typedef void * libshm_media_handle_t;
typedef libshm_media_handle_t libshmmedia_handle_t;


/**
 *  opaq    : user context
 */
typedef int (*libshm_media_readcb_t)(void *opaq, libshm_media_item_param_t *datactx);


__EXTERN_C_BEGIN
/**
 *  Functionality:
 *      Caller can callback the library's logs.
 *  this was deprecated api, please use LibShmMediaSetLogCallback to replace it.
 *  Parameters:
 *      @cb:
 *          cb, level has the types as:
 *          'i' : info log
 *          'w' : warning log
 *          'e' : error log
 *  Return:
 *      void.
 */
_LIBSHMMEDIA_DLL_
attribute_deprecated
void LibShmMediaSetLogCb(int(*cb)(int level, const char *fmt, ...));


/**
 *  Functionality:
 *      Caller can callback the library's logs, by using va_list.
 *  Parameters:
 *      @cb:
 *          cb, level has the types as:
 *          'i' : info log
 *          'w' : warning log
 *          'e' : error log
 *  Return:
 *      void.
 */
_LIBSHMMEDIA_DLL_
void LibShmMediaSetLogCallback(int(*cb)(int , const char *, va_list ap));

__EXTERN_C_END

#endif // LIBSHMMEDIA_COMMON_H
