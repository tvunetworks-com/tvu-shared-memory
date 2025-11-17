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
#ifndef LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_H
#define LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_H

#include "libshm_media_proto_common.h"
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef void * libshmmedia_bin_concat_proto_handle_t;

typedef struct libshmmedia_bin_concat_proto_Seg
{
    uint32_t    nSeg;
    const uint8_t *pSeg;
}libshmmedia_bin_concat_proto_seg_t;

#ifdef __cplusplus
    extern "C" {
#endif
    /**
      * Functionality:
      *     used to create context for bin concat protocol.
      * Parameters:
      * Return:
      *     the handle object.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    libshmmedia_bin_concat_proto_handle_t LibshmmediaBinConcatProtoCreate();

    /**
      * Functionality:
      *     used to reset the context for reusing bin concat protocol.
      * Parameters:
      * Return:
      *     the handle object.
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaBinConcatProtoReset(libshmmedia_bin_concat_proto_handle_t);

    /**
      * Functionality:
      *     used to concat segment data to one binary data.
      * Parameters:
      *     @pctx:handle context.
      *     @pSeg:segment data.
      *     @nSeg:segment length.
      * Return:
      *     true:success
      *     false:failed
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaBinConcatProtoConcatSegment(libshmmedia_bin_concat_proto_handle_t pctx, /*IN*/ const libshmmedia_bin_concat_proto_seg_t *pSeg);

    /**
      * Functionality:
      *     used to flush out the binary data.
      * Parameters:
      *     @pctx:handle context.
      *     @ppBin:store the out binary ptr.
      *     @pnBin:store the out binary length.
      * Return:
      *     true:success
      *     false:failed
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaBinConcatProtoFlushBinary(libshmmedia_bin_concat_proto_handle_t pctx, /*OUT*/const uint8_t **ppBin, /*OUT*/uint32_t *pnBin);

    /**
      * Functionality:
      *     used to split the binary data.
      * Parameters:
      *     @pctx:handle context.
      *     @pbin:binary ptr.
      *     @nbin:binary length.
      *     @bCreateBuffer:whether to create another buffer from @pbin for parsing.
      * Return:
      *     true:success
      *     false:failed
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaBinConcatProtoSplitBinary(libshmmedia_bin_concat_proto_handle_t pctx
                                              , /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin, /*IN*/bool bCreateBuffer
                                              , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppData, /*OUT*/uint32_t *pnData
                                              );

    /**
      * Functionality:
      *     used to split the binary data. It is same to LibshmmediaBinConcatProtoSplitBinary when @bCreateBuffer is false.
      * Parameters:
      *     @pctx:handle context.
      *     @pbin:binary ptr.
      *     @nbin:binary length.
      * Return:
      *     true:success
      *     false:failed
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    bool LibshmmediaBinConcatProtoParseBinary(libshmmedia_bin_concat_proto_handle_t pctx
                                             , /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin
                                             , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppData, /*OUT*/uint32_t *pnData
                                             );

    /**
      * Functionality:
      *     used to destroy context for channel layout management.
      * Parameters:
      *     @pctx:handle context.
      * Return:
      *
    **/
    _LIBSHMMEDIA_PROTO_DLL_
    void LibshmmediaBinConcatProtoDestroy(libshmmedia_bin_concat_proto_handle_t pctx);


#ifdef __cplusplus
    }
#endif

#endif // LIBSHM_MEDIA_BIN_CONCAT_PROTOCOL_H
