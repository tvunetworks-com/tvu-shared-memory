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
/********************************************************************************
 * Description:
 *      used to declare some APIs about shmmedia protocol,
 *      including parsing, constructing
 * Histotry:
 *      Lotus, April 29th 2020, initialized it.
********************************************************************************/

#ifndef LIBSHM_MEDIA_PROTOCOL_H
#define LIBSHM_MEDIA_PROTOCOL_H

#include "libshm_media_extension_protocol.h"
#include "libshm_media_protocol_log.h"
#include "libshm_media_audio_track_channel_protocol.h"
#include "libshm_media_bin_concat_protocol.h"
#include <stdint.h>
#include <stddef.h>

#define     LIBSHM_MEDIA_PICTURE_TYPE_VIDEO_MASK        0x000000FF
#define     LIBSHM_MEDIA_PICTURE_TYPE_AUDIO_MASK        0x0000FF00
#define     LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO      0
#define     LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO    1
#define     LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_AUDIO      (0<<8)
#define     LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO    (1<<8) /* 0x0100 */
/**
 *  Interlace flag, lower 2 bit to use,
 *      0x00: unknown
 *      0x01: progressive
 *      0x02: interlace
 *      0x03: reserved
**/
#define LIBSHM_MEDIA_INTERLACE_TYPE_UNKNOWN 0x00
#define LIBSHM_MEDIA_INTERLACE_TYPE_PROGRESSIVE 0x01
#define LIBSHM_MEDIA_INTERLACE_TYPE_INTERLACE 0x02
#define LIBSHM_MEDIA_INTERLACE_TYPE_RESERVED 0x03

typedef enum ELibShmMediaPictureStruct
{
    /* the definition  */
    kLibshmmediaPicStructProgressive = 0,       /* progressive frame */
    kLibshmmediaPicStructTop = 1,               /* top field */
    kLibshmmediaPicStructBot = 2,               /* bottom field */
    kLibshmmediaPicStructTopBot = 3,            /* TFF frame */
    kLibshmmediaPicStructBotTop = 4,            /* BFF frame */
    kLibshmmediaPicStructTopBotTop = 5,         /* T+B+T three slice to one frame */
    kLibshmmediaPicStructBotTopBot = 6,         /* B+T+B three slice to one frame */
    kLibshmmediaPicStructFrameDouble = 7,       /* doulbe frames */
    kLibshmmediaPicStructFrameTriple = 8,       /* triple frames */
    kLibshmmediaPicStructTopPairPrevBot = 9,    /* top field which would be merged with previous bottom field */
    kLibshmmediaPicStructBotPairPrevTop = 10,   /* bottom field which would be merged with previous top field */
    kLibshmmediaPicStructTopPairNextBot = 11,   /* top field which would be merged with next bottom field */
    kLibshmmediaPicStructBotPairNextTop = 12,   /* bottom field which would be merged with next top field */
}libshmmedia_pic_struct_t;

typedef int (*libshm_media_process_handle_t)(uint8_t *data, int len, void *userdata);

typedef struct SLibShmMediaHeadParamV0
{
    uint32_t        u_reservePrivate; // reserve for internal
    int32_t         i_vbr;      // can be ignored to set.
    int32_t         i_sarw;     // codec sarw
    int32_t         i_sarh;     // codec sarh
    int32_t         i_srcw;     // codec width
    int32_t         i_srch;     // codec height
    int32_t         i_dstw;     // output YUV width
    int32_t         i_dsth;     // output YUV heigh
    uint32_t        u_videofourcc;
    int32_t         i_duration;
    int32_t         i_scale;
    uint32_t        u_audiofourcc;
    int32_t         i_channels;
    int32_t         i_depth;
    int32_t         i_samplerate;
}libshm_media_head_param_v1_t;

typedef struct SLibShmMediaHeadParamV2
{
    uint32_t        u_reservePrivate; // reserve for internal
    int32_t         i_vbr;      // can be ignored to set.
    int32_t         i_sarw;     // codec sarw
    int32_t         i_sarh;     // codec sarh
    int32_t         i_srcw;     // codec width
    int32_t         i_srch;     // codec height
    int32_t         i_dstw;     // output YUV width
    int32_t         i_dsth;     // output YUV heigh
    uint32_t        u_videofourcc;
    int32_t         i_duration;
    int32_t         i_scale;
    uint32_t        u_audiofourcc;
    int32_t         i_channels;
    int32_t         i_depth;
    int32_t         i_samplerate;
    /**
     *  1. For reader, h_channel ptr is from caller, the caller need to provide the h_channel for library setting,
     *      also the caller need to backup the channel object for comparing by LibshmmediaAudioChannelLayoutCompare to check whether it changed.
     *  2. For writer, h_channel is from caller, the library just put the binrary to the shm. h_channel binary creating
     *     API is LibshmmediaAudioChannelLayoutSerializeToBinary.
    **/
    const libshmmedia_audio_channel_layout_object_t *h_channel;
}libshm_media_head_param_v2_t;

typedef libshm_media_head_param_v2_t libshm_media_head_param_t;

#define LIBSHM_MEDIA_VIDEO_COPIED_FLAG    0x01
#define LIBSHM_MEDIA_AUDIO_COPIED_FLAG    0x02
#define LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG 0x04
#define LIBSHM_MEDIA_EXT_COPIED_FLAG      0x08
#define LIBSHM_MEDIA_CLOSED_CAPTION_COPIED_FLAG   0x08
#define LIBSHM_MEDIA_USER_DATA_COPIED_FLAG   0x10

typedef struct SLibShmMediaItemParamV0
{
    uint32_t    u_reservePrivate; // reserve for internal
    int         i_totalLen;
    const uint8_t   *p_vData;
    int         i_vLen;
    int64_t     i64_vpts;
    int64_t     i64_vdts;
    int64_t     i64_vct;
    const uint8_t    *p_aData;
    int         i_aLen;
    int64_t     i64_apts;
    int64_t     i64_adts;
    int64_t     i64_act;
    const uint8_t     *p_sData;
    int         i_sLen;
    int64_t     i64_spts;
    int64_t     i64_sdts;
    int64_t     i64_sct;
    const uint8_t    *p_CCData;
    int         i_CCLen;
    const uint8_t    *p_timeCode;
    int         i_timeCode;
    uint32_t    u_frameType;
    uint32_t    u_picType;
    const uint8_t    *p_userData; /* user self definition */
    int         i_userDataLen;
    int64_t     i64_userDataCT;/* data's create time */
    int         i_userDataType;
    uint32_t    i_interlaceFlag;

/**
 *  It was used for sending data to shm.
 *  Sometimes, data(video/audio...) had been copied to shm,
 *  So it was to present the copied status.
 *                                                          3        2         1         0
 *   --------- --------- --------- --------- --------- --------- --------- --------- ---------
 *  |         |         |         |         |         |         |         |         |         |
 *  |         |         |         |         |         |    e    |    s    |    a    |    v    |
 *  |         |         |         |         |         |         |         |         |         |
 *   --------- --------- --------- --------- --------- --------- --------- --------- ---------
 *                                                       extend   subtitle    audio     video
 *
 */
    uint32_t    u_copied_flags;

    uint32_t    u_read_index;
    void                                *p_opaq;
    libshm_media_process_handle_t       h_media_process;
}libshm_media_item_param_v1_t;

typedef libshm_media_item_param_v1_t libshm_media_item_param_t;

typedef struct SLibShmMediaItemAddrLayout
{
    uint32_t        i_totalLen;
    uint32_t        i_vOffset;
    uint32_t        i_aOffset;
    uint32_t        i_userOffset;
    uint32_t        i_sOffset;
    uint32_t        i_ccOffset;
    uint32_t        i_timecodeOffset;
    uint32_t        i_keyValueAreaOffset;
    uint8_t         *p_vData;
    uint8_t         *p_aData;
    uint8_t         *p_userData;
    uint8_t         *p_sData;
    uint8_t         *p_CCData;
    uint8_t         *p_timeCode;
    uint8_t         *p_keyValuePtr;
}libshm_media_item_addr_layout_t;

/* if LIBSHM_MEDIA_RAW_DATA_OPT structure */
typedef struct SLibShmMediaItemRawDataParamV1
{
    /**
     * 1.  write, caller's point,
     * 2. read/appply, shm's item point
    **/
    uint8_t   *pRawData_;
    size_t    uRawData_;
}libshmmedia_raw_data_param_v1_t;

typedef struct SLibShmMediaItemRawDataParamV2
{
    uint8_t   *pRawData_;
    size_t    uRawData_;
    uint32_t  uReserverPrivate_;
}libshmmedia_raw_data_param_v2_t;

typedef libshmmedia_raw_data_param_v2_t libshmmedia_raw_data_param_t;

typedef struct SLibShmMediaItemRawHeadParamV1
{
    uint8_t   *pRawHead_;
    size_t    uRawHead_;
}libshmmedia_raw_head_param_v1_t;

typedef struct SLibShmMediaItemRawHeadParamV2
{
    uint8_t   *pRawHead_;
    size_t    uRawHead_;
    uint32_t  uReservePrivate_;
}libshmmedia_raw_head_param_v2_t;

typedef libshmmedia_raw_head_param_v2_t libshmmedia_raw_head_param_t;

/**
 * Return:
 *  0--not searched, 1--searched
**/
typedef int (*libshmmedia_item_checking_fn_t)(void *user, const libshm_media_head_param_t *, const libshm_media_item_param_t*);

/* endif LIBSHM_MEDIA_RAW_DATA_OPT structure */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Functionality:
 *      used to get total data size of item parameters.
 *  Parameters:.
 *      @pvi[IN]  : item paramters.
 *  Reutrn:
 *      >=0   : the total size.
 */
_LIBSHMMEDIA_PROTO_DLL_
uint32_t LibShmMediaProGetItemParamDataLen(const libshm_media_item_param_t *pvi);


/**
 *  Functionality:
 *      used to get the head version from header's buffer.
 *  Parameters:.
 *      @headBuf[IN]  : it can be gotten from the LibShmMediaProtoGetHeadVersion api.
 *  Reutrn:
 *      0   : Invalid version number
 *      >0   : now support version 1,2,3,4
 */
_LIBSHMMEDIA_PROTO_DLL_
unsigned int LibShmMediaProtoGetHeadVersion(const uint8_t *headBuf);

/**
 *  Functionality:
 *      used to read out the item buffer layout.
 *  Parameters:
 *      @pmh[OUT]    : some head parameters.
 *      @pmi[OUT]    : item parameters.
 *      @pItemAddr[IN]  : item address point.
 *      @nItem[IN]  : pItemAddr total buffer sizes.
 *  Reutrn:
 *      >0   : reading the length
 *      <=0   : errors, as the item buffer was invalid, or nItem was not matching the buffer
 */
_LIBSHMMEDIA_PROTO_DLL_
int  LibShmMediaProtoReadItemBufferLayout(/*OUT*/libshm_media_head_param_t *pmh
                                     , /*OUT*/libshm_media_item_param_t *pmi
                                     , /*IN*/const uint8_t *pItemAddr, uint32_t nItem);

/**
 *  Functionality:
 *      used to read out the item buffer layout with head version.
 *  Parameters:
 *      @pmh[OUT]    : some head parameters.
 *      @pmi[OUT]    : item parameters.
 *      @pItemAddr[IN]  : item address point.
 *      @headVer[IN]  : it can be gotten from the LibShmMediaProtoGetHeadVersion/LibShmMediaGeHeadVersion api.
 *  Reutrn:
 *      >0   : reading the length
 *      <=0   : errors, as the item buffer was invalid, or headVer invalid
 */
_LIBSHMMEDIA_PROTO_DLL_
int  LibShmMediaProtoReadItemBufferLayoutWithHeadVer(/*OUT*/libshm_media_head_param_t *pmh
                                     , /*OUT*/libshm_media_item_param_t *pmi
                                     , /*IN*/const uint8_t *pItemAddr, uint32_t headVer);

/**
 *  Functionality:
 *      used to get buffer layout status for user write its data directly.
 *  Parameters:
 *      @pmi[IN]    : item different length status.
 *      @pItemAddr[IN]  : item address point.
 *      @itemLen[IN]    : item buffer size.
 *      @pLayout[OUT]   : return the layout status to the this struct.
 *  Reutrn:
 *      >0   :  possible pre-written length,
 *              to make sure pItemAddr size was larger than this value before write the buffer.
 *      <=0   :  possible privilege issue, or not support.
 *      <0   :  failed.
 */
_LIBSHMMEDIA_PROTO_DLL_
int  LibShmMediaProtoGetWriteItemBufferLayout(/*IN*/const libshm_media_item_param_t *pmi
    , /*IN*/uint8_t *pItemAddr, /*IN*/uint32_t itemLen, /*OUT*/libshm_media_item_addr_layout_t *playout);

/**
 *  Functionality:
 *      used to compute the required buffer length for writing.
 *  Parameters:
 *      @pmi[IN]    : item different length status.
 *  Reutrn:
 *      >0   :   the required item length for writing,
 *      <=0   :  invalid.
 */
_LIBSHMMEDIA_PROTO_DLL_
int  LibShmMediaProtoRequireWriteItemBufferLength(/*IN*/const libshm_media_item_param_t *pmi);

/**
 *  Functionality:
 *      used to write item head only, but not write data.
 *  Parameters:
 *      @pmh[IN]    : head information point
 *      @pmi[IN]    : item information point
 *      @pItemAddr[OUT]  : item address point which would be written.
 *  Reutrn:
 *      0   :   not writable, need to wait
 *      <0  :   failed, not supported.
 *      >0  :   success that the written bytes to shm.
 */
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaProtoWriteItemBuffer(
        const libshm_media_head_param_t *pmh,
        const libshm_media_item_param_t *pmi,
        uint8_t *pItemAddr);

/**
 *  Functionality:
 *      used to intialize the head param object.
 *  Parameters:
 *      @p[IN]    : head param object point
 *      @structSize[IN]    : the object structure size
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaHeadParamInit(libshm_media_head_param_t *p, uint32_t structSize);

/**
 *  Functionality:
 *      used to release the head param object.
 *  Parameters:
 *      @p[IN]    : head param object point
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
void LibShmMediaHeadParamRelease(libshm_media_head_param_t *p);

/**
 *  Functionality:
 *      used to intialize the item param object.
 *  Parameters:
 *      @p[IN]    : item param object point
 *      @structSize[IN]    : the object structure size
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaItemParamInit(libshm_media_item_param_t *p, uint32_t structSize);

/**
 *  Functionality:
 *      used to release the item param object.
 *  Parameters:
 *      @p[IN]    : item param object point
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
void LibShmMediaItemParamRelease(libshm_media_item_param_t *p);

/**
 *  Functionality:
 *      used to get the pts of the item frame.
 *  Parameters:
 *      @p[IN]    : item param object point
 *      @type[IN]   : item frame type.
 *                  'v' -- video
 *                  'a' -- audio
 *                  's' -- subtitle
 *                  'd' -- meta data
 *                  0   -- automtically find the first valid pts.
 *  Reutrn:
 *
 */
uint64_t LibShmMediaItemParamGetPts(libshm_media_item_param_t *p, const char type);


/**
 *  Functionality:
 *      used to intialize the raw data param object.
 *  Parameters:
 *      @p[IN]    : raw data param object point
 *      @structSize[IN]    : the object structure size
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaRawDataParamInit(libshmmedia_raw_data_param_t *p, uint32_t structSize);

///**
// *  Functionality:
// *      used to release the raw data param object.
// *  Parameters:
// *      @p[IN]    : raw data param object point
// *  Reutrn:
// *
// */
//_LIBSHMMEDIA_PROTO_DLL_
//void LibShmMediaRawDataParamRelease(libshmmedia_raw_data_param_t *p);


/**
 *  Functionality:
 *      used to intialize the raw head object.
 *  Parameters:
 *      @p[IN]    : raw head object point
 *      @structSize[IN]    : the object structure size
 *  Reutrn:
 *
 */
_LIBSHMMEDIA_PROTO_DLL_
int LibShmMediaRawHeadParamInit(libshmmedia_raw_head_param_t *p, uint32_t structSize);

///**
// *  Functionality:
// *      used to release the item param object.
// *  Parameters:
// *      @p[IN]    : raw head object point
// *  Reutrn:
// *
// */
//_LIBSHMMEDIA_PROTO_DLL_
//void LibShmMediaRawHeadParamRelease(libshmmedia_raw_head_param_t *p);

#ifdef __cplusplus
}
#endif

#endif // LIBSHM_MEDIA_PROTOCOL_H
