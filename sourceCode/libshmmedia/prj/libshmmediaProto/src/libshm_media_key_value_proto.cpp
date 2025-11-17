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
#include "libshm_media_key_value_proto_internal.h"
#include "libshm_media_media_head_proto_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include "libshm_media_protocol_internal.h"
#include "libshm_util_common_internal.h"

namespace tvushm {

    int _keyValueSetMediaHead(KeyValParam &op, const uint8_t *p, uint32_t n)
    {
        if (!p || !n)
        {
            return 0;
        }
        op.SetParamAsBytesReference(kLibshmmediaKeyValueProtoTypeMediaHead, p, n);
        return op.GetCompactBytesNumNeeded();
    }

    int keyValueProtoAppendToBuffer(BufferController_t &buff, const libshmmedia_audio_channel_layout_object_t *hChannel)
    {
        BufferController_t tmp;
        KeyValParam op;
        if (_mediaHeadSetChannelLayout(op, hChannel) <= 0)
        {
            return -1;
        }

        {
            op.AppendToBuffer(tmp);
        }
        const uint8_t *pout = NULL;
        uint32_t nout = 0;
        {
            pout = tvushm::BufferCtrlGetOrigPtr(&tmp);
            nout = tvushm::BufferCtrlGetBufLen(&tmp);
        }

        op.Clear();
        if (_keyValueSetMediaHead(op, pout, nout) <= 0)
        {
            return -1;
        }

        return op.AppendToBuffer(buff);
    }

    int keyValueProtoExtractFromBuffer(libshmmedia_audio_channel_layout_object_t *hChannel, const uint8_t *p, uint32_t n)
    {
        BufferController_t tmp;
        {
            BufferCtrlAttachExternalReadBuffer(&tmp, p, n);
        }

        KeyValParam par;
        int ret = par.ExtractFromBuffer(tmp, true);

        if (ret <= 0)
        {
            return ret;
        }

        uint32_t k = kLibshmmediaKeyValueProtoTypeMediaHead;
        if (par.HasParameter(k) && hChannel) {
            do
            {
                const Variant &var = par.GetParameter(k);
                if (var.GetType() != Variant::BytesType)
                {
                    ret = 0;
                    break;
                }

                const Bytes &channelLayoutBin = var.GetAsBytes();
                const uint8_t *pBin = channelLayoutBin.GetBufAddr();
                uint32_t nBin = channelLayoutBin.GetBufLen();
                ret = _mediaHeadGetChannelLayout(pBin, nBin, hChannel);
            }while(0);
        }
        return ret;
    }

}
