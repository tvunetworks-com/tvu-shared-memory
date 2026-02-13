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
#ifndef SHAREMEMORY_INTERNAL_H
#define SHAREMEMORY_INTERNAL_H
#include "sharememory.h"
#include "libshm_time_internal.h"

#if defined (TVU_WINDOWS) || defined (TVU_MINGW)
#   include <Winsock2.h>
#   include "stdint.h"
#   include <sys/timeb.h>
#else
#   include <sys/ipc.h>
#   include <sys/shm.h>
#   include <sys/time.h>
#   include <unistd.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif
#include <inttypes.h>
#include <ctype.h>

#define DEBUG_COMMON_LOG(level, fmt, ...)        \
    do {\
        libsharememory_set_log_print_internal(level, fmt, ##__VA_ARGS__);\
    } while (0)

#define _LIBSHMMEDIA_TAGGED_HEARTBEAT_MULTIPLE_LOG(tagName,seconds,maxIntervalCount,level,fmt, ...) \
    do \
    {\
        uint64_t _tvu_now = _libshm_get_sys_ms64();; \
        bool boutLog = false; \
        static uint64_t _tvu_lastTime_##__LINE__ = 0;\
        static uint32_t _tvu_counts_##__LINE__ = 0;\
        static uint64_t _tvu_lastLastTime##__LINE__ = 0; \
        if (!_tvu_lastLastTime##__LINE__) {\
            _tvu_lastLastTime##__LINE__ = _tvu_now; \
        } \
        _tvu_counts_##__LINE__++;\
        bool b_internal_time_out = _tvu_now>=_tvu_lastTime_##__LINE__+(seconds*(int64_t)1000) || _tvu_now<_tvu_lastTime_##__LINE__; \
        uint32_t _tvu_count_copy = _tvu_counts_##__LINE__; \
        if (b_internal_time_out) { \
            if (_tvu_lastTime_##__LINE__ != 0) { \
                _tvu_lastLastTime##__LINE__ = _tvu_lastTime_##__LINE__;\
            } \
            _tvu_lastTime_##__LINE__=_tvu_now; \
            _tvu_counts_##__LINE__ = 0;\
            boutLog = true; \
        }  else if (_tvu_counts_##__LINE__ < maxIntervalCount) {\
           boutLog = true;\
        } \
        if (boutLog) {\
            char stime[50] = {0};\
            DEBUG_COMMON_LOG(level, "T[%s][%s,%ld][%" PRIu64 ",%u%s%s] %c-" fmt \
            , tagName, __FUNCTION__, __LINE__\
            , _tvu_now, _tvu_count_copy, b_internal_time_out?"/":"@" \
            , _libshmmediaInternalMilliSec2Str(_tvu_now-_tvu_lastLastTime##__LINE__, stime, sizeof(stime))\
            , toupper(char(level)) \
            , ##__VA_ARGS__);\
            if (b_internal_time_out) {\
                _tvu_lastLastTime##__LINE__ = _tvu_lastTime_##__LINE__;\
            } \
        } \
    } while (0)

#define DEBUG_INFO(fmt, ...)        do {\
                                        _LIBSHMMEDIA_TAGGED_HEARTBEAT_MULTIPLE_LOG("shmm", 1, 1, 'i', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_WARN(fmt, ...)        do {\
                                        _LIBSHMMEDIA_TAGGED_HEARTBEAT_MULTIPLE_LOG("shmm", 10, 10, 'w', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_ERROR(fmt, ...)       do {\
                                        _LIBSHMMEDIA_TAGGED_HEARTBEAT_MULTIPLE_LOG("shmm", 10, 10, 'e', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_ERROR_CR(fmt, ...)    DEBUG_ERROR(fmt "\n", ##__VA_ARGS__)
#define DEBUG_WARN_CR(fmt, ...)     DEBUG_WARN(fmt "\n", ##__VA_ARGS__)
#define DEBUG_INFO_CR(fmt, ...)     DEBUG_INFO(fmt "\n", ##__VA_ARGS__)


void libsharememory_set_log_cabllback_internal(int(*cb)(int , const char *, ...));
void libsharememory_set_log_cabllback_internal_v2(int(*cb)(int , const char *, va_list ap));
int libsharememory_set_log_print_internal(int level, const char *fmt, ...);

#endif // SHAREMEMORY_INTERNAL_H
