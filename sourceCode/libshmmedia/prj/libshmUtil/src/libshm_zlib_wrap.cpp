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
#include "libshm_zlib_wrap_internal.h"
#include "libshm_time_internal.h"
#include <zlib.h>

namespace tvushm {

    unsigned long ZlibWrapEstimateCompressSize(unsigned long iSouceLen)
    {
        return compressBound(iSouceLen);
    }

    int ZlibWrapCompress(uint8_t *dest,   unsigned long *destLen,
                         const uint8_t *source, unsigned long sourceLen)
    {
        int ret = Z_OK;

        if (!dest || !destLen || !(*destLen))
        {
            return Z_ERRNO;
        }

        if (!source || !sourceLen)
        {
            return Z_ERRNO;
        }

        ret = ::compress(
            dest,
            destLen,
            source,
            sourceLen
        );

        return ret;
    }

    int ZlibWrapUnCompress(uint8_t *dest,   unsigned long *destLen,
                           const uint8_t *source, unsigned long sourceLen)
    {
        int ret = Z_OK;

        if (!dest || !destLen || !(*destLen))
        {
            return Z_ERRNO;
        }

        if (!source || !sourceLen)
        {
            return Z_ERRNO;
        }

        ret = ::uncompress(
            dest,
            destLen,
            source,
            sourceLen
        );
        return ret;
    }


}
