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
#include "libshm_media_audio_track_channel_proto_internal.h"
#include "buffer_controller.h"
#include "libshm_zlib_wrap_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

namespace tvushm {

    AudioChannelLayoutInternal::AudioChannelLayoutInternal()
    {
        _init();
    }

    AudioChannelLayoutInternal::~AudioChannelLayoutInternal()
    {
        Release();
    }

    void AudioChannelLayoutInternal::Release()
    {
        _bufBin.Release();
        _bufChannel.Release();
        _init();
    }

    bool AudioChannelLayoutInternal::IsPlanar()const
    {
        return _bPlanar;
    }

    uint16_t AudioChannelLayoutInternal::GetChannelNum()const
    {
        uint16_t nDest = _bufChannel.GetBufLen() / sizeof(ChannelVal_t);
        return nDest;
    }

    bool AudioChannelLayoutInternal::GetChannelArr(/*OUT*/const uint16_t *&pDest, /*OUT*/uint16_t &nDest)const
    {
        pDest = (uint16_t *)_bufChannel.GetBufAddr();
        nDest = _bufChannel.GetBufLen() / sizeof(ChannelVal_t);
        return true;
    }

    bool AudioChannelLayoutInternal::GetBinArr(/*OUT*/const uint8_t *&pDest, /*OUT*/uint32_t &nDest)const
    {
        pDest = _bufBin.GetBufAddr();
        nDest = _bufBin.GetBufLen();
        return true;
    }

    bool AudioChannelLayoutInternal::SerializeToBinary(const ChannelVal_t *pChan, uint16_t nChan, bool bPlanar)
    {
        if (!pChan || !nChan)
        {
            return false;
        }

        if (isSameChannel(pChan, nChan, bPlanar))
        {
            return true;
        }

        /* convert to binary */
        const uint8_t *pSource = (const uint8_t*)pChan;
        unsigned long nSource = (unsigned long)nChan * sizeof(ChannelVal_t);
        BufferController_t op;
        {
            BufferCtrlInit(&op);
        }

        if (!_tvuutilIsBigEndian())
        {
            for (uint16_t i=0; i<nChan; i++)
            {
                FAILED_PUSH_DATA_RET(BufferCtrlWBe16(&op, pChan[i]),false);
            }

            pSource = BufferCtrlGetOrigPtr(&op);
        }

        CacheBuffer tmp;

        bool b = zlibCompress(tmp, pSource, nSource);

        BufferCtrlRelease(&op);

        if (!b)
        {
            return false;
        }

        b = copyChannel(pChan, nChan, bPlanar);
        if (!b)
        {
            return false;
        }

        {
            BufferController_t op2;
            {
                BufferCtrlInit(&op2);
            }
            FAILED_PUSH_DATA_RET(BufferCtrlCompactEncodeValueU16(&op2, nChan),false);
            FAILED_PUSH_DATA_RET(BufferCtrlCompactEncodeValueU32(&op2, tmp.GetBufLen()),false);
            FAILED_PUSH_DATA_RET(BufferCtrlWriteBinary(&op2, tmp.GetBufAddr(), tmp.GetBufLen()),false);
            FAILED_PUSH_DATA_RET(BufferCtrlCompactEncodeValueU8(&op2, _bPlanar?1:0),false);

            copyBin(BufferCtrlGetOrigPtr(&op2), BufferCtrlGetBufLen(&op2));

            BufferCtrlRelease(&op2);
        }

        return true;
    }

    bool AudioChannelLayoutInternal::ParseFromBinary(const uint8_t *pBin, uint32_t nBin)
    {
        if (!pBin || !nBin)
        {
            return false;
        }

        if (isSameBin(pBin, nBin))
        {
            return true;
        }

        BufferController_t op;
        {
            BufferCtrlInit(&op);
            FAILED_PUSH_DATA_RET(BufferCtrlWriteBinary(&op, pBin, nBin), false);
            BufferCtrlRewind(&op);
        }

        uint16_t nChan = 0;
        FAILED_READ_DATA_RET(BufferCtrlCompactDecodeValueU16(&op,nChan), false);

        uint32_t nbuf = 0;
        FAILED_READ_DATA_RET(BufferCtrlCompactDecodeValueU32(&op,nbuf), false);
        uint32_t leftlen =  (uint32_t)BufferCtrlReadBufLeftLen(&op);

        if (nbuf > leftlen)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout binary parsed failed for invalid binary length."
                                          "nb:%u,left:%u"
                                          ",cp:%d,to:%u"
                                          , nbuf, leftlen
                                          , BufferCtrlTellCurPos(&op)
                                          , nBin
                                          );
            BufferCtrlRelease(&op);
            return false;
        }

        const uint8_t *pBuf = BufferCtrlGetCurPtr(&op);
        {
            BufferCtrlReadSkip(&op, nbuf);
        }

        leftlen =  (uint32_t)BufferCtrlReadBufLeftLen(&op);
        if (leftlen >= 1)
        {
            /* parse planar */
            uint8_t bPlanar = 0;
            FAILED_READ_DATA_RET(BufferCtrlCompactDecodeValueU8(&op,bPlanar), false);
            if (bPlanar == 0x01)
            {
                _bPlanar = true;
            }
        }

        CacheBuffer buf;
        uint32_t xbuf = (uint32_t)nChan*sizeof(ChannelVal_t);
        bool b = zlibUncompress(buf, xbuf, pBuf, nbuf);

        if (!b)
        {
            BufferCtrlRelease(&op);
            return false;
        }

        uint32_t buflen = buf.GetBufLen();

        if (buflen != xbuf)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout uncompress with invalid data length."
                                          "prel:%u, retl:%u"
                                          , xbuf, buflen
                                          );
            return false;
        }

        uint16_t *pSource = (uint16_t*)buf.GetBufAddr();
        uint16_t nSource = nChan;
        BufferController_t op2;
        {
            BufferCtrlInit(&op2);
        }

        if (!_tvuutilIsBigEndian())
        {
            for (uint16_t i=0; i<nChan; i++)
            {
                FAILED_PUSH_DATA_RET(BufferCtrlWBe16(&op2, pSource[i]),false);
            }

            pSource = (uint16_t *)BufferCtrlGetOrigPtr(&op2);
        }

        copyChannel(pSource, nSource, _bPlanar);
        copyBin(pBin, nBin);

        BufferCtrlRelease(&op);
        BufferCtrlRelease(&op2);
        return true;
    }

    int AudioChannelLayoutInternal::Compare(const AudioChannelLayoutInternal &other)const
    {
        int ret = _bufBin.Compare(other._bufBin);
        return ret;
    }

    int AudioChannelLayoutInternal::Copy(const AudioChannelLayoutInternal &other)
    {
        int ret = 0, x = -1;
        _bPlanar = other._bPlanar;
        ret += sizeof(bool);
        x = _bufBin.Copy(other._bufBin);
        if (x < 0)
        {
            return x;
        }
        ret += x;
        x = _bufChannel.Copy(other._bufChannel);
        if (x < 0)
        {
            return x;
        }
        ret += x;
        _bPlanar = other._bPlanar;
        return ret;
    }

    void AudioChannelLayoutInternal::_init()
    {
        _bPlanar = false;
    }

    bool AudioChannelLayoutInternal::isSameChannel(const ChannelVal_t *p2, uint16_t n2, bool bPlanar)const
    {
        return ((bPlanar==_bPlanar) && !_bufChannel.Compare((const uint8_t *)p2, ((uint32_t)n2) * sizeof(uint16_t)));
    }

    bool AudioChannelLayoutInternal::isSameBin(const uint8_t *p2, uint32_t n2)const
    {
        return (!_bufBin.Compare(p2, n2));
    }

    bool AudioChannelLayoutInternal::zlibCompress(CacheBuffer &dest, const uint8_t *source, unsigned long sourceLen)
    {
        unsigned long zlen = ZlibWrapEstimateCompressSize(sourceLen);
        CacheBuffer &buf = dest;
        bool b = buf.Alloc(zlen);

        if (!b)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout binary compress alloc failed."
                                          "sl:%ld,dl:%ld"
                                          , sourceLen, zlen
                                          );
            return false;
        }

        zlen = buf.GetAllocSize();
        int ret = ZlibWrapCompress(buf.GetBufAddr(), &zlen, source, sourceLen);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout compress failed."
                                          "ret:%d,sl:%ld,dl:%ld"
                                          , ret, sourceLen, zlen
                                          );
            buf.SetBufLen(0);
            return false;
        }
        else if (ret > 0)
        {
            DEBUG_SHMMEDIA_PROTO_WARN_CR("audio channel layout compress with no zero return."
                                          "ret:%d,sl:%ld,dl:%ld"
                                          , ret, sourceLen, zlen
                                          );
        }

        buf.SetBufLen(zlen);
        return true;
    }

    bool AudioChannelLayoutInternal::zlibUncompress(CacheBuffer &dstBuf,   unsigned long destLen,
                                                   const uint8_t *source, unsigned long sourceLen)
    {
        unsigned long zlen = destLen;
        CacheBuffer &buf = dstBuf;
        bool b = buf.Alloc(zlen);

        if (!b)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout binary uncompress alloc failed."
                                          "sl:%ld,dl:%ld"
                                          , sourceLen, zlen
                                          );
            return false;
        }

        zlen = buf.GetAllocSize();
        int ret = ZlibWrapUnCompress(buf.GetBufAddr(), &zlen, source, sourceLen);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("audio channel layout uncompress failed."
                                          "ret:%d,sl:%ld,dl:%ld"
                                          , ret, sourceLen, zlen
                                          );
            buf.SetBufLen(0);
            return false;
        }
        else if (ret > 0)
        {
            DEBUG_SHMMEDIA_PROTO_WARN_CR("audio channel layout uncompress with no zero return."
                                          "ret:%d,sl:%ld,dl:%ld"
                                          , ret, sourceLen, zlen
                                          );
        }

        buf.SetBufLen(zlen);
        return true;
    }

    bool AudioChannelLayoutInternal::copyBin(const uint8_t *p, uint32_t n)
    {
        bool b = false;

        if (!p || !n)
        {
            return false;
        }

        if (isSameBin(p, n))
        {
            return true;
        }

        CacheBuffer &buff = _bufBin;
        uint32_t nSrc = n;
        const uint8_t *pSrc = p;

        b = buff.Alloc(nSrc);

        if (!b)
        {
            return false;
        }

        buff.Copy(pSrc, nSrc);
        return true;
    }

    bool AudioChannelLayoutInternal::copyChannel(const ChannelVal_t *pChan, uint16_t nChan, bool bPlanar)
    {
        bool b = false;

        if (!pChan || !nChan)
        {
            return false;
        }

        if (isSameChannel(pChan, nChan, bPlanar))
        {
            return true;
        }

        CacheBuffer &buff = _bufChannel;
        uint32_t nSrc = (uint32_t)nChan * sizeof(ChannelVal_t);
        const uint8_t *pSrc = (const uint8_t *)pChan;

        b = buff.Alloc(nSrc);

        if (!b)
        {
            return false;
        }

        buff.Copy(pSrc, nSrc);
        _bPlanar = bPlanar;
        return true;
    }

}

libshmmedia_audio_channel_layout_object_t *LibshmmediaAudioChannelLayoutCreate()
{
    return new tvushm::AudioChannelLayoutInternal();
}

bool LibshmmediaAudioChannelLayoutReset(libshmmedia_audio_channel_layout_object_t *pctx)
{
    tvushm::AudioChannelLayoutInternal *p = (tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    p->Release();
    return true;
}

bool LibshmmediaAudioChannelLayoutIsPlanar(const libshmmedia_audio_channel_layout_object_t *pctx)
{
    const tvushm::AudioChannelLayoutInternal *p = (const tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    return p->IsPlanar();
}

uint16_t LibshmmediaAudioChannelLayoutGetChannelNum(const libshmmedia_audio_channel_layout_object_t *pctx)
{
    const tvushm::AudioChannelLayoutInternal *p = (const tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return 0;
    }

    return p->GetChannelNum();
}

bool LibshmmediaAudioChannelLayoutGetChannelArr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint16_t **ppDest, /*OUT*/uint16_t *pnDest)
{
    const tvushm::AudioChannelLayoutInternal *p = (const tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    const uint16_t *pDest = NULL;
    uint16_t nDest = 0;
    {
        p->GetChannelArr(pDest, nDest);
    }

    bool bret = false;
    if (pDest && nDest)
    {
        if (ppDest)
        {
            *ppDest = pDest;
        }

        if (pnDest)
        {
            *pnDest = nDest;
        }
        bret = true;
    }

    return bret;
}

bool LibshmmediaAudioChannelLayoutGetBinaryAddr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint8_t **ppDest, /*OUT*/uint32_t *pnDest)
{
    const tvushm::AudioChannelLayoutInternal *p = (const tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    const uint8_t *pDest = NULL;
    uint32_t nDest = 0;
    {
        p->GetBinArr(pDest, nDest);
    }

    bool bret = false;
    if (pDest && nDest)
    {
        if (ppDest)
        {
            *ppDest = pDest;
        }

        if (pnDest)
        {
            *pnDest = nDest;
        }
        bret = true;
    }

    return bret;
}

bool LibshmmediaAudioChannelLayoutSerializeToBinary(libshmmedia_audio_channel_layout_object_t *pctx
                                                   , const uint16_t *pChan, uint16_t nChan, bool bPlanar
                                                   )
{
    bool b = false;

    tvushm::AudioChannelLayoutInternal *p = (tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    b = p->SerializeToBinary(pChan, nChan, bPlanar);

    if (!b)
    {
        return false;
    }


    return b;
}

bool LibshmmediaAudioChannelLayoutParseFromBinary(libshmmedia_audio_channel_layout_object_t *pctx
                                              , const uint8_t *pBin, uint32_t nBin
                                              )
{
    bool b = false;

    tvushm::AudioChannelLayoutInternal *p = (tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return false;
    }

    b = p->ParseFromBinary(pBin, nBin);

    if (!b)
    {
        return false;
    }

    return b;
}

void LibshmmediaAudioChannelLayoutDestroy(libshmmedia_audio_channel_layout_object_t *pctx)
{
    tvushm::AudioChannelLayoutInternal *p = (tvushm::AudioChannelLayoutInternal *)pctx;

    if (!p)
    {
        return;
    }

    delete p;
    return;
}

int LibshmmediaAudioChannelLayoutCompare(const libshmmedia_audio_channel_layout_object_t *p1
                                              , const libshmmedia_audio_channel_layout_object_t *p2
                                              )
{
    if (p1 == p2)
    {
        return 0;
    }

    if (!p1)
    {
        return -1;
    }

    if (!p2)
    {
        return 1;
    }

    const tvushm::AudioChannelLayoutInternal *px1 = (const tvushm::AudioChannelLayoutInternal *)p1;
    const tvushm::AudioChannelLayoutInternal *px2 = (const tvushm::AudioChannelLayoutInternal *)p2;

    return px1->Compare(*px2);
}

int LibshmmediaAudioChannelLayoutCopy(libshmmedia_audio_channel_layout_object_t *pdst
                                              , const libshmmedia_audio_channel_layout_object_t *psrc
                                              )
{
    if (!pdst || !psrc)
    {
        return -1;
    }

    tvushm::AudioChannelLayoutInternal *px1 = (tvushm::AudioChannelLayoutInternal *)pdst;
    const tvushm::AudioChannelLayoutInternal *px2 = (const tvushm::AudioChannelLayoutInternal *)psrc;

    return px1->Copy(*px2);
}
