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
#ifndef LIBSHM_MEDIA_EXTENSION_PROTOCOL_H
#define LIBSHM_MEDIA_EXTENSION_PROTOCOL_H

#include <stdint.h>
#include "libshm_media_proto_common.h"
#include "libshm_media_subtitle_private_protocol.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define LIBSHM_MEDIA_TVULIVE_FOURCCTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

typedef enum ELibShmMediaType
{
    LIBSHM_MEDIA_TYPE_INVALID = 0,
    LIBSHM_MEDIA_TYPE_TVU_UID = 1,
    LIBSHM_MEDIA_TYPE_TVU_INTERLACE_FLAG = 2,
    LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA = 3,
    LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2 = 4,

    LIBSHM_MEDIA_TYPE_MAX_NUM = 1025,

    /* if frame protocol to use */
    LIBSHM_MEDIA_TYPE_TVULIVE_AUDIO = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'L', 'V', 'A'),
    LIBSHM_MEDIA_TYPE_TVULIVE_HEADER = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'L', 'V', 'H'),
    LIBSHM_MEDIA_TYPE_TVULIVE_VIDEO = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'L', 'V', 'V'),
    // user define the protocol at the tvulive data type
    LIBSHM_MEDIA_TYPE_TVULIVE_DATA = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'L', 'V', 'D'),
    LIBSHM_MEDIA_TYPE_ENCODING_DATA = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'E', 'N', 'C'),
    LIBSHM_MEDIA_TYPE_CONTROL_DATA = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'C', 'T', 'L'),
    LIBSHM_MEDIA_TYPE_MPEG_TS_DATA = LIBSHM_MEDIA_TVULIVE_FOURCCTAG('T', 'M', 'T', 'S'), /* used to transfer mpegts source stream */
    /* endif */

    LIBSHM_MEDIA_TYPE_TVU_OTHER_MEDIA_INFO = 0xffffffff,
    /* others is used for caller's self definition */
}libshm_media_type_t;

enum ELibShmMediaExtenstionSubtitleType
{
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PRIVATE_PROTOCOL_EXTENTION_STRUCTURE = 0x10000,
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT = 0x10001, /* dvb teletext */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_SUBTITLE = 0x10002, /* dvb subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVD_SUBTITLE = 0x10003, /* dvd subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT = 0x10004, /* webvtt subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT = 0x10005, /* SubRip subtitle with embedded timing */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP = 0x10006, /* SubRip subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT = 0x10007, /* raw UTF-8 text */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_TTML = 0x10008, /* Timed Text Markup Language */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_VPLAYER = 0x10009, /* VPlayer subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_EIA608 = 0x1000a, /* EIA-608 closed captions */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA = 0x1000b, /* SSA (SubStation Alpha) subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_ASS = 0x1000c, /* ASS (Advanced SSA) subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_STL = 0x1000d, /* Spruce subtitle format */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEW = 0x1000e, /* SubViewer subtitle */
    LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBVIEWV1 = 0x1000f, /* SubViewer V1 subtitle */
};

#define TVU_SHM_USER_DATA_SIZE  1024
#define TVU_SHM_PRODUCER_STREAM_INFO_BUFFER_LENGTH 128

typedef struct _ShmExtendDataStruct
{
    const uint8_t*      p_uuid_data;
    int                 i_uuid_length;
    const uint8_t*      p_cc608_cdp_data;
    int                 i_cc608_cdp_length;
    const uint8_t*      p_caption_text;
    int                 i_caption_text_length;
    const uint8_t*      p_producer_stream_info;
    int                 i_producer_stream_info_length;
    const uint8_t*      p_receiver_info;
    int                 i_receiver_info_length;
    const uint8_t*      p_scte104_data;
    int                 i_scte104_data_len;
    const uint8_t*      p_scte35_data;
    int                 i_scte35_data_len;
    /* old timecode, deprecated --> */
    const uint8_t*      p_timecode_index;
    int                 i_timecode_index_length;
    const uint8_t*      p_start_timecode;
    int                 i_start_timecode_length;
    /* <-- old timecode, deprecated */
    const uint8_t*      p_hdr_metadata;
    int                 i_hdr_metadata;
    /* new timecode, which has fps/index --> */
    const uint8_t*      p_timecode_fps_index;
    int                 i_timecode_fps_index;
    /* <-- new timecode, which has fps/index */
    const uint8_t*      p_pic_struct;
    int                 i_pic_struct;
    const uint8_t*      p_source_timestamp;
    int                 i_source_timestamp;
    /* timecode for muxer, standard hh::mm::ss.frame --> */
    const uint8_t*      p_timecode;
    int                 i_timecode;
    /* <-- timecode for muxer, standard hh::mm::ss.frame */

    /* <-- metadata pts, 64bit timestamp */
    const uint8_t*      p_metaDataPts;// little endian uint64 data.
    int                 i_metaDataPts;
    /* <-- metadata pts, 64bit timestamp */
    const uint8_t*      p_source_timebase;
    int                 i_source_timebase;
    /* <-- smpte afd metadata */
    const uint8_t*      p_smpte_afd_data;
    int                 i_smpte_afd_data;

    /* <-- source action timestamp was used to record the input source original action timestamp, milliseconds, 8bytes. */
    const uint8_t*      p_source_action_timestamp;
    int                 i_source_action_timestamp;
    /* --> source action timestamp */

    /* <-- vanc_smpte2038 was used to store vanc smpte 2038 meta data */
    const uint8_t*      p_vanc_smpte2038;
    int                 i_vanc_smpte2038;
    /* --> vanc_smpte2038 */

    /* <-- used to express gop poc information, uint32 little endian. */
    const uint8_t*      p_gop_poc;
    int                 i_gop_poc;
    /* --> used to express gop poc information */

    /* <-- used to express sutitle protocol. */
    uint32_t            u_subtitle_type;
    const uint8_t*      p_subtitle;
    int                 i_subtitle;
    /* --> used to express sutitle protocol. */

    /* <-- used to express smpte 336M data, using bin-concat protocol. */
    const uint8_t*      p_smpte336m;
    int                 i_smpte336m;
    /* --> used to express smpte 336M data, using bin-concat protocol. */

    /**
      * Chromaticity coordinates of the source primaries.
      * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.1 and ITU-T H.273.
      */
    bool                bHasColorPrimariesVal_;
    uint32_t            uColorPrimariesVal_;

    /**
     * Color Transfer Characteristic.
     * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.2.
     */
    bool                bHasColorTransferCharacteristicVal_;
    uint32_t            uColorTransferCharacteristicVal_;

    /**
     * YUV colorspace type.
     * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.3.
     */
    bool                bHasColorSpaceVal_;
    uint32_t            uColorSpaceVal_;

    /**
     * YUV colorspace type.
     * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.3.
     */
    bool                bHasVideoFullRangeFlagVal_;
    uint32_t            uVideoFullRangeFlagVal_;

    /**
     * used to store the tvutimestamp status.
     * similar to p_timecode_fps_index
     */
    bool                bGotTvutimestamp;
    uint64_t            u64Tvutimestamp;

}Lib_shm_extend_data_t, libshmmedia_extend_data_info_t;

/**
 *  Functionality:
 *      parsing the pShmUserData data out, assign the value to pExtendData.
 *
 *  Parameters:
 *      @pShmUserData[IN]   :   external buffer point.
 *      @dataSize[IN]       :   input buffer real size.
 *      @pExtendData[OUT]   :   destination external data structure.
 *  Return:
 *      void.
 *
 *  ===============================================
 *       v1 user data structure
 *  ===============================================
 *
 *  |---------------------------|
 *  |DataFlag(4Bytes)           |
 *  |---------------------------|
 *  |len(4bytes)                |
 *  |data(bin)                  |
 *  |---------------------------|
 *  |len(4bytes)                |
 *  |data(bin)                  |
 *  |---------------------------|
 *  |           ...             |
 *  |---------------------------|
 *
**/

//old deprecated API for read v1 user data,use libShmReadExtendDataV2 to instead
//_LIBSHMMEDIA_PROTO_DLL_
//void libShmReadExtendData(Lib_shm_extend_data_t* pExtendData,const uint8_t* pShmUserData,int dataSize);


typedef void * libshmmedia_extended_data_context_t;

/************************************************************************/
/* a wrap for parser user data by userDataType                          */
/*      return 0 for success,<0 for error                               */
/************************************************************************/
/**
 *  Description:
 *      used to read out extended data, the data point was not at @pShmUserData but at the internal buffer of @v2DataCtx
**/
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaReadExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx);

/**
 *  Description:
 *      used to read out extended data, the data point was not at @pShmUserData but at the internal buffer of @v2DataCtx
**/
_LIBSHMMEDIA_PROTO_DLL_
int libShmReadExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx);

/**
 *  Description:
 *      used to read out extended data, the data point was at @pShmUserData
 *  Return:
 *      0 - success, <0 - failed
**/
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMeidaParseExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType);
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMeidaParseExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize);


/**
 *  Functionality:
 *      used to estimate the buffer size for extend data.
 *  Parameters:
 *      @pExtendData[IN]    :   src external data information structure.
 *  Return:
 *      > 0     :   buffer size.
 *      = 0     :   no buffer needed.
 *      < 0     :   impossible
**/
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaEstimateExtendDataSize(/*IN*/const libshmmedia_extend_data_info_t* pExtendData);

/**
 *  Functionality:
 *      a func to create V2 user data buffer API
 *  Parameters:
 *      @dataBuffer[OUT]    :   destination buffer
 *      @buffersize[IN]     :   destination buffer size
 *      @pExtendData[IN]    :   src external data information structure.
 *  Return:
 *      >= 0    :   return the real writing size to the buffer.
 *      < 0     :   failed to write.
**/
_LIBSHMMEDIA_PROTO_DLL_
int libShmWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData);
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData);

_LIBSHMMEDIA_PROTO_DLL_
libshmmedia_extended_data_context_t
LibshmMediaExtDataCreateHandle();

_LIBSHMMEDIA_PROTO_DLL_
void LibshmMediaExtDataResetEntry(libshmmedia_extended_data_context_t h);

/**
 *  Return:
 *      the real writing buffer size
**/
_LIBSHMMEDIA_PROTO_DLL_
unsigned int LibshmMediaExtDataGetEntryBuffSize(libshmmedia_extended_data_context_t h);

/**
 *  Return:
 *      >=0 : entry couts to parse out.
 *      <0  : failed to parse.
**/
_LIBSHMMEDIA_PROTO_DLL_
int LibshmMediaExtDataParseBuff(libshmmedia_extended_data_context_t h, const uint8_t *pbuf, uint32_t ibuflen);

/**
 *  Return:
 *      current entry couts.
**/
_LIBSHMMEDIA_PROTO_DLL_
unsigned int LibshmMediaExtDataGetEntryCounts(libshmmedia_extended_data_context_t h);

_LIBSHMMEDIA_PROTO_DLL_
void LibshmMediaExtDataDestroyHandle(libshmmedia_extended_data_context_t *ph);

#ifdef __cplusplus
}
#endif

#endif // LIBSHM_MEDIA_EXTENSION_PROTOCOL_H
