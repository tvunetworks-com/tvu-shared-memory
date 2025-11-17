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
#ifndef LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTO_INTERNAL_H
#define LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTO_INTERNAL_H
#include "libshm_media_audio_track_channel_protocol.h"
#include "libshm_cache_buffer.h"
#include "libshm_util_endian.h"
namespace tvushm {

    class AudioChannelLayoutInternal
    {
    public:
        typedef uint16_t ChannelVal_t;
    public:
        AudioChannelLayoutInternal();
        virtual ~AudioChannelLayoutInternal();

        void Release();

        bool IsPlanar()const;
        uint16_t GetChannelNum()const;
        bool GetChannelArr(/*OUT*/const ChannelVal_t *&pDest, /*OUT*/uint16_t &nDest)const;
        bool GetBinArr(/*OUT*/const uint8_t *&pDest, /*OUT*/uint32_t &nDest)const;
        bool SerializeToBinary(const ChannelVal_t *pChan, uint16_t nChan, bool bPlanar);
        bool ParseFromBinary(const uint8_t *pBin, uint32_t nBin);
        int Compare(const AudioChannelLayoutInternal &other)const;
        int Copy(const AudioChannelLayoutInternal &other);
    private:
        void _init();
        bool isSameChannel(const ChannelVal_t *pChan, uint16_t nChan, bool bPlanar)const;
        bool isSameBin(const uint8_t *pBin, uint32_t nBin)const;
        bool zlibCompress(CacheBuffer &dstBuf, const uint8_t *source, unsigned long sourceLen);
        bool zlibUncompress(CacheBuffer &dstBuf,   unsigned long destLen,
                           const uint8_t *source, unsigned long sourceLen);

        bool copyBin(const uint8_t *p, uint32_t n);
        bool copyChannel(const ChannelVal_t *pChan, uint16_t nChan, bool bPlanar);
    private:
        bool                                    _bPlanar;
        CacheBuffer                             _bufBin;
        CacheBuffer                             _bufChannel;
    };

}

#endif // LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTO_INTERNAL_H
