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
#ifndef LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_INTERNAL_H
#define LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_INTERNAL_H

#include <stdint.h>
#include <string.h>

#include "libshm_media_subtitle_private_protocol.h"

#pragma pack(push, 1)

enum _libshmmediaSubtitlePrivateProtoVer
{
    kLibshmmediaSubtitlePrivateProtoVer1 = 1,
};

typedef
struct _libshmmediaSubtitleDataInfoV1
{
    uint16_t    infoStructSize_;
    uint32_t    dataOffSet_;
    uint32_t    dataSize_;
    uint32_t    type_;
    uint64_t    timestampStart_;
    uint32_t    duration_;
//    uint32_t    posXNum_;
//    uint32_t    posXDen_;
//    uint32_t    posYNum_;
//    uint32_t    posYDen_;
//    uint32_t    fontSize_;
//    uint16_t    align_; // 0-lefttop, 1-leftbottom, 2-rightbottom, 3-right top.

}libshmmedia_subtitle_data_info_v1_t;

typedef  libshmmedia_subtitle_data_info_v1_t libshmmedia_subtitle_data_info_t;

typedef struct _libshmmediaSubtitleProtocol
{
    uint8_t     protoVer_; // first version 1.
    uint8_t     counts_;
    uint16_t    reserver2_;
    uint16_t    dataInfoOffset_;
    //libshmmedia_subtitle_data_info_t dataInfo_[0];

    //.....
    uint8_t     data_[0];
}libshmmedia_subtitle_protocol_t;

#pragma pack(pop)

typedef struct _libshmmediaSubtitleDataInternalExtInfor
{
    uint32_t timestamPos_;
    uint32_t dataPos_;
}libshmmedia_subtitle_data_internal_ext_infor_t;

typedef struct LibShmmediaSubtitlePrivateProtocolInternalEntries
{
    LibShmmediaSubtitlePrivateProtocolInternalEntries(libshmmedia_subtitle_private_proto_entries_t*p)
        :p_(p)
    {
        memset(&extInfo_, 0, sizeof(extInfo_));
    }
    libshmmedia_subtitle_private_proto_entries_t * const p_;
    libshmmedia_subtitle_data_internal_ext_infor_t extInfo_[LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS];
}libshmmedia_subtitle_private_proto_internal_entries_t;

#endif // LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_INTERNAL_H
