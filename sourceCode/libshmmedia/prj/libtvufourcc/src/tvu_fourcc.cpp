/******************************************************************************
 *  Description:
 *      tvu fourcc and ffmpeg's pix format mapping accomplishment.
 *  History:
 *      Lotus/TVU, initializing on Nov 14th, 2018.
 *  CopyRight:
 *      TVU Ltm co.
 *****************************************************************************/
#include "tvu_fourcc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//struct SPrivateTableContext
//{
//    int   i_table_num;
//    tvu_pixfmt_fourcc_map_t *p_table;
//};

//struct SPrivateTableContext g_video_chroma_table = {0, NULL};

const struct STvuPixFmtFourccMap *TvuFourccPixfmtTableFindPixfmtByFourcc(const struct STvuPixFmtFourccMap *p_table, unsigned int nArr, unsigned int fourcc)
{
    const struct STvuPixFmtFourccMap *ptmp = NULL;

    for (unsigned int i = 0; i < nArr; i++)
    {
        if (p_table[i].fourcc == fourcc)
        {
            ptmp    = &p_table[i];
            break;
        }
    }

    return ptmp;
}

const struct STvuPixFmtFourccMap *TvuFourccPixfmtTableFindFourccByPixfmt(const struct STvuPixFmtFourccMap *p_table, unsigned int nArr, unsigned int pixfmt)
{
    const struct STvuPixFmtFourccMap *ptmp = NULL;

    for (unsigned int i = 0; i < nArr; i++)
    {
        if (p_table[i].pix_fmt == pixfmt)
        {
            ptmp    = &p_table[i];
            break;
        }
    }

    return ptmp;
}

#if 0
static const tvu_pixfmt_fourcc_map_t g_video_chroma_table[] =
{
    { AV_PIX_FMT_YUV420P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P                       /*_TVU_LE_FOURCCTAG('I', '4', '2', '0'),                map to AV_PIX_FMT_YUV420P */},
    { AV_PIX_FMT_YUYV422,              K_TVU_PIXFMT_VIDEO_FOURCC_YUYV422                       /*_TVU_LE_FOURCCTAG('V', '4', '2', '2'),                map to AV_PIX_FMT_YUYV422 */},
    { AV_PIX_FMT_RGB24,                K_TVU_PIXFMT_VIDEO_FOURCC_RGB24                         /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 24 ),                map to AV_PIX_FMT_RGB24 */},
    { AV_PIX_FMT_BGR24,                K_TVU_PIXFMT_VIDEO_FOURCC_BGR24                         /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 24 ),                map to AV_PIX_FMT_BGR24 */},
    { AV_PIX_FMT_YUV422P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P                       /*_TVU_LE_FOURCCTAG('I', '4', '2', '2'),                map to AV_PIX_FMT_YUV422P */},
    { AV_PIX_FMT_YUV444P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P                       /*_TVU_LE_FOURCCTAG('I', '4', '4', '4'),                map to AV_PIX_FMT_YUV444P */},
    { AV_PIX_FMT_YUV410P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV410P                       /*_TVU_LE_FOURCCTAG('I', '4', '1', '0'),                map to AV_PIX_FMT_YUV410P */},
    { AV_PIX_FMT_YUV411P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV411P                       /*_TVU_LE_FOURCCTAG('I', '4', '1', '1'),                map to AV_PIX_FMT_YUV411P */},
    { AV_PIX_FMT_GRAY8,                K_TVU_PIXFMT_VIDEO_FOURCC_GRAY8                         /*_TVU_LE_FOURCCTAG('G', 'R', 'E', 'Y'),                map to AV_PIX_FMT_GRAY8 */},
    { AV_PIX_FMT_MONOWHITE,            K_TVU_PIXFMT_VIDEO_FOURCC_MONOWHITE                     /*_TVU_LE_FOURCCTAG('B', '1', 'W', '0'),                map to AV_PIX_FMT_MONOWHITE */},
    { AV_PIX_FMT_MONOBLACK,            K_TVU_PIXFMT_VIDEO_FOURCC_MONOBLACK                     /*_TVU_LE_FOURCCTAG('B', '0', 'W', '1'),                map to AV_PIX_FMT_MONOBLACK */},
    { AV_PIX_FMT_PAL8,                 K_TVU_PIXFMT_VIDEO_FOURCC_PAL8                          /*_TVU_LE_FOURCCTAG('P', 'A', 'L', '8'),                map to AV_PIX_FMT_PAL8 */},
    { AV_PIX_FMT_YUVJ420P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ420P                      /*_TVU_LE_FOURCCTAG('J', '4', '2', '0'),                map to AV_PIX_FMT_YUVJ420P */},
    { AV_PIX_FMT_YUVJ422P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ422P                      /*_TVU_LE_FOURCCTAG('J', '4', '2', '2'),                map to AV_PIX_FMT_YUVJ422P */},
    { AV_PIX_FMT_YUVJ444P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ444P                      /*_TVU_LE_FOURCCTAG('J', '4', '4', '4'),                map to AV_PIX_FMT_YUVJ444P */},
    //{PIX_FMT_XVMC_MPEG2_MC,          K_TVU_PIXFMT_VIDEO_FOURCC_XVMC_MPEG2_MC                 /*_TVU_LE_FOURCCTAG('v', 'm', 'c', 'm'),                map to AV_PIX_FMT_XVMC_MPEG2_MC */},
    //{PIX_FMT_XVMC_MPEG2_IDCT,        K_TVU_PIXFMT_VIDEO_FOURCC_XVMC_MPEG2_IDCT               /*_TVU_LE_FOURCCTAG('v', 'm', 'c', 'i'),                map to AV_PIX_FMT_XVMC_MPEG2_IDCT */},
    { AV_PIX_FMT_UYVY422,              K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422                       /*_TVU_LE_FOURCCTAG('U', 'Y', 'V', 'Y'),                map to AV_PIX_FMT_UYVY422 */},
    { AV_PIX_FMT_UYYVYY411,            K_TVU_PIXFMT_VIDEO_FOURCC_UYYVYY411                     /*_TVU_LE_FOURCCTAG('Y', '4', '1', '1'),                map to AV_PIX_FMT_UYYVYY411 */},
    { AV_PIX_FMT_BGR8,                 K_TVU_PIXFMT_VIDEO_FOURCC_BGR8                          /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 8  ),                map to AV_PIX_FMT_BGR8 */},
    { AV_PIX_FMT_BGR4,                 K_TVU_PIXFMT_VIDEO_FOURCC_BGR4                          /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 4  ),                map to AV_PIX_FMT_BGR4 */},
    { AV_PIX_FMT_BGR4_BYTE,            K_TVU_PIXFMT_VIDEO_FOURCC_BGR4_BYTE                     /*_TVU_LE_FOURCCTAG('R', '4', 'B', 'Y'),                map to AV_PIX_FMT_BGR4_BYTE */},
    { AV_PIX_FMT_RGB8,                 K_TVU_PIXFMT_VIDEO_FOURCC_RGB8                          /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 8  ),                map to AV_PIX_FMT_RGB8 */},
    { AV_PIX_FMT_RGB4,                 K_TVU_PIXFMT_VIDEO_FOURCC_RGB4                          /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 4  ),                map to AV_PIX_FMT_RGB4 */},
    { AV_PIX_FMT_RGB4_BYTE,            K_TVU_PIXFMT_VIDEO_FOURCC_RGB4_BYTE                     /*_TVU_LE_FOURCCTAG('B', '4', 'B', 'Y'),                map to AV_PIX_FMT_RGB4_BYTE */},
    { AV_PIX_FMT_NV12,                 K_TVU_PIXFMT_VIDEO_FOURCC_NV12                          /*_TVU_LE_FOURCCTAG('N', 'V', '1', '2'),                map to AV_PIX_FMT_NV12 */},
    { AV_PIX_FMT_NV21,                 K_TVU_PIXFMT_VIDEO_FOURCC_NV21                          /*_TVU_LE_FOURCCTAG('N', 'V', '2', '1'),                map to AV_PIX_FMT_NV21 */},
    { AV_PIX_FMT_P010,                 K_TVU_PIXFMT_VIDEO_FOURCC_P010LE                        /*_TVU_LE_FOURCCTAG('P', '0', '1' , '0'),               map to AV_PIX_FMT_P010 */},
    { AV_PIX_FMT_ARGB,                 K_TVU_PIXFMT_VIDEO_FOURCC_ARGB                          /*_TVU_LE_FOURCCTAG('A', 'R', 'G', 'B'),                map to AV_PIX_FMT_ARGB */},
    { AV_PIX_FMT_RGBA,                 K_TVU_PIXFMT_VIDEO_FOURCC_RGBA                          /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 'A'),                map to AV_PIX_FMT_RGBA */},
    { AV_PIX_FMT_ABGR,                 K_TVU_PIXFMT_VIDEO_FOURCC_ABGR                          /*_TVU_LE_FOURCCTAG('A', 'B', 'G', 'R'),                map to AV_PIX_FMT_ABGR */},
    { AV_PIX_FMT_BGRA,                 K_TVU_PIXFMT_VIDEO_FOURCC_BGRA                          /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 'A'),                map to AV_PIX_FMT_BGRA */},
    { AV_PIX_FMT_GRAY16BE,             K_TVU_PIXFMT_VIDEO_FOURCC_GRAY16BE                      /*_TVU_LE_FOURCCTAG(16,  0, '1', 'Y'),                  map to AV_PIX_FMT_GRAY16BE */},
    { AV_PIX_FMT_GRAY16LE,             K_TVU_PIXFMT_VIDEO_FOURCC_GRAY16LE                      /*_TVU_LE_FOURCCTAG('Y', '1', 0 , 16 ),                 map to AV_PIX_FMT_GRAY16LE */},
    { AV_PIX_FMT_YUV440P,              K_TVU_PIXFMT_VIDEO_FOURCC_YUV440P                       /*_TVU_LE_FOURCCTAG('I', '4', '4', '0'),                map to AV_PIX_FMT_YUV440P */},
    { AV_PIX_FMT_YUVJ440P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ440P                      /*_TVU_LE_FOURCCTAG('J', '4', '4', '0'),                map to AV_PIX_FMT_YUVJ440P */},
    { AV_PIX_FMT_YUVA420P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVA420P                      /*_TVU_LE_FOURCCTAG('Y', '4', 11,  8 ),                 map to AV_PIX_FMT_YUVA420P */},
   //{AV_PIX_FMT_VDPAU_H264,          // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_H264                  /* _TVU_LE_FOURCCTAG('', '', '', ''),                   map to AV_PIX_FMT_VDPAU_H264 */},
   //{AV_PIX_FMT_VDPAU_MPEG1,         // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG1                 /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VDPAU_MPEG1 */},
   //{AV_PIX_FMT_VDPAU_MPEG2,         // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG2                 /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VDPAU_MPEG2 */},
   //{AV_PIX_FMT_VDPAU_WMV3,          // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_WMV3                  /* _TVU_LE_FOURCCTAG('', '', '', ''),                   map to AV_PIX_FMT_VDPAU_WMV3 */},
   //{AV_PIX_FMT_VDPAU_VC1,           // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_VC1                   /* _TVU_LE_FOURCCTAG('', '', '', ''),                   map to AV_PIX_FMT_VDPAU_VC1 */},
    { AV_PIX_FMT_RGB48BE,              K_TVU_PIXFMT_VIDEO_FOURCC_RGB48BE                       /*_TVU_LE_FOURCCTAG(48, 'B', 'G', 'R'),                 map to AV_PIX_FMT_RGB48BE */},
    { AV_PIX_FMT_RGB48LE,              K_TVU_PIXFMT_VIDEO_FOURCC_RGB48LE                       /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 48),                 map to AV_PIX_FMT_RGB48LE */},
    { AV_PIX_FMT_RGB565BE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB565BE                      /*_TVU_LE_FOURCCTAG(16, 'B', 'G', 'R'),                 map to AV_PIX_FMT_RGB565BE */},
    { AV_PIX_FMT_RGB565LE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB565LE                      /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 16 ),                map to AV_PIX_FMT_RGB565LE */},
    { AV_PIX_FMT_RGB555BE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB555BE                      /*_TVU_LE_FOURCCTAG(15, 'B', 'G', 'R'),                 map to AV_PIX_FMT_RGB555BE */},
    { AV_PIX_FMT_RGB555LE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB555LE                      /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 15 ),                map to AV_PIX_FMT_RGB555LE */},
    { AV_PIX_FMT_BGR565BE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR565BE                      /*_TVU_LE_FOURCCTAG(16, 'R', 'G', 'B'),                 map to AV_PIX_FMT_BGR565BE */},
    { AV_PIX_FMT_BGR565LE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR565LE                      /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 16 ),                map to AV_PIX_FMT_BGR565LE */},
    { AV_PIX_FMT_BGR555BE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR555BE                      /*_TVU_LE_FOURCCTAG(15, 'R', 'G', 'B'),                 map to AV_PIX_FMT_BGR555BE */},
    { AV_PIX_FMT_BGR555LE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR555LE                      /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 15 ),                map to AV_PIX_FMT_BGR555LE */},
    //{AV_PIX_FMT_VAAPI_MOCO,          // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_MOCO                 /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VAAPI_MOCO */},
    //{AV_PIX_FMT_VAAPI_IDCT,          // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_IDCT                 /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VAAPI_IDCT */},
    //{AV_PIX_FMT_VAAPI_VLD,           // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_VLD                  /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VAAPI_VLD */},
    { AV_PIX_FMT_YUV420P16LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P16LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 11, 16 ),                 map to AV_PIX_FMT_YUV420P16LE */},
    { AV_PIX_FMT_YUV420P16BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P16BE                   /*_TVU_LE_FOURCCTAG(16, 11, '3', 'Y'),                  map to AV_PIX_FMT_YUV420P16BE */},
    { AV_PIX_FMT_YUV422P16LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P16LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 10, 16 ),                 map to AV_PIX_FMT_YUV422P16LE */},
    { AV_PIX_FMT_YUV422P16BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P16BE                   /*_TVU_LE_FOURCCTAG(16, 10, '3', 'Y'),                  map to AV_PIX_FMT_YUV422P16BE */},
    { AV_PIX_FMT_YUV444P16LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P16LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 0 , 16 ),                 map to AV_PIX_FMT_YUV444P16LE */},
    { AV_PIX_FMT_YUV444P16BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P16BE                   /*_TVU_LE_FOURCCTAG(16, 0 , '3', 'Y'),                  map to AV_PIX_FMT_YUV444P16BE */},
    //{AV_PIX_FMT_VDPAU_MPEG4,         // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG4                /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VDPAU_MPEG4 */},
    //{AV_PIX_FMT_DXVA2_VLD,           // K_TVU_PIXFMT_VIDEO_FOURCC_DXVA2_VLD                  /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_DXVA2_VLD */},
    { AV_PIX_FMT_RGB444LE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB444LE                      /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 12 ),                map to AV_PIX_FMT_RGB444LE */},
    { AV_PIX_FMT_RGB444BE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGB444BE                      /*_TVU_LE_FOURCCTAG(12, 'B', 'G', 'R'),                 map to AV_PIX_FMT_RGB444BE */},
    { AV_PIX_FMT_BGR444LE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR444LE                      /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 12 ),                map to AV_PIX_FMT_BGR444LE */},
    { AV_PIX_FMT_BGR444BE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGR444BE                      /*_TVU_LE_FOURCCTAG(12, 'R', 'G', 'B'),                 map to AV_PIX_FMT_BGR444BE */},
    { AV_PIX_FMT_GRAY8A,               K_TVU_PIXFMT_VIDEO_FOURCC_GRAY8A                        /*_TVU_LE_FOURCCTAG('Y', '2', 0 , 8  ),                 map to AV_PIX_FMT_GRAY8A */},
    { AV_PIX_FMT_BGR48BE,              K_TVU_PIXFMT_VIDEO_FOURCC_BGR48BE                       /*_TVU_LE_FOURCCTAG(48, 'R', 'G', 'B'),                 map to AV_PIX_FMT_BGR48BE */},
    { AV_PIX_FMT_BGR48LE,              K_TVU_PIXFMT_VIDEO_FOURCC_BGR48LE                       /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 48 ),                map to AV_PIX_FMT_BGR48LE */},
    { AV_PIX_FMT_YUV420P9BE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P9BE                    /*_TVU_LE_FOURCCTAG(9 , 11, '3', 'Y'),                  map to AV_PIX_FMT_YUV420P9BE */},
    { AV_PIX_FMT_YUV420P9LE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P9LE                    /*_TVU_LE_FOURCCTAG('Y', '3', 11, 9  ),                 map to AV_PIX_FMT_YUV420P9LE */},
    { AV_PIX_FMT_YUV420P10BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P10BE                   /*_TVU_LE_FOURCCTAG(10, 11, '3', 'Y'),                  map to AV_PIX_FMT_YUV420P10BE */},
    { AV_PIX_FMT_YUV420P10LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P10LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 11, 10 ),                 map to AV_PIX_FMT_YUV420P10LE */},
    { AV_PIX_FMT_YUV422P10BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P10BE                   /*_TVU_LE_FOURCCTAG(10, 10, '3', 'Y'),                  map to AV_PIX_FMT_YUV422P10BE */},
    { AV_PIX_FMT_YUV422P10LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P10LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 10, 10 ),                 map to AV_PIX_FMT_YUV422P10LE */},
    { AV_PIX_FMT_YUV444P9BE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P9BE                    /*_TVU_LE_FOURCCTAG(9 , 3 , '3', 'Y'),                  map to AV_PIX_FMT_YUV444P9BE */},
    { AV_PIX_FMT_YUV444P9LE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P9LE                    /*_TVU_LE_FOURCCTAG('Y', '3', 0 , 9  ),                 map to AV_PIX_FMT_YUV444P9LE */},
    { AV_PIX_FMT_YUV444P10BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P10BE                   /*_TVU_LE_FOURCCTAG(10, 0 , '3', 'Y'),                  map to AV_PIX_FMT_YUV444P10BE */},
    { AV_PIX_FMT_YUV444P10LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P10LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 0 , 10 ),                 map to AV_PIX_FMT_YUV444P10LE */},
    { AV_PIX_FMT_YUV422P9BE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P9BE                    /*_TVU_LE_FOURCCTAG(9 , 10, '3', 'Y'),                  map to AV_PIX_FMT_YUV422P9BE */},
    { AV_PIX_FMT_YUV422P9LE,           K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P9LE                    /*_TVU_LE_FOURCCTAG('Y', '3', 10, 9  ),                 map to AV_PIX_FMT_YUV422P9LE */},
    //{AV_PIX_FMT_VDA_VLD,             // K_TVU_PIXFMT_VIDEO_FOURCC_VDA_VLD                    /*_TVU_LE_FOURCCTAG('', '', '', ''),                    map to AV_PIX_FMT_VDA_VLD */},
    { AV_PIX_FMT_RGBA64BE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGBA64BE                      /*_TVU_LE_FOURCCTAG(64, 'R', 'B', 'A'),                 map to AV_PIX_FMT_RGBA64BE */},
    { AV_PIX_FMT_RGBA64LE,             K_TVU_PIXFMT_VIDEO_FOURCC_RGBA64LE                      /*_TVU_LE_FOURCCTAG('R', 'B', 'A', 64 ),                map to AV_PIX_FMT_RGBA64LE */},
    { AV_PIX_FMT_BGRA64BE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGRA64BE                      /*_TVU_LE_FOURCCTAG(64, 'B', 'R', 'A'),                 map to AV_PIX_FMT_BGRA64BE */},
    { AV_PIX_FMT_BGRA64LE,             K_TVU_PIXFMT_VIDEO_FOURCC_BGRA64LE                      /*_TVU_LE_FOURCCTAG('B', 'R', 'A', 64 ),                map to AV_PIX_FMT_BGRA64LE */},
    { AV_PIX_FMT_GBRP,                 K_TVU_PIXFMT_VIDEO_FOURCC_GBRP                          /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 0  ),                map to AV_PIX_FMT_GBRP */},
    { AV_PIX_FMT_GBRP9BE,              K_TVU_PIXFMT_VIDEO_FOURCC_GBRP9BE                       /*_TVU_LE_FOURCCTAG(9 , 'R', 'B', 'G'),                 map to AV_PIX_FMT_GBRP9BE */},
    { AV_PIX_FMT_GBRP9LE,              K_TVU_PIXFMT_VIDEO_FOURCC_GBRP9LE                       /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 9  ),                map to AV_PIX_FMT_GBRP9LE */},
    { AV_PIX_FMT_GBRP10BE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP10BE                      /*_TVU_LE_FOURCCTAG(10, 'R', 'B', 'G'),                 map to AV_PIX_FMT_GBRP10BE */},
    { AV_PIX_FMT_GBRP10LE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP10LE                      /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 10 ),                map to AV_PIX_FMT_GBRP10LE */},
    { AV_PIX_FMT_GBRP16BE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP16BE                      /*_TVU_LE_FOURCCTAG(16, 'R', 'B', 'G'),                 map to AV_PIX_FMT_GBRP16BE */},
    { AV_PIX_FMT_GBRP16LE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP16LE                      /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 16 ),                map to AV_PIX_FMT_GBRP16LE */},
    { AV_PIX_FMT_0RGB,                 K_TVU_PIXFMT_VIDEO_FOURCC_0RGB                          /*_TVU_LE_FOURCCTAG(0 , 'R', 'G', 'B'),                 map to AV_PIX_FMT_0RGB */},
    { AV_PIX_FMT_RGB0,                 K_TVU_PIXFMT_VIDEO_FOURCC_RGB0                          /*_TVU_LE_FOURCCTAG('R', 'G', 'B', 0  ),                map to AV_PIX_FMT_RGB0 */},
    { AV_PIX_FMT_0BGR,                 K_TVU_PIXFMT_VIDEO_FOURCC_0BGR                          /*_TVU_LE_FOURCCTAG(0 , 'B', 'G', 'R'),                 map to AV_PIX_FMT_0BGR */},
    { AV_PIX_FMT_BGR0,                 K_TVU_PIXFMT_VIDEO_FOURCC_BGR0                          /*_TVU_LE_FOURCCTAG('B', 'G', 'R', 0  ),                map to AV_PIX_FMT_BGR0 */},
    { AV_PIX_FMT_YUVA444P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVA444P                      /*_TVU_LE_FOURCCTAG('Y', '4', 0 , 8  ),                 map to AV_PIX_FMT_YUVA444P */},
    { AV_PIX_FMT_YUVA422P,             K_TVU_PIXFMT_VIDEO_FOURCC_YUVA422P                      /*_TVU_LE_FOURCCTAG('Y', '4', 10, 8  ),                 map to AV_PIX_FMT_YUVA422P */},
    { AV_PIX_FMT_YUV420P12BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P12BE                   /*_TVU_LE_FOURCCTAG(12, 11, '3', 'Y'),                  map to AV_PIX_FMT_YUV420P12BE */},
    { AV_PIX_FMT_YUV420P12LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P12LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 11, 12 ),                 map to AV_PIX_FMT_YUV420P12LE */},
    { AV_PIX_FMT_YUV420P14BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P14BE                   /*_TVU_LE_FOURCCTAG(14, 11, '3', 'Y'),                  map to AV_PIX_FMT_YUV420P14BE */},
    { AV_PIX_FMT_YUV420P14LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P14LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 11, 14 ),                 map to AV_PIX_FMT_YUV420P14LE */},
    { AV_PIX_FMT_YUV422P12BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P12BE                   /*_TVU_LE_FOURCCTAG(12, 10, '3', 'Y'),                  map to AV_PIX_FMT_YUV422P12BE */},
    { AV_PIX_FMT_YUV422P12LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P12LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 10, 12 ),                 map to AV_PIX_FMT_YUV422P12LE */},
    { AV_PIX_FMT_YUV422P14BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P14BE                   /*_TVU_LE_FOURCCTAG(14, 10, '3', 'Y'),                  map to AV_PIX_FMT_YUV422P14BE */},
    { AV_PIX_FMT_YUV422P14LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P14LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 10, 14 ),                 map to AV_PIX_FMT_YUV422P14LE */},
    { AV_PIX_FMT_YUV444P12BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P12BE                   /*_TVU_LE_FOURCCTAG(12, 0 , '3', 'Y'),                  map to AV_PIX_FMT_YUV444P12BE */},
    { AV_PIX_FMT_YUV444P12LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P12LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 0 , 12 ),                 map to AV_PIX_FMT_YUV444P12LE */},
    { AV_PIX_FMT_YUV444P14BE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P14BE                   /*_TVU_LE_FOURCCTAG(14, 0 , '3', 'Y'),                  map to AV_PIX_FMT_YUV444P14BE */},
    { AV_PIX_FMT_YUV444P14LE,          K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P14LE                   /*_TVU_LE_FOURCCTAG('Y', '3', 0 , 14 ),                 map to AV_PIX_FMT_YUV444P14LE */},
    { AV_PIX_FMT_GBRP12BE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP12BE                      /*_TVU_LE_FOURCCTAG(12, 'R', 'B', 'G'),                 map to AV_PIX_FMT_GBRP12BE */},
    { AV_PIX_FMT_GBRP12LE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP12LE                      /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 12 ),                map to AV_PIX_FMT_GBRP12LE */},
    { AV_PIX_FMT_GBRP14BE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP14BE                      /*_TVU_LE_FOURCCTAG(14, 'R', 'B', 'G'),                 map to AV_PIX_FMT_GBRP14BE */},
    { AV_PIX_FMT_GBRP14LE,             K_TVU_PIXFMT_VIDEO_FOURCC_GBRP14LE                      /*_TVU_LE_FOURCCTAG('G', 'B', 'R', 14 ),                map to AV_PIX_FMT_GBRP14LE */},
    { AV_PIX_FMT_Y210LE,               K_TVU_PIXFMT_VIDEO_FOURCC_Y210LE                        /*_TVU_LE_FOURCCTAG('Y', '2', '1' , '0'),               map to AV_PIX_FMT_Y210LE, y210 y-packed, 2-4:2:2, 10-10bit LE */},
    { AV_PIX_FMT_Y210BE,               K_TVU_PIXFMT_VIDEO_FOURCC_Y210BE                        /*_TVU_BE_FOURCCTAG('Y', '2', '1' , '0'),               map to AV_PIX_FMT_Y210BE, y210 y-packed, 2-4:2:2, 10-10bit BE */},
    { AV_PIX_FMT_P210LE,               K_TVU_PIXFMT_VIDEO_FOURCC_P210LE                        /*_TVU_LE_FOURCCTAG('P', '2', '1' , '0'),               map to AV_PIX_FMT_P210LE, p210 p-planar, 2-4:2:2, 10-10bit LE */},
    { AV_PIX_FMT_P210BE,               K_TVU_PIXFMT_VIDEO_FOURCC_P210BE                        /*_TVU_BE_FOURCCTAG('P', '2', '1' , '0'),               map to AV_PIX_FMT_P210BE, p210 p-planar, 2-4:2:2, 10-10bit BE */},
    //{ AV_PIX_FMT_Y216LE,             K_TVU_PIXFMT_VIDEO_FOURCC_Y216LE                        /*_TVU_LE_FOURCCTAG('Y', '2', '1' , '6'),               map to AV_PIX_FMT_Y216LE, y216 y-packed, 2-4:2:2, 16-16bit LE */},
    //{ AV_PIX_FMT_Y216BE,             K_TVU_PIXFMT_VIDEO_FOURCC_Y216BE                        /*_TVU_BE_FOURCCTAG('Y', '2', '1' , '6'),               map to AV_PIX_FMT_Y216BE, y216 y-packed, 2-4:2:2, 16-16bit BE */},
    { AV_PIX_FMT_P216LE,               K_TVU_PIXFMT_VIDEO_FOURCC_P216LE                        /*_TVU_LE_FOURCCTAG('P', '2', '1' , '6'),               map to AV_PIX_FMT_P216LE, p216 p-planar, 2-4:2:2, 16-16bit LE */},
    { AV_PIX_FMT_P216BE,               K_TVU_PIXFMT_VIDEO_FOURCC_P216BE                        /*_TVU_BE_FOURCCTAG('P', '2', '1' , '6'),               map to AV_PIX_FMT_P216BE, p216 p-planar, 2-4:2:2, 16-16bit BE */},
    { AV_PIX_FMT_NONE,                 0                            },
};
#endif

