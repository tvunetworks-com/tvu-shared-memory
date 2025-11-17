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
/******************************************************************************
 *  Description:
 *      It is used to control memory buffer.
 *
 *  CopyRight:
 *      lotus/TVU
 *
 *  History:
 *      March 19th, 2015, initialize it.
******************************************************************************/
#ifndef  _TVUSHM_BUFFERCTRL_H
#define _TVUSHM_BUFFERCTRL_H

#include "libshm_variant.h"
#include "libshm_uint128.h"
#include <stdint.h>
#include <stdio.h>

#define FAILED_PUSH_DATA(s)  do{int _ret##__LINE__ = s;if (_ret##__LINE__ < 0) {return _ret##__LINE__;}} while (0)
#define FAILED_WRITE_DATA_STEP(s, n)  do{int _ret##__LINE__ = s;\
    if (_ret##__LINE__ < 0) {return _ret##__LINE__;}\
    n += _ret##__LINE__; \
    } while (0)
#define FAILED_READ_DATA(s)  do{int _ret##__LINE__ = s;if (_ret##__LINE__ <= 0) {return _ret##__LINE__;}} while (0)
#define FAILED_PUSH_DATA_RET(s,retVal)  do{int _ret##__LINE__ = s;if (_ret##__LINE__ < 0) {return retVal;}} while (0)
#define FAILED_WRITE_DATA_RET(s,retVal)  do{int _ret##__LINE__ = s;if (_ret##__LINE__ <= 0) {return retVal;}} while (0)
#define FAILED_READ_DATA_RET(s,retVal)  do{int _ret##__LINE__ = s;if (_ret##__LINE__ <= 0) {return retVal;}} while (0)

namespace tvushm {

    typedef
    struct SBufferController
    {
    public:
        SBufferController();
        virtual ~SBufferController();
        const uint8_t *GetOrigPtr()const;
        int GetBufLength()const;
    public:
        uint8_t     *p_buff;
        int         i_start;
        int         i_allocSize;
        int         i_current;
        int         i_bufWritePos;
        bool        b_externalBuffer;
    }BufferController_t;

    //#ifdef __cplusplus
    //extern "C" {
    //#endif

    int BufferCtrlInit(BufferController_t *p);

    BufferController_t* BufferCtrlNew();

    int BufferCtrlAttachExternalReadBuffer(BufferController_t *p, const uint8_t *pBuff, uint32_t nBuff);
    void BufferCtrlFree(BufferController_t *p);
    void BufferCtrlRelease(BufferController_t *p);
    int BufferCtrlAllocBuf(BufferController_t *p, int s);

    uint8_t *BufferCtrlGetCurPtr(const BufferController_t *p);
    uint8_t *BufferCtrlGetOrigPtr(const BufferController_t *p);

    int BufferCtrlPushData(BufferController_t *p, const uint8_t  *data, int len);
    int BufferCtrlPopData(BufferController_t *p, uint8_t *data, int len);
    int BufferCtrlWriteBinary(BufferController_t *p, const uint8_t  *data, int len);
    //int BufferCtrlReadBinary(BufferController_t *p, const uint8_t  **ppData, int *pnLen);

    int BufferCtrlMemMove(BufferController_t *p, int pos_dst, int pos_src, int n);

    int BufferCtrlW8(BufferController_t *p, uint8_t b);

    int BufferCtrlWl16(BufferController_t *p, uint16_t val);

    int BufferCtrlWl32(BufferController_t *p, uint32_t val);

    int BufferCtrlWl64(BufferController_t *p, uint64_t val);

    int BufferCtrlWLe16(BufferController_t *p, uint16_t val);

    int BufferCtrlWLe32(BufferController_t *p, uint32_t val);

    int BufferCtrlWLe64(BufferController_t *p, uint64_t val);

    int BufferCtrlWLe128(BufferController_t *p, const uint128_t &val);

    int BufferCtrlWBe16(BufferController_t *p, uint16_t val);

    int BufferCtrlWBe32(BufferController_t *p, uint32_t val);

    int BufferCtrlWBe64(BufferController_t *p, uint64_t val);

    int BufferCtrlWBe128(BufferController_t *p, const uint128_t &val);


    uint32_t BufferCtrlGetAllocSize(const BufferController_t *p);

    uint32_t BufferCtrlR8(BufferController_t *p);

    uint32_t BufferCtrlRl16(BufferController_t *p);

    uint32_t BufferCtrlRl32(BufferController_t *s);

    uint64_t BufferCtrlRl64(BufferController_t *s);

    uint32_t BufferCtrlRLe16(BufferController_t *p);

    uint32_t BufferCtrlRLe32(BufferController_t *s);

    uint64_t BufferCtrlRLe64(BufferController_t *s);

    uint32_t BufferCtrlRBe16(BufferController_t *p);

    uint32_t BufferCtrlRBe32(BufferController_t *s);

    uint64_t BufferCtrlRBe64(BufferController_t *s);

    uint128_t BufferCtrlRBe128(BufferController_t *s);


    int BufferCtrlWriteRawDataU8(BufferController_t *p, uint8_t b);
    int BufferCtrlWriteRawDataLeU16(BufferController_t *p, uint16_t val);
    int BufferCtrlWriteRawDataLeU32(BufferController_t *p, uint32_t val);
    int BufferCtrlWriteRawDataLeU64(BufferController_t *p, uint64_t val);
    int BufferCtrlWriteRawDataLeU128(BufferController_t *p, const uint128_t &val);

    int BufferCtrlWriteRawDataBeU16(BufferController_t *p, uint16_t val);
    int BufferCtrlWriteRawDataBeU32(BufferController_t *p, uint32_t val);
    int BufferCtrlWriteRawDataBeU64(BufferController_t *p, uint64_t val);
    int BufferCtrlWriteRawDataBeU128(BufferController_t *p, const uint128_t &val);

    int BufferCtrlReadRawDataU8(BufferController_t *p, uint8_t &val);
    int BufferCtrlReadRawDataLeU16(BufferController_t *p, uint16_t &val);
    int BufferCtrlReadRawDataLeU32(BufferController_t *p, uint32_t &val);
    int BufferCtrlReadRawDataLeU64(BufferController_t *p, uint64_t &val);
    int BufferCtrlReadRawDataLeU128(BufferController_t *p, uint128_t &val);

    int BufferCtrlReadRawDataBeU16(BufferController_t *p, uint16_t &val);
    int BufferCtrlReadRawDataBeU32(BufferController_t *p, uint32_t &val);
    int BufferCtrlReadRawDataBeU64(BufferController_t *p, uint64_t &val);
    int BufferCtrlReadRawDataBeU128(BufferController_t *p, uint128_t &val);


    int BufferCtrlReadSkip(BufferController_t *p, int n);

    int BufferCtrlSeek(BufferController_t  *p, int offset, int whence);

    int BufferCtrlRewind(BufferController_t  *p);
    int BufferCtrlReset(BufferController_t  *p);

    int BufferCtrlTellCurPos(BufferController_t *p);
    int BufferCtrlTellSize(BufferController_t *p);


    int BufferCtrlGetBufLen(const BufferController_t *p);
    int BufferCtrlReadBufLeftLen(const BufferController_t *p);
    int BufferCtrlExtendSize(BufferController_t *p, uint32_t s);
    int BufferCtrlCompactGetBytesCountU8(uint8_t value);
    int BufferCtrlCompactGetBytesCountU16(uint16_t value);
    int BufferCtrlCompactGetBytesCountU32(uint32_t value);
    int BufferCtrlCompactGetBytesCountU64(uint64_t value);
    int BufferCtrlCompactGetBytesCountVariant(const Variant& value);

    int BufferCtrlCompactEncodeValueU8(BufferController_t *p, uint8_t value);
    int BufferCtrlCompactEncodeValueU16(BufferController_t *p, uint16_t value);
    int BufferCtrlCompactEncodeValueU32(BufferController_t *p, uint32_t value);
    int BufferCtrlCompactEncodeValueU64(BufferController_t *p, uint64_t value);


    int BufferCtrlCompactDecodeValueU8(BufferController_t *p, uint8_t&valueRef);
    int BufferCtrlCompactDecodeValueU16(BufferController_t *p, uint16_t&valueRef);
    int BufferCtrlCompactDecodeValueU32(BufferController_t *p, uint32_t&valueRef);
    int BufferCtrlCompactDecodeValueU64(BufferController_t *p, uint64_t&valueRef);

    int BufferCtrlCompactEncodeVariant(BufferController_t *p, const Variant& value);
    int BufferCtrlCompactDecodeVariant(BufferController_t *p, Variant&valueRef, bool referenceOnly);

    //#ifdef __cplusplus
    //}
    //#endif

}

#endif /* _TVUSHM_BUFFERCTRL_H */
