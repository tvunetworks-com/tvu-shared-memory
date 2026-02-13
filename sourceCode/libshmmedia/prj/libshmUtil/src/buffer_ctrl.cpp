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

#include "buffer_controller_internal.h"

#define BFCTRL_MALLOC(s)        malloc(s)
#define BFCTRL_REALLOC(p, s)    realloc(p, s)
#define BFCTRL_FREE(p)          free((void *)p)
#define BFCTRL_SAFE_FREE(p)     do {if (p) {BFCTRL_FREE(p);p=NULL;}} while (0)
#define BFCTRL_MIN(a, b)        (a) < (b) ? (a) : (b)

#define BUFFER_CTRL_ERR_LOG_PRINT(fmt, ...)       printf("error, @[bufferctrl, %s:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define BUFFER_CTRL_ERROR_VAL(e)    (-(e))

namespace tvushm {

    SBufferController::SBufferController()
    {
        BufferCtrlInit(this);
    }
    SBufferController::~SBufferController()
    {
        BufferCtrlRelease(this);
    }

    const uint8_t *SBufferController::GetOrigPtr()const
    {
        return BufferCtrlGetOrigPtr(this);
    }

    int SBufferController::GetBufLength()const
    {
        return BufferCtrlGetBufLen(this);
    }

    int BufferCtrlInit(BufferController_t *p)
    {
        memset((void *)p, 0, sizeof(BufferController_t));
        return 0;
    }

    BufferController_t* BufferCtrlNew()
    {
        BufferController_t     *p  = new SBufferController();

        if (!p)
            return NULL;

        BufferCtrlInit(p);
        return p;
    }

    int BufferCtrlAttachExternalReadBuffer(BufferController_t *p, const uint8_t *pBuff, uint32_t nBuff)
    {
        p->p_buff = (uint8_t *)pBuff;
        p->i_bufWritePos = nBuff;
        p->b_externalBuffer = true;
        return nBuff;
    }

    void BufferCtrlFreeOnlyData(BufferController_t *p)
    {
        BufferCtrlFreeOnlyBuff(p);
        if (p)
        {
            BufferCtrlInit(p);
        }
        return;
    }

    void BufferCtrlFreeOnlyBuff(BufferController_t *p)
    {
        if (p && !p->b_externalBuffer)
        {
            BFCTRL_SAFE_FREE(p->p_buff);
        }
        return;
    }

    void BufferCtrlFree(BufferController_t *p)
    {
        BufferCtrlRelease(p);
        if (p)
        {
            delete p;
        }
        return;
    }

    void BufferCtrlRelease(BufferController_t *p)
    {
        BufferCtrlFreeOnlyData(p);
    }

    int BufferCtrlSeek(BufferController_t  *p, int offset, int whence)
    {
        if (SEEK_SET == whence)
        {
            p->i_current   = p->i_start + offset;
        }
        else if (SEEK_END == whence)
        {
            p->i_current   = p->i_allocSize + offset;
        }
        else
        {
            p->i_current   = p->i_current + offset;
        }

        return  p->i_current;
    }

    int BufferCtrlRewind(BufferController_t  *p)
    {
        return BufferCtrlSeek(p, 0, SEEK_SET);
    }

    int BufferCtrlReset(BufferController_t  *p)
    {
        p->i_bufWritePos = 0;
        BufferCtrlRewind(p);
        return 0;
    }

    int BufferCtrlTellCurPos(BufferController_t *p)
    {
        return  p->i_current;
    }

    int BufferCtrlTellSize(BufferController_t *p)
    {
        return p->i_bufWritePos;
    }

    uint8_t *BufferCtrlGetCurPtr(const BufferController_t *p)
    {
        return p->p_buff+p->i_current;
    }

    uint8_t *BufferCtrlGetOrigPtr(const BufferController_t *p)
    {
        return p->p_buff;
    }

    int BufferCtrlPushData(BufferController_t *p, const uint8_t  *data, int len)
    {
        if (p->b_externalBuffer)
        {
            return BUFFER_CTRL_ERROR_VAL(EPERM);
        }

        int     total   = p->i_current + len;
        int ret = BufferCtrlAllocBuf(p, total);
        if (ret < 0)
        {
            return ret;
        }

        ret = len;
        memcpy(p->p_buff + p->i_current, data, len);
        p->i_current    += len;
        p->i_bufWritePos    += len;

        return ret;
    }

    int BufferCtrlWriteBinary(BufferController_t *p, const uint8_t  *data, int len)
    {
        return BufferCtrlPushData(p, data, len);
    }

    int BufferCtrlMemMove(BufferController_t *p, /* dest */int pos_dst, /* src */int pos_src, int n)
    {
        memmove(p->p_buff+pos_dst, p->p_buff+pos_src, n);
        return n;
    }

    int BufferCtrlW8(BufferController_t *p, uint8_t b)
    {
        int     total   = p->i_current + 1;
        int ret = BufferCtrlAllocBuf(p, total);
        if (ret < 0)
        {
            return ret;
        }

        *(p->p_buff + p->i_current) = (b & 0xff);
        p->i_current++;
        p->i_bufWritePos++;

        ret = 1;
        return ret;
    }

    int BufferCtrlWl16(BufferController_t *p, uint16_t val)
    {
        int     ret = -1;

        if ((ret = BufferCtrlW8(p, (uint8_t) val)) < 0)
            return ret;

        if ((ret = BufferCtrlW8(p, (uint8_t) (val>>8))) < 0)
            return ret;

        ret = 2;
        return ret;
    }

    int BufferCtrlWl32(BufferController_t *p, uint32_t val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWl16(p, (uint16_t) val)) < 0)
            return ret;

        if ((ret = BufferCtrlWl16(p, (uint16_t) (val >> 16))) < 0)
            return ret;

        ret = 4;
        return ret;
    }

    int BufferCtrlWl64(BufferController_t *p, uint64_t val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWl32(p, (uint32_t) val)) < 0)
            return ret;

        if ((ret = BufferCtrlWl32(p, (uint32_t) (val >> 32))) < 0)
            return ret;

        ret = 8;
        return ret;
    }

    int BufferCtrlWLe16(BufferController_t *p, uint16_t val)
    {
        return BufferCtrlWl16(p, val);
    }

    int BufferCtrlWLe32(BufferController_t *p, uint32_t val)
    {
        return BufferCtrlWl32(p, val);
    }

    int BufferCtrlWLe64(BufferController_t *p, uint64_t val)
    {
        return BufferCtrlWl64(p, val);
    }

    int BufferCtrlWLe128(BufferController_t *p, const uint128_t &val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWLe64(p, (uint64_t) (val.namedQwords.lowQword))) < 0)
            return ret;

        if ((ret = BufferCtrlWLe64(p, (uint64_t)(val.namedQwords.highQword))) < 0)
            return ret;

        ret = 16;
        return ret;
    }

    int BufferCtrlWBe16(BufferController_t *p, uint16_t val)
    {
        int     ret = -1;

        if ((ret = BufferCtrlW8(p, (uint8_t) (val>>8))) < 0)
            return ret;

        if ((ret = BufferCtrlW8(p, (uint8_t) val)) < 0)
            return ret;

        ret = 2;
        return ret;
    }

    int BufferCtrlWBe32(BufferController_t *p, uint32_t val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWBe16(p, (uint16_t) (val >> 16))) < 0)
            return ret;

        if ((ret = BufferCtrlWBe16(p, (uint16_t) val)) < 0)
            return ret;

        ret = 4;
        return ret;
    }

    int BufferCtrlWBe64(BufferController_t *p, uint64_t val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWBe32(p, (uint32_t) (val >> 32))) < 0)
            return ret;

        if ((ret = BufferCtrlWBe32(p, (uint32_t) val)) < 0)
            return ret;

        ret = 8;
        return ret;
    }

    int BufferCtrlWBe128(BufferController_t *p, const uint128_t &val)
    {
        int     ret     = -1;

        if ((ret = BufferCtrlWBe64(p, (uint64_t) (val.namedQwords.highQword))) < 0)
            return ret;

        if ((ret = BufferCtrlWBe64(p, (uint64_t)(val.namedQwords.lowQword))) < 0)
            return ret;

        ret = 16;
        return ret;
    }

    int BufferCtrlPopData(BufferController_t *p, uint8_t *data, int len)
    {
        int     ret     = 0;

        if (!p || !p->p_buff)
            return 0;

        ret     = BFCTRL_MIN(len, p->i_bufWritePos - p->i_current);

        if (data)
            memcpy(data, p->p_buff + p->i_current, ret);

        p->i_current    += ret;

        return ret;
    }

    static uint32_t BufferCtrlSize(BufferController_t *p)
    {
        return p->i_allocSize - p->i_current;
    }

    uint32_t BufferCtrlGetAllocSize(const BufferController_t *p)
    {
        return p->i_allocSize;
    }

    uint32_t BufferCtrlR8(BufferController_t *p)
    {
        uint32_t     val     = 0;
        if (!p || !p->p_buff)
            return 0;

        if (p->i_current < p->i_bufWritePos)
        {
            val = *(p->p_buff + p->i_current) & 0xff;
            p->i_current++;
        }

        return val;
    }

    uint32_t BufferCtrlRl16(BufferController_t *p)
    {
        uint32_t     val = 0;
        val = BufferCtrlR8(p);
        val |= BufferCtrlR8(p) << 8;
        return val;
    }

    uint32_t BufferCtrlRl32(BufferController_t *s)
    {
        uint32_t val;
        val = BufferCtrlRl16(s);
        val |= BufferCtrlRl16(s) << 16;

        return val;
    }

    uint64_t BufferCtrlRl64(BufferController_t *s)
    {
        uint64_t val;
        val = (uint64_t)BufferCtrlRl32(s);
        val |= (uint64_t)BufferCtrlRl32(s) << 32;
        return val;
    }

    uint32_t BufferCtrlRLe16(BufferController_t *p)
    {
        return BufferCtrlRl16(p);
    }

    uint32_t BufferCtrlRLe32(BufferController_t *p)
    {
        return BufferCtrlRl32(p);
    }

    uint64_t BufferCtrlRLe64(BufferController_t *p)
    {
        return BufferCtrlRl64(p);
    }

    uint32_t BufferCtrlRBe16(BufferController_t *p)
    {
        uint32_t     val = 0;
        val = BufferCtrlR8(p) << 8;
        val |= BufferCtrlR8(p);
        return val;
    }

    uint32_t BufferCtrlRBe32(BufferController_t *s)
    {
        uint32_t val;
        val = BufferCtrlRBe16(s) << 16;
        val |= BufferCtrlRBe16(s);

        return val;
    }

    uint64_t BufferCtrlRBe64(BufferController_t *s)
    {
        uint64_t val;
        val = (uint64_t)BufferCtrlRBe32(s) << 32;
        val |= (uint64_t)BufferCtrlRBe32(s);
        return val;
    }

    uint128_t BufferCtrlRBe128(BufferController_t *s)
    {
        uint128_t val;
        val.namedQwords.highQword = (uint64_t)BufferCtrlRBe64(s);
        val.namedQwords.lowQword |= (uint64_t)BufferCtrlRBe64(s);
        return val;
    }

    int BufferCtrlWriteRawDataU8(BufferController_t *p, uint8_t val)
    {
        return BufferCtrlW8(p, val);
    }
    int BufferCtrlWriteRawDataLeU16(BufferController_t *p, uint16_t val)
    {
        return BufferCtrlWLe16(p, val);
    }
    int BufferCtrlWriteRawDataLeU32(BufferController_t *p, uint32_t val)
    {
        return BufferCtrlWLe32(p, val);
    }
    int BufferCtrlWriteRawDataLeU64(BufferController_t *p, uint64_t val)
    {
        return BufferCtrlWLe64(p, val);
    }
    int BufferCtrlWriteRawDataLeU128(BufferController_t *p, const uint128_t &val)
    {
        return BufferCtrlWLe128(p, val);
    }
    int BufferCtrlWriteRawDataBeU16(BufferController_t *p, uint16_t val)
    {
        return BufferCtrlWBe16(p, val);
    }
    int BufferCtrlWriteRawDataBeU32(BufferController_t *p, uint32_t val)
    {
        return BufferCtrlWBe32(p, val);
    }
    int BufferCtrlWriteRawDataBeU64(BufferController_t *p, uint64_t val)
    {
        return BufferCtrlWBe64(p, val);
    }
    int BufferCtrlWriteRawDataBeU128(BufferController_t *p, const uint128_t &val)
    {
        return BufferCtrlWBe128(p, val);
    }

    int BufferCtrlReadRawDataU8(BufferController_t *p, uint8_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlR8(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }

    int BufferCtrlReadRawDataLeU16(BufferController_t *p, uint16_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRLe16(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataLeU32(BufferController_t *p, uint32_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRLe32(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataLeU64(BufferController_t *p, uint64_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRLe64(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataLeU128(BufferController_t *p, uint128_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        {
            val.namedQwords.lowQword = (uint64_t)BufferCtrlRLe64(p);
            val.namedQwords.highQword |= (uint64_t)BufferCtrlRLe64(p);
        }
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }

    int BufferCtrlReadRawDataBeU16(BufferController_t *p, uint16_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRBe16(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataBeU32(BufferController_t *p, uint32_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRBe32(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataBeU64(BufferController_t *p, uint64_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        val = BufferCtrlRBe64(p);
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }
    int BufferCtrlReadRawDataBeU128(BufferController_t *p, uint128_t &val)
    {
        int pos0 = BufferCtrlTellCurPos(p);
        {
            val.namedQwords.highQword = (uint64_t)BufferCtrlRBe64(p);
            val.namedQwords.lowQword |= (uint64_t)BufferCtrlRBe64(p);
        }
        int pos1 = BufferCtrlTellCurPos(p);
        return (pos1-pos0);
    }

    int BufferCtrlReadSkip(BufferController_t *p, int n)
    {
        int     ret     = 0;

        if (!p || !p->p_buff)
            return 0;

        ret = BFCTRL_MIN(n, p->i_bufWritePos - p->i_current);

        p->i_current += n;

        return ret;
    }

    int BufferCtrlAllocBuf(BufferController_t *p, int total)
    {
        if (p->b_externalBuffer)
        {
            return BUFFER_CTRL_ERROR_VAL(EPERM);
        }

        int allocSize = p->i_allocSize;
        if (p->i_allocSize < total) {
            allocSize    = (((total >> BUFF_BITS) + 1) << BUFF_BITS);
            p->p_buff   = (uint8_t *)BFCTRL_REALLOC(p->p_buff, allocSize);

            if (!p->p_buff)
            {
                BUFFER_CTRL_ERR_LOG_PRINT("no memory.s:%u\n", allocSize);
                p->i_current    = 0;
                p->i_bufWritePos = 0;
                p->i_allocSize = 0;
                return BUFFER_CTRL_ERROR_VAL(ENOMEM);
            }
            p->i_allocSize = allocSize;
        }

        return allocSize;
    }

    int BufferCtrlGetBufLen(const BufferController_t *p)
    {
        return p->i_bufWritePos;
    }

    int BufferCtrlReadBufLeftLen(const BufferController_t *p)
    {
        if (p->i_bufWritePos <= p->i_current)
        {
            return 0;
        }

        return (p->i_bufWritePos - p->i_current);
    }

    int BufferCtrlExtendSize(BufferController_t *p, uint32_t s)
    {
        int     total   = p->i_current + s;
        return BufferCtrlAllocBuf(p, total);
    }

    int BufferCtrlCompactGetBytesCountU8(uint8_t value)
    {
        return BufferCtrlCompactGetBytesCountU32((uint32_t)value);
    }

    int BufferCtrlCompactGetBytesCountU16(uint16_t value)
    {
        return BufferCtrlCompactGetBytesCountU32((uint32_t)value);
    }

    int BufferCtrlCompactGetBytesCountU32(uint32_t value)
    {
        if (value<(1<<7))
        {
            return 1;
        }
        else if (value<(1<<14))
        {
            return 2;
        }
        else if (value<(1<<21))
        {
            return 3;
        }
        else if (value<(1<<28))
        {
            return 4;
        }
        else
        {
            return 5;
        }
    }

    int BufferCtrlCompactGetBytesCountU64(uint64_t value)
    {
        if (value<(1<<7))
        {
            return 1;
        }
        else if (value<(1<<14))
        {
            return 2;
        }
        else if (value<(1<<21))
        {
            return 3;
        }
        else if (value<(1<<28))
        {
            return 4;
        }
        else if (value<(TVU_1LL<<35))
        {
            return 5;
        }
        else if (value<(TVU_1LL<<42))
        {
            return 6;
        }
        else if (value<(TVU_1LL<<49))
        {
            return 7;
        }
        else if (value<(TVU_1LL<<56))
        {
            return 8;
        }
        else if (value<(TVU_1LL<<63))
        {
            return 9;
        }
        else
        {
            return 10;
        }
    }

    int BufferCtrlCompactGetBytesCountVariant(const Variant& value)
    {
        Variant::ValueType valueType=value.GetType();
        int bytesNeeded=BufferCtrlCompactGetBytesCountU32((uint32)valueType);
        switch(valueType)
        {
        case Variant::CharType:
        case Variant::ByteType:
        case Variant::ShortType:
        case Variant::WordType:
        case Variant::Int32Type:
        case Variant::Uint32Type:
        case Variant::FloatType:
            bytesNeeded+=BufferCtrlCompactGetBytesCountU32(value.GetAsUint32());
            break;
        case Variant::Int64Type:
        case Variant::Uint64Type:
        case Variant::DoubleType:
            bytesNeeded+=BufferCtrlCompactGetBytesCountU64(value.GetAsUint64());
            break;
        case Variant::Uint128Type:
            bytesNeeded+=sizeof(uint128_t);
            break;
        case Variant::StringType:
            {
                const std::string&stringValue=value.GetAsString();
                uint32 stringLength=(uint32)stringValue.length();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU32((uint32)stringLength);
                bytesNeeded+=(int)stringLength;
                break;
            }
        case Variant::BytesType:
            {
                const Bytes&bytesValue=value.GetAsBytes();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU32((uint32)bytesValue.GetSize());
                bytesNeeded+=(int)bytesValue.GetSize();
                break;
            }
        default:
            break;
        }
        return bytesNeeded;
    }

    int BufferCtrlCompactEncodeValueU8(BufferController_t *p, uint8_t value)
    {
        return BufferCtrlCompactEncodeValueU32(p, (uint32_t)value);
    }

    int BufferCtrlCompactEncodeValueU16(BufferController_t *p, uint16_t value)
    {
        return BufferCtrlCompactEncodeValueU32(p, (uint32_t)value);
    }

    int BufferCtrlCompactEncodeValueU32(BufferController_t *p, uint32_t value)
	{
		int bytesNeeded=BufferCtrlCompactGetBytesCountU32(value);
        if (BufferCtrlExtendSize(p, bytesNeeded) < 0)
        {
            return -1;
        }

        if (bytesNeeded>16)
        {
            return -1;
        }

        uint8_t bytes[16];
        if (bytesNeeded<1)
        {
            bytesNeeded=1;
        }
        bytes[bytesNeeded-1]=(value&0x7F);
        value>>=7;
        if (bytesNeeded>1)
        {
            for (int i=bytesNeeded-2;i>=0;i--)
            {
                bytes[i]=(value&0x7F)|0x80;
                value>>=7;
            }
        }

        BufferCtrlPushData(p, bytes, bytesNeeded);
        return bytesNeeded;
	}

    int BufferCtrlCompactEncodeValueU64(BufferController_t *p, uint64_t value)
    {
        int bytesNeeded=BufferCtrlCompactGetBytesCountU64(value);
        if (BufferCtrlExtendSize(p, bytesNeeded) < 0)
        {
            return -1;
        }

        if (bytesNeeded>16)
        {
            return -1;
        }

        uint8_t bytes[16];
        if (bytesNeeded<1)
        {
            bytesNeeded=1;
        }
        bytes[bytesNeeded-1]=(value&0x7F);
        value>>=7;
        if (bytesNeeded>1)
        {
            for (int i=bytesNeeded-2;i>=0;i--)
            {
                bytes[i]=(value&0x7F)|0x80;
                value>>=7;
            }
        }

        FAILED_PUSH_DATA(BufferCtrlPushData(p, bytes, bytesNeeded));

        return bytesNeeded;
    }

    int BufferCtrlCompactDecodeValueU8(BufferController_t *p, uint8_t&valueRef)
    {
        unsigned int maxBytes=2;
        unsigned int value=0;
        int curpos = BufferCtrlTellCurPos(p);
        uint32_t size = BufferCtrlReadBufLeftLen(p);
        for (unsigned int i=0;i<maxBytes;i++)
        {
            if (size<=i)
            {
                BufferCtrlSeek(p, curpos, SEEK_SET);
                return 0; //more bytes expected.
            }
            uint8_t singleByte=BufferCtrlR8(p);
            value=(value<<7)|(singleByte&0x7F);
            if ((singleByte & 0x80)==0)//terminal
            {
                if (value>=0x100)
                {
                    BufferCtrlSeek(p, curpos, SEEK_SET);
                    return -1;
                }
                valueRef=(uint8_t)value; //may truncate.
                return i+1;
            }
        }
        BufferCtrlSeek(p, curpos, SEEK_SET);
        return -1; //the last byte should not start with highest bit 1.
    }

    int BufferCtrlCompactDecodeValueU16(BufferController_t *p, uint16_t&valueRef)
    {
        unsigned int maxBytes=3;
        unsigned int value=0;
        uint32_t size = BufferCtrlReadBufLeftLen(p);
        int curpos = BufferCtrlTellCurPos(p);
        for (unsigned int i=0;i<maxBytes;i++)
        {
            if (size<=i)
            {
                BufferCtrlSeek(p, curpos, SEEK_SET);
                return 0; //more bytes expected.
            }
            uint8_t singleByte=BufferCtrlR8(p);
            value=(value<<7)|(singleByte&0x7F);
            if ((singleByte & 0x80)==0)//terminal
            {
                if (value>=0x10000)
                {
                    BufferCtrlSeek(p, curpos, SEEK_SET);
                    return -1; //overflow.
                }
                valueRef=(unsigned short)value; //may truncate.
                return i+1;
            }
        }
        BufferCtrlSeek(p, curpos, SEEK_SET);
        return -1; //the last byte should not start with highest bit 1.
    }

    int BufferCtrlCompactDecodeValueU32(BufferController_t *p, uint32_t&valueRef)
    {
        unsigned int maxBytes=5; //32/7=4...4
        unsigned int value=0;
        int curpos = BufferCtrlTellCurPos(p);
        uint32_t size = BufferCtrlReadBufLeftLen(p);
        for (unsigned int i=0;i<maxBytes;i++)
        {
            if (size<=i)
            {
                BufferCtrlSeek(p, curpos, SEEK_SET);
                return 0; //more bytes expected.
            }
            uint8_t singleByte=BufferCtrlR8(p);

            if (i==maxBytes-1){
                if ((value>>(32-7))!=0)
                {
                    BufferCtrlSeek(p, curpos, SEEK_SET);
                    return -1; //overflow.
                }
            }
            value=(value<<7)|(singleByte&0x7F);
            if ((singleByte & 0x80)==0)//terminal
            {
                valueRef=value;
                return i+1;
            }
        }
        BufferCtrlSeek(p, curpos, SEEK_SET);
        return -1; //the last byte should not start with highest bit 1.
    }

    int BufferCtrlCompactDecodeValueU64(BufferController_t *p, uint64_t&valueRef)
    {
        unsigned int maxBytes=10; //64/7=9...1
        uint64_t value=0;
        int curpos = BufferCtrlTellCurPos(p);
        uint32_t size = BufferCtrlReadBufLeftLen(p);
        for (unsigned int i=0;i<maxBytes;i++)
        {
            if (size<=i)
            {
                BufferCtrlSeek(p, curpos, SEEK_SET);
                return 0; //more bytes expected.
            }
            uint8_t singleByte=BufferCtrlR8(p);

            if (i==maxBytes-1){
                if ((value>>(64-7))!=0)
                {
                    BufferCtrlSeek(p, curpos, SEEK_SET);
                    return -1; //overflow.
                }
            }
            value=(value<<7)|(singleByte&0x7F);
            if ((singleByte & 0x80)==0) //terminal
            {
                valueRef=value;
                return i+1;
            }
        }
        BufferCtrlSeek(p, curpos, SEEK_SET);
        return -1; //the last byte should not start with highest bit 1.
    }

    int BufferCtrlCompactEncodeVariant(BufferController_t *p, const Variant& value)
    {
        Variant::ValueType valueType=value.GetType();
        int bytesNeeded=BufferCtrlCompactGetBytesCountU32((uint32_t)valueType);
        int bytesEncoded=0;
        switch(valueType)
        {
        case Variant::NullType:
            FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32)valueType), bytesEncoded);
            break;
        case Variant::CharType:
        case Variant::ByteType:
        case Variant::ShortType:
        case Variant::WordType:
        case Variant::Int32Type:
        case Variant::Uint32Type:
        case Variant::FloatType:
            {
                uint32_t uint32Value=value.GetAsUint32();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU32(uint32Value);
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32_t)valueType), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, uint32Value), bytesEncoded);
                break;
            }
        case Variant::Int64Type:
        case Variant::Uint64Type:
        case Variant::DoubleType:
            {
                uint64 uint64Value=value.GetAsUint64();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU64(uint64Value);

                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p,(uint32)valueType), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU64(p, uint64Value), bytesEncoded);
                break;
            }
        case Variant::Uint128Type:
            {
                bytesNeeded+=sizeof(uint128_t);

                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p,(uint32)valueType), bytesEncoded);
                const Uint128&uint128Value=value.GetAsUint128();
                FAILED_WRITE_DATA_STEP(BufferCtrlWriteRawDataBeU128(p, uint128Value), bytesEncoded);
                break;
            }
        case Variant::StringType:
            {
                const std::string&stringValue=value.GetAsString();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU32(stringValue.length());
                bytesNeeded+=(int)stringValue.length();
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32)valueType), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32)stringValue.length()), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlWriteBinary(p, (const uint8_t *)stringValue.c_str(),(int)stringValue.length()), bytesEncoded);
                break;
            }
        case Variant::BytesType:
            {
                const Bytes&bytesValue=value.GetAsBytes();
                bytesNeeded+=BufferCtrlCompactGetBytesCountU32((uint32)bytesValue.GetSize());
                bytesNeeded+=bytesValue.GetSize();

                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32)valueType), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlCompactEncodeValueU32(p, (uint32)bytesValue.GetSize()), bytesEncoded);
                FAILED_WRITE_DATA_STEP(BufferCtrlWriteBinary(p, bytesValue.GetBufAddr(),bytesValue.GetSize()), bytesEncoded);
                break;
            }
        default:
            return -1;
        }
        return bytesNeeded;
    }

    int BufferCtrlCompactDecodeVariant(BufferController_t *p, Variant&valueRef, bool referenceOnly)
    {
        uint32_t size = BufferCtrlReadBufLeftLen(p);
        const uint8*bytes=(const uint8_t*)BufferCtrlGetCurPtr(p);
        int totalBytesDecoded=0;
        Variant::ValueType variantType;
        {
            uint32 decodedVariantType=0;
            int bytesDecoded=BufferCtrlCompactDecodeValueU32(p, decodedVariantType);
            if (bytesDecoded<=0)
            {
                return bytesDecoded;
            }
            variantType=(Variant::ValueType)decodedVariantType;
            bytes+=bytesDecoded;
            size-=(uint32_t)bytesDecoded;
            totalBytesDecoded+=bytesDecoded;
        }
        switch(variantType)
        {
        case Variant::NullType:
            valueRef.Clear();
            break;
        case Variant::CharType:
        case Variant::ByteType:
        case Variant::ShortType:
        case Variant::WordType:
        case Variant::Int32Type:
        case Variant::Uint32Type:
        case Variant::FloatType:
            {
                uint32 decodedValue;
                int bytesDecoded=BufferCtrlCompactDecodeValueU32(p, decodedValue);
                if (bytesDecoded<=0)
                {
                    return bytesDecoded;
                }
                valueRef.SetAsUint32(decodedValue,variantType);
                totalBytesDecoded+=bytesDecoded;
            }
            break;
        case Variant::Int64Type:
        case Variant::Uint64Type:
        case Variant::DoubleType:
            {
                uint64 decodedValue;
                int bytesDecoded=BufferCtrlCompactDecodeValueU64(p, decodedValue);
                if (bytesDecoded<=0)
                {
                    return bytesDecoded;
                }
                valueRef.SetAsUint64(decodedValue,variantType);
                totalBytesDecoded+=bytesDecoded;
            }
            break;
        case Variant::Uint128Type:
            {
                Uint128 decodedValue;
                int bytesDecoded=BufferCtrlReadRawDataBeU128(p, decodedValue);
                if (bytesDecoded<=0)
                {
                    return bytesDecoded;
                }
                valueRef.SetAsUint128(decodedValue);
                totalBytesDecoded+=bytesDecoded;
            }
            break;
        case Variant::StringType:
            {
                uint32_t decodedStringLength;
                int bytesDecoded=BufferCtrlCompactDecodeValueU32(p, decodedStringLength);
                if (bytesDecoded<=0)
                {
                    return bytesDecoded;
                }
                size-=(uint32_t)bytesDecoded;
                if (size<decodedStringLength)
                {
                    return 0;
                }
                bytes+=bytesDecoded;
                totalBytesDecoded+=bytesDecoded;
                valueRef.SetAsString((const char*)bytes,decodedStringLength);
                totalBytesDecoded+=(int)decodedStringLength;
                BufferCtrlReadSkip(p, decodedStringLength);
            }
            break;
        case Variant::BytesType:
            {
                uint32 decodedBytesNum;
                int bytesDecoded=BufferCtrlCompactDecodeValueU32(p, decodedBytesNum);
                if (bytesDecoded<=0)
                {
                    return bytesDecoded;
                }
                size-=(uint32_t)bytesDecoded;
                if (size<decodedBytesNum)
                {
                    return 0;
                }
                bytes+=bytesDecoded;
                totalBytesDecoded+=bytesDecoded;
                if (referenceOnly)
                {
                    valueRef.SetAsBytesByReference((byte*)bytes,decodedBytesNum);
                }
                else
                {
                    valueRef.SetAsBytes(bytes,decodedBytesNum);
                }
                totalBytesDecoded+=(int)decodedBytesNum;
                BufferCtrlReadSkip(p, decodedBytesNum);
            }
            break;
        default:
            return -1;
        }
        return totalBytesDecoded;
    }

}
