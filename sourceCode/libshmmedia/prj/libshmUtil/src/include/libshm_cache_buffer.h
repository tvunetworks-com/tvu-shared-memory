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
#ifndef LIBSHM_CACHE_BUFFER_H
#define LIBSHM_CACHE_BUFFER_H

#include <stdint.h>
#include <string.h>

namespace tvushm {

    class CacheBuffer
    {
    public:
        CacheBuffer();
        virtual ~CacheBuffer();
        void Release();
        void Reset();
        int Compare(const uint8_t *p, uint32_t n)const;
        int Compare(const CacheBuffer &)const;
        int Copy(const uint8_t *p, uint32_t n);
        int Copy(const CacheBuffer &o);
        bool Alloc(uint32_t ndest);
        uint32_t GetAllocSize()const;
        void SetBufLen(uint32_t l);
        uint8_t *GetBufAddr();
        const uint8_t *GetBufAddr()const;
        uint32_t GetBufLen()const;
        uint32_t GetSize()const;
        bool AttachBuf(const uint8_t *pb, uint32_t nb);
    private:
        void _init();
        void release();

    private:
        uint32_t    _nBin;
        uint32_t    _allocBin;
        uint8_t     *_pBin;
        bool        _bAttach;
    };
}
#endif // LIBSHM_CACHE_BUFFER_H
