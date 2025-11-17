
#include "tvu_fourcc.h"
#include "libavutil/pixfmt.h"
#include <stdio.h>

static const tvu_pixfmt_fourcc_map_t _video_chroma_table[] =
{
    { AV_PIX_FMT_YUV420P,           _TVU_LE_FOURCCTAG('I', '4', '2', '0')},
    { AV_PIX_FMT_YUYV422,           _TVU_LE_FOURCCTAG('V', '4', '2', '2')},
    { AV_PIX_FMT_RGB24,             _TVU_LE_FOURCCTAG('R', 'G', 'B', 24 )},
    { AV_PIX_FMT_BGR24,             _TVU_LE_FOURCCTAG('B', 'G', 'R', 24 )},
    { AV_PIX_FMT_YUV422P,           _TVU_LE_FOURCCTAG('I', '4', '2', '2')},
    { AV_PIX_FMT_YUV444P,           _TVU_LE_FOURCCTAG('I', '4', '4', '4')},
    { AV_PIX_FMT_YUV410P,           _TVU_LE_FOURCCTAG('I', '4', '1', '0')},
    { AV_PIX_FMT_YUV411P,           _TVU_LE_FOURCCTAG('I', '4', '1', '1')},
    { AV_PIX_FMT_GRAY8,             _TVU_LE_FOURCCTAG('G', 'R', 'E', 'Y')},
    { AV_PIX_FMT_MONOWHITE,         _TVU_LE_FOURCCTAG('B', '1', 'W', '0')},
    { AV_PIX_FMT_MONOBLACK,         _TVU_LE_FOURCCTAG('B', '0', 'W', '1')},
    { AV_PIX_FMT_PAL8,              _TVU_LE_FOURCCTAG('P', 'A', 'L', '8')},
    { AV_PIX_FMT_YUVJ420P,          _TVU_LE_FOURCCTAG('J', '4', '2', '0')},
    { AV_PIX_FMT_YUVJ422P,          _TVU_LE_FOURCCTAG('J', '4', '2', '2')},
    { AV_PIX_FMT_YUVJ444P,          _TVU_LE_FOURCCTAG('J', '4', '4', '4')},
    //{PIX_FMT_XVMC_MPEG2_MC,     _TVU_LE_FOURCCTAG('v', 'm', 'c', 'm')},
    //{PIX_FMT_XVMC_MPEG2_IDCT,   _TVU_LE_FOURCCTAG('v', 'm', 'c', 'i')},
    { AV_PIX_FMT_UYVY422,           _TVU_LE_FOURCCTAG('U', 'Y', 'V', 'Y')},
    { AV_PIX_FMT_UYYVYY411,         _TVU_LE_FOURCCTAG('Y', '4', '1', '1')},
    { AV_PIX_FMT_BGR8,              _TVU_LE_FOURCCTAG('B', 'G', 'R', 8  )},
    { AV_PIX_FMT_BGR4,              _TVU_LE_FOURCCTAG('B', 'G', 'R', 4  )},
    { AV_PIX_FMT_BGR4_BYTE,         _TVU_LE_FOURCCTAG('R', '4', 'B', 'Y')},
    { AV_PIX_FMT_RGB8,              _TVU_LE_FOURCCTAG('R', 'G', 'B', 8  )},
    { AV_PIX_FMT_RGB4,              _TVU_LE_FOURCCTAG('R', 'G', 'B', 4  )},
    { AV_PIX_FMT_RGB4_BYTE,         _TVU_LE_FOURCCTAG('B', '4', 'B', 'Y')},
    { AV_PIX_FMT_NV12,              _TVU_LE_FOURCCTAG('N', 'V', '1', '2')},
    { AV_PIX_FMT_NV21,              _TVU_LE_FOURCCTAG('N', 'V', '2', '1')},
    { AV_PIX_FMT_ARGB,              _TVU_LE_FOURCCTAG('A', 'R', 'G', 'B')},
    { AV_PIX_FMT_RGBA,              _TVU_LE_FOURCCTAG('R', 'G', 'B', 'A')},
    { AV_PIX_FMT_ABGR,              _TVU_LE_FOURCCTAG('A', 'B', 'G', 'R')},
    { AV_PIX_FMT_BGRA,              _TVU_LE_FOURCCTAG('B', 'G', 'R', 'A')},
    { AV_PIX_FMT_GRAY16BE,          _TVU_LE_FOURCCTAG(16 ,  0 , '1', 'Y')},
    { AV_PIX_FMT_GRAY16LE,          _TVU_LE_FOURCCTAG('Y', '1', 0  , 16 )},
    { AV_PIX_FMT_YUV440P,           _TVU_LE_FOURCCTAG('I', '4', '4', '0')},
    { AV_PIX_FMT_YUVJ440P,          _TVU_LE_FOURCCTAG('J', '4', '4', '0')},
    { AV_PIX_FMT_YUVA420P,          _TVU_LE_FOURCCTAG('Y', '4', 11 ,  8 )},
    //{AV_PIX_FMT_VDPAU_H264,        _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VDPAU_MPEG1,       _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VDPAU_MPEG2,       _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VDPAU_WMV3,        _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VDPAU_VC1,         _TVU_LE_FOURCCTAG('', '', '', '')},
    { AV_PIX_FMT_RGB48BE,           _TVU_LE_FOURCCTAG(48 , 'B', 'G', 'R')},
    { AV_PIX_FMT_RGB48LE,           _TVU_LE_FOURCCTAG('R', 'G', 'B', 48 )},
    { AV_PIX_FMT_RGB565BE,          _TVU_LE_FOURCCTAG(16 , 'B', 'G', 'R')},
    { AV_PIX_FMT_RGB565LE,          _TVU_LE_FOURCCTAG('R', 'G', 'B', 16 )},
    { AV_PIX_FMT_RGB555BE,          _TVU_LE_FOURCCTAG(15 , 'B', 'G', 'R')},
    { AV_PIX_FMT_RGB555LE,          _TVU_LE_FOURCCTAG('R', 'G', 'B', 15 )},
    { AV_PIX_FMT_BGR565BE,          _TVU_LE_FOURCCTAG(16 , 'R', 'G', 'B')},
    { AV_PIX_FMT_BGR565LE,          _TVU_LE_FOURCCTAG('B', 'G', 'R', 16 )},
    { AV_PIX_FMT_BGR555BE,          _TVU_LE_FOURCCTAG(15 , 'R', 'G', 'B')},
    { AV_PIX_FMT_BGR555LE,          _TVU_LE_FOURCCTAG('B', 'G', 'R', 15 )},
    //{AV_PIX_FMT_VAAPI_MOCO,        _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VAAPI_IDCT,        _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_VAAPI_VLD,         _TVU_LE_FOURCCTAG('', '', '', '')},
    { AV_PIX_FMT_YUV420P16LE,       _TVU_LE_FOURCCTAG('Y', '3', 11 , 16 )},
    { AV_PIX_FMT_YUV420P16BE,       _TVU_LE_FOURCCTAG(16 , 11 , '3', 'Y')},
    { AV_PIX_FMT_YUV422P16LE,       _TVU_LE_FOURCCTAG('Y', '3', 10 , 16 )},
    { AV_PIX_FMT_YUV422P16BE,       _TVU_LE_FOURCCTAG(16 , 10 , '3', 'Y')},
    { AV_PIX_FMT_YUV444P16LE,       _TVU_LE_FOURCCTAG('Y', '3', 0  , 16 )},
    { AV_PIX_FMT_YUV444P16BE,       _TVU_LE_FOURCCTAG(16 , 0  , '3', 'Y')},
    //{AV_PIX_FMT_VDPAU_MPEG4,       _TVU_LE_FOURCCTAG('', '', '', '')},
    //{AV_PIX_FMT_DXVA2_VLD,         _TVU_LE_FOURCCTAG('', '', '', '')},
    { AV_PIX_FMT_RGB444LE,          _TVU_LE_FOURCCTAG('R', 'G', 'B', 12 )},
    { AV_PIX_FMT_RGB444BE,          _TVU_LE_FOURCCTAG(12 , 'B', 'G', 'R')},
    { AV_PIX_FMT_BGR444LE,          _TVU_LE_FOURCCTAG('B', 'G', 'R', 12 )},
    { AV_PIX_FMT_BGR444BE,          _TVU_LE_FOURCCTAG(12 , 'R', 'G', 'B')},
    { AV_PIX_FMT_GRAY8A,            _TVU_LE_FOURCCTAG('Y', '2', 0  , 8  )},
    { AV_PIX_FMT_BGR48BE,           _TVU_LE_FOURCCTAG(48 , 'R', 'G', 'B')},
    { AV_PIX_FMT_BGR48LE,           _TVU_LE_FOURCCTAG('B', 'G', 'R', 48 )},
    { AV_PIX_FMT_YUV420P9BE,        _TVU_LE_FOURCCTAG(9  , 11 , '3', 'Y')},
    { AV_PIX_FMT_YUV420P9LE,        _TVU_LE_FOURCCTAG('Y', '3', 11 , 9  )},
    { AV_PIX_FMT_YUV420P10BE,       _TVU_LE_FOURCCTAG(10 , 11 , '3', 'Y')},
    { AV_PIX_FMT_YUV420P10LE,       _TVU_LE_FOURCCTAG('Y', '3', 11 , 10 )},
    { AV_PIX_FMT_YUV422P10BE,       _TVU_LE_FOURCCTAG(10 , 10 , '3', 'Y')},
    { AV_PIX_FMT_YUV422P10LE,       _TVU_LE_FOURCCTAG('Y', '3', 10 , 10 )},
    { AV_PIX_FMT_YUV444P9BE,        _TVU_LE_FOURCCTAG(9  , 3  , '3', 'Y')},
    { AV_PIX_FMT_YUV444P9LE,        _TVU_LE_FOURCCTAG('Y', '3', 0  , 9  )},
    { AV_PIX_FMT_YUV444P10BE,       _TVU_LE_FOURCCTAG(10 , 0  , '3', 'Y')},
    { AV_PIX_FMT_YUV444P10LE,       _TVU_LE_FOURCCTAG('Y', '3', 0  , 10 )},
    { AV_PIX_FMT_YUV422P9BE,        _TVU_LE_FOURCCTAG(9  , 10 , '3', 'Y')},
    { AV_PIX_FMT_YUV422P9LE,        _TVU_LE_FOURCCTAG('Y', '3', 10 , 9  )},
    //{AV_PIX_FMT_VDA_VLD,           _TVU_LE_FOURCCTAG('', '', '', '')},
    { AV_PIX_FMT_RGBA64BE,          _TVU_LE_FOURCCTAG(64 , 'R', 'B', 'A')},
    { AV_PIX_FMT_RGBA64LE,          _TVU_LE_FOURCCTAG('R', 'B', 'A', 64 )},
    { AV_PIX_FMT_BGRA64BE,          _TVU_LE_FOURCCTAG(64 , 'B', 'R', 'A')},
    { AV_PIX_FMT_BGRA64LE,          _TVU_LE_FOURCCTAG('B', 'R', 'A', 64 )},
    { AV_PIX_FMT_GBRP,              _TVU_LE_FOURCCTAG('G', 'B', 'R', 0  )},
    { AV_PIX_FMT_GBRP9BE,           _TVU_LE_FOURCCTAG(9  , 'R', 'B', 'G')},
    { AV_PIX_FMT_GBRP9LE,           _TVU_LE_FOURCCTAG('G', 'B', 'R', 9  )},
    { AV_PIX_FMT_GBRP10BE,          _TVU_LE_FOURCCTAG(10 , 'R', 'B', 'G')},
    { AV_PIX_FMT_GBRP10LE,          _TVU_LE_FOURCCTAG('G', 'B', 'R', 10 )},
    { AV_PIX_FMT_GBRP16BE,          _TVU_LE_FOURCCTAG(16 , 'R', 'B', 'G')},
    { AV_PIX_FMT_GBRP16LE,          _TVU_LE_FOURCCTAG('G', 'B', 'R', 16 )},
    { AV_PIX_FMT_0RGB,              _TVU_LE_FOURCCTAG(0  , 'R', 'G', 'B')},
    { AV_PIX_FMT_RGB0,              _TVU_LE_FOURCCTAG('R', 'G', 'B', 0  )},
    { AV_PIX_FMT_0BGR,              _TVU_LE_FOURCCTAG(0  , 'B', 'G', 'R')},
    { AV_PIX_FMT_BGR0,              _TVU_LE_FOURCCTAG('B', 'G', 'R', 0  )},
    { AV_PIX_FMT_YUVA444P,          _TVU_LE_FOURCCTAG('Y', '4', 0  , 8  )},
    { AV_PIX_FMT_YUVA422P,          _TVU_LE_FOURCCTAG('Y', '4', 10 , 8  )},
    { AV_PIX_FMT_YUV420P12BE,       _TVU_LE_FOURCCTAG(12 , 11 , '3', 'Y')},
    { AV_PIX_FMT_YUV420P12LE,       _TVU_LE_FOURCCTAG('Y', '3', 11 , 12 )},
    { AV_PIX_FMT_YUV420P14BE,       _TVU_LE_FOURCCTAG(14 , 11 , '3', 'Y')},
    { AV_PIX_FMT_YUV420P14LE,       _TVU_LE_FOURCCTAG('Y', '3', 11 , 14 )},
    { AV_PIX_FMT_YUV422P12BE,       _TVU_LE_FOURCCTAG(12 , 10 , '3', 'Y')},
    { AV_PIX_FMT_YUV422P12LE,       _TVU_LE_FOURCCTAG('Y', '3', 10 , 12 )},
    { AV_PIX_FMT_YUV422P14BE,       _TVU_LE_FOURCCTAG(14 , 10 , '3', 'Y')},
    { AV_PIX_FMT_YUV422P14LE,       _TVU_LE_FOURCCTAG('Y', '3', 10 , 14 )},
    { AV_PIX_FMT_YUV444P12BE,       _TVU_LE_FOURCCTAG(12 , 0  , '3', 'Y')},
    { AV_PIX_FMT_YUV444P12LE,       _TVU_LE_FOURCCTAG('Y', '3', 0  , 12 )},
    { AV_PIX_FMT_YUV444P14BE,       _TVU_LE_FOURCCTAG(14 , 0  , '3', 'Y')},
    { AV_PIX_FMT_YUV444P14LE,       _TVU_LE_FOURCCTAG('Y', '3', 0  , 14 )},
    { AV_PIX_FMT_GBRP12BE,          _TVU_LE_FOURCCTAG(12 , 'R', 'B', 'G')},
    { AV_PIX_FMT_GBRP12LE,          _TVU_LE_FOURCCTAG('G', 'B', 'R', 12 )},
    { AV_PIX_FMT_GBRP14BE,          _TVU_LE_FOURCCTAG(14 , 'R', 'B', 'G')},
    { AV_PIX_FMT_GBRP14LE,          _TVU_LE_FOURCCTAG('G', 'B', 'R', 14 )},
    { AV_PIX_FMT_NONE,              0                            },
};

int main(int argc, const char *argv[])
{
    unsigned int fourcc = K_TVU_PIXFMT_VIDEO_FOURCC_BGR444LE;
    unsigned int pixfmt = (unsigned int)AV_PIX_FMT_ABGR;
    unsigned int map_pixfmt = (unsigned int) AV_PIX_FMT_BGR444LE;
    unsigned int map_fourcc = K_TVU_PIXFMT_VIDEO_FOURCC_ABGR;

    int ret = TvuFourccPixfmtTableInit(sizeof(_video_chroma_table)/sizeof(_video_chroma_table[0]), _video_chroma_table);

    if (ret < 0)
    {
        printf("table init failed\n");
        return ret;
    }

    unsigned int map_pixfmt1 = (unsigned int )AV_PIX_FMT_BGR444LE;
    const struct STvuPixFmtFourccMap * ptmp1 = TvuFourccPixfmtTableFindPixfmtByFourcc(_video_chroma_table, sizeof(_video_chroma_table)/sizeof(_video_chroma_table[0]), fourcc);
    if (ptmp1)
    {
        map_pixfmt1 = ptmp1->pix_fmt;
    }

    unsigned int map_fourcc1 = 0;
    const struct STvuPixFmtFourccMap * ptmp2 = TvuFourccPixfmtTableFindFourccByPixfmt(_video_chroma_table, sizeof(_video_chroma_table)/sizeof(_video_chroma_table[0]), pixfmt);
    if (ptmp2)
    {
        map_fourcc1 = ptmp2->fourcc;
    }

    printf("fourcc[%c,%c, %c, %c], pixfmt %d, map %d\n"
        , fourcc & 0xFF, (fourcc >> 8)&0xFF, (fourcc >> 16)&0xFF, (fourcc >> 24)&0xFF
        , map_pixfmt, map_pixfmt1);
    printf("fourcc[%c,%c, %c, %c], map fourcc[%c,%c, %c, %c], pixfmt %d\n"
        , map_fourcc & 0xFF, (map_fourcc >> 8)&0xFF, (map_fourcc >> 16)&0xFF, (map_fourcc >> 24)&0xFF
        , map_fourcc1 & 0xFF, (map_fourcc1 >> 8)&0xFF, (map_fourcc1 >> 16)&0xFF, (map_fourcc1 >> 24)&0xFF
        , pixfmt);

    return 0;
}


