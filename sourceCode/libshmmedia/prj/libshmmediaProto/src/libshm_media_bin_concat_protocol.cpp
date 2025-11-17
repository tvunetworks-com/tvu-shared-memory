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
#include "libshm_media_bin_concat_protocol_internal.h"
#include "libshm_media_protocol_log_internal.h"
#include <string.h>
#include <assert.h>

namespace tvushm {

    BinConcatProto::BinConcatProto()
    {
        _count = 0;
        _info.reserve(16);
        BufferCtrlInit(&_bc);
    }

    BinConcatProto::~BinConcatProto()
    {
        _count = 0;
        Reset();
    }

    bool BinConcatProto::ConcatSegment(/*IN*/ const uint8_t *pSeg, /*IN*/uint32_t nSeg)
    {
        if (!pSeg || !nSeg)
        {
            return false;
        }

        BufferController_t &bc = _bc;
        std::vector<OffsetLenPair_t> &ov = _info;
        FAILED_PUSH_DATA_RET(BufferCtrlCompactEncodeValueU32(&bc,nSeg),false);

        uint32_t off = BufferCtrlTellCurPos(&bc);;
        FAILED_PUSH_DATA_RET(BufferCtrlWriteBinary(&_bc,pSeg,nSeg),false);

        {
            OffsetLenPair_t pair;
            {
                memset(&pair, 0, sizeof(pair));
            }
            pair.len = nSeg;
            pair.off = off;
            ov.push_back(pair);
            _count++;
        }
        return true;
    }

    bool BinConcatProto::FlushBinary(/*OUT*/const uint8_t **ppBin, /*OUT*/uint32_t *pnBin)
    {
        BufferController_t &bc = _bc;
        uint8_t *pBin = BufferCtrlGetOrigPtr(&bc);
        uint32_t nBin = BufferCtrlGetBufLen(&bc);

        if (!FlushSegInfo())
        {
            return false;
        }

        if (ppBin)
        {
            *ppBin = pBin;
        }

        if (pnBin)
        {
            *pnBin = nBin;
        }
        return true;
    }

    bool BinConcatProto::FlushSegInfo(libshmmedia_bin_concat_proto_seg_t **ppSeg, uint32_t *pnSeg)
    {
        uint32_t count = _count;
        std::vector<OffsetLenPair_t> &ov = _info;
        assert(count == ov.size());

        CacheBuffer &segInfo = _segInfo;
        uint32_t segSize = count * sizeof(libshmmedia_bin_concat_proto_seg_t);

        if (!segInfo.Alloc(segSize))
        {
            DEBUG_SHMMEDIA_PROTO_ERROR_CR("flush segment infor failed for seginfor allock."
                                          "cnt:%u"
                                          , count
                                          );
            return false;
        }

        BufferController_t &bc = _bc;
        uint8_t *pBin = BufferCtrlGetOrigPtr(&bc);
        libshmmedia_bin_concat_proto_seg_t *pseg = (libshmmedia_bin_concat_proto_seg_t *)segInfo.GetBufAddr();
        for (uint32_t i = 0; i < count; i++)
        {
            pseg[i].nSeg = ov[i].len;
            pseg[i].pSeg = pBin+ov[i].off;
        }
        segInfo.SetBufLen(segSize);

        if (ppSeg)
        {
            *ppSeg = pseg;
        }

        if (pnSeg)
        {
            *pnSeg = count;
        }
        return true;
    }

    bool BinConcatProto::SplitBinary(
            /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin, /*IN*/bool bCreateBuffer
            , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppSeg, /*OUT*/uint32_t *pnSeg
            )
    {
        BufferController_t &bc = _bc;
        if (bCreateBuffer)
        {
            FAILED_PUSH_DATA_RET(BufferCtrlWriteBinary(&bc, pBin, nBin),false);
            BufferCtrlRewind(&bc);
        }
        else
        {
            FAILED_PUSH_DATA_RET(BufferCtrlAttachExternalReadBuffer(&bc, pBin, nBin),false);
        }

        uint32_t count = 0;
        std::vector<OffsetLenPair_t> &ov = _info;
        bool faileParse = false;
        do
        {
            int left = BufferCtrlReadBufLeftLen(&bc);

            if (left <= 0)
            {
                break;
            }

            uint32_t len = 0;
            FAILED_READ_DATA_RET(BufferCtrlCompactDecodeValueU32(&bc, len),false);

            if (len > (uint32_t)left)
            {
                DEBUG_SHMMEDIA_PROTO_ERROR_CR("parse bin concat protocol failed for invlid length."
                                              "len:%u,left:%d"
                                              , len, left
                                              );
                faileParse = true;
                break;
            }


            uint32_t off = BufferCtrlTellCurPos(&bc);
            BufferCtrlReadSkip(&bc, len);

            {
                OffsetLenPair_t pair;
                {
                    memset(&pair, 0, sizeof(pair));
                }
                pair.len = len;
                pair.off = off;
                ov.push_back(pair);
                count++;
            }

        }while (1);

        if (faileParse)
        {
            return false;
        }

        _count = count;
        /*flush seg infor.*/
        return FlushSegInfo(ppSeg, pnSeg);
    }

     void BinConcatProto::Reset()
     {
         BufferCtrlRelease(&_bc);
         _count = 0;
         _info.clear();
         _segInfo.Release();
     }

     void BinConcatProto::Release()
     {
         BufferCtrlRelease(&_bc);
         _count = 0;
         _info.clear();
         _segInfo.Release();
     }

}

//int LibshmmediaBinConcatProtoDataInit(libshmmedia_bin_concat_proto_data_t *p)
//{
//    memset(p, 0, sizeof(libshmmedia_bin_concat_proto_data_t));
//    return 0;
//}

//void LibshmmediaBinConcatProtoDataRelease(libshmmedia_bin_concat_proto_data_t *)
//{
//    return;
//}

libshmmedia_bin_concat_proto_handle_t LibshmmediaBinConcatProtoCreate()
{
    tvushm::BinConcatProto *p = new tvushm::BinConcatProto();
    return p;
}

bool LibshmmediaBinConcatProtoReset(libshmmedia_bin_concat_proto_handle_t pctx)
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    if (p)
    {
        p->Reset();
    }
    return true;
}

bool LibshmmediaBinConcatProtoConcatSegment(libshmmedia_bin_concat_proto_handle_t pctx, /*IN*/ const libshmmedia_bin_concat_proto_seg_t *pSeg)
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    if (!pSeg)
    {
        return false;
    }
    return p->ConcatSegment(pSeg->pSeg, pSeg->nSeg);
}


bool LibshmmediaBinConcatProtoFlushBinary(libshmmedia_bin_concat_proto_handle_t pctx, /*OUT*/const uint8_t **ppBin, /*OUT*/uint32_t *pnBin)
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    return p->FlushBinary(ppBin, pnBin);
}


bool LibshmmediaBinConcatProtoSplitBinary(libshmmedia_bin_concat_proto_handle_t pctx
                                          , /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin, /*IN*/bool bCreateBuffer
                                          , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppData, /*OUT*/uint32_t *pnData
                                          )
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    return p->SplitBinary(pBin, nBin, bCreateBuffer, ppData, pnData);
}


bool LibshmmediaBinConcatProtoParseBinary(libshmmedia_bin_concat_proto_handle_t pctx
                                         , /*IN*/const uint8_t *pBin, /*IN*/uint32_t nBin
                                         , /*OUT*/libshmmedia_bin_concat_proto_seg_t **ppData, /*OUT*/uint32_t *pnData
                                         )
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    return p->SplitBinary(pBin, nBin, false, ppData, pnData);
}

void LibshmmediaBinConcatProtoDestroy(libshmmedia_bin_concat_proto_handle_t pctx)
{
    tvushm::BinConcatProto *p = (tvushm::BinConcatProto *)pctx;
    if (p)
    {
        p->Release();
        delete p;
    }
    return;
}

