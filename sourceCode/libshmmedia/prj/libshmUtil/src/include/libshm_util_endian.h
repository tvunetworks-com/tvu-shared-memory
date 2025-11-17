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
#ifndef LIBSHM_UTIL_ENDIAN_H
#define LIBSHM_UTIL_ENDIAN_H

#include "libshm_util_common_internal.h"

#include <stdint.h>

#if defined(TVU_LINUX)
#include <arpa/inet.h>
#endif
namespace  tvushm {

    static inline int _tvuutilIsBigEndian()
    {
        static int _result = -1;/*unknown*/

        if (_result != -1)
        {
            return _result;
        }

        const uint32_t test = 0x12345678;
        const uint8_t *p = (const uint8_t *)&test;
        if (p[0] == 0x12)
        {
            _result = 1;
        }
        else
        {
            _result = 0;
        }
        return _result;
    }

    static inline uint16_t _tvuutil_letoh16(uint16_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint16_t l8 = v & TVUUTIL_UINT8_MASK;
        uint16_t h8 = (v>>8);
        return ((l8<<8)|h8);
    }

    static inline uint32_t _tvuutil_letoh32(uint32_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint32_t l16 = _tvuutil_letoh16(v & TVUUTIL_UINT16_MASK);
        uint32_t h16 = _tvuutil_letoh16(v>>16);
        return ((l16<<16)|h16);
    }

    static inline uint64_t _tvuutil_letoh64(uint64_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint64_t l32 = _tvuutil_letoh32(v & TVUUTIL_UINT32_MASK);
        uint64_t h32 = _tvuutil_letoh32(v>>32);
        return ((l32<<32)|h32);
    }

    static inline uint16_t _tvuutil_htole16(uint16_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint16_t l8 = v & 0xFF;
        uint16_t h8 = (v>>8);
        return ((l8<<8)|h8);
    }

    static inline uint32_t _tvuutil_htole32(uint32_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint32_t l = _tvuutil_htole16(v & TVUUTIL_UINT16_MASK);
        uint32_t h = _tvuutil_htole16(v>>16);
        return ((l<<16)|h);
    }

    static inline uint64_t _tvuutil_htole64(uint64_t v)
    {
        if (!_tvuutilIsBigEndian())
        {
            return v;
        }
        uint64_t l = _tvuutil_htole32(v & TVUUTIL_UINT32_MASK);
        uint64_t h = _tvuutil_htole32(v>>32);
        return ((l<<32)|h);
    }

    static inline uint16_t _tvuutil_ntoh16(uint16_t v)
    {
        return ntohs(v);
    }

    static inline uint32_t _tvuutil_ntoh32(uint32_t v)
    {
        return ntohl(v);
    }

    static inline uint64_t _tvuutil_ntoh64(uint64_t v)
    {
        if (_tvuutilIsBigEndian())
        {
            return v;
        }
        uint64_t ret = 0;
        uint64_t l32 = ntohl(v&TVUUTIL_UINT32_MASK);
        uint64_t h32 = ntohl(v>>32);

        ret = (l32 << 32) | h32;
        return ret;
    }

    static inline uint16_t _tvuutil_hton16(uint16_t v)
    {
        return htons(v);
    }

    static inline uint32_t _tvuutil_hton32(uint32_t v)
    {
        return htonl(v);
    }

    static inline uint64_t _tvuutil_hton64(uint64_t v)
    {
        if (_tvuutilIsBigEndian())
        {
            return v;
        }
        uint64_t ret = 0;
        uint64_t l32 = htonl(v&TVUUTIL_UINT32_MASK);
        uint64_t h32 = htonl(v>>32);

        ret = (l32 << 32) | h32;
        return ret;
    }

}

#endif // LIBSHM_UTIL_ENDIAN_H
