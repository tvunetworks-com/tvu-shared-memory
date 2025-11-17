
/******************************************************************************
 *  Description:
 *      tvu fourcc definition.
 *  History:
 *      Lotus/TVU, initializing on Nov 14th, 2018.
 *  CopyRight:
 *      TVU Ltm co.
 *****************************************************************************/

#ifndef LIBTVU_MEDIA_FOURCC_H
#define LIBTVU_MEDIA_FOURCC_H

#ifdef __cplusplus
extern "C"
{
#endif

#define _TVU_LE_FOURCCTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define _TVU_BE_FOURCCTAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

enum ETVUPixfmtVideoFourCC
{
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P   = _TVU_LE_FOURCCTAG('I', '4', '2', '0'),                /* map to PIX_FMT_YUV420P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUYV422   = _TVU_LE_FOURCCTAG('V', '4', '2', '2'),                /* map to PIX_FMT_YUYV422 */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB24     = _TVU_LE_FOURCCTAG('R', 'G', 'B', 24 ),                /* map to PIX_FMT_RGB24 */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR24     = _TVU_LE_FOURCCTAG('B', 'G', 'R', 24 ),                /* map to PIX_FMT_BGR24 */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P   = _TVU_LE_FOURCCTAG('I', '4', '2', '2'),                /* map to PIX_FMT_YUV422P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P   = _TVU_LE_FOURCCTAG('I', '4', '4', '4'),                /* map to PIX_FMT_YUV444P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV410P   = _TVU_LE_FOURCCTAG('I', '4', '1', '0'),                /* map to PIX_FMT_YUV410P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV411P   = _TVU_LE_FOURCCTAG('I', '4', '1', '1'),                /* map to PIX_FMT_YUV411P */
    K_TVU_PIXFMT_VIDEO_FOURCC_GRAY8     = _TVU_LE_FOURCCTAG('G', 'R', 'E', 'Y'),                /* map to PIX_FMT_GRAY8 */
    K_TVU_PIXFMT_VIDEO_FOURCC_MONOWHITE = _TVU_LE_FOURCCTAG('B', '1', 'W', '0'),                /* map to PIX_FMT_MONOWHITE */
    K_TVU_PIXFMT_VIDEO_FOURCC_MONOBLACK = _TVU_LE_FOURCCTAG('B', '0', 'W', '1'),                /* map to PIX_FMT_MONOBLACK */
    K_TVU_PIXFMT_VIDEO_FOURCC_PAL8      = _TVU_LE_FOURCCTAG('P', 'A', 'L', '8'),                /* map to PIX_FMT_PAL8 */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ420P  = _TVU_LE_FOURCCTAG('J', '4', '2', '0'),                /* map to PIX_FMT_YUVJ420P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ422P  = _TVU_LE_FOURCCTAG('J', '4', '2', '2'),                /* map to PIX_FMT_YUVJ422P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ444P  = _TVU_LE_FOURCCTAG('J', '4', '4', '4'),                /* map to PIX_FMT_YUVJ444P */
    /* K_TVU_PIXFMT_VIDEO_FOURCC_XVMC_MPEG2_MC      =  _TVU_LE_FOURCCTAG('v', 'm', 'c', 'm'),*/ /* map to PIX_FMT_XVMC_MPEG2_MC */
    /* K_TVU_PIXFMT_VIDEO_FOURCC_XVMC_MPEG2_IDCT    = _TVU_LE_FOURCCTAG('v', 'm', 'c', 'i'),*/  /* map to PIX_FMT_XVMC_MPEG2_IDCT */
    K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422   = _TVU_LE_FOURCCTAG('U', 'Y', 'V', 'Y'),                /* map to PIX_FMT_UYVY422 */
    K_TVU_PIXFMT_VIDEO_FOURCC_UYYVYY411 = _TVU_LE_FOURCCTAG('Y', '4', '1', '1'),                /* map to PIX_FMT_UYYVYY411 */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR8      = _TVU_LE_FOURCCTAG('B', 'G', 'R', 8  ),                /* map to PIX_FMT_BGR8 */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR4      = _TVU_LE_FOURCCTAG('B', 'G', 'R', 4  ),                /* map to PIX_FMT_BGR4 */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR4_BYTE = _TVU_LE_FOURCCTAG('R', '4', 'B', 'Y'),                /* map to PIX_FMT_BGR4_BYTE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB8      = _TVU_LE_FOURCCTAG('R', 'G', 'B', 8  ),                /* map to PIX_FMT_RGB8 */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB4      = _TVU_LE_FOURCCTAG('R', 'G', 'B', 4  ),                /* map to PIX_FMT_RGB4 */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB4_BYTE = _TVU_LE_FOURCCTAG('B', '4', 'B', 'Y'),                /* map to PIX_FMT_RGB4_BYTE */
    K_TVU_PIXFMT_VIDEO_FOURCC_NV12      = _TVU_LE_FOURCCTAG('N', 'V', '1', '2'),                /* map to PIX_FMT_NV12 */
    K_TVU_PIXFMT_VIDEO_FOURCC_NV21      = _TVU_LE_FOURCCTAG('N', 'V', '2', '1'),                /* map to PIX_FMT_NV21 */
    K_TVU_PIXFMT_VIDEO_FOURCC_P010LE    = _TVU_LE_FOURCCTAG('P', '0', '1' , '0'),               /* map to PIX_FMT_P010 */
    K_TVU_PIXFMT_VIDEO_FOURCC_ARGB      = _TVU_LE_FOURCCTAG('A', 'R', 'G', 'B'),                /* map to PIX_FMT_ARGB */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGBA      = _TVU_LE_FOURCCTAG('R', 'G', 'B', 'A'),                /* map to PIX_FMT_RGBA */
    K_TVU_PIXFMT_VIDEO_FOURCC_ABGR      = _TVU_LE_FOURCCTAG('A', 'B', 'G', 'R'),                /* map to PIX_FMT_ABGR */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGRA      = _TVU_LE_FOURCCTAG('B', 'G', 'R', 'A'),                /* map to PIX_FMT_BGRA */
    K_TVU_PIXFMT_VIDEO_FOURCC_GRAY16BE  = _TVU_LE_FOURCCTAG(16,  0, '1', 'Y'),                  /* map to PIX_FMT_GRAY16BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GRAY16LE  = _TVU_LE_FOURCCTAG('Y', '1', 0 , 16 ),                 /* map to PIX_FMT_GRAY16LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV440P   = _TVU_LE_FOURCCTAG('I', '4', '4', '0'),                /* map to PIX_FMT_YUV440P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVJ440P  = _TVU_LE_FOURCCTAG('J', '4', '4', '0'),                /* map to PIX_FMT_YUVJ440P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVA420P  = _TVU_LE_FOURCCTAG('Y', '4', 11,  8 ),                 /* map to PIX_FMT_YUVA420P */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_H264 =  _TVU_LE_FOURCCTAG('', '', '', ''),               /* map to PIX_FMT_VDPAU_H264 */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG1 = _TVU_LE_FOURCCTAG('', '', '', ''),               /* map to PIX_FMT_VDPAU_MPEG1 */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG2 = _TVU_LE_FOURCCTAG('', '', '', ''),               /* map to PIX_FMT_VDPAU_MPEG2 */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_WMV3 =  _TVU_LE_FOURCCTAG('', '', '', ''),               /* map to PIX_FMT_VDPAU_WMV3 */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_VC1 =  _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_VDPAU_VC1 */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB48BE   = _TVU_LE_FOURCCTAG(48, 'B', 'G', 'R'),                 /* map to PIX_FMT_RGB48BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB48LE   = _TVU_LE_FOURCCTAG('R', 'G', 'B', 48),                 /* map to PIX_FMT_RGB48LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB565BE  = _TVU_LE_FOURCCTAG(16, 'B', 'G', 'R'),                 /* map to PIX_FMT_RGB565BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB565LE  = _TVU_LE_FOURCCTAG('R', 'G', 'B', 16 ),                /* map to PIX_FMT_RGB565LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB555BE  = _TVU_LE_FOURCCTAG(15, 'B', 'G', 'R'),                 /* map to PIX_FMT_RGB555BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB555LE  = _TVU_LE_FOURCCTAG('R', 'G', 'B', 15 ),                /* map to PIX_FMT_RGB555LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR565BE  = _TVU_LE_FOURCCTAG(16, 'R', 'G', 'B'),                 /* map to PIX_FMT_BGR565BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR565LE  = _TVU_LE_FOURCCTAG('B', 'G', 'R', 16 ),                /* map to PIX_FMT_BGR565LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR555BE  = _TVU_LE_FOURCCTAG(15, 'R', 'G', 'B'),                 /* map to PIX_FMT_BGR555BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR555LE  = _TVU_LE_FOURCCTAG('B', 'G', 'R', 15 ),                /* map to PIX_FMT_BGR555LE */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_MOCO = _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_VAAPI_MOCO */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_IDCT = _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_VAAPI_IDCT */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VAAPI_VLD =  _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_VAAPI_VLD */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P16LE = _TVU_LE_FOURCCTAG('Y', '3', 11, 16 ),               /* map to PIX_FMT_YUV420P16LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P16BE = _TVU_LE_FOURCCTAG(16, 11, '3', 'Y'),                /* map to PIX_FMT_YUV420P16BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P16LE = _TVU_LE_FOURCCTAG('Y', '3', 10, 16 ),               /* map to PIX_FMT_YUV422P16LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P16BE = _TVU_LE_FOURCCTAG(16, 10, '3', 'Y'),                /* map to PIX_FMT_YUV422P16BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P16LE = _TVU_LE_FOURCCTAG('Y', '3', 0 , 16 ),               /* map to PIX_FMT_YUV444P16LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P16BE = _TVU_LE_FOURCCTAG(16, 0 , '3', 'Y'),                /* map to PIX_FMT_YUV444P16BE */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDPAU_MPEG4 = _TVU_LE_FOURCCTAG('', '', '', ''),               /* map to PIX_FMT_VDPAU_MPEG4 */
    // K_TVU_PIXFMT_VIDEO_FOURCC_DXVA2_VLD =  _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_DXVA2_VLD */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB444LE  = _TVU_LE_FOURCCTAG('R', 'G', 'B', 12 ),                /* map to PIX_FMT_RGB444LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB444BE  = _TVU_LE_FOURCCTAG(12, 'B', 'G', 'R'),                 /* map to PIX_FMT_RGB444BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR444LE  = _TVU_LE_FOURCCTAG('B', 'G', 'R', 12 ),                /* map to PIX_FMT_BGR444LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR444BE  = _TVU_LE_FOURCCTAG(12, 'R', 'G', 'B'),                 /* map to PIX_FMT_BGR444BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GRAY8A    = _TVU_LE_FOURCCTAG('Y', '2', 0 , 8  ),                 /* map to PIX_FMT_GRAY8A */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR48BE   = _TVU_LE_FOURCCTAG(48, 'R', 'G', 'B'),                 /* map to PIX_FMT_BGR48BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR48LE   = _TVU_LE_FOURCCTAG('B', 'G', 'R', 48 ),                /* map to PIX_FMT_BGR48LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P9BE = _TVU_LE_FOURCCTAG(9 , 11, '3', 'Y'),                 /* map to PIX_FMT_YUV420P9BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P9LE = _TVU_LE_FOURCCTAG('Y', '3', 11, 9  ),                /* map to PIX_FMT_YUV420P9LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P10BE = _TVU_LE_FOURCCTAG(10, 11, '3', 'Y'),                /* map to PIX_FMT_YUV420P10BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P10LE = _TVU_LE_FOURCCTAG('Y', '3', 11, 10 ),               /* map to PIX_FMT_YUV420P10LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P10BE = _TVU_LE_FOURCCTAG(10, 10, '3', 'Y'),                /* map to PIX_FMT_YUV422P10BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P10LE = _TVU_LE_FOURCCTAG('Y', '3', 10, 10 ),               /* map to PIX_FMT_YUV422P10LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P9BE = _TVU_LE_FOURCCTAG(9 , 3 , '3', 'Y'),                 /* map to PIX_FMT_YUV444P9BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P9LE = _TVU_LE_FOURCCTAG('Y', '3', 0 , 9  ),                /* map to PIX_FMT_YUV444P9LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P10BE = _TVU_LE_FOURCCTAG(10, 0 , '3', 'Y'),                /* map to PIX_FMT_YUV444P10BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P10LE = _TVU_LE_FOURCCTAG('Y', '3', 0 , 10 ),               /* map to PIX_FMT_YUV444P10LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P9BE = _TVU_LE_FOURCCTAG(9 , 10, '3', 'Y'),                 /* map to PIX_FMT_YUV422P9BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P9LE = _TVU_LE_FOURCCTAG('Y', '3', 10, 9  ),                /* map to PIX_FMT_YUV422P9LE */
    // K_TVU_PIXFMT_VIDEO_FOURCC_VDA_VLD =    _TVU_LE_FOURCCTAG('', '', '', ''),                /* map to PIX_FMT_VDA_VLD */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGBA64BE  = _TVU_LE_FOURCCTAG(64, 'R', 'B', 'A'),                 /* map to PIX_FMT_RGBA64BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGBA64LE  = _TVU_LE_FOURCCTAG('R', 'B', 'A', 64 ),                /* map to PIX_FMT_RGBA64LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGRA64BE  = _TVU_LE_FOURCCTAG(64, 'B', 'R', 'A'),                 /* map to PIX_FMT_BGRA64BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGRA64LE  = _TVU_LE_FOURCCTAG('B', 'R', 'A', 64 ),                /* map to PIX_FMT_BGRA64LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP      = _TVU_LE_FOURCCTAG('G', 'B', 'R', 0  ),                /* map to PIX_FMT_GBRP */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP9BE   = _TVU_LE_FOURCCTAG(9 , 'R', 'B', 'G'),                 /* map to PIX_FMT_GBRP9BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP9LE   = _TVU_LE_FOURCCTAG('G', 'B', 'R', 9  ),                /* map to PIX_FMT_GBRP9LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP10BE  = _TVU_LE_FOURCCTAG(10, 'R', 'B', 'G'),                 /* map to PIX_FMT_GBRP10BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP10LE  = _TVU_LE_FOURCCTAG('G', 'B', 'R', 10 ),                /* map to PIX_FMT_GBRP10LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP16BE  = _TVU_LE_FOURCCTAG(16, 'R', 'B', 'G'),                 /* map to PIX_FMT_GBRP16BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP16LE  = _TVU_LE_FOURCCTAG('G', 'B', 'R', 16 ),                /* map to PIX_FMT_GBRP16LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_0RGB      = _TVU_LE_FOURCCTAG(0 , 'R', 'G', 'B'),                 /* map to PIX_FMT_0RGB */
    K_TVU_PIXFMT_VIDEO_FOURCC_RGB0      = _TVU_LE_FOURCCTAG('R', 'G', 'B', 0  ),                /* map to PIX_FMT_RGB0 */
    K_TVU_PIXFMT_VIDEO_FOURCC_0BGR      = _TVU_LE_FOURCCTAG(0 , 'B', 'G', 'R'),                 /* map to PIX_FMT_0BGR */
    K_TVU_PIXFMT_VIDEO_FOURCC_BGR0      = _TVU_LE_FOURCCTAG('B', 'G', 'R', 0  ),                /* map to PIX_FMT_BGR0 */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVA444P  = _TVU_LE_FOURCCTAG('Y', '4', 0 , 8  ),                 /* map to PIX_FMT_YUVA444P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUVA422P  = _TVU_LE_FOURCCTAG('Y', '4', 10, 8  ),                 /* map to PIX_FMT_YUVA422P */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P12BE = _TVU_LE_FOURCCTAG(12, 11, '3', 'Y'),                /* map to PIX_FMT_YUV420P12BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P12LE = _TVU_LE_FOURCCTAG('Y', '3', 11, 12 ),               /* map to PIX_FMT_YUV420P12LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P14BE = _TVU_LE_FOURCCTAG(14, 11, '3', 'Y'),                /* map to PIX_FMT_YUV420P14BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P14LE = _TVU_LE_FOURCCTAG('Y', '3', 11, 14 ),               /* map to PIX_FMT_YUV420P14LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P12BE = _TVU_LE_FOURCCTAG(12, 10, '3', 'Y'),                /* map to PIX_FMT_YUV422P12BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P12LE = _TVU_LE_FOURCCTAG('Y', '3', 10, 12 ),               /* map to PIX_FMT_YUV422P12LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P14BE = _TVU_LE_FOURCCTAG(14, 10, '3', 'Y'),                /* map to PIX_FMT_YUV422P14BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P14LE = _TVU_LE_FOURCCTAG('Y', '3', 10, 14 ),               /* map to PIX_FMT_YUV422P14LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P12BE = _TVU_LE_FOURCCTAG(12, 0 , '3', 'Y'),                /* map to PIX_FMT_YUV444P12BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P12LE = _TVU_LE_FOURCCTAG('Y', '3', 0, 12 ),                /* map to PIX_FMT_YUV444P12LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P14BE = _TVU_LE_FOURCCTAG(14, 0 , '3', 'Y'),                /* map to PIX_FMT_YUV444P14BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P14LE = _TVU_LE_FOURCCTAG('Y', '3', 0, 14 ),                /* map to PIX_FMT_YUV444P14LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP12BE  = _TVU_LE_FOURCCTAG(12, 'R', 'B', 'G'),                 /* map to PIX_FMT_GBRP12BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP12LE  = _TVU_LE_FOURCCTAG('G', 'B', 'R', 12 ),                /* map to PIX_FMT_GBRP12LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP14BE  = _TVU_LE_FOURCCTAG(14, 'R', 'B', 'G'),                 /* map to PIX_FMT_GBRP14BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_GBRP14LE  = _TVU_LE_FOURCCTAG('G', 'B', 'R', 14 ),                /* map to PIX_FMT_GBRP14LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_JPEG      = _TVU_LE_FOURCCTAG('J', 'P', 'E', 'G' ),               /* map to JPEG Encoding*/
    K_TVU_PIXFMT_VIDEO_FOURCC_Y210LE    = _TVU_LE_FOURCCTAG('Y', '2', '1', '0'),                /* map to PIX_FMT_Y210LE, y210 y-packed, 2-4:2:2, 10-10bit LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_Y210BE    = _TVU_BE_FOURCCTAG('Y', '2', '1', '0'),                /* map to PIX_FMT_Y210BE, y210 y-packed, 2-4:2:2, 10-10bit BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_P210LE    = _TVU_LE_FOURCCTAG('P', '2', '1', '0'),                /* map to PIX_FMT_P210LE, p210 p-planar, 2-4:2:2, 10-10bit LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_P210BE    = _TVU_BE_FOURCCTAG('P', '2', '1', '0'),                /* map to PIX_FMT_P210BE, p210 p-planar, 2-4:2:2, 10-10bit BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_Y216LE    = _TVU_LE_FOURCCTAG('Y', '2', '1', '6'),                /* map to PIX_FMT_Y216LE, y216 y-packed, 2-4:2:2, 16-16bit LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_Y216BE    = _TVU_BE_FOURCCTAG('Y', '2', '1', '6'),                /* map to PIX_FMT_Y216BE, y216 y-packed, 2-4:2:2, 16-16bit BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_P216LE    = _TVU_LE_FOURCCTAG('P', '2', '1', '6'),                /* map to PIX_FMT_P216LE, p216 p-planar, 2-4:2:2, 16-16bit LE */
    K_TVU_PIXFMT_VIDEO_FOURCC_P216BE    = _TVU_BE_FOURCCTAG('P', '2', '1', '6'),                /* map to PIX_FMT_P216BE, p216 p-planar, 2-4:2:2, 16-16bit BE */
    K_TVU_PIXFMT_VIDEO_FOURCC_Y410LE    = _TVU_LE_FOURCCTAG('Y', '4', '1', '0'),                /* map to PIX_FMT_XV30LE, p216 p-planar, packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), little-endian, variant of Y410 where alpha channel is left undefined */
    K_TVU_PIXFMT_VIDEO_FOURCC_Y410BE    = _TVU_BE_FOURCCTAG('Y', '4', '1', '0'),                /* map to PIX_FMT_XV30BE, packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), big-endian, variant of Y410 where alpha channel is left undefined */
    K_TVU_PIXFMT_VIDEO_FOURCC_NONE      = 0                   ,                                 /* map to PIX_FMT_NONE */
};

enum ETVUPixfmtAudioFourCC
{
    K_TVU_AUDIO_FOURCC_WAVE_48K_16      = _TVU_LE_FOURCCTAG('W', 'A', 'V', '1'),
};


/* if CodecTag */

typedef enum ETvuCodecTagFourCC
{
    K_TVU_CODEC_TAG_NONE            = _TVU_LE_FOURCCTAG(0, 0, 0, 0),

    /* video */
    K_TVU_CODEC_TAG_MPEG1VIDEO      = _TVU_LE_FOURCCTAG('m', 'p', '1', 'v'),
    K_TVU_CODEC_TAG_MPEG2VIDEO      = _TVU_LE_FOURCCTAG('m', 'p', '2', 'v'),
    K_TVU_CODEC_TAG_H261            = _TVU_LE_FOURCCTAG('h', '2', '6', '1'),
    K_TVU_CODEC_TAG_H263            = _TVU_LE_FOURCCTAG('h', '2', '6', '3'),
    K_TVU_CODEC_TAG_H264            = _TVU_LE_FOURCCTAG('h', '2', '6', '4'),
    K_TVU_CODEC_TAG_HEVC            = _TVU_LE_FOURCCTAG('h', 'e', 'v', 'c'),
    K_TVU_CODEC_TAG_VP8             = _TVU_LE_FOURCCTAG('v', 'p', '8', 'v'),
    K_TVU_CODEC_TAG_VP9             = _TVU_LE_FOURCCTAG('v', 'p', '9', 'v'),
    K_TVU_CODEC_TAG_AVS1            = _TVU_LE_FOURCCTAG('a', 'v', 's', '1'),
    K_TVU_CODEC_TAG_AVS2            = _TVU_LE_FOURCCTAG('a', 'v', 's', '2'),

    /* audio */
    K_TVU_CODEC_TAG_MP2             = _TVU_LE_FOURCCTAG('m', 'p', '2', 'a'),
    K_TVU_CODEC_TAG_MP3             = _TVU_LE_FOURCCTAG('m', 'p', '3', 'a'),
    K_TVU_CODEC_TAG_AAC             = _TVU_LE_FOURCCTAG('a', 'a', 'c', 'a'),
    K_TVU_CODEC_TAG_AC3             = _TVU_LE_FOURCCTAG('a', 'c', '3', 'a'),
    K_TVU_CODEC_TAG_DTS             = _TVU_LE_FOURCCTAG('d', 't', 's', 'a'),
    K_TVU_CODEC_TAG_VORBIS          = _TVU_LE_FOURCCTAG('v', 'b', 'i', 's'),
    K_TVU_CODEC_TAG_OPUS            = _TVU_LE_FOURCCTAG('o', 'p', 'u', 'a'),
    K_TVU_CODEC_TAG_WMAV1           = _TVU_LE_FOURCCTAG('w', 'm', 'a', '1'),
    K_TVU_CODEC_TAG_WMAV2           = _TVU_LE_FOURCCTAG('w', 'm', 'a', '2'),
    K_TVU_CODEC_TAG_MACE3           = _TVU_LE_FOURCCTAG('m', 'c', 'e', '3'),
    K_TVU_CODEC_TAG_MACE6           = _TVU_LE_FOURCCTAG('m', 'c', 'e', '6'),
    K_TVU_CODEC_TAG_VMDAUDIO        = _TVU_LE_FOURCCTAG('v', 'm', 'd', 'a'),
    K_TVU_CODEC_TAG_FLAC            = _TVU_LE_FOURCCTAG('f', 'l', 'a', 'c'),
    K_TVU_CODEC_TAG_AMRNB           = _TVU_LE_FOURCCTAG('a', 'm', 'r', 'n'),
    K_TVU_CODEC_TAG_AMRWB           = _TVU_LE_FOURCCTAG('a', 'm', 'r', 'w'),
    K_TVU_CODEC_TAG_JPEG            = _TVU_LE_FOURCCTAG('j', 'p', 'e', 'g'),
}tvu_codec_tag_fourcc_t;

/* endif CodecTag */

#ifdef __cplusplus
}
#endif

#endif // LIBTVU_MEDIA_FOURCC_H

