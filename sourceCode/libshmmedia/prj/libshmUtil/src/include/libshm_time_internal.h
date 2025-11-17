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
#ifndef LIBSHM_TIME_INTERNAL_H
#define LIBSHM_TIME_INTERNAL_H

#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static inline
    int64_t _libshm_get_sys_us64()
{
#if defined TVU_WINDOWS
    int64_t tmNow = 0;
    struct _timeb timebuffer;
    _ftime_s(&timebuffer);
    tmNow = timebuffer.time;
    tmNow *= 1000000;
    tmNow += (int64_t)timebuffer.millitm*1000;
    return tmNow;
#elif defined(TVU_MINGW)
    int64_t tmNow = 0;
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    tmNow = timebuffer.time;
    tmNow *= 1000000;
    tmNow += (int64_t)timebuffer.millitm*1000;
    return tmNow;
#elif defined(TVU_LINUX)
    int64_t tmNow = 0;
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    tmNow = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    return tmNow;
#else
#error unsupport os
#endif
}

static inline
int64_t _libshm_get_sys_ms64()
{
    return _libshm_get_sys_us64()/1000;
}

static inline
    void _libshm_common_usleep(int n)
{
#if defined TVU_WINDOWS || defined TVU_MINGW
    Sleep(n/1000);
#else
    usleep(n);
#endif
    return;
}

static inline
void _libshm_common_msleep(int n)
{
    _libshm_common_usleep(n*1000);
    return;
}

static const char *_libshmmediaInternalMicroSec2Str(uint64_t us, char out[], int nout)
{
#define _LISHMMEDIA_INTERNAL_COMMON_1LL      ((uint64_t)1)
#define _LISHMMEDIA_INTERNAL_COMMON_1KLL     ((uint64_t)1000)
#define _LISHMMEDIA_INTERNAL_COMMON_1MLL     ((uint64_t)1000000)
#define _LISHMMEDIA_INTERNAL_COMMON_1KMLL    (1000*(uint64_t)1000000)
#define _LISHMMEDIA_INTERNAL_COMMON_1GLL     (_LISHMMEDIA_INTERNAL_COMMON_1KMLL)

    static const uint64_t ms_unit = _LISHMMEDIA_INTERNAL_COMMON_1KLL;
    static const uint64_t sec_unit = _LISHMMEDIA_INTERNAL_COMMON_1MLL;
    static const uint64_t min_unit = _LISHMMEDIA_INTERNAL_COMMON_1MLL*60;
    static const uint64_t hour_unit = _LISHMMEDIA_INTERNAL_COMMON_1MLL*60*60;

    memset(out, 0, nout);

    if (us >= hour_unit)
    {
        snprintf(out, nout-1, "%0.02fh", us*1.0/hour_unit);
    }
    else if (us >= min_unit)
    {
        snprintf(out, nout-1, "%0.02fm", us*1.0/min_unit);
    }
    else if (us >= sec_unit)
    {
        snprintf(out, nout-1, "%0.01fs", us*1.0/sec_unit);
    }
    else if (us >= ms_unit)
    {
        snprintf(out, nout-1, "%" PRIu64 "ms", us/ms_unit);
    }
    else if (us == 0)
    {
        snprintf(out, nout-1, "%" PRIu64 "s", us/sec_unit);
    }
    else
    {
        snprintf(out, nout-1, "%" PRIu64 "us", us);
    }

    return out;
}

static const char *_libshmmediaInternalMilliSec2Str(uint64_t ms, char s[], int n)
{
    return _libshmmediaInternalMicroSec2Str(ms*1000, s, n);
}

#endif // LIBSHM_TIME_INTERNAL_H
