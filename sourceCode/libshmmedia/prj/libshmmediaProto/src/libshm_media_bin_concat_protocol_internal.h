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
#ifndef LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_INTERNAL_H
#define LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_INTERNAL_H

#include "libshm_media_bin_concat_protocol.h"
#include "libshm_cache_buffer.h"
#include "buffer_controller.h"
#include <vector>

namespace tvushm {

    class BinConcatProto
    {
    public:
        typedef struct OffsetLenPair{
            uint32_t off;
            uint32_t len;
        } OffsetLenPair_t;

        BinConcatProto();
        virtual ~BinConcatProto();
        bool ConcatSegment(/*IN*/ const uint8_t *pSeg, /*IN*/uint32_t nSeg);
        bool FlushBinary(/*OUT*/const uint8_t **ppBin, /*OUT*/uint32_t *pnBin);
        bool SplitBinary(
                /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin, /*IN*/bool bCreateBuffer
                , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppData, /*OUT*/uint32_t *pnData
                );

        void Reset();
        void Release();
    private:
        bool FlushSegInfo(libshmmedia_bin_concat_proto_seg_t **ppSeg = NULL, uint32_t *pnSeg = NULL);
    private:
        BufferController_t _bc;
        uint32_t _count;
        std::vector<OffsetLenPair_t> _info;
        CacheBuffer _segInfo;
    };
}


#endif // LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_INTERNAL_H
