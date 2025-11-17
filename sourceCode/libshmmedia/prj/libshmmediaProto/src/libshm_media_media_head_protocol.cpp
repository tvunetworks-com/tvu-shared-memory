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
#include "libshm_key_value.h"
#include "libshm_media_media_head_proto_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include "libshm_util_common_internal.h"

namespace tvushm {

    int _mediaHeadSetChannelLayout(KeyValParam &op, const libshmmedia_audio_channel_layout_object_t *hChanenl)
    {
        if (!hChanenl)
        {
            return 0;
        }
        const uint8_t *pDest = NULL;
        uint32_t nDest = 0;
        bool b = LibshmmediaAudioChannelLayoutGetBinaryAddr(hChanenl, &pDest, &nDest);
        if (!b)
        {
            return 0;
        }
        op.SetParamAsBytesReference(kLibshmmediaMediaHeadTrackChannelLayout, pDest, nDest);
        return op.GetCompactBytesNumNeeded();
    }

    int _mediaHeadGetChannelLayout(const uint8_t *p, uint32_t n, libshmmedia_audio_channel_layout_object_t *hChannel)
    {
        if (!n || !p)
        {
            return 0;
        }

        KeyValParam par;
        BufferController_t tmp2;
        {
            BufferCtrlAttachExternalReadBuffer(&tmp2, p, n);
        }
        if (par.ExtractFromBuffer(tmp2, true) <= 0)
        {
            return 0;
        }

        uint32_t k = kLibshmmediaMediaHeadTrackChannelLayout;
        if (!par.HasParameter(k))
        {
            return 0;
        }

        const Variant &var = par.GetParameter(k);

        if (var.GetType() != Variant::BytesType)
        {
            return 0;
        }

        const Bytes &buf = var.GetAsBytes();

        const uint8_t *pBin = buf.GetBufAddr();
        uint32_t nBin = buf.GetBufLen();

        if (!nBin || !pBin)
        {
            return 0;
        }

        if (!LibshmmediaAudioChannelLayoutParseFromBinary(hChannel, pBin, nBin))
        {
            char hxstr[256] = {0};
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("parse channel layout failed."
                                          "n:%u,p:{%s}"
                                          , nBin
                                          , _tvuutilTransBinaryToHexString(pBin, nBin, hxstr, sizeof(hxstr))
                                          );
            return 0;
        }

        return nBin;
    }

}
