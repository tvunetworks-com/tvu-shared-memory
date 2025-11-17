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
#ifndef LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_H
#define LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_H
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
#endif

#else
#define _LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_ __attribute__((visibility("default")))
#endif

#define LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS 8

typedef
struct LibShmmediaSubtitlePrivateProtocolEntryItemV1
{
    uint32_t    strucSize;
    uint32_t    type;
/**
 * type can reuse as this.
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT = 0x10001,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_SUBTITLE = 0x10002,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVD_SUBTITLE = 0x10003,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT = 0x10004,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT = 0x10005,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP = 0x10006,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT = 0x10007,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_TTML = 0x10008,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_VPLAYER = 0x10009,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_EIA608 = 0x1000a,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA = 0x1000b,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_ASS = 0x1000c,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_STL = 0x1000d,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEW = 0x1000e,
//    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEWV1 = 0x1000f,
**/
    uint64_t    timestamp;
    uint32_t    duration;
    uint32_t    dataLen;
//    uint32_t    posXNum;
//    uint32_t    posXDen;
//    uint32_t    posYNum;
//    uint32_t    posYDen;
//    uint32_t    fontSize;
//    uint8_t     align; // 0-lefttop, 1-leftbottom, 2-rightbottom, 3-right top.
    const uint8_t *data;
}libshmmedia_subtitle_private_proto_entry_item_v1_t;

typedef libshmmedia_subtitle_private_proto_entry_item_v1_t libshmmedia_subtitle_private_proto_entry_item_t;

typedef
struct LibShmmediaSubtitlePrivateProtocolEntryHeadV1
{
    int counts;
}libshmmedia_subtitle_private_proto_entry_head_v1_t;

typedef libshmmedia_subtitle_private_proto_entry_head_v1_t libshmmedia_subtitle_private_proto_entry_head_t;

typedef struct LibShmmediaSubtitlePrivateProtocolEntries
{
    libshmmedia_subtitle_private_proto_entry_head_t head;
    libshmmedia_subtitle_private_proto_entry_item_t entries[LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS];
}libshmmedia_subtitle_private_proto_entries_t;


#ifdef __cplusplus
extern "C" {
#endif

_LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
bool LibshmmediaSubtitlePrivateProtoIsCorrectType(uint32_t);

_LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
int LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(const libshmmedia_subtitle_private_proto_entries_t *);

/**
 *  RETURN:the length of writing.
 *      <=0:failed
 *      >0:length of writing.
**/
_LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
int LibshmmediaSubtitlePrivateProtoWriteBufferSize(const libshmmedia_subtitle_private_proto_entries_t *, uint8_t *buffer, int bufferSize);

/**
 *  RETURN:
 *      < 0:failed
 *      0:success
**/
_LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
int LibshmmediaSubtitlePrivateProtoParseBufferSize(libshmmedia_subtitle_private_proto_entries_t *, const uint8_t *buffer, int bufferSize);

/**
 *  RETURN:
 *      < 0:failed
 *      0:success
**/
_LIBSHMMEDIA_PROTO_EXT_PRIVATE_DLL_
int LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(uint8_t *buffer, int bufferSize, int offsetMs);


#ifdef __cplusplus
};
#endif


#endif // LIBSHM_MEDIA_SUBTITLE_PRIVATE_PROTOCOL_H
