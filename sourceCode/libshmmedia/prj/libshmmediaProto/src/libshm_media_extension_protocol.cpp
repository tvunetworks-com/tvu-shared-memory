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
#include "libshm_media_extension_protocol_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include "libshm_media_protocol_internal.h"
#include "libshm_key_value.h"
#include "libshm_tvu_timestamp.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/*****************************extended data accomplishment****************************************/

enum ELibShmExtendDataType
{
    LIBSHM_EXTEND_DATA_TYPE_UID = 1,
    LIBSHM_EXTEND_DATA_TYPE_CC608_CDP = 1 << 1,
    LIBSHM_EXTEND_DATA_TYPE_CAPTION_TEXT = 1 << 2,
    LIBSHM_EXTEBD_DATA_TYPE_PRODUCER_STREAM_INFO = 1 << 3,
    LIBSHM_EXTEND_DATA_TYPE_RECEIVER_INFO = 1 << 4,
    LIBSHM_EXTEND_DATA_TYPE_SCTE104_DATA = 1 << 5
};

static inline void _write8(uint8_t *p, uint8_t v)
{
    p[0] = v &0xFF;
}

static inline void _writeLe16(uint8_t *p, uint16_t v)
{
    _write8(p, v&0xFF);
    _write8(p+1, (v>>8) & 0xFF);
}

static inline void _writeLe32(uint8_t *p, uint32_t v)
{
    _writeLe16(p, v&0xFFFF);
    _writeLe16(p+2, (v>>16)&0xFFFF);
}

static inline void _writeLe64(uint8_t *p, uint64_t v)
{
    _writeLe32(p, v&0xFFFFFFFF);
    _writeLe32(p+4, (v>>32)&0xFFFFFFFF);
}

static inline void _writeBe16(uint8_t *p, uint16_t v)
{
    _write8(p, (v>>8) & 0xFF);
    _write8(p+1, v&0xFF);
}

static inline void _writeBe32(uint8_t *p, uint32_t v)
{
    _writeBe16(p, (v>>16)&0xFFFF);
    _writeBe16(p+2, v&0xFFFF);
}

static inline void _writeBe64(uint8_t *p, uint64_t v)
{
    _writeBe32(p, (v>>32)&0xFFFFFFFF);
    _writeBe32(p+4, v&0xFFFFFFFF);
}

static inline uint8_t  _read8(const uint8_t *p)
{
    return *p;
}

static inline uint16_t _readLe16(const uint8_t *p)
{
    uint16_t    v1 = _read8(p);
    uint16_t    v2 = _read8(p+1);
    uint16_t    v  = v1 | (v2<<8);
    return v;
}

static inline uint32_t _readLe32(const uint8_t *p)
{
    uint32_t    v1 = _readLe16(p);
    uint32_t    v2 = _readLe16(p+2);
    uint32_t    v  = v1 | (v2<<16);
    return v;
}

static inline uint64_t _readLe64(const uint8_t *p)
{
    uint64_t    v1 = _readLe32(p);
    uint64_t    v2 = _readLe32(p+4);
    uint64_t    v  = v1 | (v2<<32);
    return v;
}

static inline uint16_t _readBe16(const uint8_t *p)
{
    uint16_t    v1 = _read8(p);
    uint16_t    v2 = _read8(p+1);
    uint16_t    v  = v2 | (v1<<8);
    return v;
}

static inline uint32_t _readBe32(const uint8_t *p)
{
    uint32_t    v1 = _readBe16(p);
    uint32_t    v2 = _readBe16(p+2);
    uint32_t    v  = v2 | (v1<<16);
    return v;
}

static inline uint64_t _readBe64(const uint8_t *p)
{
    uint64_t    v1 = _readBe32(p);
    uint64_t    v2 = _readBe32(p+4);
    uint64_t    v  = v2 | (v1<<32);
    return v;
}

static int _readExtendDataV1(libshmmedia_extend_data_info_t* pUserData, const uint8_t* pShmUserData, int dataSize)
{
    ::memset(pUserData, 0, sizeof(libshmmedia_extend_data_info_t));
    uint32_t dataFlag = 0;
    memcpy(&dataFlag,pShmUserData, 4);
    uint32_t offset = 4;
    uint32_t tempLength = 0;
    if (dataFlag & LIBSHM_EXTEND_DATA_TYPE_UID)
    {
        memcpy(&tempLength, pShmUserData + offset, 4);
        offset += 4;
        pUserData->p_uuid_data = pShmUserData + offset;
        pUserData->i_uuid_length = tempLength;
        offset += tempLength;
    }
    if (dataFlag & LIBSHM_EXTEND_DATA_TYPE_CC608_CDP)
    {
        memcpy(&tempLength, pShmUserData + offset, 4);
        offset += 4;
        pUserData->p_cc608_cdp_data = pShmUserData + offset;
        pUserData->i_cc608_cdp_length = tempLength;
        offset += tempLength;
    }
    if (dataFlag & LIBSHM_EXTEND_DATA_TYPE_CAPTION_TEXT)
    {
        memcpy(&tempLength, pShmUserData + offset, 4);
        offset += 4;
        pUserData->p_caption_text = pShmUserData + offset;
        pUserData->i_caption_text_length = tempLength;
        offset += tempLength;
    }
    if (dataFlag & LIBSHM_EXTEBD_DATA_TYPE_PRODUCER_STREAM_INFO)
    {
        memcpy(&tempLength, pShmUserData + offset, 4);
        offset += 4;
        pUserData->p_producer_stream_info = pShmUserData + offset;
        pUserData->i_producer_stream_info_length = tempLength;
        offset += tempLength;
    }
    if (dataFlag & LIBSHM_EXTEND_DATA_TYPE_RECEIVER_INFO)
    {
        memcpy(&tempLength, pShmUserData + offset, 4);
        offset += 4;
        pUserData->p_receiver_info = pShmUserData + offset;
        pUserData->i_receiver_info_length = tempLength;
        offset += tempLength;
    }
    if (dataFlag & LIBSHM_EXTEND_DATA_TYPE_SCTE104_DATA)
    {
        memcpy(&tempLength,pShmUserData + offset,4);
        offset += 4;
        pUserData->p_scte104_data = pShmUserData + offset;
        pUserData->i_scte104_data_len = tempLength;
        offset += tempLength;
    }
    return 0;
}

static int createKeyValue(tvushm::BufferController_t &buff, const libshmmedia_extend_data_info_t* p)
{
    if (!p)
    {
        return 0;
    }
    tvushm::KeyValParam op;
    if (p->bHasColorPrimariesVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorPrimariesVal, p->uColorPrimariesVal_);
    }

    if (p->bHasColorTransferCharacteristicVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorTransferCharacteristicVal, p->uColorTransferCharacteristicVal_);
    }

    if (p->bHasColorSpaceVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorSpaceVal, p->uColorSpaceVal_);
    }

    if (p->bHasVideoFullRangeFlagVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeVideoFullRangeFlagVal, p->uVideoFullRangeFlagVal_);
    }

    if (p->bGotTvutimestamp)
    {
        op.SetParamAsU64(kLibShmMediaMetaKeyValueTypeTvutimestampVal, p->u64Tvutimestamp);
    }

    if (op.IsEmpty())
    {
        return 0;
    }

    op.AppendToBuffer(buff);
    return tvushm::BufferCtrlGetBufLen(&buff);
}

static int estimateKeyValue(const libshmmedia_extend_data_info_t* p)
{
    if (!p)
    {
        return 0;
    }
    tvushm::KeyValParam op;
    if (p->bHasColorPrimariesVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorPrimariesVal, p->uColorPrimariesVal_);
    }

    if (p->bHasColorTransferCharacteristicVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorTransferCharacteristicVal, p->uColorTransferCharacteristicVal_);
    }

    if (p->bHasColorSpaceVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeColorSpaceVal, p->uColorSpaceVal_);
    }

    if (p->bHasVideoFullRangeFlagVal_)
    {
        op.SetParamAsU32(kLibShmMediaMetaKeyValueTypeVideoFullRangeFlagVal, p->uVideoFullRangeFlagVal_);
    }

    if (p->bGotTvutimestamp)
    {
        op.SetParamAsU64(kLibShmMediaMetaKeyValueTypeTvutimestampVal, p->u64Tvutimestamp);
    }

    if (op.IsEmpty())
    {
        return 0;
    }

    return op.GetCompactBytesNumNeeded();
}

static int parseKeyValue(libshmmedia_extend_data_info_t* pExtendData, const uint8_t *pBuf, uint32_t nBuf)
{
    if (!nBuf || !pBuf)
    {
        return 0;
    }

    int ret = 0;
    tvushm::KeyValParam params;
    tvushm::BufferController_t buffer;
    BufferCtrlAttachExternalReadBuffer(&buffer, pBuf, nBuf);

    ret = params.ExtractFromBuffer(buffer, true);
    if (ret <= 0)
    {
        return ret;
    }

    uint32_t key = kLibShmMediaMetaKeyValueTypeColorPrimariesVal;
    if (params.HasParameter(key))
    {
        pExtendData->bHasColorPrimariesVal_ = true;
        pExtendData->uColorPrimariesVal_ = params.GetParameter(key).GetAsUint32();
    }

    key = kLibShmMediaMetaKeyValueTypeColorTransferCharacteristicVal;
    if (params.HasParameter(key))
    {
        pExtendData->bHasColorTransferCharacteristicVal_ = true;
        pExtendData->uColorTransferCharacteristicVal_ = params.GetParameter(key).GetAsUint32();
    }

    key = kLibShmMediaMetaKeyValueTypeColorSpaceVal;
    if (params.HasParameter(key))
    {
        pExtendData->bHasColorSpaceVal_ = true;
        pExtendData->uColorSpaceVal_ = params.GetParameter(key).GetAsUint32();
    }

    key = kLibShmMediaMetaKeyValueTypeVideoFullRangeFlagVal;
    if (params.HasParameter(key))
    {
        pExtendData->bHasVideoFullRangeFlagVal_ = true;
        pExtendData->uVideoFullRangeFlagVal_ = params.GetParameter(key).GetAsUint32();
    }

    key = kLibShmMediaMetaKeyValueTypeTvutimestampVal;
    if (params.HasParameter(key))
    {
        pExtendData->bGotTvutimestamp = true;
        pExtendData->u64Tvutimestamp = params.GetParameter(key).GetAsUint64();
    }

    return ret;
}

static void libShmParserV2UserDataEntry(libshmmedia_extend_data_info_t* pExtendData, libshmmedia_extended_entry_data_t* v2Entry)
{
    ELibShmMediaExtendDataTypeV2 veType = (ELibShmMediaExtendDataTypeV2)v2Entry->u_type;
    switch (veType)
    {
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID:
        {
            pExtendData->p_uuid_data = v2Entry->p_data;
            pExtendData->i_uuid_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP:
        {
            pExtendData->p_cc608_cdp_data = v2Entry->p_data;
            pExtendData->i_cc608_cdp_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CAPTION_TEXT:
        {
            pExtendData->p_caption_text = v2Entry->p_data;
            pExtendData->i_caption_text_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PRODUCER_STREAM_INFO:
        {
            pExtendData->p_producer_stream_info = v2Entry->p_data;
            pExtendData->i_producer_stream_info_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_RECEIVER_INFO:
        {
            pExtendData->p_receiver_info = v2Entry->p_data;
            pExtendData->i_receiver_info_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE104_DATA:
        {
            pExtendData->p_scte104_data = v2Entry->p_data;
            pExtendData->i_scte104_data_len = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE35_DATA:
        {
            pExtendData->p_scte35_data = v2Entry->p_data;
            pExtendData->i_scte35_data_len = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX:
        {
            pExtendData->p_timecode_index = v2Entry->p_data;
            pExtendData->i_timecode_index_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_STARTIMECODE:
        {
            pExtendData->p_start_timecode = v2Entry->p_data;
            pExtendData->i_start_timecode_length = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_HDR_METADATA:
        {
            pExtendData->p_hdr_metadata = v2Entry->p_data;
            pExtendData->i_hdr_metadata = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE_WITH_FPS_INDEX:
        {
            pExtendData->p_timecode_fps_index = v2Entry->p_data;
            pExtendData->i_timecode_fps_index = v2Entry->u_len;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PIC_STRUCT:
        {
            pExtendData->i_pic_struct = v2Entry->u_len;
            pExtendData->p_pic_struct = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMESTAMP:
        {
            pExtendData->i_source_timestamp = v2Entry->u_len;
            pExtendData->p_source_timestamp = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE:
        {
            pExtendData->i_timecode = v2Entry->u_len;
            pExtendData->p_timecode = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_METADATA_PTS:
        {
            pExtendData->i_metaDataPts = v2Entry->u_len;
            pExtendData->p_metaDataPts = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMEBASE:
        {
            pExtendData->i_source_timebase = v2Entry->u_len;
            pExtendData->p_source_timebase = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE_AFD_METADATA:
        {
            pExtendData->i_smpte_afd_data = v2Entry->u_len;
            pExtendData->p_smpte_afd_data = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_ACTION_TIMESTAMP:
        {
            pExtendData->i_source_action_timestamp = v2Entry->u_len;
            pExtendData->p_source_action_timestamp = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_VANC_SMPTE2038:
        {
            pExtendData->i_vanc_smpte2038 = v2Entry->u_len;
            pExtendData->p_vanc_smpte2038 = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_GOP_POC_V1:
        {
            pExtendData->i_gop_poc = v2Entry->u_len;
            pExtendData->p_gop_poc = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE336M:
        {
            pExtendData->i_smpte336m = v2Entry->u_len;
            pExtendData->p_smpte336m = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVB_TELETEXT:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVB_SUBTITLE:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_DVD_SUBTITLE:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_WEBVTT:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SRT:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBRIP:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_RAW_TEXT:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_TTML:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_VPLAYER:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_EIA608:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SSA:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_ASS:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_STL:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBVIEW:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUB_SUBVIEWV1:
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PROTOCOL_EXTENTION_RESERVER:
        {
            pExtendData->u_subtitle_type = v2Entry->u_type;
            pExtendData->i_subtitle = v2Entry->u_len;
            pExtendData->p_subtitle = v2Entry->p_data;
            break;
        }
        case LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_KEY_TYPE_VALUE_PROTO:
        {
            if (pExtendData && v2Entry->p_data && v2Entry->u_len)
            {
                parseKeyValue(pExtendData, v2Entry->p_data, v2Entry->u_len);
            }
            break;
        }
        default:
        {

        }
        break;
    }
}

/**
 *  Description:
 *      used to set using external buffer while parsing entry data
 *  Return:
 *      void
**/
static void LibshmMediaExtDataSetUsingExternalBuf(libshmmedia_extended_data_context_t h, bool bUsingExtBuf)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;

    if (p)
    {
        p->setUsingExternalBuf(bUsingExtBuf);
    }

    return;
}

static int _readExtDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t *pShmUserData, int dataSize, libshmmedia_extended_data_context_t v2DataCtx)
{
    int  entryCount = LibshmMediaExtDataParseBuff(v2DataCtx, pShmUserData, dataSize);
    if (entryCount < 0)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataParseBuff failed,ret=%d\n", entryCount);
        return -1;
    }
    int ret = 0;
    libshmmedia_extended_entry_data_t newEntry;
    {
        memset(&newEntry, 0, sizeof(newEntry));
    }
    for (int i = 0; i < entryCount; ++i)
    {
        ret = LibshmMediaExtDataGetOneEntry(v2DataCtx,i, &newEntry);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataGetOneEntry failed\n");
            return -1;
        }
        libShmParserV2UserDataEntry(pExtendData, &newEntry);
    }
    return 0;
}

int LibShmMeidaParseExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType)
{
    if (userDataType != LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libShmReadExtendData failed, user data type %d invalid\n", userDataType);
        return -1;
    }

    if (!pShmUserData || dataSize <= 0)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libShmReadExtendDataV2 failed, user data is null or invalid data size %d\n", dataSize);
        return -1;
    }

    libshmmedia_extended_data_context_t v2DataCtx = LibshmMediaExtDataCreateHandle();

    if (!v2DataCtx)
    {
        return -1;
    }

    LibshmMediaExtDataSetUsingExternalBuf(v2DataCtx, true);

    libshmmedia_extend_data_info_t oExt;
    {
        memset(&oExt, 0, sizeof(libshmmedia_extend_data_info_t));
    }

    int ret = _readExtDataV2(&oExt, pShmUserData, dataSize, v2DataCtx);

    if (ret < 0)
    {
        goto  EXIT;
    }

    *pExtendData = oExt;
    ret = 0;

EXIT:
    LibshmMediaExtDataDestroyHandle(&v2DataCtx);
    return ret;
}

int LibShmMeidaParseExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize)
{
    return LibShmMeidaParseExtendData(pExtendData, pShmUserData, dataSize, LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2);
}

int LibShmMediaReadExtendData(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx)
{
    return libShmReadExtendDataV2(pExtendData, pShmUserData, dataSize, userDataType, v2DataCtx);
}

int libShmReadExtendDataV2(libshmmedia_extend_data_info_t* pExtendData, const uint8_t* pShmUserData, int dataSize, int userDataType, libshmmedia_extended_data_context_t v2DataCtx)
{
    if (userDataType == LIBSHM_MEDIA_TYPE_TVU_UID)
    {
        ::memset(pExtendData, 0, sizeof(libshmmedia_extend_data_info_t));
        pExtendData->p_uuid_data = pShmUserData;
        pExtendData->i_uuid_length = dataSize;
        return 0;
    }
    else if (userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA)
    {
        _readExtendDataV1(pExtendData, pShmUserData, dataSize);
        return 0;
    }
    else if (userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2)
    {
        if (!pShmUserData || dataSize <= 0 || !v2DataCtx)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("libShmReadExtendDataV2 failed, parameter invalid\n");
            return -1;
        }

        libshmmedia_extend_data_info_t oExt;
        {
            memset(&oExt, 0, sizeof(libshmmedia_extend_data_info_t));
        }

        int ret = _readExtDataV2(&oExt, pShmUserData, dataSize, v2DataCtx);

        if (ret < 0)
        {
            return ret;
        }

        *pExtendData = oExt;
        return 0;
    }
    return -1;
}

int LibShmMediaEstimateExtendDataSize(/*IN*/const libshmmedia_extend_data_info_t* pExtendData)
{
    int ret = 0;

#if 0
    const uint8_t *p = (const uint8_t *)pExtendData;
    size_t struct_szie = sizeof(libshmmedia_extend_data_info_t);
    size_t pair_size = sizeof(const uint8_t*) + sizeof(int);
    size_t nPairs = struct_szie/pair_size;

    for (size_t i = 0; i < nPairs; i++)
    {
        int entry_len = *(int *)(p + sizeof(const uint8_t*));
        if (entry_len > 0)
        {
            ret += 4 + 4 + entry_len;
        }
        p  += pair_size;
    }

#else
    int tmp_len = 0;
    if (pExtendData->i_uuid_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_uuid_length;
    }
    if (pExtendData->i_cc608_cdp_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_cc608_cdp_length;
    }
    if (pExtendData->i_caption_text_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_caption_text_length;
    }
    if (pExtendData->i_producer_stream_info_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_producer_stream_info_length;
    }
    if (pExtendData->i_receiver_info_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_receiver_info_length;
    }
    if (pExtendData->i_scte104_data_len > 0)
    {
        ret += 4 + 4 + pExtendData->i_scte104_data_len;
    }
    if (pExtendData->i_scte35_data_len > 0)
    {
        ret += 4 + 4 + pExtendData->i_scte35_data_len;
    }
    if (pExtendData->i_timecode_index_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_timecode_index_length;
    }
    if (pExtendData->i_start_timecode_length > 0)
    {
        ret += 4 + 4 + pExtendData->i_start_timecode_length;
    }
    if (pExtendData->i_hdr_metadata > 0)
    {
        ret += 4 + 4 + pExtendData->i_hdr_metadata;
    }
    if (pExtendData->i_timecode_fps_index > 0)
    {
        ret += 4 + 4 + pExtendData->i_timecode_fps_index;
    }
    if (pExtendData->i_pic_struct > 0)
    {
        ret += 4 + 4 + pExtendData->i_pic_struct;
    }
    if (pExtendData->i_source_timestamp > 0)
    {
        ret += 4 + 4 + pExtendData->i_source_timestamp;
    }
    if (pExtendData->i_timecode > 0)
    {
        ret += 4 + 4 + pExtendData->i_timecode;
    }

    if (pExtendData->i_metaDataPts > 0)
    {
        ret += 4 + 4 + pExtendData->i_metaDataPts;
    }

    tmp_len = pExtendData->i_source_timebase;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = pExtendData->i_smpte_afd_data;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = pExtendData->i_source_action_timestamp;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = pExtendData->i_vanc_smpte2038;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = pExtendData->i_gop_poc;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = pExtendData->i_subtitle;
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }

    tmp_len = estimateKeyValue(pExtendData);
    if (tmp_len > 0)
    {
        ret += 4 + 4 + tmp_len;
    }
#endif

    if (ret > 0)
        ret += 16;/* 4 version + 4 tag + 4 total len + 4 count*/
    return ret;
}

int LibShmMediaWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData)
{
    return libShmWriteExtendData(dataBuffer, bufferSize, pExtendData);
}

int libShmWriteExtendData(/*OUT*/uint8_t dataBuffer[], /*IN*/int bufferSize, /*IN*/const libshmmedia_extend_data_info_t* pExtendData)
{
    libshmmedia_extended_data_context_t v2WriteCtx = LibshmMediaExtDataCreateHandle();
    if (!v2WriteCtx)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataCreateHandle failed\n");
        return -1;
    }

    int ret = 0;
    LibshmMediaExtDataResetEntry(v2WriteCtx);
    libshmmedia_extended_entry_data_t newEntry;
    {
        memset((void *)&newEntry, 0, sizeof(newEntry));
    }

    if (pExtendData->i_uuid_length > 0)
    {
        newEntry.p_data = pExtendData->p_uuid_data;
        newEntry.u_len = pExtendData->i_uuid_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_cc608_cdp_length > 0)
    {
        newEntry.p_data = pExtendData->p_cc608_cdp_data;
        newEntry.u_len = pExtendData->i_cc608_cdp_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_caption_text_length > 0)
    {
        newEntry.p_data = pExtendData->p_caption_text;
        newEntry.u_len = pExtendData->i_caption_text_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CAPTION_TEXT;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_producer_stream_info_length > 0)
    {
        newEntry.p_data = pExtendData->p_producer_stream_info;
        newEntry.u_len = pExtendData->i_producer_stream_info_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PRODUCER_STREAM_INFO;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_receiver_info_length > 0)
    {
        newEntry.p_data = pExtendData->p_receiver_info;
        newEntry.u_len = pExtendData->i_receiver_info_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_RECEIVER_INFO;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n",newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_scte104_data_len > 0)
    {
        newEntry.p_data = pExtendData->p_scte104_data;
        newEntry.u_len = pExtendData->i_scte104_data_len;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE104_DATA;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_scte35_data_len > 0)
    {
        newEntry.p_data = pExtendData->p_scte35_data;
        newEntry.u_len = pExtendData->i_scte35_data_len;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SCTE35_DATA;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_timecode_index_length > 0)
    {
        newEntry.p_data = pExtendData->p_timecode_index;
        newEntry.u_len = pExtendData->i_timecode_index_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODEINDEX;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_start_timecode_length > 0)
    {
        newEntry.p_data = pExtendData->p_start_timecode;
        newEntry.u_len = pExtendData->i_start_timecode_length;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_STARTIMECODE;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_hdr_metadata > 0)
    {
        newEntry.p_data = pExtendData->p_hdr_metadata;
        newEntry.u_len = pExtendData->i_hdr_metadata;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_HDR_METADATA;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_timecode_fps_index > 0)
    {
        newEntry.p_data = pExtendData->p_timecode_fps_index;
        newEntry.u_len = pExtendData->i_timecode_fps_index;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE_WITH_FPS_INDEX;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_pic_struct > 0)
    {
        newEntry.p_data = pExtendData->p_pic_struct;
        newEntry.u_len = pExtendData->i_pic_struct;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_PIC_STRUCT;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_source_timestamp > 0)
    {
        newEntry.p_data = pExtendData->p_source_timestamp;
        newEntry.u_len = pExtendData->i_source_timestamp;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMESTAMP;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }
    if (pExtendData->i_timecode > 0)
    {
        newEntry.p_data = pExtendData->p_timecode;
        newEntry.u_len = pExtendData->i_timecode;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_TIMECODE;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_metaDataPts > 0)
    {
        newEntry.p_data = pExtendData->p_metaDataPts;
        newEntry.u_len = pExtendData->i_metaDataPts;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_METADATA_PTS;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_source_timebase > 0)
    {
        newEntry.p_data = pExtendData->p_source_timebase;
        newEntry.u_len = pExtendData->i_source_timebase;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_TIMEBASE;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_smpte_afd_data > 0)
    {
        newEntry.p_data = pExtendData->p_smpte_afd_data;
        newEntry.u_len = pExtendData->i_smpte_afd_data;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE_AFD_METADATA;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_source_action_timestamp)
    {
        newEntry.p_data = pExtendData->p_source_action_timestamp;
        newEntry.u_len = pExtendData->i_source_action_timestamp;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SOURCE_ACTION_TIMESTAMP;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_vanc_smpte2038)
    {
        newEntry.p_data = pExtendData->p_vanc_smpte2038;
        newEntry.u_len = pExtendData->i_vanc_smpte2038;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_VANC_SMPTE2038;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_gop_poc)
    {
        newEntry.p_data = pExtendData->p_gop_poc;
        newEntry.u_len = pExtendData->i_gop_poc;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_GOP_POC_V1;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_smpte336m)
    {
        newEntry.p_data = pExtendData->p_smpte336m;
        newEntry.u_len = pExtendData->i_smpte336m;
        newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SMPTE336M;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    if (pExtendData->i_subtitle)
    {
        newEntry.p_data = pExtendData->p_subtitle;
        newEntry.u_len = pExtendData->i_subtitle;
        newEntry.u_type = pExtendData->u_subtitle_type;
        ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
        if (ret < 0)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
            ret = -1;
            goto EXIT;
        }
    }

    {
        tvushm::BufferController_t buff;
        int n = createKeyValue(buff, pExtendData);
        if (n > 0)
        {
            newEntry.p_data = tvushm::BufferCtrlGetOrigPtr(&buff);
            newEntry.u_len = n;
            newEntry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_KEY_TYPE_VALUE_PROTO;
            ret = LibshmMediaExtDataAddOneEntry(v2WriteCtx, &newEntry, dataBuffer, bufferSize);
            if (ret < 0)
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("LibshmMediaExtDataAddOneEntry failed,type = %d\n", newEntry.u_type);
                ret = -1;
                goto EXIT;
            }
        }
    }

    ret = LibshmMediaExtDataGetEntryBuffSize(v2WriteCtx);

EXIT:
    LibshmMediaExtDataDestroyHandle(&v2WriteCtx);
    return ret;
}

#if 0
int libShmWriteExtendDataV1(uint8_t dateBuffer[], int bufferSize,const Lib_shm_extend_data_t* pExtendData)
{
    uint32_t dataFlag = 0;
    uint8_t* pDest = dateBuffer;
    int offset = 0;
    offset += 4;

    if(pExtendData->i_uuid_length > 0)
    {
        if (offset +  pExtendData->i_uuid_length + 4 > bufferSize)
        {
            DEBUG_WARN("uuid extend data buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEND_DATA_TYPE_UID;
            memcpy(pDest + offset, &pExtendData->i_uuid_length, 4);
            offset += 4;
            memcpy(pDest + offset, pExtendData->p_uuid_data, pExtendData->i_uuid_length);
            offset += pExtendData->i_uuid_length;
        }
    }

    if (pExtendData->i_cc608_cdp_length > 0)
    {
        if (offset +  pExtendData->i_cc608_cdp_length + 4 > bufferSize)
        {
            DEBUG_WARN("cc608 cdp extend data buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEND_DATA_TYPE_CC608_CDP;
            memcpy(pDest + offset, &pExtendData->i_cc608_cdp_length, 4);
            offset += 4;
            memcpy(pDest + offset, pExtendData->p_cc608_cdp_data, pExtendData->i_cc608_cdp_length);
            offset += pExtendData->i_cc608_cdp_length;
        }
    }

    if (pExtendData->i_caption_text_length > 0)
    {
        if (offset +  pExtendData->i_caption_text_length + 4 > bufferSize)
        {
            DEBUG_WARN("capture text data buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEND_DATA_TYPE_CAPTION_TEXT;
            memcpy(pDest + offset, &pExtendData->i_caption_text_length, 4);
            offset += 4;
            memcpy(pDest + offset, pExtendData->p_caption_text, pExtendData->i_caption_text_length);
            offset += pExtendData->i_caption_text_length;
        }
    }

    if (pExtendData->i_producer_stream_info_length > 0)
    {
        if (offset +  pExtendData->i_producer_stream_info_length + 4 > bufferSize)
        {
            DEBUG_WARN("producer stream info buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEBD_DATA_TYPE_PRODUCER_STREAM_INFO;
            memcpy(pDest + offset, &pExtendData->i_producer_stream_info_length, 4);
            offset += 4;
            memcpy(pDest + offset, pExtendData->p_producer_stream_info, pExtendData->i_producer_stream_info_length);
            offset += pExtendData->i_producer_stream_info_length;
        }
    }


    if (pExtendData->i_receiver_info_length > 0)
    {
        if (offset +  pExtendData->i_receiver_info_length + 4 > bufferSize)
        {
            DEBUG_WARN("receiver info buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEND_DATA_TYPE_RECEIVER_INFO;
            memcpy(pDest + offset, &pExtendData->i_receiver_info_length, 4);
            offset += 4;
            memcpy(pDest + offset, pExtendData->p_receiver_info, pExtendData->i_receiver_info_length);
            offset += pExtendData->i_receiver_info_length;
        }
    }



    if (pExtendData->i_scte104_data_len > 0)
    {
        if (offset +  pExtendData->i_scte104_data_len + 4 > bufferSize)
        {
            DEBUG_WARN("scted 104 data buffer, memory overflow\n");
            goto EXIT;
        }
        else
        {
            dataFlag |= LIBSHM_EXTEND_DATA_TYPE_SCTE104_DATA;
            memcpy(pDest + offset,&pExtendData->i_scte104_data_len,4);
            offset += 4;
            memcpy(pDest + offset,pExtendData->p_scte104_data,pExtendData->i_scte104_data_len);
            offset += pExtendData->i_scte104_data_len;
        }
    }

EXIT:
    memcpy(pDest, &dataFlag, 4);
    return offset;
}
#endif

CLibShmMediaExtendedDataV2::CLibShmMediaExtendedDataV2()
{
    m_uVersion  = 0x01;
    m_uAllocEntryCounts = 0;
    m_uEntryCounts = 0;
    m_pBuffer = NULL;
    m_uAllocBufSize = 0;
    m_uBufSize = 0;
    m_uBufOffsetW = 0;
    m_uBufOffsetR = 0;
    m_bEntryNotDone = false;
    m_pEntryOffsetArr = NULL;
    _bUsingExternalDataBuf = false;
}

CLibShmMediaExtendedDataV2::~CLibShmMediaExtendedDataV2()
{
    m_uVersion  = LIBSHMMEDIA_EXTENTED_DATA_STRUCT_VERSION_1;
    if (m_pBuffer)
    {
        free((void *)m_pBuffer);
        m_pBuffer = NULL;
    }
    m_uAllocBufSize = 0;
    m_uBufSize = 0;
    m_uBufOffsetW = 0;
    m_uBufOffsetR = 0;

    m_uEntryCounts = 0;
    m_uAllocEntryCounts = 0;
    if (m_pEntryOffsetArr)
    {
        free((void *)m_pEntryOffsetArr);
        m_pEntryOffsetArr = NULL;
    }
}

int CLibShmMediaExtendedDataV2::allocEntryArr()
{
    int ret = 0;
    if (m_uAllocEntryCounts < m_uEntryCounts + 1)
    {
        int alloc_size = sizeof(libshmmedia_extended_entry_data_t) * (m_uAllocEntryCounts+32);
        m_pEntryOffsetArr = (libshmmedia_extended_entry_data_t *)realloc((void *)m_pEntryOffsetArr, alloc_size);
        if (!m_pEntryOffsetArr)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("m_pEntryOffsetArr alloc size %d failed\n", alloc_size);
            ret = -ENOMEM;
        }
        m_uAllocEntryCounts += 32;
    }

    return ret;
}

int CLibShmMediaExtendedDataV2::allocBuf(unsigned int needsize)
{
    int ret = 0;
    if (needsize > m_uAllocBufSize)
    {
        int alloc_size = needsize + 1024;
        m_pBuffer   = (uint8_t *)realloc((void*)m_pBuffer, alloc_size);

        if (!m_pBuffer)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("m_pBuffer alloc size %d failed\n", alloc_size);
            ret = -5;
        }

        m_uAllocBufSize = alloc_size;
    }

    return ret;
}

int CLibShmMediaExtendedDataV2::resetEntry()
{
    int ret = 0;
    m_uEntryCounts = 0;
    m_uBufOffsetW = 0;
    m_uBufOffsetR = 0;
    m_bEntryNotDone = true;
    return ret;
}

unsigned int CLibShmMediaExtendedDataV2::getBufWriteSize()
{
    return m_uBufOffsetW;
}

int CLibShmMediaExtendedDataV2::addEntryToBuff(
    const uint8_t *pEntryData, uint32_t iEntryType, uint32_t iEntryLen
    , uint8_t *pbuf, int buf_size)
{
    /* alloc entry */
    int ret  = 0;
    int entry_head_size = 0;
    int ext_head_size = 0;
    int version_offset = 0;
    int tag_offset = 0;
    int total_length_offset = 0;
    int counts_offset = 0;
    uint32_t entry_begin_offset = 0;

    switch(m_uVersion)
    {
        case LIBSHMMEDIA_EXTENTED_DATA_STRUCT_VERSION_1:
        {
            entry_head_size = 8;
            ext_head_size = 16;
            version_offset = 0;
            tag_offset = 4;
            total_length_offset = 8;
            counts_offset = 12;
        }
        break;
        default:
            break;
    }

    if (!m_uEntryCounts || !m_uBufOffsetW)
    {
        m_uBufOffsetW = ext_head_size;
    }

    entry_begin_offset = m_uBufOffsetW;

    if (pbuf
        && (m_uBufOffsetW + entry_head_size + iEntryLen <= (unsigned int)buf_size)
    )
    {
        _writeLe32(pbuf+version_offset, m_uVersion);
        _writeLe32(pbuf+tag_offset, LIBSHMMEDIA_EXTENTED_DATA_HEAD_TAG);
        _writeLe32(pbuf+m_uBufOffsetW, iEntryType);
        m_uBufOffsetW += 4;
        _writeLe32(pbuf+m_uBufOffsetW, iEntryLen);
        m_uBufOffsetW += 4;
        memcpy(pbuf+m_uBufOffsetW, pEntryData, iEntryLen);
        m_uBufOffsetW += iEntryLen;
        m_uEntryCounts++;
        _writeLe32(pbuf+counts_offset, m_uEntryCounts);
        _writeLe32(pbuf+total_length_offset, m_uBufOffsetW);
    }

    ret = m_uBufOffsetW - entry_begin_offset;
    return ret;
}

int CLibShmMediaExtendedDataV2::_parseBuff(const uint8_t *pbuf, uint32_t ibuflen)
{
    int ret = -1;
    uint32_t version = 0;
    uint32_t ureadOffset = 0;

    m_uEntryCounts = 0;

    version = _readLe32(pbuf);
    ureadOffset += 4;

    switch(version)
    {
        case LIBSHMMEDIA_EXTENTED_DATA_STRUCT_VERSION_1:
        {
            uint32_t total_length = 0;
            int     counts = 0;
            uint32_t shm_tag = 0;
            int     ic = 0;

            shm_tag = _readLe32(pbuf + ureadOffset);
            ureadOffset += 4;

            if (shm_tag != LIBSHMMEDIA_EXTENTED_DATA_HEAD_TAG)
            {
                ret = -EINVAL;
                DEBUG_SHMMEDIA_PROTO_ERROR("data is invalide, shm tag is 0x%08x invalid.\n", shm_tag);
                return ret;
            }

            total_length = _readLe32(pbuf + ureadOffset);
            ureadOffset += 4;
            if (total_length != ibuflen)
            {
                ret = -EINVAL;
                DEBUG_SHMMEDIA_PROTO_ERROR("data is invalide, total legth %u not equal parameter %u\n", total_length, ibuflen);
                return ret;
            }

            counts = _readLe32(pbuf + ureadOffset);
            ureadOffset += 4;
            for(ic = 0; ic < counts; ic++)
            {
                unsigned int uentryLen = 0;
                if ((ret = allocEntryArr()) != 0)
                {
                    goto EXIT;
                }
                m_pEntryOffsetArr[m_uEntryCounts].u_type = _readLe32(pbuf + ureadOffset);
                ureadOffset += 4;
                uentryLen                               =
                m_pEntryOffsetArr[m_uEntryCounts].u_len = _readLe32(pbuf + ureadOffset);
                ureadOffset += 4;
                m_pEntryOffsetArr[m_uEntryCounts].p_data = pbuf + ureadOffset;
                ureadOffset += uentryLen;
                m_uEntryCounts++;
            }
        }
        break;
        default:
        {
            goto EXIT;
        }
        break;
    }

    m_bEntryNotDone = false;
    ret = m_uEntryCounts;
EXIT:
    return ret;
}

int CLibShmMediaExtendedDataV2::parseBuff(const uint8_t *porigbuf, uint32_t ibuflen)
{
    int ret = -1;
    const uint8_t *pbuf = NULL;

    if (!ibuflen || !porigbuf)
    {
        return 0;
    }

    if (_bUsingExternalDataBuf)
    {
        return _parseBuff(porigbuf, ibuflen);
    }

    if (allocBuf(ibuflen) != 0)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("allocBuff %d failed\n", ibuflen);
        ret = -ENOMEM;
        return ret;
    }

    memcpy(m_pBuffer, porigbuf, ibuflen);
    m_uBufSize = ibuflen;
    pbuf = m_pBuffer;

    return _parseBuff(pbuf, ibuflen);
}

unsigned int CLibShmMediaExtendedDataV2::getEntryCouts()
{
    unsigned int ret = 0;
    if (!m_bEntryNotDone)
    {
        ret = m_uEntryCounts;
    }
    return ret;
}

int CLibShmMediaExtendedDataV2::getOneEntryLen(int entryIndex)
{
    int entry_len = 0;
    if ((unsigned int)entryIndex < m_uEntryCounts)
    {
        entry_len = m_pEntryOffsetArr[entryIndex].u_len;
    }

    return entry_len;
}

unsigned int CLibShmMediaExtendedDataV2::getOneEntryType(int entryIndex)
{
    unsigned int entry_type = 0;
    if ((unsigned int)entryIndex < m_uEntryCounts)
    {
        entry_type = m_pEntryOffsetArr[entryIndex].u_type;
    }

    return entry_type;
}

int CLibShmMediaExtendedDataV2::getOneEntryData(int entryIndex, uint8_t *pbuf, int ibufsize)
{
    int ret = 0;
    if ((unsigned int)entryIndex < m_uEntryCounts)
    {
        int entry_len = m_pEntryOffsetArr[entryIndex].u_len;

        if (entry_len <= ibufsize)
        {
            memcpy(pbuf, m_pEntryOffsetArr[entryIndex].p_data, entry_len);
            ret = entry_len;
        }
    }

    return ret;
}

int CLibShmMediaExtendedDataV2::getOneEntry(int entryIndex, libshmmedia_extended_entry_data_t *pEntryData)
{
    int ret = 0;
    if ((unsigned int)entryIndex < m_uEntryCounts)
    {
        pEntryData->u_len = m_pEntryOffsetArr[entryIndex].u_len;
        pEntryData->u_type = m_pEntryOffsetArr[entryIndex].u_type;
        pEntryData->p_data = m_pEntryOffsetArr[entryIndex].p_data;
    }
    else
    {
        pEntryData->u_len = 0;
        pEntryData->u_type = 0;
        pEntryData->p_data = NULL;
        ret = -1;
    }

    return ret;
}

libshmmedia_extended_data_context_t
LibshmMediaExtDataCreateHandle()
{
    CLibShmMediaExtendedDataV2 *p = new CLibShmMediaExtendedDataV2();

    return (libshmmedia_extended_data_context_t)p;
}

unsigned int LibshmMediaExtDataGetEntryBuffSize(libshmmedia_extended_data_context_t h)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;
    unsigned int ret = 0;

    if (p)
    {
        ret = p->getBufWriteSize();
    }

    return ret;
}

int LibshmMediaExtDataAddOneEntry(libshmmedia_extended_data_context_t h
    , libshmmedia_extended_entry_data_t *pEntryData
    , uint8_t *pDstBuf, uint32_t iDstBufSize
)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;
    int ret = 0;

    if (p)
    {
        ret = p->addEntryToBuff(pEntryData->p_data, pEntryData->u_type, pEntryData->u_len, pDstBuf, iDstBufSize);
    }

    return ret;
}

void LibshmMediaExtDataResetEntry(libshmmedia_extended_data_context_t h)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;

    if (p)
    {
        p->resetEntry();
    }

    return;
}

int LibshmMediaExtDataParseBuff(libshmmedia_extended_data_context_t h, const uint8_t *pbuf, uint32_t ibuflen)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;
    int ret = 0;

    if (p)
    {
        ret = p->parseBuff(pbuf, ibuflen);
    }

    return ret;
}

unsigned int LibshmMediaExtDataGetEntryCounts(libshmmedia_extended_data_context_t h)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;
    int ret = 0;

    if (p)
    {
        ret = p->getEntryCouts();
    }

    return ret;
}

int LibshmMediaExtDataGetOneEntry(libshmmedia_extended_data_context_t h
    , int entryIndex
    , libshmmedia_extended_entry_data_t *pEntry)
{
    CLibShmMediaExtendedDataV2 *p = (CLibShmMediaExtendedDataV2 *)h;
    int ret = -1;

    if (p)
    {
        ret = p->getOneEntry(entryIndex, pEntry);
    }

    return ret;
}

void LibshmMediaExtDataDestroyHandle(libshmmedia_extended_data_context_t *ph)
{
    CLibShmMediaExtendedDataV2 *p = NULL;

    if (ph)
    {
        p = (CLibShmMediaExtendedDataV2 *)*ph;
        if (p)
        {
            delete p;
        }

        *ph = NULL;
    }

    return;
}
