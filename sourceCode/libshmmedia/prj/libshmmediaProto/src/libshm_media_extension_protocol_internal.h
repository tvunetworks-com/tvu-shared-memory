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
#ifndef LIBSHM_MEDIA_EXTENSION_PROTOCOL_INTERNAL_H
#define LIBSHM_MEDIA_EXTENSION_PROTOCOL_INTERNAL_H
#include "libshm_media_extension_protocol.h"
#include <stdint.h>

/**
 *  new extented data structure.
 *
 *  |---------------------------|
 *  |version(4Bytes)            |
 *  |0x 's', 'h', 'm', 'e'      |
 *  |total length(4Bytes)       |
 *  |counts(4Byte)              |
 *  |---------------------------|
 *  |type(4bytes)               |
 *  |len(4bytes)                |
 *  |data(bin)                  |
 *  |---------------------------|
 *  |type(4bytes)               |
 *  |len(4bytes)                |
 *  |data(bin)                  |
 *  |---------------------------|
 *  |                           |
 *  |           ...             |
 *  |                           |
 *  |---------------------------|
 *
**/

enum ELibShmMediaExtendDataTypeV2
{
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UNKNOWN = 0,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID = 1,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP = 2,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CAPTION_TEXT = 3,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PRODUCER_STREAM_INFO = 4,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_RECEIVER_INFO = 5,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE104_DATA = 6,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX = 7,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_STARTIMECODE = 8,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_HDR_METADATA = 9,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE35_DATA = 10,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE_WITH_FPS_INDEX = 11,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PIC_STRUCT = 12,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMESTAMP = 13,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE = 14,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_METADATA_PTS = 15,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMEBASE = 16,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE_AFD_METADATA = 17,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_ACTION_TIMESTAMP = 18,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_VANC_SMPTE2038 = 19,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_GOP_POC_V1 = 20,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_MEDIA_HEAD_V1 = 21, /* this is reserved for media head with key-value */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE336M = 22, /* this used the concat-binary protocol */

     /* reserver protocol of subtitle for future. */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PROTOCOL_EXTENTION_RESERVER = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PRIVATE_PROTOCOL_EXTENTION_STRUCTURE, /* reserver protocol of subtitle for future. */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVB_TELETEXT = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT, /* dvb teletext */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVB_SUBTITLE = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_SUBTITLE, /* dvb subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVD_SUBTITLE = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVD_SUBTITLE, /* dvd subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_WEBVTT = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT, /* webvtt subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SRT = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT, /* SubRip subtitle with embedded timing */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBRIP = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP, /* SubRip subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_RAW_TEXT = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT, /* raw UTF-8 text */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_TTML = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_TTML, /* Timed Text Markup Language */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_VPLAYER = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_VPLAYER, /* VPlayer subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_EIA608 = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_EIA608, /* EIA-608 closed captions */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SSA = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA, /* SSA (SubStation Alpha) subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_ASS = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_ASS, /* ASS (Advanced SSA) subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_STL = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_STL, /* Spruce subtitle format */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBVIEW = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEW, /* SubViewer subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBVIEWV1 = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEWV1, /* SubViewer V1 subtitle */
};

typedef struct _LibShmMediaExtendedEntryData
{
    uint32_t    u_len;
    uint32_t    u_type;
    const uint8_t *p_data;
}libshmmedia_extended_entry_data_t;

//struct SLibShmMediaUserDataContext
//{
//    int         i_counts;
//    int         i_len;
//    uint8_t     *p_user_data;
//};

#define LIBSHMMEDIA_EXTENTED_DATA_STRUCT_VERSION_1  0x01
#define LIBSHMMEDIA_EXTENTED_DATA_HEAD_TAG  (('s' << 24) | ('h' << 16) | ('m' << 8) | 'e')

class CLibShmMediaExtendedDataV2
{
public:
    int resetEntry();
    /**
     *  Return:
     *      >=0, the real writing bytes to dst buffer.
     *      <0, adding entry failed.
    **/
    int addEntryToBuff(/*IN*/const uint8_t *pEntryData, /*IN*/uint32_t iEntrType, /*IN*/uint32_t iEntryLen
        , /*OUT*/uint8_t *pDstBuf, /*IN*/int iBufSize);
    unsigned int getBufWriteSize();
    int parseBuff(const uint8_t *pbuf, uint32_t ibuflen);
    unsigned int getEntryCouts();
    int getOneEntryLen(int entryIndex);
    unsigned int getOneEntryType(int entryIndex);
    int getOneEntryData(int entryIndex, uint8_t *pbuf, int ibufsize);
    int getOneEntry(int entryIndex, libshmmedia_extended_entry_data_t *pEntryData);
    void setUsingExternalBuf(bool b) {_bUsingExternalDataBuf = b;}

    CLibShmMediaExtendedDataV2();
    ~CLibShmMediaExtendedDataV2();

private:
    uint32_t    m_uVersion;
    uint32_t    m_uAllocEntryCounts;
    uint32_t    m_uEntryCounts;
    uint8_t     *m_pBuffer;
    uint32_t    m_uAllocBufSize;
    uint32_t    m_uBufSize;
    uint32_t    m_uBufOffsetW;
    uint32_t    m_uBufOffsetR;
    bool        m_bEntryNotDone;
    bool        _bUsingExternalDataBuf;
    libshmmedia_extended_entry_data_t    *m_pEntryOffsetArr;

    int allocEntryArr();
    int allocBuf(unsigned int needsize);
    int _parseBuff(const uint8_t *pbuf, uint32_t ibuflen);
};

#ifdef __cplusplus
    extern "C" {
#endif

/**
 *  Return:
 *      >=0, the real writing bytes to dst buffer.
 *      <0, adding entry failed.
**/
int LibshmMediaExtDataAddOneEntry(libshmmedia_extended_data_context_t h
    , libshmmedia_extended_entry_data_t *pEntryData
    , uint8_t *pDstBuf, uint32_t iDstBufSize
);

/**
 *  Return:
 *      0 : get entry data successfully.
 *      <0  : failed to get.
**/
int LibshmMediaExtDataGetOneEntry(libshmmedia_extended_data_context_t h
    , int entryIndex
    , libshmmedia_extended_entry_data_t *pEntry);

#ifdef __cplusplus
    }
#endif

#endif // LIBSHM_MEDIA_EXTENSION_PROTOCOL_INTERNAL_H
