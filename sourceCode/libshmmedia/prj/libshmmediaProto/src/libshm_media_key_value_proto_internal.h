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
#ifndef LIBSHM_MEDIA_KEY_VALUE_PROTO_INTERNAL_H
#define LIBSHM_MEDIA_KEY_VALUE_PROTO_INTERNAL_H

#include "libshm_key_value.h"
#include "libshm_media_media_head_proto_internal.h"

enum ELibshmmediaKeyValueProtoValue
{
    kLibshmmediaKeyValueProtoTypeUnknown = 0,
    kLibshmmediaKeyValueProtoTypeMediaHead = 1,
};

namespace tvushm {
    int _keyValueSetMediaHead(KeyValParam &op, const uint8_t *p, uint32_t n);
    int keyValueProtoAppendToBuffer(BufferController_t &buff, const libshmmedia_audio_channel_layout_object_t *hChannel);
    int keyValueProtoExtractFromBuffer(libshmmedia_audio_channel_layout_object_t *hChannel, const uint8_t *p, uint32_t n);
}

#endif // LIBSHM_MEDIA_KEY_VALUE_PROTO_INTERNAL_H
