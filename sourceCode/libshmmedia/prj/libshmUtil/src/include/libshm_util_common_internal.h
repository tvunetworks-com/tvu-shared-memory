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
#ifndef LIBSHM_UTIL_COMMON_INTERNAL_H
#define LIBSHM_UTIL_COMMON_INTERNAL_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#if defined(TVU_WINDOWS)
#if defined(NO_TVU_MEMCPY)
#define TVUUTIL_SAFE_MEMCPY(dst, src, size)     do {if (size > 0)memcpy((dst), (src), size);}while(0)
#else
extern "C" void * tvu_memcpy(void *dst, const void *src, int nBytes);
#define TVUUTIL_SAFE_MEMCPY(dst, src, size)     do {if (size > 0)memcpy((dst), (src), size);}while(0)
#endif
#else
#define TVUUTIL_SAFE_MEMCPY(dst, src, size)     do {if (size > 0)memcpy((dst), (src), size);}while(0)

#endif

#define _IN_
#define _OUT_

#define TVUUTIL_MAX(a,b)                    ((a) < (b)) ? (b) : (a)
#define TVUUTIL_MIN(a,b)                    ((a) < (b)) ? (a) : (b)
#define TVUUTIL_MALLOC(size)                malloc(size)
#define TVUUTIL_STRDUP(p)                   strdup(p)
#define TVUUTIL_FREE(ptr)                   free(ptr)
#define TVUUTIL_SAFE_FREE(p)                do {if (p) {TVUUTIL_FREE((void *)p);p=NULL;}} while (0)
#define TVUUTIL_REALLOC(ptr, size)          realloc(ptr, size)
#define TVUUTIL_CALLOC(num, size)           calloc(num, size)

#define TVUUTIL_COMMON_1LL      ((uint64_t)1)
#define TVUUTIL_COMMON_1KLL     ((uint64_t)1000)
#define TVUUTIL_COMMON_1MLL     ((uint64_t)1000000)
#define TVUUTIL_COMMON_1KMLL    (1000*(uint64_t)1000000)
#define TVUUTIL_COMMON_1GLL     (TVUUTIL_COMMON_1KMLL)

#define TVUUTIL_UINT8_MASK (0xFF)
#define TVUUTIL_UINT16_MASK (0xFFFF)
#define TVUUTIL_UINT32_MASK (0xFFFFFFFF)
#define TVUUTIL_UINT64_MASK (0xFFFFFFFFFFFFFFFF)


namespace tvushm {
    static const char* _tvuutilTransBinaryToHexString(const uint8_t *data, int len, char hx_str[], int hx_str_len)
    {
        int ix = 0;

        for (int i = 0; i < len; i++)
        {
            if (i == 0)
            {
                snprintf(hx_str + ix, hx_str_len - ix, "%02hhx", data[i]);
            }
            else
            {
                snprintf(hx_str + ix, hx_str_len - ix, ",%02hhx", data[i]);
            }
            ix = strlen(hx_str);
        }

        return hx_str;
    }
}
#endif // LIBSHM_UTIL_COMMON_INTERNAL_H
