/******************************************************************************
 *  Description:
 *      tvu fourcc definition.
 *  History:
 *      Lotus/TVU, initializing on Nov 14th, 2018.
 *  CopyRight:
 *      TVU Ltm co.
 *****************************************************************************/

#ifndef _TVU_FOURCC_H
#define _TVU_FOURCC_H

#include "libtvu_media_fourcc.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct STvuPixFmtFourccMap
{
    unsigned int  pix_fmt;
    unsigned int  fourcc;
}tvu_pixfmt_fourcc_map_t;


const struct STvuPixFmtFourccMap *TvuFourccPixfmtTableFindPixfmtByFourcc(const struct STvuPixFmtFourccMap *ptable, unsigned int n, unsigned int fourcc);
const struct STvuPixFmtFourccMap *TvuFourccPixfmtTableFindFourccByPixfmt(const struct STvuPixFmtFourccMap *ptable, unsigned int n, unsigned int pixfmt);

#ifdef __cplusplus
}
#endif

#endif

