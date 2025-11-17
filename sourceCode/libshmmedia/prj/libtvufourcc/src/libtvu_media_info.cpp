
#include "libtvu_media_info.h"
#include <stdio.h>
#include <strings.h>

#define FPS_2398_DURATION   1001
#define FPS_2398_SCALE      24000
#define FPS_2400_DURATION   1000
#define FPS_2400_SCALE      24000
#define FPS_2997_DURATION   1001
#define FPS_2997_SCALE      30000
#define FPS_3000_DURATION   1000
#define FPS_3000_SCALE      30000
#define FPS_2500_DURATION   1000
#define FPS_2500_SCALE      25000
#define FPS_5000_DURATION   1000
#define FPS_5000_SCALE      50000
#define FPS_5994_DURATION   1001
#define FPS_5994_SCALE      60000
#define FPS_6000_DURATION   1000
#define FPS_6000_SCALE      60000
#define FPS_9000_DURATION   1000
#define FPS_9000_SCALE      90000
#define FPS_12000_DURATION   1000
#define FPS_12000_SCALE      120000
#define FPS_15000_DURATION   1000
#define FPS_15000_SCALE      150000
#define FPS_18000_DURATION   1000
#define FPS_18000_SCALE      180000
#define FPS_21000_DURATION   1000
#define FPS_21000_SCALE      210000
#define FPS_24000_DURATION   1000
#define FPS_24000_SCALE      240000
#define FPS_27000_DURATION   1000
#define FPS_27000_SCALE      270000
#define FPS_30000_DURATION   1000
#define FPS_30000_SCALE      300000
#define RES_SD_480__WIDTH   720
#define RES_SD_480__HEIGHT  480
#define RES_SD_576__WIDTH   720
#define RES_SD_576__HEIGHT  576
#define RES_NHD_640_WIDTH   640
#define RES_NHD_640_HEIGHT  360
#define RES_HD_1080_WIDTH   1920
#define RES_HD_1080_HEIGHT  1080
#define RES_HD_720__WIDTH   1280
#define RES_HD_720__HEIGHT  720
#define RES_4K_2160_WIDTH   3840
#define RES_4K_2160_HEIGHT  2160
#define RES_4K_DCI__WIDTH   4096
#define RES_4K_DCI__HEIGHT  2160
#define RES_2K_DCI__WIDTH   2048 //DCI digital cinema
#define RES_2K_DCI__HEIGHT  1080
#define RES_2K_1556_WIDTH   2048
#define RES_2K_1556_HEIGHT  1556


#define LIBTVUMEDIA_VIDEOFMTID_ENTRY_MAXNUM_1    kLibTvuMediaVideoFmtId_MaxNum1

static const libtvu_media_info_video_fmt_entry_t _gTvuMediaInfoVideoTbl[LIBTVUMEDIA_VIDEOFMTID_ENTRY_MAXNUM_1] =
{
    {kLibTvuMediaVideoFmtId_NTSC         ,kLibTvuMediaVideoFmtFourccNTSC,           RES_SD_480__WIDTH, RES_SD_480__HEIGHT, FPS_2997_DURATION, FPS_2997_SCALE, 1, "ntsc", NULL},
    {kLibTvuMediaVideoFmtId_NTSC2398     ,kLibTvuMediaVideoFmtFourccNTSC2398,       RES_SD_480__WIDTH, RES_SD_480__HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 1, "ntsc2398", NULL},
    {kLibTvuMediaVideoFmtId_PAL          ,kLibTvuMediaVideoFmtFourccPAL,            RES_SD_576__WIDTH, RES_SD_576__HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 1, "pal", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P2398  ,kLibTvuMediaVideoFmtFourccHD1080p2398,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 0, "1080p2398", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P24    ,kLibTvuMediaVideoFmtFourccHD1080p24,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "1080p24", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P25    ,kLibTvuMediaVideoFmtFourccHD1080p25,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "1080p25", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P2997  ,kLibTvuMediaVideoFmtFourccHD1080p2997,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2997_DURATION, FPS_2997_SCALE, 0, "1080p2997", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P30    ,kLibTvuMediaVideoFmtFourccHD1080p30,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_3000_DURATION, FPS_3000_SCALE, 0, "1080p30", NULL},
    {kLibTvuMediaVideoFmtId_HD1080I50    ,kLibTvuMediaVideoFmtFourccHD1080i50,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 1, "1080i50", NULL},
    {kLibTvuMediaVideoFmtId_HD1080I5994  ,kLibTvuMediaVideoFmtFourccHD1080i5994,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_2997_DURATION, FPS_2997_SCALE, 1, "1080i5994", NULL},
    {kLibTvuMediaVideoFmtId_HD1080I6000  ,kLibTvuMediaVideoFmtFourccHD1080i6000,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_3000_DURATION, FPS_3000_SCALE, 1, "1080i60", NULL},
    {kLibTvuMediaVideoFmtId_HD720P50     ,kLibTvuMediaVideoFmtFourccHD720p50,       RES_HD_720__WIDTH, RES_HD_720__HEIGHT, FPS_5000_DURATION, FPS_5000_SCALE, 0, "720p50", NULL},
    {kLibTvuMediaVideoFmtId_HD720P5994   ,kLibTvuMediaVideoFmtFourccHD720p5994,     RES_HD_720__WIDTH, RES_HD_720__HEIGHT, FPS_5994_DURATION, FPS_5994_SCALE, 0, "720p5994", NULL},
    {kLibTvuMediaVideoFmtId_HD720P60     ,kLibTvuMediaVideoFmtFourccHD720p60,       RES_HD_720__WIDTH, RES_HD_720__HEIGHT, FPS_6000_DURATION, FPS_6000_SCALE, 0, "720p60", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P50    ,kLibTvuMediaVideoFmtFourccHD1080p50,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_5000_DURATION, FPS_5000_SCALE, 0, "1080p50", NULL},
    {kLibTvuMediaVideoFmtId_HD1080P5994  ,kLibTvuMediaVideoFmtFourccHD1080p5994,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_5994_DURATION, FPS_5994_SCALE, 0, "1080p5994", NULL},
    {kLibTvuMediaVideoFmtId_4K2160P25    ,kLibTvuMediaVideoFmtFourcc4K2160p25,      RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "2160p25", "4kp25"},
    {kLibTvuMediaVideoFmtId_4K2160P2997  ,kLibTvuMediaVideoFmtFourcc4K2160p2997,    RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_2997_DURATION, FPS_2997_SCALE, 0, "2160p2997", "4kp2997"},
    {kLibTvuMediaVideoFmtId_2KDCI24      ,kLibTvuMediaVideoFmtFourcc2kDCI24,        RES_2K_DCI__WIDTH, RES_2K_DCI__HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "2kdci24", NULL},
    {kLibTvuMediaVideoFmtId_2KDCI25      ,kLibTvuMediaVideoFmtFourcc2kDCI25,        RES_2K_DCI__WIDTH, RES_2K_DCI__HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "2kdci25", NULL},
    {kLibTvuMediaVideoFmtId_4K2160P2398  ,kLibTvuMediaVideoFmtFourcc4K2160p2398,    RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 0, "2160p2398", "4kp2398"},
    {kLibTvuMediaVideoFmtId_4K2160P24    ,kLibTvuMediaVideoFmtFourcc4K2160p24,      RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "2160p24", "4kp24"},
    {kLibTvuMediaVideoFmtId_HD1080P6000  ,kLibTvuMediaVideoFmtFourccHD1080p6000,    RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_6000_DURATION, FPS_6000_SCALE, 0, "1080p60", NULL},
    {kLibTvuMediaVideoFmtId_2KDCI2398    ,kLibTvuMediaVideoFmtFourcc2kDCI2398,      RES_2K_DCI__WIDTH, RES_2K_DCI__HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 0, "2kdci2398", NULL},
    {kLibTvuMediaVideoFmtId_4K2160P30    ,kLibTvuMediaVideoFmtFourcc4K2160p30,      RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_3000_DURATION, FPS_3000_SCALE, 0, "2160p30", "4kp30"},
    {kLibTvuMediaVideoFmtId_4KDCI2398    ,kLibTvuMediaVideoFmtFourcc4kDCI2398,      RES_4K_DCI__WIDTH, RES_4K_DCI__HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 0, "4kdci2398", NULL},
    {kLibTvuMediaVideoFmtId_4KDCI24      ,kLibTvuMediaVideoFmtFourcc4kDCI24,        RES_4K_DCI__WIDTH, RES_4K_DCI__HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "4kdci24", NULL},
    {kLibTvuMediaVideoFmtId_4KDCI25      ,kLibTvuMediaVideoFmtFourcc4kDCI25,        RES_4K_DCI__WIDTH, RES_4K_DCI__HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "4kdci25", NULL},
    {kLibTvuMediaVideoFmtId_4K2160P5994  ,kLibTvuMediaVideoFmtFourcc4K2160p5994,    RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_5994_DURATION, FPS_5994_SCALE, 0, "2160p5994", "4kp5994"},
    {kLibTvuMediaVideoFmtId_4K2160P60    ,kLibTvuMediaVideoFmtFourcc4K2160p60,      RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_6000_DURATION, FPS_6000_SCALE, 0, "2160p60", "4kp60"},
    {kLibTvuMediaVideoFmtId_4K2160P50    ,kLibTvuMediaVideoFmtFourcc4K2160p50,      RES_4K_2160_WIDTH, RES_4K_2160_HEIGHT, FPS_5000_DURATION, FPS_5000_SCALE, 0, "2160p50", "4kp50"},
    {kLibTvuMediaVideoFmtId_NTSCp        ,kLibTvuMediaVideoFmtFourccNTSCp,          RES_SD_480__WIDTH, RES_SD_480__HEIGHT, FPS_2997_DURATION, FPS_2997_SCALE, 0, "ntscp", NULL},
    {kLibTvuMediaVideoFmtId_PALp         ,kLibTvuMediaVideoFmtFourccPALp,           RES_SD_576__WIDTH, RES_SD_576__HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "palp", NULL},
    {kLibTvuMediaVideoFmtId_2K2398       ,kLibTvuMediaVideoFmtFourcc2k2398,         RES_2K_1556_WIDTH, RES_2K_1556_HEIGHT, FPS_2398_DURATION, FPS_2398_SCALE, 0, "1556p2398", NULL},
    {kLibTvuMediaVideoFmtId_2K24         ,kLibTvuMediaVideoFmtFourcc2k24,           RES_2K_1556_WIDTH, RES_2K_1556_HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "1556p24", NULL},
    {kLibTvuMediaVideoFmtId_2K25         ,kLibTvuMediaVideoFmtFourcc2k25,           RES_2K_1556_WIDTH, RES_2K_1556_HEIGHT, FPS_2500_DURATION, FPS_2500_SCALE, 0, "1556p24", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P90    ,kLibTvuMediaVideoFmtFourccHD1080P90,      RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_9000_DURATION,  FPS_9000_SCALE, 0, "1080p90", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P120   ,kLibTvuMediaVideoFmtFourccHD1080p120,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_12000_DURATION, FPS_12000_SCALE, 0, "1080p120", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P150   ,kLibTvuMediaVideoFmtFourccHD1080P150,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_15000_DURATION, FPS_15000_SCALE, 0, "1080p150", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P180   ,kLibTvuMediaVideoFmtFourccHD1080P180,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_18000_DURATION, FPS_18000_SCALE, 0, "1080p180", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P210   ,kLibTvuMediaVideoFmtFourccHD1080P210,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_21000_DURATION, FPS_21000_SCALE, 0, "1080p210", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P240   ,kLibTvuMediaVideoFmtFourccHD1080P240,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_24000_DURATION, FPS_24000_SCALE, 0, "1080p240", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P270   ,kLibTvuMediaVideoFmtFourccHD1080P270,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_27000_DURATION, FPS_27000_SCALE, 0, "1080p270", NULL},
//    {kLibTvuMediaVideoFmtId_HD1080P300   ,kLibTvuMediaVideoFmtFourccHD1080P300,     RES_HD_1080_WIDTH, RES_HD_1080_HEIGHT, FPS_30000_DURATION, FPS_30000_SCALE, 0, "1080p300", NULL},
//    {
//        kLibTvuMediaVideoFmtId_HD720P2398,
//        kLibTvuMediaVideoFmtFourccHD720p2398,
//        RES_HD_720__WIDTH,
//        RES_HD_720__HEIGHT,
//        FPS_2398_DURATION,
//        FPS_2398_SCALE,
//        0,
//        "720p2398",
//        NULL
//    },
//    {kLibTvuMediaVideoFmtId_NHDP24       ,kLibTvuMediaVideoFmtFourccNHDP24,         RES_NHD_640_WIDTH, RES_NHD_640_HEIGHT, FPS_2400_DURATION, FPS_2400_SCALE, 0, "nhdp24", NULL},
//    {kLibTvuMediaVideoFmtId_NHDP30       ,kLibTvuMediaVideoFmtFourccNHDP30,         RES_NHD_640_WIDTH, RES_NHD_640_HEIGHT, FPS_3000_DURATION, FPS_3000_SCALE, 0, "nhdp30", NULL},
//    {kLibTvuMediaVideoFmtId_NHDP60       ,kLibTvuMediaVideoFmtFourccNHDP60,         RES_NHD_640_WIDTH, RES_NHD_640_HEIGHT, FPS_6000_DURATION, FPS_6000_SCALE, 0, "nhdp60", NULL},
};

const libtvu_media_info_video_fmt_entry_t* LibTvuMediaInfoFindEntryByFmtid(unsigned int id)
{
    unsigned int n = sizeof(_gTvuMediaInfoVideoTbl)/sizeof(_gTvuMediaInfoVideoTbl[0]);
    if (id >= n)
        return NULL;
    return &_gTvuMediaInfoVideoTbl[id];
}

const libtvu_media_info_video_fmt_entry_t* LibTvuMediaInfoFindEntryByName(const char *name)
{
    unsigned int n = sizeof(_gTvuMediaInfoVideoTbl)/sizeof(_gTvuMediaInfoVideoTbl[0]);
    if (!name)
        return NULL;
    for (unsigned int i = 0; i < n; i++)
    {
        const libtvu_media_info_video_fmt_entry_t &node = _gTvuMediaInfoVideoTbl[i];
        if (!strcasecmp(node.name, name))
        {
            return &node;
        }

        if (node.aliasName && !strcasecmp(node.aliasName, name))
        {
            return &node;
        }
    }
    return NULL;
}
