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
#include "libshm_cache_buffer.h"
#include "libshm_util_common_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SHM_ALIGE(x, a) (((x)+(a)-1)&~((a)-1))
#define SHM_ALIGE16(x) SHM_ALIGE(x,16)
#define SHM_ALIGE8(x) SHM_ALIGE(x,8)

namespace tvushm {

    CacheBuffer::CacheBuffer()
    {
        _init();
    }

    CacheBuffer::~CacheBuffer()
    {
        release();
    }

    void CacheBuffer::Release()
    {
        release();
        return;
    }

    void CacheBuffer::Reset()
    {
        SetBufLen(0);
    }

    int CacheBuffer::Compare(const uint8_t *p2, uint32_t n2)const
    {
        uint32_t n = _nBin;
        const uint8_t *p = _pBin;
        if (n2 != n)
        {
            return (n > n2) ? 1 : -1;
        }

        if (!n2)
        {
            return -1;
        }

        if (!p || !p2)
        {
            return -1;
        }

        return memcmp(p, p2, n);
    }

    int CacheBuffer::Compare(const CacheBuffer &x)const
    {
        return Compare(x._pBin, x._nBin);
    }

    int CacheBuffer::Copy(const uint8_t *p, uint32_t n)
    {
        if (!p || !n)
        {
            return 0;
        }

        if (!Alloc(n))
        {
            return -ENOMEM;
        }
        memcpy(_pBin, p, n);
        SetBufLen(n);
        return n;
    }

    int CacheBuffer::Copy(const CacheBuffer &o)
    {
        return Copy(o.GetBufAddr(), o.GetBufLen());
    }

    int CacheBuffer::MultiCopy(const uint8_t *p, uint32_t n)
    {
        if (!p || !n)
        {
            return 0;
        }

        uint32_t curLen = GetBufLen();
        uint32_t nx = curLen + n;

        if (!Alloc(nx))
        {
            return -ENOMEM;
        }
        memcpy(_pBin+curLen, p, n);
        SetBufLen(nx);
        return n;
    }

    void CacheBuffer::_init()
    {
        _nBin = 0;
        _allocBin = 0;
        _pBin = NULL;
        _bAttach = false;
    }

    void CacheBuffer::release()
    {
        if (!_bAttach)
        {
            TVUUTIL_SAFE_FREE(_pBin);
        }
        else
        {
            _pBin = NULL;
        }
        _nBin = 0;
        _allocBin = 0;
    }

    bool CacheBuffer::Alloc(uint32_t ndest)
    {
        bool b = true;
        uint32_t &allocSize = _allocBin;
        if (allocSize < ndest)
        {
            uint32_t n = SHM_ALIGE16(ndest);
            {
                uint8_t *&pb = _pBin;
                pb = (uint8_t *)TVUUTIL_REALLOC(pb, n);
                if (!pb)
                {
                    allocSize = 0;
                    return false;
                }
                allocSize = n;
            }
        }

        return b;
    }

    uint32_t CacheBuffer::GetAllocSize()const
    {
        return _allocBin;
    }

    void CacheBuffer::SetBufLen(uint32_t l)
    {
        _nBin = l;
        return;
    }

    uint8_t *CacheBuffer::GetBufAddr()
    {
        return _pBin;
    }

    const uint8_t *CacheBuffer::GetBufAddr()const
    {
        return _pBin;
    }

    uint32_t CacheBuffer::GetBufLen()const
    {
        return _nBin;
    }

    uint32_t CacheBuffer::GetSize()const
    {
        return GetBufLen();
    }

    bool CacheBuffer::AttachBuf(const uint8_t *pb, uint32_t nb)
    {
        _bAttach = true;
        _pBin = (uint8_t *)pb;
        _nBin = nb;
        return true;
    }
}

