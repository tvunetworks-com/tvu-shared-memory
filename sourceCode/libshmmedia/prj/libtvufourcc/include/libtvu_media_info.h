#ifndef LIBVU_MEDIA_INFO_H
#define LIBVU_MEDIA_INFO_H

typedef enum eLibTvuMediaVideoFormatId
{
    kLibTvuMediaVideoFmtId_NTSC         = 0,
    kLibTvuMediaVideoFmtId_NTSC2398     = 1,
    kLibTvuMediaVideoFmtId_PAL          = 2,
    kLibTvuMediaVideoFmtId_HD1080P2398  = 3,
    kLibTvuMediaVideoFmtId_HD1080P24    = 4,
    kLibTvuMediaVideoFmtId_HD1080P25    = 5,
    kLibTvuMediaVideoFmtId_HD1080P2997  = 6,
    kLibTvuMediaVideoFmtId_HD1080P30    = 7,
    kLibTvuMediaVideoFmtId_HD1080I50    = 8,
    kLibTvuMediaVideoFmtId_HD1080I5994  = 9,
    kLibTvuMediaVideoFmtId_HD1080I6000  = 10,
    kLibTvuMediaVideoFmtId_HD720P50     = 11,
    kLibTvuMediaVideoFmtId_HD720P5994   = 12,
    kLibTvuMediaVideoFmtId_HD720P60     = 13,
    kLibTvuMediaVideoFmtId_HD1080P50    = 14,
    kLibTvuMediaVideoFmtId_HD1080P5994  = 15,
    kLibTvuMediaVideoFmtId_4K2160P25    = 16,
    kLibTvuMediaVideoFmtId_4K2160P2997  = 17,
    kLibTvuMediaVideoFmtId_2KDCI24      = 18,
    kLibTvuMediaVideoFmtId_2KDCI25      = 19,
    kLibTvuMediaVideoFmtId_4K2160P2398  = 20,
    kLibTvuMediaVideoFmtId_4K2160P24    = 21,
    kLibTvuMediaVideoFmtId_HD1080P6000  = 22,
    kLibTvuMediaVideoFmtId_2KDCI2398    = 23,
    kLibTvuMediaVideoFmtId_4K2160P30    = 24,
    kLibTvuMediaVideoFmtId_4KDCI2398    = 25,
    kLibTvuMediaVideoFmtId_4KDCI24      = 26,
    kLibTvuMediaVideoFmtId_4KDCI25      = 27,
    kLibTvuMediaVideoFmtId_4K2160P5994  = 28,
    kLibTvuMediaVideoFmtId_4K2160P60    = 29,
    kLibTvuMediaVideoFmtId_4K2160P50    = 30,
    kLibTvuMediaVideoFmtId_NTSCp        = 31,
    kLibTvuMediaVideoFmtId_PALp         = 32,
    kLibTvuMediaVideoFmtId_2K2398       = 33,
    kLibTvuMediaVideoFmtId_2K24         = 34,
    kLibTvuMediaVideoFmtId_2K25         = 35,

    kLibTvuMediaVideoFmtId_MaxNum1      = 36,

#if 0 /* it could be open for future, but hide them at first */
    // defined for ai frame interpolation
    kLibTvuMediaVideoFmtId_HD1080P90    = 80,
    kLibTvuMediaVideoFmtId_HD1080P120   = 81,
    kLibTvuMediaVideoFmtId_HD1080P150   = 82,
    kLibTvuMediaVideoFmtId_HD1080P180   = 83,
    kLibTvuMediaVideoFmtId_HD1080P210   = 84,
    kLibTvuMediaVideoFmtId_HD1080P240   = 85,
    kLibTvuMediaVideoFmtId_HD1080P270   = 86,
    kLibTvuMediaVideoFmtId_HD1080P300   = 87,

    // defined format for HD720p2398
    kLibTvuMediaVideoFmtId_HD720P2398   = 100,

    // defined format for nhd(640*360), Ninth HD
    kLibTvuMediaVideoFmtId_NHDP24       = 110,
    kLibTvuMediaVideoFmtId_NHDP30       = 111,
    kLibTvuMediaVideoFmtId_NHDP60       = 112,
#endif

    kLibTvuMediaVideoFmtId_NONE         = 0xffffffff,
}libtvu_media_video_format_id_t;

typedef enum _eLibTvuMediaVideoFormatFourccMode {

    /* SD Modes */

    kLibTvuMediaVideoFmtFourccNTSC                                                  = /* 'ntsc' */ 0x6E747363,
    kLibTvuMediaVideoFmtFourccNTSC2398                                              = /* 'nt23' */ 0x6E743233, // 3:2 pulldown
    kLibTvuMediaVideoFmtFourccPAL                                                   = /* 'pal ' */ 0x70616C20,
    kLibTvuMediaVideoFmtFourccNTSCp                                                 = /* 'ntsp' */ 0x6E747370,
    kLibTvuMediaVideoFmtFourccPALp                                                  = /* 'palp' */ 0x70616C70,

    /* HD 1080 Modes */

    kLibTvuMediaVideoFmtFourccHD1080p2398                                           = /* '23ps' */ 0x32337073,
    kLibTvuMediaVideoFmtFourccHD1080p24                                             = /* '24ps' */ 0x32347073,
    kLibTvuMediaVideoFmtFourccHD1080p25                                             = /* 'Hp25' */ 0x48703235,
    kLibTvuMediaVideoFmtFourccHD1080p2997                                           = /* 'Hp29' */ 0x48703239,
    kLibTvuMediaVideoFmtFourccHD1080p30                                             = /* 'Hp30' */ 0x48703330,
    kLibTvuMediaVideoFmtFourccHD1080p4795                                           = /* 'Hp47' */ 0x48703437,
    kLibTvuMediaVideoFmtFourccHD1080p48                                             = /* 'Hp48' */ 0x48703438,
    kLibTvuMediaVideoFmtFourccHD1080p50                                             = /* 'Hp50' */ 0x48703530,
    kLibTvuMediaVideoFmtFourccHD1080p5994                                           = /* 'Hp59' */ 0x48703539,
    kLibTvuMediaVideoFmtFourccHD1080p6000                                           = /* 'Hp60' */ 0x48703630, // N.B. This _really_ is 60.00 Hz.
    kLibTvuMediaVideoFmtFourccHD1080p9590                                           = /* 'Hp95' */ 0x48703935,
    kLibTvuMediaVideoFmtFourccHD1080p96                                             = /* 'Hp96' */ 0x48703936,
    kLibTvuMediaVideoFmtFourccHD1080p100                                            = /* 'Hp10' */ 0x48703130,
    kLibTvuMediaVideoFmtFourccHD1080p11988                                          = /* 'Hp11' */ 0x48703131,
    kLibTvuMediaVideoFmtFourccHD1080p120                                            = /* 'Hp12' */ 0x48703132,
    kLibTvuMediaVideoFmtFourccHD1080i50                                             = /* 'Hi50' */ 0x48693530,
    kLibTvuMediaVideoFmtFourccHD1080i5994                                           = /* 'Hi59' */ 0x48693539,
    kLibTvuMediaVideoFmtFourccHD1080i6000                                           = /* 'Hi60' */ 0x48693630, // N.B. This _really_ is 60.00 Hz.
    kLibTvuMediaVideoFmtFourccHD1080P90                                             = /* 'Hp90' */ 0x48703930,
    kLibTvuMediaVideoFmtFourccHD1080P150                                            = /* 'Hp15' */ 0x48703135,
    kLibTvuMediaVideoFmtFourccHD1080P180                                            = /* 'Hp18' */ 0x48703138,
    kLibTvuMediaVideoFmtFourccHD1080P210                                            = /* 'Hp21' */ 0x48703231,
    kLibTvuMediaVideoFmtFourccHD1080P240                                            = /* 'Hp24' */ 0x48703234,
    kLibTvuMediaVideoFmtFourccHD1080P270                                            = /* 'Hp27' */ 0x48703237,
    kLibTvuMediaVideoFmtFourccHD1080P300                                            = /* 'p300' */ 0x70333030,

    /* HD 720 Modes */
    kLibTvuMediaVideoFmtFourccHD720p2398                                            = /* 'hp23' */ 0x68703233,
    kLibTvuMediaVideoFmtFourccHD720p50                                              = /* 'hp50' */ 0x68703530,
    kLibTvuMediaVideoFmtFourccHD720p5994                                            = /* 'hp59' */ 0x68703539,
    kLibTvuMediaVideoFmtFourccHD720p60                                              = /* 'hp60' */ 0x68703630,

    /* 2K Modes */

    kLibTvuMediaVideoFmtFourcc2k2398                                                = /* '2k23' */ 0x326B3233,
    kLibTvuMediaVideoFmtFourcc2k24                                                  = /* '2k24' */ 0x326B3234,
    kLibTvuMediaVideoFmtFourcc2k25                                                  = /* '2k25' */ 0x326B3235,

    /* 2K DCI Modes */

    kLibTvuMediaVideoFmtFourcc2kDCI2398                                             = /* '2d23' */ 0x32643233,
    kLibTvuMediaVideoFmtFourcc2kDCI24                                               = /* '2d24' */ 0x32643234,
    kLibTvuMediaVideoFmtFourcc2kDCI25                                               = /* '2d25' */ 0x32643235,
    kLibTvuMediaVideoFmtFourcc2kDCI2997                                             = /* '2d29' */ 0x32643239,
    kLibTvuMediaVideoFmtFourcc2kDCI30                                               = /* '2d30' */ 0x32643330,
    kLibTvuMediaVideoFmtFourcc2kDCI4795                                             = /* '2d47' */ 0x32643437,
    kLibTvuMediaVideoFmtFourcc2kDCI48                                               = /* '2d48' */ 0x32643438,
    kLibTvuMediaVideoFmtFourcc2kDCI50                                               = /* '2d50' */ 0x32643530,
    kLibTvuMediaVideoFmtFourcc2kDCI5994                                             = /* '2d59' */ 0x32643539,
    kLibTvuMediaVideoFmtFourcc2kDCI60                                               = /* '2d60' */ 0x32643630,
    kLibTvuMediaVideoFmtFourcc2kDCI9590                                             = /* '2d95' */ 0x32643935,
    kLibTvuMediaVideoFmtFourcc2kDCI96                                               = /* '2d96' */ 0x32643936,
    kLibTvuMediaVideoFmtFourcc2kDCI100                                              = /* '2d10' */ 0x32643130,
    kLibTvuMediaVideoFmtFourcc2kDCI11988                                            = /* '2d11' */ 0x32643131,
    kLibTvuMediaVideoFmtFourcc2kDCI120                                              = /* '2d12' */ 0x32643132,

    /* 4K UHD Modes */

    kLibTvuMediaVideoFmtFourcc4K2160p2398                                           = /* '4k23' */ 0x346B3233,
    kLibTvuMediaVideoFmtFourcc4K2160p24                                             = /* '4k24' */ 0x346B3234,
    kLibTvuMediaVideoFmtFourcc4K2160p25                                             = /* '4k25' */ 0x346B3235,
    kLibTvuMediaVideoFmtFourcc4K2160p2997                                           = /* '4k29' */ 0x346B3239,
    kLibTvuMediaVideoFmtFourcc4K2160p30                                             = /* '4k30' */ 0x346B3330,
    kLibTvuMediaVideoFmtFourcc4K2160p4795                                           = /* '4k47' */ 0x346B3437,
    kLibTvuMediaVideoFmtFourcc4K2160p48                                             = /* '4k48' */ 0x346B3438,
    kLibTvuMediaVideoFmtFourcc4K2160p50                                             = /* '4k50' */ 0x346B3530,
    kLibTvuMediaVideoFmtFourcc4K2160p5994                                           = /* '4k59' */ 0x346B3539,
    kLibTvuMediaVideoFmtFourcc4K2160p60                                             = /* '4k60' */ 0x346B3630,
    kLibTvuMediaVideoFmtFourcc4K2160p9590                                           = /* '4k95' */ 0x346B3935,
    kLibTvuMediaVideoFmtFourcc4K2160p96                                             = /* '4k96' */ 0x346B3936,
    kLibTvuMediaVideoFmtFourcc4K2160p100                                            = /* '4k10' */ 0x346B3130,
    kLibTvuMediaVideoFmtFourcc4K2160p11988                                          = /* '4k11' */ 0x346B3131,
    kLibTvuMediaVideoFmtFourcc4K2160p120                                            = /* '4k12' */ 0x346B3132,

    /* 4K DCI Modes */

    kLibTvuMediaVideoFmtFourcc4kDCI2398                                             = /* '4d23' */ 0x34643233,
    kLibTvuMediaVideoFmtFourcc4kDCI24                                               = /* '4d24' */ 0x34643234,
    kLibTvuMediaVideoFmtFourcc4kDCI25                                               = /* '4d25' */ 0x34643235,
    kLibTvuMediaVideoFmtFourcc4kDCI2997                                             = /* '4d29' */ 0x34643239,
    kLibTvuMediaVideoFmtFourcc4kDCI30                                               = /* '4d30' */ 0x34643330,
    kLibTvuMediaVideoFmtFourcc4kDCI4795                                             = /* '4d47' */ 0x34643437,
    kLibTvuMediaVideoFmtFourcc4kDCI48                                               = /* '4d48' */ 0x34643438,
    kLibTvuMediaVideoFmtFourcc4kDCI50                                               = /* '4d50' */ 0x34643530,
    kLibTvuMediaVideoFmtFourcc4kDCI5994                                             = /* '4d59' */ 0x34643539,
    kLibTvuMediaVideoFmtFourcc4kDCI60                                               = /* '4d60' */ 0x34643630,
    kLibTvuMediaVideoFmtFourcc4kDCI9590                                             = /* '4d95' */ 0x34643935,
    kLibTvuMediaVideoFmtFourcc4kDCI96                                               = /* '4d96' */ 0x34643936,
    kLibTvuMediaVideoFmtFourcc4kDCI100                                              = /* '4d10' */ 0x34643130,
    kLibTvuMediaVideoFmtFourcc4kDCI11988                                            = /* '4d11' */ 0x34643131,
    kLibTvuMediaVideoFmtFourcc4kDCI120                                              = /* '4d12' */ 0x34643132,

    /* 8K UHD Modes */

    kLibTvuMediaVideoFmtFourcc8K4320p2398                                           = /* '8k23' */ 0x386B3233,
    kLibTvuMediaVideoFmtFourcc8K4320p24                                             = /* '8k24' */ 0x386B3234,
    kLibTvuMediaVideoFmtFourcc8K4320p25                                             = /* '8k25' */ 0x386B3235,
    kLibTvuMediaVideoFmtFourcc8K4320p2997                                           = /* '8k29' */ 0x386B3239,
    kLibTvuMediaVideoFmtFourcc8K4320p30                                             = /* '8k30' */ 0x386B3330,
    kLibTvuMediaVideoFmtFourcc8K4320p4795                                           = /* '8k47' */ 0x386B3437,
    kLibTvuMediaVideoFmtFourcc8K4320p48                                             = /* '8k48' */ 0x386B3438,
    kLibTvuMediaVideoFmtFourcc8K4320p50                                             = /* '8k50' */ 0x386B3530,
    kLibTvuMediaVideoFmtFourcc8K4320p5994                                           = /* '8k59' */ 0x386B3539,
    kLibTvuMediaVideoFmtFourcc8K4320p60                                             = /* '8k60' */ 0x386B3630,

    /* 8K DCI Modes */

    kLibTvuMediaVideoFmtFourcc8kDCI2398                                             = /* '8d23' */ 0x38643233,
    kLibTvuMediaVideoFmtFourcc8kDCI24                                               = /* '8d24' */ 0x38643234,
    kLibTvuMediaVideoFmtFourcc8kDCI25                                               = /* '8d25' */ 0x38643235,
    kLibTvuMediaVideoFmtFourcc8kDCI2997                                             = /* '8d29' */ 0x38643239,
    kLibTvuMediaVideoFmtFourcc8kDCI30                                               = /* '8d30' */ 0x38643330,
    kLibTvuMediaVideoFmtFourcc8kDCI4795                                             = /* '8d47' */ 0x38643437,
    kLibTvuMediaVideoFmtFourcc8kDCI48                                               = /* '8d48' */ 0x38643438,
    kLibTvuMediaVideoFmtFourcc8kDCI50                                               = /* '8d50' */ 0x38643530,
    kLibTvuMediaVideoFmtFourcc8kDCI5994                                             = /* '8d59' */ 0x38643539,
    kLibTvuMediaVideoFmtFourcc8kDCI60                                               = /* '8d60' */ 0x38643630,

    /* PC Modes */

    kLibTvuMediaVideoFmtFourcc640x480p60                                            = /* 'vga6' */ 0x76676136,
    kLibTvuMediaVideoFmtFourcc800x600p60                                            = /* 'svg6' */ 0x73766736,
    kLibTvuMediaVideoFmtFourcc1440x900p50                                           = /* 'wxg5' */ 0x77786735,
    kLibTvuMediaVideoFmtFourcc1440x900p60                                           = /* 'wxg6' */ 0x77786736,
    kLibTvuMediaVideoFmtFourcc1440x1080p50                                          = /* 'sxg5' */ 0x73786735,
    kLibTvuMediaVideoFmtFourcc1440x1080p60                                          = /* 'sxg6' */ 0x73786736,
    kLibTvuMediaVideoFmtFourcc1600x1200p50                                          = /* 'uxg5' */ 0x75786735,
    kLibTvuMediaVideoFmtFourcc1600x1200p60                                          = /* 'uxg6' */ 0x75786736,
    kLibTvuMediaVideoFmtFourcc1920x1200p50                                          = /* 'wux5' */ 0x77757835,
    kLibTvuMediaVideoFmtFourcc1920x1200p60                                          = /* 'wux6' */ 0x77757836,
    kLibTvuMediaVideoFmtFourcc1920x1440p50                                          = /* '1945' */ 0x31393435,
    kLibTvuMediaVideoFmtFourcc1920x1440p60                                          = /* '1946' */ 0x31393436,
    kLibTvuMediaVideoFmtFourcc2560x1440p50                                          = /* 'wqh5' */ 0x77716835,
    kLibTvuMediaVideoFmtFourcc2560x1440p60                                          = /* 'wqh6' */ 0x77716836,
    kLibTvuMediaVideoFmtFourcc2560x1600p50                                          = /* 'wqx5' */ 0x77717835,
    kLibTvuMediaVideoFmtFourcc2560x1600p60                                          = /* 'wqx6' */ 0x77717836,

    /* RAW Modes for Cintel (input only) */

    kLibTvuMediaVideoFmtFourccCintelRAW                                             = /* 'rwci' */ 0x72776369, // Frame size up to 4096x3072, variable frame rate
    kLibTvuMediaVideoFmtFourccCintelCompressedRAW                                   = /* 'rwcc' */ 0x72776363, // Frame size up to 4096x3072, variable frame rate

    /* NHD special formats, normally used for limited streaming on mobile device */ 
    kLibTvuMediaVideoFmtFourccNHDP24                                                = /* 'nhd2' */ 0x6E686432,
    kLibTvuMediaVideoFmtFourccNHDP30                                                = /* 'nhd3' */ 0x6E686433,
    kLibTvuMediaVideoFmtFourccNHDP60                                                = /* 'nhd6' */ 0x6E686436,

    /* Special Modes */

    kLibTvuMediaVideoFmtFourccUnknown                                               = /* 'iunk' */ 0x69756E6B
}libtvu_media_video_format_fourcc_t;

#define kLibTvuMediaVideoConSDIString               "SDI"
#define kLibTvuMediaVideoConHDMIString              "HDMI"
#define kLibTvuMediaVideoConOpticalSDIString        "OpticalSDI"
#define kLibTvuMediaVideoConComponentString         "Component"
#define kLibTvuMediaVideoConCompositeString         "Composite"
#define kLibTvuMediaVideoConSVideoString            "SVideo"
enum _eLibTvuMediaVideoConnection {
    kLibTvuMediaVideoConnectionSDI                                        = 1,
    kLibTvuMediaVideoConnectionHDMI                                       = 2,
    kLibTvuMediaVideoConnectionOpticalSDI                                 = 3,
    kLibTvuMediaVideoConnectionComponent                                  = 4,
    kLibTvuMediaVideoConnectionComposite                                  = 5,
    kLibTvuMediaVideoConnectionSVideo                                     = 6,
    kLibTvuMediaVideoConnectionIPSource                                   = 7,
    kLibTvuMediaVideoConnectionTvuPreview                                 = 8,
};

#define kLibTvuMediaAudioConEmbeddedString          "Embedded"
#define kLibTvuMediaAudioConAESEBUString            "AESEBU"
#define kLibTvuMediaAudioConAnalogString            "Analog"
#define kLibTvuMediaAudioConAnalogXLRString         "AnalogXLR"
#define kLibTvuMediaAudioConAnalogRCAString         "AnalogRCA"
#define kLibTvuMediaAudioConMicrophoneString        "Microphone"
#define kLibTvuMediaAudioConHeadphonesString        "Headphones"
enum _eLibTvuMediaAudioConnection {
    kLibTvuMediaAudioConnectionEmbedded                                   = 1,
    kLibTvuMediaAudioConnectionAESEBU                                     = 2,
    kLibTvuMediaAudioConnectionAnalog                                     = 3,
    kLibTvuMediaAudioConnectionAnalogXLR                                  = 4,
    kLibTvuMediaAudioConnectionAnalogRCA                                  = 5,
    kLibTvuMediaAudioConnectionMicrophone                                 = 6,
    kLibTvuMediaAudioConnectionHeadphones                                 = 7,
};

typedef struct _STvuMediaInfoVideoFormatEntry
{
    libtvu_media_video_format_id_t      id;
    libtvu_media_video_format_fourcc_t  fourcc;
    int                                 width;
    int                                 height;
    int                                 fps_duration;
    int                                 fps_scale;
    int                                 interlace;
    const char*                         name;
    const char*                         aliasName;
}libtvu_media_info_video_fmt_entry_t;

/**
 *  Functionality:
 *      used to get video format entry
 *  Parameter:
 *      @id: value is from eLibTvuMediaVideoFormatId
 *  Return:
 *      NULL: means not found
 *      else means valid entry point.
**/
const libtvu_media_info_video_fmt_entry_t* LibTvuMediaInfoFindEntryByFmtid(unsigned int id);
const libtvu_media_info_video_fmt_entry_t* LibTvuMediaInfoFindEntryByName(const char *name);
#endif // LIBVU_MEDIA_INFO_H
