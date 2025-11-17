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
#ifndef LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTOCOL_H
#define LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTOCOL_H

#include "libshm_media_proto_common.h"
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef void libshmmedia_audio_channel_layout_object_t;
typedef void *libshmmedia_audio_channel_layout_handle_t;

#ifdef __cplusplus
    extern "C" {
#endif

    /**
      * Functionality:
      *     used to create context for channel layout management.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    libshmmedia_audio_channel_layout_object_t *LibshmmediaAudioChannelLayoutCreate();

//    /**
//      * Functionality:
//      *     used to reset the context stats.
//    **/
//    _LIBSHMMEDIA_PROTO_DLL_
//    bool LibshmmediaAudioChannelLayoutReset(libshmmedia_audio_channel_layout_t *pctx);

    /**
      * Functionality:
      *     used to get the planar setting.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaAudioChannelLayoutIsPlanar(const libshmmedia_audio_channel_layout_object_t *pctx);

    /**
      * Functionality:
      *     used to get the channel total size.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    uint16_t LibshmmediaAudioChannelLayoutGetChannelNum(const libshmmedia_audio_channel_layout_object_t *pctx);

    /**
      * Functionality:
      *     used to get the channel array from the context.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaAudioChannelLayoutGetChannelArr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint16_t **ppDest, /*OUT*/uint16_t *pnDest);

    /**
      * Functionality:
      *     used to get the binary address and length from the context.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaAudioChannelLayoutGetBinaryAddr(const libshmmedia_audio_channel_layout_object_t *pctx, /*OUT*/const uint8_t **ppDest, /*OUT*/uint32_t *pnDest);

    /**
      * Functionality:
      *     used to serialize the channel-layout to the binary.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaAudioChannelLayoutSerializeToBinary(libshmmedia_audio_channel_layout_object_t *pctx
                                                       , const uint16_t *pChan, uint16_t nChan, bool bPlanar
                                                       );

    /**
      * Functionality:
      *     used to parse the binary to channel-layout.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaAudioChannelLayoutParseFromBinary(libshmmedia_audio_channel_layout_object_t *pctx
                                                  , const uint8_t *pBin, uint32_t nBin
                                                  );

    /**
      * Functionality:
      *     used to destroy context for channel layout management.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    void LibshmmediaAudioChannelLayoutDestroy(libshmmedia_audio_channel_layout_object_t *pctx);

    /**
      * Functionality:
      *     used to compare the two object content.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    int LibshmmediaAudioChannelLayoutCompare(const libshmmedia_audio_channel_layout_object_t *p1
                                                  , const libshmmedia_audio_channel_layout_object_t *p2
                                                  );

    /**
      * Functionality:
      *     copy the content.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    int LibshmmediaAudioChannelLayoutCopy(libshmmedia_audio_channel_layout_object_t *pdst
                                                  , const libshmmedia_audio_channel_layout_object_t *psrc
                                                  );


#ifdef __cplusplus
    }
#endif

#endif // LIBSHM_MEDIA_AUDIO_TRACK_CHANNEL_PROTOCOL_H
