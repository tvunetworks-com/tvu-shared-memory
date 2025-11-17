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
#include "libshm_media_subtitle_private_protocol_internal.h"
#include "libshm_media_extension_protocol_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include "buffer_ctrl.h"
#include "libshm_util_endian.h"
#include <string.h>
#include <errno.h>
#include <assert.h>

bool LibshmmediaSubtitlePrivateProtoIsCorrectType(uint32_t type)
{
    return (type == LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PRIVATE_PROTOCOL_EXTENTION_STRUCTURE);
}

int LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(const libshmmedia_subtitle_private_proto_entries_t *in)
{
    libshmmedia_subtitle_protocol_t odata = {0};
    int counts = in->head.counts;
    int ret = sizeof(odata.protoVer_)
            +sizeof(odata.counts_)
            +sizeof(odata.reserver2_)
            +sizeof(odata.dataInfoOffset_)
            + counts * sizeof(libshmmedia_subtitle_data_info_t)
            ;

    for (int i = 0; i < counts; i++)
    {
        const libshmmedia_subtitle_private_proto_entry_item_t *pi = &in->entries[i];
        ret += pi->dataLen;
    }

    return ret;
}

int LibshmmediaSubtitlePrivateProtoWriteBufferSize(const libshmmedia_subtitle_private_proto_entries_t *in, uint8_t *buffer, int bufferSize)
{
    int ret = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(in);

    if (bufferSize < ret)
    {
        return -ENOMEM;
    }

    tvushm::BufferController_t octrl;
    tvushm::BufferCtrlInit(&octrl);

    int bufPos = 0;
    {
        /* write data */
        libshmmedia_subtitle_protocol_t odata = {0};
        int counts = in->head.counts;
        int dataInfoOffset = sizeof(odata.protoVer_)
                +sizeof(odata.counts_)
                +sizeof(odata.reserver2_)
                +sizeof(odata.dataInfoOffset_)
                ;
        const int protoVers = kLibshmmediaSubtitlePrivateProtoVer1;
        FAILED_PUSH_DATA(tvushm::BufferCtrlW8(&octrl, protoVers));
        FAILED_PUSH_DATA(tvushm::BufferCtrlW8(&octrl, counts));
        FAILED_PUSH_DATA(tvushm::BufferCtrlWl16(&octrl, 0)); /*reserve*/
        FAILED_PUSH_DATA(tvushm::BufferCtrlWl16(&octrl, dataInfoOffset));

        const int firstdataOffset = dataInfoOffset + counts * sizeof(libshmmedia_subtitle_data_info_t);
        int dataOffset = firstdataOffset;
        for (int i = 0; i < counts; i++)
        {
            const libshmmedia_subtitle_private_proto_entry_item_t *pi = &in->entries[i];
            int dataLen = pi->dataLen;
            int type = pi->type;
            uint64_t timestamp = pi->timestamp;
            uint32_t duration = pi->duration;
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl16(&octrl, sizeof(libshmmedia_subtitle_data_info_t)));
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl32(&octrl, dataOffset));
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl32(&octrl, dataLen));
            dataOffset += dataLen;
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl32(&octrl, type));
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl64(&octrl, timestamp));
            FAILED_PUSH_DATA(tvushm::BufferCtrlWl32(&octrl, duration));
        }


        bufPos = tvushm::BufferCtrlTellCurPos(&octrl);
        dataOffset = firstdataOffset;
        {
            assert(dataOffset >= bufPos);
            tvushm::BufferCtrlSeek(&octrl, dataOffset, SEEK_SET);
        }

        for (int i = 0; i < counts; i++)
        {
            const libshmmedia_subtitle_private_proto_entry_item_t *pi = &in->entries[i];
            int dataLen = pi->dataLen;
            const uint8_t *data = pi->data;
            FAILED_PUSH_DATA(tvushm::BufferCtrlPushData(&octrl, data, dataLen));
        }
    }

    bufPos = tvushm::BufferCtrlTellCurPos(&octrl);

    if(!((bufPos <= bufferSize) && (bufPos==ret)))
    {
        return -EIO;
    }

    //if (bufPos < bufferSize)
    {
        memcpy(buffer, tvushm::BufferCtrlGetOrigPtr(&octrl), bufPos);
    }


    tvushm::BufferCtrlRelease(&octrl);

    return ret;
}

static int _parseV1(libshmmedia_subtitle_private_proto_internal_entries_t *out, tvushm::BufferController_t *pbuf, const uint8_t *buffer)
{
    int counts = tvushm::BufferCtrlR8(pbuf);
    tvushm::BufferCtrlRl16(pbuf); // reserved
    uint32_t dataInfoOffset = tvushm::BufferCtrlRl16(pbuf);
    int ret = -1;

    if (counts > LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS)
    {
        return -EIO;
    }
    const uint32_t mindataOffset = dataInfoOffset + counts * sizeof(libshmmedia_subtitle_data_info_v1_t);
    uint32_t lastEntryDataOffset = 0;
    uint32_t lastDataSize = 0;

    /* read out data info */
    tvushm::BufferCtrlSeek(pbuf, dataInfoOffset, SEEK_SET);
    for (int i = 0; i < counts; i++)
    {
        libshmmedia_subtitle_private_proto_entry_item_t *pi = &out->p_->entries[i];
        libshmmedia_subtitle_data_internal_ext_infor_t &extInfo = out->extInfo_[i];
        int32_t infoStructSize = tvushm::BufferCtrlRl16(pbuf);
        if (infoStructSize >= (int)sizeof(libshmmedia_subtitle_data_info_v1_t))
        {
            uint32_t dataoffset = tvushm::BufferCtrlRl32(pbuf);
            if (dataoffset < mindataOffset)
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle dataoffset less than mini dataoffset."
                                           "of:%u, min:%u"
                                           "\n"
                                           , dataoffset, mindataOffset
                                           );
                return  -EINVAL;
            }
            else if(dataoffset < lastEntryDataOffset + lastDataSize)
            {
                DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle dataoffset less than last dataoffset."
                                           "of:%u, last:{%u,%u}"
                                           "\n"
                                           , dataoffset, lastEntryDataOffset, lastDataSize
                                           );
                return  -EINVAL;
            }

            uint32_t dataSize = tvushm::BufferCtrlRl32(pbuf);
            uint32_t dataType = tvushm::BufferCtrlRl32(pbuf);
            uint32_t timestampPos = tvushm::BufferCtrlTellCurPos(pbuf);
            uint64_t dataTimestamp = tvushm::BufferCtrlRl64(pbuf);
            uint32_t dataDuration = tvushm::BufferCtrlRl32(pbuf);

            {
                pi->type = dataType;
                pi->timestamp = dataTimestamp;
                pi->duration = dataDuration;
                pi->dataLen = dataSize;
                pi->data = buffer + dataoffset;
            }

            {
                extInfo.timestamPos_ = timestampPos;
                extInfo.dataPos_ = dataoffset;
            }

            lastEntryDataOffset = dataoffset;
            lastDataSize = dataSize;
        }
        else
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle infoStructSize invalid."
                                       "v:%d"
                                       "\n"
                                       , infoStructSize
                                       );
            return -EINVAL;
        }
    }

    out->p_->head.counts = counts;
    ret = 0;
    return ret;
}

static int
_libshmmediaSubtitlePrivateProtoParse(libshmmedia_subtitle_private_proto_internal_entries_t *out, tvushm::BufferController_t &octrl, const uint8_t *origBuf)
{
    int protoVers = 0;
    int ret = -1;

    protoVers = tvushm::BufferCtrlR8(&octrl);

    switch (protoVers) {
    case kLibshmmediaSubtitlePrivateProtoVer1:
    {
        ret = _parseV1(out, &octrl, origBuf);
        break;
    }
    default:
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle unsupported version."
                                   "v:%d"
                                   "\n"
                                   , protoVers
                                   );
        ret = -EPERM;
        break;
    }
    }

    return ret;
}

static int
_libshmmediaSubtitlePrivateProtoTuneTimestampOffset(uint8_t *buff, uint32_t bufferSize, const libshmmedia_subtitle_private_proto_internal_entries_t *pInterEntry, int offsetMs)
{
    const libshmmedia_subtitle_private_proto_entries_t * pentry = pInterEntry->p_;
    int counts = pentry->head.counts;

    for (int i = 0; i < counts; i++)
    {
        //const libshmmedia_subtitle_private_proto_entry_item_t &oi = pentry->entries[i];
        const libshmmedia_subtitle_data_internal_ext_infor_t &extInfo = pInterEntry->extInfo_[i];
        if (extInfo.timestamPos_ + sizeof(uint64_t) > bufferSize)
        {
            DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle timestamp pos invalid."
                                       "pos:%d, size:%d"
                                       "\n"
                                       , extInfo.timestamPos_
                                       , bufferSize
                                       );
            return  -EINVAL;
        }

        {
            int64_t  timestamp = tvushm::_tvuutil_letoh64(*((int64_t*)(buff+extInfo.timestamPos_)));
            timestamp += offsetMs;
            *((int64_t*)(buff+extInfo.timestamPos_)) = tvushm::_tvuutil_htole64(timestamp);
        }
    }

    return 0;
}

int LibshmmediaSubtitlePrivateProtoParseBufferSize(libshmmedia_subtitle_private_proto_entries_t *out, const uint8_t *buffer, int bufferSize)
{
    tvushm::BufferController_t octrl;
    tvushm::BufferCtrlInit(&octrl);
    {
        FAILED_PUSH_DATA(tvushm::BufferCtrlPushData(&octrl, buffer, bufferSize));
        tvushm::BufferCtrlSeek(&octrl,0,SEEK_SET);
    }

    libshmmedia_subtitle_private_proto_internal_entries_t oEntry(out);
    {
        memset(&oEntry.extInfo_, 0, sizeof(oEntry.extInfo_));
    }

    int protoVers = 0;
    int ret = -1;

    protoVers = tvushm::BufferCtrlR8(&octrl);

    switch (protoVers) {
    case kLibshmmediaSubtitlePrivateProtoVer1:
    {
        ret = _parseV1(&oEntry, &octrl, buffer);
        break;
    }
    default:
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle unsupported version 1."
                                   "v:%d"
                                   "\n"
                                   , protoVers
                                   );
        ret = -EPERM;
        break;
    }
    }

    tvushm::BufferCtrlRelease(&octrl);

    return ret;
}

int LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(uint8_t *buffer, int bufferSize, int offsetMs)
{
    libshmmedia_subtitle_private_proto_entries_t out = {};
    {
        memset(&out, 0, sizeof(out));
    }
    libshmmedia_subtitle_private_proto_internal_entries_t oEntry(&out);
    {
        memset(&oEntry.extInfo_, 0, sizeof(oEntry.extInfo_));
    }

    tvushm::BufferController_t octrl;
    tvushm::BufferCtrlInit(&octrl);
    {
        FAILED_PUSH_DATA(tvushm::BufferCtrlPushData(&octrl, buffer, bufferSize));
        tvushm::BufferCtrlSeek(&octrl,0,SEEK_SET);
    }

    int ret = _libshmmediaSubtitlePrivateProtoParse(&oEntry, octrl, buffer);

    if (ret < 0)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle proto parse failed."
                                   "ret:%d"
                                   "\n"
                                   , ret
                                   );
        goto EXT;
    }

    ret = _libshmmediaSubtitlePrivateProtoTuneTimestampOffset(buffer, bufferSize, &oEntry, offsetMs);

    if (ret < 0)
    {
        DEBUG_SHMMEDIA_PROTO_ERROR("libshmmedia_subtitle timestamp change failed."
                                   "ret:%d"
                                   "\n"
                                   , ret
                                   );
        goto EXT;
    }

EXT:

    tvushm::BufferCtrlRelease(&octrl);
    return ret;
}

