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
#ifndef LIBSHM_MEDIA_MEDIA_HEAD_PROTO_INTERNAL_H
#define LIBSHM_MEDIA_MEDIA_HEAD_PROTO_INTERNAL_H

#include "libshm_key_value.h"
#include "libshm_media_audio_track_channel_proto_internal.h"
#include "libshm_media_media_head_protocol.h"
#include "buffer_controller.h"


enum ELibshmmediaMediaHeadKey
{
    kLibshmmediaMediaHeadUnknown = 0,
    kLibshmmediaMediaHeadTrackChannelLayout = 1,
};

namespace tvushm {
    int _mediaHeadSetChannelLayout(KeyValParam &par, const libshmmedia_audio_channel_layout_object_t *hChanenl);

    int _mediaHeadGetChannelLayout(const uint8_t *pBin, uint32_t nBin, libshmmedia_audio_channel_layout_object_t *hChanenl);

}

#endif // LIBSHM_MEDIA_MEDIA_HEAD_PROTO_INTERNAL_H
