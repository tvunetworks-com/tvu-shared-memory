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
#ifndef LIBSHM_MEDIA_PROTOCOL_LOG_INTERNAL_H
#define LIBSHM_MEDIA_PROTOCOL_LOG_INTERNAL_H

#include "libshm_media_protocol_log.h"
#include "libshm_time_internal.h"
#include <stdarg.h>
#include <ctype.h>

#define _LIBSHMMEDIA_PROTO_COMMON_LOG(level, fmt, ...)        \
    do {\
        LibShmMediaProtoLogPrintInternal(level, "@[%s,%d] %c- " fmt \
        , __FUNCTION__, __LINE__, toupper(char(level)), ##__VA_ARGS__);\
    } while (0)


#define _LIBSHMMEDIA_PROTO_HEARTBEAT_MULTIPLE_LOG(seconds,maxIntervalCount,level,fmt, ...) \
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
            LibShmMediaProtoLogPrintInternal(level, "@[%s,%ld][%" PRIu64 ",%u%s%s] %c-" fmt \
            , __FUNCTION__, __LINE__\
            , _tvu_now, _tvu_count_copy, b_internal_time_out?"/":"@" \
            , _libshmmediaInternalMilliSec2Str(_tvu_now-_tvu_lastLastTime##__LINE__, stime, sizeof(stime))\
            , toupper(char(level)) \
            , ##__VA_ARGS__);\
            if (b_internal_time_out) {\
                _tvu_lastLastTime##__LINE__ = _tvu_lastTime_##__LINE__;\
            } \
        } \
    } while (0)

#define DEBUG_SHMMEDIA_PROTO_INFO(fmt, ...)        do {\
                                            _LIBSHMMEDIA_PROTO_COMMON_LOG('i', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_SHMMEDIA_PROTO_WARN(fmt, ...)        do {\
                                            _LIBSHMMEDIA_PROTO_HEARTBEAT_MULTIPLE_LOG(1,1,'w', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_SHMMEDIA_PROTO_ERROR(fmt, ...)       do {\
                                            _LIBSHMMEDIA_PROTO_HEARTBEAT_MULTIPLE_LOG(1, 1, 'e', fmt, ##__VA_ARGS__);\
                                    } while (0)

#define DEBUG_SHMMEDIA_PROTO_INFO_CR(fmt, ...) DEBUG_SHMMEDIA_PROTO_INFO(fmt "\n", ##__VA_ARGS__)
#define DEBUG_SHMMEDIA_PROTO_WARN_CR(fmt, ...) DEBUG_SHMMEDIA_PROTO_WARN(fmt "\n", ##__VA_ARGS__)
#define DEBUG_SHMMEDIA_PROTO_ERROR_CR(fmt, ...) DEBUG_SHMMEDIA_PROTO_ERROR(fmt "\n", ##__VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif

void LibShmMediaProtoSetLogCbInternal(int(*cb)(int , const char *, ...));
void LibShmMediaProtoSetLogCbInternalV2(int(*cb)(int , const char *, va_list ap));

void LibShmMediaProtoLogPrintInternal(int level, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // LIBSHM_MEDIA_PROTOCOL_LOG_INTERNAL_H
