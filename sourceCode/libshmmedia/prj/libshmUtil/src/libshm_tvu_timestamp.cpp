/************************************************************************
 *  File:
 *      tvu_timecode.cpp
 *  Description:
 *      timecode transferring.
 *  Author:
 *      Lotus.
************************************************************************/
#include "libshm_tvu_timestamp.h"

#define FPS_INDEX_VALUE_MASK      ((uint64_t)(0x00FFFFFFFFFFFFFF))

namespace tvushm
{
    class CEndian
    {
    public:
        static bool isLE()
        {
            if (_sbEndian == kUnknown)
            {
                const uint32_t endian_val = 0x12345678;
                const uint8_t  *pEndian = (const uint8_t*)&endian_val;
                if (0x78 == pEndian[0])
                {
                    _sbEndian = kLittleEndian;
                }
                else
                {
                    _sbEndian = kBigEndian;
                }
            }

            return (_sbEndian == kLittleEndian);
        }
    protected:
        enum Edian
        {
            kUnknown = 0,
            kLittleEndian = 1,
            kBigEndian = 2,
        };
    private:
        static int _sbEndian;
    };


    class CTimeCode
    {
    public:
        CTimeCode()
        {
            _fpsVal = 0;
            _indexVal = 0;
            _timecodeValue = 0;
        }
        static uint64_t transfer(uint64_t src, int dst_fps)
        {
            uint64_t dst = 0;
            uint64_t src_index = 0;
            uint64_t dst_index = 0;
            int counts = sizeof(_sTimecodeIndex) / sizeof(_sTimecodeIndex[0]);

            int src_fps = (src >> 56) & 0xFF;
            if (dst_fps >= counts || src_fps >= counts)
            {
                return TVU_TIMECODE_INVALID_VALUE;
            }

            if (dst_fps == src_fps)
                return src;

            int src_scale = _sTimecodeIndex[src_fps]._scale;
            int src_step = _sTimecodeIndex[src_fps]._step;
            int dst_scale = _sTimecodeIndex[dst_fps]._scale;
            int dst_step = _sTimecodeIndex[dst_fps]._step;
            uint64_t ratio1 = (uint64_t)src_step * dst_scale;
            uint64_t ratio2 = (uint64_t)src_scale * dst_step;

            src_index = src & FPS_INDEX_VALUE_MASK;

            dst_index = (uint64_t)(1.0* src_index / ratio2 * ratio1) & FPS_INDEX_VALUE_MASK;

            dst = dst_index | ((uint64_t)(dst_fps & 0xFF) << 56);
            return dst;
        }

        static int compare2(uint64_t t1, uint64_t t2)
        {
            if (t1 < t2)
                return -1;
            if (t1 > t2)
                return 1;
            return 0;
        }

        static int compare(uint64_t src, uint64_t dst)
        {
            uint64_t t1 = transfer(src, 0);
            uint64_t t2 = transfer(dst, 0);

            return compare2(t1, t2);
        }

        static int64_t Minus(uint64_t src, uint64_t dst)
        {
            uint64_t t1 = transfer(src, 0);
            uint64_t t2 = transfer(dst, 0);

            return (int64_t)(t1 - t2);
        }

        static bool isValid(uint64_t t)
        {
            int fpsv = (t >> 56) & 0xFF;
            int counts = sizeof(_sTimecodeIndex) / sizeof(_sTimecodeIndex[0]);
            if (fpsv >= counts)
            {
                return false;
            }
            return true;
        }

        static const struct STvuFpsPair *getFpsPairNode(int fpsvalue)
        {
            int pair_num = sizeof(_sTimecodeIndex) / sizeof(_sTimecodeIndex[0]);
            if (fpsvalue >= pair_num)
            {
                return NULL;
            }
            return &_sTimecodeIndex[fpsvalue];
        }
        static const int getFpsValue(const struct STvuFpsPair *node)
        {
            int pair_num = sizeof(_sTimecodeIndex) / sizeof(_sTimecodeIndex[0]);
            int fpsvalue = -1;

            if (!node)
                return -1;

            for (int i = 0; i < pair_num; i++)
            {
                if (_sTimecodeIndex[i]._step == node ->_step && _sTimecodeIndex[i]._scale == node->_scale)
                {
                    fpsvalue = i;
                    break;
                }
            }
            return fpsvalue;
        }
    private:
        uint8_t     _fpsVal;
        uint64_t    _indexVal;
        uint64_t    _timecodeValue;
        static const struct STvuFpsPair _sTimecodeIndex[TVU_FPS_KEY_MAX_NUM];
    };

    int CEndian::_sbEndian = CEndian::kUnknown;
#if defined(_WIN32)
    const struct STvuFpsPair CTimeCode::_sTimecodeIndex[TVU_FPS_KEY_MAX_NUM] = {
        { 1, 1000 }
        , { 1000, 10000 }
        , { 1000, 15000 }
        , { 1000, 20000 }
        , { 1001, 24000 }
        , { 1000, 24000 }
        , { 1000, 25000 }
        , { 1001, 30000 }
        , { 1000, 30000 }
        , { 1000, 50000 }
        , { 1001, 60000 }
        , { 1000, 60000 }
        , { 1, 44100 }
        , { 1, 48000 }
        , { 1, 90000 }
        , { 1, 270000 }
        , { 1, 1000000 }
        , { 1000, 120000 }
        , { 1000, 240000 }
    };
#else
    const struct STvuFpsPair CTimeCode::_sTimecodeIndex[TVU_FPS_KEY_MAX_NUM] = {
            [TVU_FPS_KEY_MILLISEC] = {1, 1000 }
            ,[TVU_FPS_KEY_10] = {1000, 10000 }
            ,[TVU_FPS_KEY_15] = {1000, 15000 }
            ,[TVU_FPS_KEY_20] = {1000, 20000 }
            ,[TVU_FPS_KEY_2398] = {1001, 24000 }
            ,[TVU_FPS_KEY_24] = {1000, 24000 }
            ,[TVU_FPS_KEY_25] = {1000, 25000 }
            ,[TVU_FPS_KEY_2997] = {1001, 30000 }
            ,[TVU_FPS_KEY_30] = {1000, 30000 }
            ,[TVU_FPS_KEY_50] = {1000, 50000 }
            ,[TVU_FPS_KEY_5994] = {1001, 60000 }
            ,[TVU_FPS_KEY_60] = {1000, 60000 }
            ,[TVU_FPS_KEY_44100] = {1, 44100 }
            ,[TVU_FPS_KEY_48000] = {1, 48000 }
            ,[TVU_FPS_KEY_90K] = {1, 90000 }
            ,[TVU_FPS_KEY_270K] = {1, 270000 }
            ,[TVU_FPS_KEY_MICROSEC] = {1, 1000000 }
            ,[TVU_FPS_KEY_120] = {1000, 120000 }
            ,[TVU_FPS_KEY_240] = {1000, 240000 }
        };
#endif
}//namespace tvu


uint64_t LibshmutilTvutimestampTransfer(uint64_t timecode, int dst_fps)
{
    return tvushm::CTimeCode::transfer(timecode, dst_fps);
}

int LibshmutilTvutimestampCompare(uint64_t t1, uint64_t t2)
{
    return tvushm::CTimeCode::compare(t1, t2);
}

int64_t LibshmutilTvutimestampMinusWithMS(uint64_t t1, uint64_t t2)
{
    return tvushm::CTimeCode::Minus(t1, t2);
}

bool LibshmutilTvutimestampValid(uint64_t t)
{
    return tvushm::CTimeCode::isValid(t);
}

uint64_t LibshmutilTvutimestampMerge(uint64_t indexV, int fpsV)
{
    uint64_t fps64 = fpsV;

    const struct STvuFpsPair *pNode = LibshmutilTvutimestampFpspairGetNode(fpsV);

    if (!pNode)
    {
        return TVU_TIMECODE_INVALID_VALUE;
    }

    return ((indexV & 0xFFFFFFFFFFFFFF) | (fps64 << 56));
}

uint64_t LibshmutilTvutimestampCreate(uint64_t indexV, const struct STvuFpsPair *pNode)
{
    int fpsV = LibshmutilTvutimestampFpspairGetValue(pNode);

    if (fpsV < 0)
    {
        return TVU_TIMECODE_INVALID_VALUE;
    }

    return LibshmutilTvutimestampMerge(indexV, fpsV);
}

uint64_t LibshmutilTvutimestampGetIndexValue(uint64_t src)
{
    return (src & 0xFFFFFFFFFFFFFF);
}

uint8_t LibshmutilTvutimestampGetFpsValue(uint64_t src)
{
    return (src >> 56) & 0xFF;
}

void LibshmutilTvutimestampSetBinary(uint64_t src, uint8_t bina[8])
{
    int i = 0;
    bool bLittleEndian = tvushm::CEndian::isLE();

    if (bLittleEndian)
    {
        uint64_t *pbin = (uint64_t *)bina;
        *pbin = src;
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            bina[i] = (src >> (8 * i)) & 0xFF;
        }
    }

    return;
}

uint64_t LibshmutilTvutimestampParseBinary(const uint8_t bina[8])
{
    bool bLittleEndian = tvushm::CEndian::isLE();
    uint64_t src = 0;

    if (bLittleEndian)
    {
        const uint64_t *pbin = (const uint64_t *)bina;
        src = *pbin;
    }
    else
    {

        int rpos = 0;
        src = bina[rpos]
            | ((uint64_t)bina[rpos + 1] << 8)
            | ((uint64_t)bina[rpos + 2] << 16)
            | ((uint64_t)bina[rpos + 3] << 24)
            | ((uint64_t)bina[rpos + 4] << 32)
            | ((uint64_t)bina[rpos + 5] << 40)
            | ((uint64_t)bina[rpos + 6] << 48)
            | ((uint64_t)bina[rpos + 7] << 56);
    }

    return src;
}

const struct STvuFpsPair *LibshmutilTvutimestampFpspairGetNode(int fpsvalue)
{
    return tvushm::CTimeCode::getFpsPairNode(fpsvalue);
}

int LibshmutilTvutimestampFpspairGetValue(const struct STvuFpsPair *node)
{
    return tvushm::CTimeCode::getFpsValue(node);
}

