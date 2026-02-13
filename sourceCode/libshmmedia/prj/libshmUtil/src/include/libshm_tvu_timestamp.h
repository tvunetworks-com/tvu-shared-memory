/************************************************************************
 *  File:
 *      tvu_timecode.h
 *  Description:
 *      timecode transferring declaration
 *  Author:
 *      Lotus.
************************************************************************/
#ifndef LIBSHM_TVU_TIMESTAMP_H
#define LIBSHM_TVU_TIMESTAMP_H

#include <stdint.h>
#include <stdio.h>

enum ETvuRFPS
{
    TVU_FPS_KEY_MILLISEC  =  0,
    TVU_FPS_KEY_10        =  1,
    TVU_FPS_KEY_15        =  2,
    TVU_FPS_KEY_20        =  3,
    TVU_FPS_KEY_2398      =  4,
    TVU_FPS_KEY_24        =  5,
    TVU_FPS_KEY_25        =  6,
    TVU_FPS_KEY_2997      =  7,
    TVU_FPS_KEY_30        =  8,
    TVU_FPS_KEY_50        =  9,
    TVU_FPS_KEY_5994      =  10,
    TVU_FPS_KEY_60        =  11,
    TVU_FPS_KEY_44100     =  12,
    TVU_FPS_KEY_48000     =  13,
    TVU_FPS_KEY_90K       =  14,
    TVU_FPS_KEY_270K      =  15,
    TVU_FPS_KEY_MICROSEC  =  16,
    TVU_FPS_KEY_120       =  17,
    TVU_FPS_KEY_240       =  18,
    TVU_FPS_KEY_MAX_NUM   =  19
};

#define TVU_TIMECODE_INVALID_VALUE ((uint64_t)(-1))

typedef struct STvuFpsPair
{
    int     _step;
    int     _scale;
}tvu_fps_pair_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  Functionality:
 *      used to transfer timecode from one scale to another scale.
 *  Parameters:
 *      @src_timecode, [IN] src timecode value
 *      @dstFpsVal, [IN] dst fps value.
 *  Return:
 *      the dst timecode value.
 *      -1(TVU_TIMECODE_INVALID_VALUE) : error, unsupported fps value
**/
uint64_t LibshmutilTvutimestampTransfer(uint64_t src_timecode, int dstFpsVal);

/**
 *  Functionality:
 *      used to compare two tvutimestamp value.
 *  Parameters:
 *      @t1, [IN] the 1st value.
 *      @t2, [IN] the 2nd value.
 *  Return:
 *      > 0
 *      = 0
 *      < 0
**/
int LibshmutilTvutimestampCompare(uint64_t t1, uint64_t t2);

/**
 *  Functionality:
 *      two tvutimestamp value minus operation.
 *  Parameters:
 *      @t1, [IN] the 1st value.
 *      @t2, [IN] the 2nd value.
 *  Return:
 *      the micro seconds value of the minus result.
**/
int64_t LibshmutilTvutimestampMinusWithMS(uint64_t t1, uint64_t t2);

/**
 *  Functionality:
 *      used to check the validation of tvutimestamp.
 *  Parameters:
 *      @t, [IN] tvutimestamp value.
 *  Return:
 *      false: invalid
 *      true:valid
**/
bool LibshmutilTvutimestampValid(uint64_t t);

/**
 *  Functionality:
 *      used to merge index value and fps value to create the timecode.
 *  Parameters:
 *      @indexValue, [IN] index value
 *      @fpsValue, [IN] fps value.
 *  Return:
 *      the timecode value.
 *      -1(TVU_TIMECODE_INVALID_VALUE) : error, unsupported fpsValue
**/
uint64_t LibshmutilTvutimestampMerge(uint64_t indexValue, int fpsValue);

/**
 *  Functionality:
 *      used to create the timecode according to index value and fps pair information ptr.
 *  Parameters:
 *      @indexValue, [IN] index value
 *      @pNode, [IN] fps pair node information point.
 *  Return:
 *      the timecode value.
 *      -1(TVU_TIMECODE_INVALID_VALUE) : error, unsupported fpsValue
**/
uint64_t LibshmutilTvutimestampCreate(uint64_t indexV, const struct STvuFpsPair *pNode);

/**
 *  Functionality:
 *      used to get timecode index value.
 *  Parameters:
 *      @timecode, [IN] timecode value
 *  Return:
 *      timecode index value.
**/
uint64_t LibshmutilTvutimestampGetIndexValue(uint64_t timecode);

/**
 *  Functionality:
 *      used to get fps value.
 *  Parameters:
 *      @timecode, [IN] timecode value
 *  Return:
 *      fps value.
**/
uint8_t LibshmutilTvutimestampGetFpsValue(uint64_t timecode);

/**
 *  Functionality:
 *      used to create the binary of the timecode.
 *  Parameters:
 *      @timecode, [IN] timecode value
 *      @bina, [OUT] binary array for store, little endian.
 *  Return:
 *
**/
void LibshmutilTvutimestampSetBinary(uint64_t timecode, uint8_t bina[8]);

/**
 *  Functionality:
 *      used to parse binary to timecode value.
 *  Parameters:
 *      @bina, [IN] binary data, little endian.
 *  Return:
 *      timecode value
**/
uint64_t LibshmutilTvutimestampParseBinary(const uint8_t bina[8]);

/**
 *  Functionality:
 *      used to get fps pair node according to fpsvalue.
 *  Parameters:
 *      @fpsvalue, [IN] fps value
 *  Return:
 *      the fps pair node point.
 *      NULL, means unsupported fpsvalue.
**/
const struct STvuFpsPair *LibshmutilTvutimestampFpspairGetNode(int fpsvalue);

/**
 *  Functionality:
 *      used to get fps value according to fps pair node point.
 *  Parameters:
 *      @pnode, [IN] fps pair node point
 *  Return:
 *      the fps value.
 *      < 0, means unspport fps pair node.
**/
int LibshmutilTvutimestampFpspairGetValue(const struct STvuFpsPair *pnode);

#ifdef __cplusplus
}
#endif

#endif // LIBSHM_TVU_TIMESTAMP_H
