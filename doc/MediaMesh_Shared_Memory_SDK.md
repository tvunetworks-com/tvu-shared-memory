# MediaMesh Shared Memory SDK

**Version 2.1.230** | **Copyright 2025 TVU Networks**

Licensed under the Apache License, Version 2.0.

---

## 1. Overview

The MediaMesh Shared Memory SDK (`libshmmedia`) provides a high-performance, POSIX shared-memory based IPC mechanism for transferring media data (video, audio, subtitles, metadata) between processes on the same machine. It uses a ring-buffer architecture where a **writer** process creates a shared memory segment and writes media frames into it, and one or more **reader** processes open the same segment and consume the frames.

The SDK provides two families of APIs:

- **LibShm** — Constant-sized item ring buffer. Each item slot has the same fixed size.
- **LibViShm** — Variable-sized item ring buffer. Items can have different sizes, suitable for compressed streams.

## 2. Getting Started

### 2.1 Release Package Contents

```
├── bin/
│   └── read_shm              # CLI tool for reading SHM data
├── include/                   # Header files
│   ├── libshmmedia.h          # Convenience wrapper (includes both APIs)
│   ├── libshm_media.h         # LibShm APIs (constant sized items)
│   ├── libshm_media_variable_item.h  # LibViShm APIs (variable sized items)
│   ├── libshm_media_protocol.h       # Data structures & protocol APIs
│   ├── libshm_media_extension_protocol.h  # Extension data APIs
│   ├── libshmmedia_common.h           # Common types & logging
│   ├── libtvu_media_fourcc.h          # FourCC definitions
│   └── ...                            # Other protocol headers
├── lib/
│   └── libshmmediawrap.so.2.1.230     # Shared library
└── test/
    ├── write_sample_code.cpp  # Writer example
    ├── read_sample_code.cpp   # Reader example
    └── Makefile
```

### 2.2 Linking

```bash
g++ -o myapp myapp.cpp -I./include -L./lib -lshmmediawrap -DTVU_LINUX=1
```

Define `TVU_LINUX=1` when compiling on Linux.

### 2.3 Include

```c
#include "libshmmedia.h"    // Includes both LibShm and LibViShm APIs
// Or include individually:
#include "libshm_media.h"              // Constant-sized item APIs only
#include "libshm_media_variable_item.h" // Variable-sized item APIs only
```

---

## 3. Common Types and Definitions

*Header: `libshmmedia_common.h`*

### 3.1 Handle Type

```c
typedef void * libshm_media_handle_t;
typedef libshm_media_handle_t libshmmedia_handle_t;
```

All API functions use `libshm_media_handle_t` as the opaque handle to a shared memory instance.

### 3.2 Read Callback

```c
typedef int (*libshm_media_readcb_t)(void *opaq, libshm_media_item_param_t *datactx);
```

Optional callback for reader-side notification. Can be `NULL`.

### 3.3 Logging

```c
// Recommended (va_list based)
void LibShmMediaSetLogCallback(int(*cb)(int level, const char *fmt, va_list ap));

// Deprecated
void LibShmMediaSetLogCb(int(*cb)(int level, const char *fmt, ...));
```

Log levels: `'i'` (info), `'w'` (warning), `'e'` (error).

---

## 4. Data Structures

*Header: `libshm_media_protocol.h`*

### 4.1 Media Head Parameters (`libshm_media_head_param_t`)

Describes the media stream format. Set by the writer, read by the reader.

```c
typedef struct SLibShmMediaHeadParamV2 {
    uint32_t  u_reservePrivate;  // Reserved for internal use
    int32_t   i_vbr;             // Video bitrate (can be ignored)
    int32_t   i_sarw;            // Sample aspect ratio width
    int32_t   i_sarh;            // Sample aspect ratio height
    int32_t   i_srcw;            // Source/codec width
    int32_t   i_srch;            // Source/codec height
    int32_t   i_dstw;            // Output/display width
    int32_t   i_dsth;            // Output/display height
    uint32_t  u_videofourcc;     // Video pixel format (see FourCC)
    int32_t   i_duration;        // FPS denominator (e.g., 1000)
    int32_t   i_scale;           // FPS numerator (e.g., 25000 for 25fps)
    uint32_t  u_audiofourcc;     // Audio format FourCC
    int32_t   i_channels;        // Audio channel layout
    int32_t   i_depth;           // Audio bit depth (e.g., 16)
    int32_t   i_samplerate;      // Audio sample rate (e.g., 48000)
    const libshmmedia_audio_channel_layout_object_t *h_channel;  // Channel layout object
} libshm_media_head_param_t;
```

**Frame rate**: Computed as `i_scale / i_duration`. Example: `25000 / 1000 = 25 fps`.

**Channel layout** (`i_channels`): Encoded as a 32-bit value where each 4-bit nibble represents the number of channels per track. Example: `0x22` = 2 tracks, each stereo. Special value `0xFFFF10` = 16 mono tracks.

**Init/Release:**
```c
int  LibShmMediaHeadParamInit(libshm_media_head_param_t *p, uint32_t structSize);
void LibShmMediaHeadParamRelease(libshm_media_head_param_t *p);
```

### 4.2 Media Item Parameters (`libshm_media_item_param_t`)

Describes one frame/item of media data.

```c
typedef struct SLibShmMediaItemParamV0 {
    uint32_t       u_reservePrivate;   // Reserved for internal use
    int            i_totalLen;         // Total data length
    // Video
    const uint8_t *p_vData;           // Video data pointer
    int            i_vLen;             // Video data length
    int64_t        i64_vpts;           // Video PTS
    int64_t        i64_vdts;           // Video DTS
    int64_t        i64_vct;            // Video creation time
    // Audio
    const uint8_t *p_aData;           // Audio data pointer
    int            i_aLen;             // Audio data length
    int64_t        i64_apts;           // Audio PTS
    int64_t        i64_adts;           // Audio DTS
    int64_t        i64_act;            // Audio creation time
    // Subtitle
    const uint8_t *p_sData;           // Subtitle data pointer
    int            i_sLen;             // Subtitle data length
    int64_t        i64_spts;           // Subtitle PTS
    int64_t        i64_sdts;           // Subtitle DTS
    int64_t        i64_sct;            // Subtitle creation time
    // Closed Caption
    const uint8_t *p_CCData;          // Closed caption data pointer
    int            i_CCLen;            // Closed caption data length
    // Timecode
    const uint8_t *p_timeCode;        // Timecode data pointer
    int            i_timeCode;         // Timecode data length
    // Frame info
    uint32_t       u_frameType;        // Frame type
    uint32_t       u_picType;          // Picture type (see LIBSHM_MEDIA_PICTURE_TYPE_*)
    // User data / Extension
    const uint8_t *p_userData;         // User-defined extension data
    int            i_userDataLen;      // User data length
    int64_t        i64_userDataCT;     // User data creation time
    int            i_userDataType;     // User data type (see libshm_media_type_t)
    // Interlace
    uint32_t       i_interlaceFlag;    // Interlace flag (0=unknown, 1=progressive, 2=interlaced)
    // Copied flags
    uint32_t       u_copied_flags;     // Bit flags: 0x01=video, 0x02=audio, 0x04=subtitle, 0x08=ext, 0x10=user
    // State
    uint32_t       u_read_index;       // Read index
    void          *p_opaq;             // Opaque user pointer
    libshm_media_process_handle_t h_media_process;  // Process callback
} libshm_media_item_param_t;
```

**Init/Release:**
```c
int  LibShmMediaItemParamInit(libshm_media_item_param_t *p, uint32_t structSize);
void LibShmMediaItemParamRelease(libshm_media_item_param_t *p);
```

**Get PTS:**
```c
uint64_t LibShmMediaItemParamGetPts(libshm_media_item_param_t *p, const char type);
// type: 'v'=video, 'a'=audio, 's'=subtitle, 'd'=metadata, 0=auto
```

### 4.3 Item Address Layout (`libshm_media_item_addr_layout_t`)

Used for direct buffer access (zero-copy write pattern).

```c
typedef struct SLibShmMediaItemAddrLayout {
    uint32_t  i_totalLen;
    uint32_t  i_vOffset;           // Video data offset
    uint32_t  i_aOffset;           // Audio data offset
    uint32_t  i_userOffset;        // User data offset
    uint32_t  i_sOffset;           // Subtitle data offset
    uint32_t  i_ccOffset;          // Closed caption offset
    uint32_t  i_timecodeOffset;    // Timecode offset
    uint32_t  i_keyValueAreaOffset;// Key-value area offset
    uint8_t  *p_vData;             // Video data pointer
    uint8_t  *p_aData;             // Audio data pointer
    uint8_t  *p_userData;          // User data pointer
    uint8_t  *p_sData;             // Subtitle data pointer
    uint8_t  *p_CCData;            // Closed caption pointer
    uint8_t  *p_timeCode;          // Timecode pointer
    uint8_t  *p_keyValuePtr;       // Key-value area pointer
} libshm_media_item_addr_layout_t;
```

### 4.4 Extension Data (`libshmmedia_extend_data_info_t`)

Rich metadata carried alongside media frames.

```c
typedef struct _ShmExtendDataStruct {
    const uint8_t *p_uuid_data;                int i_uuid_length;
    const uint8_t *p_cc608_cdp_data;           int i_cc608_cdp_length;
    const uint8_t *p_caption_text;             int i_caption_text_length;
    const uint8_t *p_producer_stream_info;     int i_producer_stream_info_length;
    const uint8_t *p_receiver_info;            int i_receiver_info_length;
    const uint8_t *p_scte104_data;             int i_scte104_data_len;
    const uint8_t *p_scte35_data;              int i_scte35_data_len;
    const uint8_t *p_timecode_index;           int i_timecode_index_length;     // deprecated
    const uint8_t *p_start_timecode;           int i_start_timecode_length;     // deprecated
    const uint8_t *p_hdr_metadata;             int i_hdr_metadata;
    const uint8_t *p_timecode_fps_index;       int i_timecode_fps_index;        // new timecode
    const uint8_t *p_pic_struct;               int i_pic_struct;
    const uint8_t *p_source_timestamp;         int i_source_timestamp;
    const uint8_t *p_timecode;                 int i_timecode;                  // hh:mm:ss.frame
    const uint8_t *p_metaDataPts;              int i_metaDataPts;               // 64-bit LE
    const uint8_t *p_source_timebase;          int i_source_timebase;
    const uint8_t *p_smpte_afd_data;           int i_smpte_afd_data;
    const uint8_t *p_source_action_timestamp;  int i_source_action_timestamp;
    const uint8_t *p_vanc_smpte2038;           int i_vanc_smpte2038;
    const uint8_t *p_gop_poc;                  int i_gop_poc;
    uint32_t       u_subtitle_type;
    const uint8_t *p_subtitle;                 int i_subtitle;
    const uint8_t *p_smpte336m;                int i_smpte336m;
    // Color properties
    bool     bHasColorPrimariesVal_;           uint32_t uColorPrimariesVal_;
    bool     bHasColorTransferCharacteristicVal_; uint32_t uColorTransferCharacteristicVal_;
    bool     bHasColorSpaceVal_;               uint32_t uColorSpaceVal_;
    bool     bHasVideoFullRangeFlagVal_;       uint32_t uVideoFullRangeFlagVal_;
    // TVU timestamp
    bool     bGotTvutimestamp;                 uint64_t u64Tvutimestamp;
} libshmmedia_extend_data_info_t;
```

### 4.5 Raw Data Structures

For raw binary data transfer (no media parsing).

```c
typedef struct {
    uint8_t  *pRawData_;
    size_t    uRawData_;
    uint32_t  uReserverPrivate_;
} libshmmedia_raw_data_param_t;

typedef struct {
    uint8_t  *pRawHead_;
    size_t    uRawHead_;
    uint32_t  uReservePrivate_;
} libshmmedia_raw_head_param_t;
```

**Init:**
```c
int LibShmMediaRawDataParamInit(libshmmedia_raw_data_param_t *p, uint32_t structSize);
int LibShmMediaRawHeadParamInit(libshmmedia_raw_head_param_t *p, uint32_t structSize);
```

### 4.6 Picture Type Constants

```c
#define LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO     0
#define LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO   1
#define LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_AUDIO     (0<<8)
#define LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO   (1<<8)
```

### 4.7 Interlace Types

```c
#define LIBSHM_MEDIA_INTERLACE_TYPE_UNKNOWN      0x00
#define LIBSHM_MEDIA_INTERLACE_TYPE_PROGRESSIVE  0x01
#define LIBSHM_MEDIA_INTERLACE_TYPE_INTERLACE    0x02
```

### 4.8 Picture Structure Enum

```c
typedef enum {
    kLibshmmediaPicStructProgressive = 0,
    kLibshmmediaPicStructTop         = 1,
    kLibshmmediaPicStructBot         = 2,
    kLibshmmediaPicStructTopBot      = 3,   // TFF
    kLibshmmediaPicStructBotTop      = 4,   // BFF
    kLibshmmediaPicStructTopBotTop   = 5,
    kLibshmmediaPicStructBotTopBot   = 6,
    kLibshmmediaPicStructFrameDouble = 7,
    kLibshmmediaPicStructFrameTriple = 8,
    kLibshmmediaPicStructTopPairPrevBot = 9,
    kLibshmmediaPicStructBotPairPrevTop = 10,
    kLibshmmediaPicStructTopPairNextBot = 11,
    kLibshmmediaPicStructBotPairNextTop = 12,
} libshmmedia_pic_struct_t;
```

### 4.9 Copied Flags

Bit flags indicating which data components have been copied into SHM:

| Flag | Value | Meaning |
|------|-------|---------|
| `LIBSHM_MEDIA_VIDEO_COPIED_FLAG` | `0x01` | Video data copied |
| `LIBSHM_MEDIA_AUDIO_COPIED_FLAG` | `0x02` | Audio data copied |
| `LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG` | `0x04` | Subtitle data copied |
| `LIBSHM_MEDIA_EXT_COPIED_FLAG` | `0x08` | Extension data copied |
| `LIBSHM_MEDIA_USER_DATA_COPIED_FLAG` | `0x10` | User data copied |

---

## 5. LibShm APIs — Constant Sized Items

*Header: `libshm_media.h`*

These APIs use a fixed-size ring buffer where every item slot has the same size (suitable for uncompressed video/audio).

### 5.1 Creating a Shared Memory Handle

```c
libshm_media_handle_t LibShmMediaCreate(
    const char *pMemoryName,
    uint32_t header_len,
    uint32_t item_count,
    uint32_t item_length
);
```

Creates (or opens if it already exists) a SHM handle for **writing**.

| Parameter | Description |
|-----------|-------------|
| `pMemoryName` | POSIX shared memory name (e.g., `"/my_shm"`) |
| `header_len` | Size reserved for the media head |
| `item_count` | Number of ring buffer slots |
| `item_length` | Size of each item slot in bytes |

**Returns:** A `libshm_media_handle_t` handle, or `NULL` on failure.

### 5.2 Creating with Permission Mode

```c
libshm_media_handle_t LibShmMediaCreate2(
    const char *pMemoryName,
    uint32_t header_len,
    uint32_t item_count,
    uint32_t item_length,
    mode_t mode
);
```

Same as `LibShmMediaCreate` but allows specifying POSIX permission bits for `shm_open`.

| Parameter | Description |
|-----------|-------------|
| `mode` | POSIX permission bits (e.g., `S_IRUSR \| S_IWUSR \| S_IRGRP \| S_IWGRP`) |

Default when using `LibShmMediaCreate`: `S_IRUSR | S_IWUSR` (owner read/write only).

### 5.3 Opening for Reading

```c
libshm_media_handle_t LibShmMediaOpen(
    const char *pMemoryName,
    libshm_media_readcb_t cb,
    void *opaq
);
```

Opens an existing SHM for **reading**.

| Parameter | Description |
|-----------|-------------|
| `pMemoryName` | Shared memory name |
| `cb` | Read callback (can be `NULL`) |
| `opaq` | User-provided opaque data passed to callback |

**Returns:** A `libshm_media_handle_t` handle, or `NULL` on failure.

### 5.4 Destroying a Handle

```c
void LibShmMediaDestroy(libshm_media_handle_t h);
```

Destroys the handle and releases resources. Call this when done.

### 5.5 Polling Sendability (Writer)

```c
int LibShmMediaPollSendable(libshm_media_handle_t h, unsigned int timeout);
```

Checks if the writer can send data (i.e., there is a free slot).

| Return | Meaning |
|--------|---------|
| `> 0` | Ready to send |
| `0` | Not ready (buffer full), try again |
| `< 0` | I/O error — destroy and recreate the handle |

### 5.6 Polling Readability (Reader)

```c
int LibShmMediaPollReadable(libshm_media_handle_t h, unsigned int timeout);
```

Checks if data is available to read.

| Return | Meaning |
|--------|---------|
| `> 0` | Data available |
| `0` | No data yet, try again |
| `< 0` | I/O error — destroy and recreate the handle |

### 5.7 Sending Data (Writer)

```c
int LibShmMediaSendData(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
);
```

Writes one frame of media data to the SHM ring buffer.

| Return | Meaning |
|--------|---------|
| `> 0` | Success (bytes written) |
| `0` | Not ready |
| `< 0` | I/O error |

### 5.8 Sending Data with Rate Limit

```c
int LibShmMediaSendDataWithFrequency1000(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
);
```

Same as `LibShmMediaSendData` but limits to max 1 item per millisecond.

### 5.9 Reading Head Info

```c
int LibShmMediaPollReadHead(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    unsigned int timeout
);
```

Reads the media head parameters (format info). Call once after opening to learn the stream format.

| Return | Meaning |
|--------|---------|
| `> 0` | Success |
| `0` | Wait and retry |
| `< 0` | Failure |

### 5.10 Reading Data

```c
int LibShmMediaPollReadData(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi,
    unsigned int timeout
);
```

Reads one frame with timeout. Advances the read index on success.

```c
int LibShmMediaPollReadDataV2(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi,
    libshmmedia_extend_data_info_t *pext,
    unsigned int timeout
);
```

Same but also outputs parsed extension data.

```c
int LibShmMediaReadData(libshm_media_handle_t h,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi);
int LibShmMediaReadDataV2(libshm_media_handle_t h,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    libshmmedia_extend_data_info_t *pext);
```

Non-blocking variants (equivalent to timeout=0).

### 5.11 Read Without Index Step

```c
int LibShmMediaReadDataWithoutIndexStep(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi
);
```

Reads data **without** advancing the read index. Use with `LibShmMediaReadIndexStep()` to manually advance.

```c
void LibShmMediaReadIndexStep(libshm_media_handle_t h);
```

Advances the read index by one. Pair with `LibShmMediaReadDataWithoutIndexStep`.

### 5.12 Index Management

```c
unsigned int LibShmMediaGetWriteIndex(libshm_media_handle_t h);
unsigned int LibShmMediaGetReadIndex(libshm_media_handle_t h);
unsigned int LibShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h);
unsigned int LibShmMediaSeekReadIndex(libshm_media_handle_t h, uint32_t idx);
unsigned int LibShmMediaSeekReadIndexToRingStart(libshm_media_handle_t h);
unsigned int LibShmMediaSetReadIndex(libshm_media_handle_t h, char type, int64_t pts);
```

| Function | Description |
|----------|-------------|
| `GetWriteIndex` | Current write position |
| `GetReadIndex` | Current read position |
| `SeekReadIndexToWriteIndex` | Jump reader to latest write position |
| `SeekReadIndex` | Set reader to specific index |
| `SeekReadIndexToRingStart` | Jump reader to oldest available data |
| `SetReadIndex` | Seek by media type + PTS (`'v'`/`'a'`/`'s'`) |

### 5.13 SHM Properties

```c
uint32_t    LibShmMediaGetVersion(libshm_media_handle_t h);
uint32_t    LibShmMediaGetHeadVersion(libshm_media_handle_t h);
const uint8_t *LibShmMediaGeHeadAddr(libshm_media_handle_t h);
uint32_t    LibShmMediaGeHeadVersion(libshm_media_handle_t h);
unsigned int LibShmMediaGeHeadLength(libshm_media_handle_t h);
unsigned int LibShmMediaGetItemLength(libshm_media_handle_t h);
unsigned int LibShmMediaGetItemCounts(libshm_media_handle_t h);
unsigned int LibShmMediaGetItemOffset(libshm_media_handle_t h);
const char  *LibShmMediaGetName(libshm_media_handle_t h);
int          LibShmMediaIsCreator(libshm_media_handle_t h);
uint8_t     *LibShmMediaGetItemDataAddr(libshm_media_handle_t h, unsigned int index);
```

### 5.14 Timestamp Search

```c
bool LibShmMediaSearchItemWithTvutimestamp(
    libshm_media_handle_t h, uint64_t tvutimestamp,
    libshm_media_item_param_t *pmi);

bool LibShmMediaSearchItemWithTvutimestampV2(
    libshm_media_handle_t h, uint64_t tvutimestamp,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    libshmmedia_extend_data_info_t *pext);

int LibShmMediaReadItemWithTvutimestamp(
    libshm_media_handle_t h, uint64_t tvutimestamp,
    char type, uint64_t pts,
    bool *bFoundTvutimestamp, bool *bFoundPts,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi);

int LibShmMediaReadItemWithTvutimestampV2(
    libshm_media_handle_t h, uint64_t tvutimestamp,
    char type, uint64_t pts,
    bool *bFoundTvutimestamp, bool *bFoundPts,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    libshmmedia_extend_data_info_t *pext);
```

Search for items matching a TVU timestamp and/or PTS value. The `type` parameter selects: `'v'` (video), `'a'` (audio), `'s'` (subtitle), `'d'` (user data).

### 5.15 Remove SHM (Linux)

```c
int LibShmMediaRemoveShmidFromSystem(const char *pMemoryName);
```

Removes the POSIX shared memory segment from the system. Returns `0` on success, `< 0` on failure.

---

## 6. LibViShm APIs — Variable Sized Items

*Header: `libshm_media_variable_item.h`*

These APIs use a variable-size ring buffer where each item can have a different length (suitable for compressed streams like H.264/HEVC).

### 6.1 Creating a Handle

```c
libshm_media_handle_t LibViShmMediaCreate(
    const char *pMemoryName,
    uint32_t header_len,
    uint32_t item_count,
    uint64_t total_size
);
```

| Parameter | Description |
|-----------|-------------|
| `pMemoryName` | Shared memory name |
| `header_len` | Size reserved for media head |
| `item_count` | Number of ring buffer slots |
| `total_size` | Total ring buffer size in bytes |

**Returns:** Handle or `NULL`.

### 6.2 Creating with Permission Mode

```c
libshm_media_handle_t LibViShmMediaCreate2(
    const char *pMemoryName,
    uint32_t header_len,
    uint32_t item_count,
    uint64_t total_size,
    mode_t mode
);
```

Same as `LibViShmMediaCreate` with explicit POSIX permission bits.

### 6.3 Opening for Reading

```c
libshm_media_handle_t LibViShmMediaOpen(
    const char *pMemoryName,
    libshm_media_readcb_t cb,
    void *opaq
);
```

### 6.4 Destroying

```c
void LibViShmMediaDestroy(libshm_media_handle_t h);
```

### 6.5 Polling

```c
int LibViShmMediaPollSendable(libshm_media_handle_t h, uint32_t timeout);
int LibViShmMediaPollReadable(libshm_media_handle_t h, uint32_t timeout);
```

Same return conventions as LibShm: `> 0` ready, `0` not ready, `< 0` error.

### 6.6 Sending Data

```c
int LibViShmMediaSendData(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
);

int LibViShmMediaSendDataWithFrequency1000(
    libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi
);
```

### 6.7 Reading Data

```c
int LibViShmMediaPollReadHead(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    uint32_t timeout
);

int LibViShmMediaPollReadData(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi,
    uint32_t timeout
);

int LibViShmMediaReadData(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi
);
```

### 6.8 Read Without Index Step

```c
int LibViShmMediaReadDataWithoutIndexStep(
    libshm_media_handle_t h,
    libshm_media_head_param_t *pmh,
    libshm_media_item_param_t *pmi
);
void LibViShmMediaReadIndexStep(libshm_media_handle_t h);
```

### 6.9 Index Management

```c
uint64_t LibViShmMediaGetWriteIndex(libshm_media_handle_t h);
uint64_t LibViShmMediaGetReadIndex(libshm_media_handle_t h);
void     LibViShmMediaSeekReadIndexToWriteIndex(libshm_media_handle_t h);
void     LibViShmMediaSeekReadIndexToZero(libshm_media_handle_t h);
void     LibViShmMediaSeekReadIndex(libshm_media_handle_t h, uint64_t rindex);
```

### 6.10 Properties

```c
uint32_t     LibViShmMediaGetVersion(libshm_media_handle_t h);
unsigned int LibViShmMediaGetItemLength(libshm_media_handle_t h);  // Returns total payload size
unsigned int LibViShmMediaGetTotalPayloadSize(libshm_media_handle_t h);
unsigned int LibViShmMediaGetItemCounts(libshm_media_handle_t h);
unsigned int LibViShmMediaGetHeadLen(libshm_media_handle_t h);
unsigned int LibViShmMediaGetItemOffset(libshm_media_handle_t h);
const char  *LibViShmMediaGetName(libshm_media_handle_t h);
int          LibViShmMediaIsCreator(libshm_media_handle_t h);
void         LibViShmMediaSetCloseflag(libshm_media_handle_t h, int bclose);
int          LibViShmMediaCheckCloseflag(libshm_media_handle_t h);
```

### 6.11 Search Items

```c
typedef int (*libshmmedia_item_checking_fn_t)(
    void *user, const libshm_media_head_param_t *, const libshm_media_item_param_t *);

int LibViShmMediaSearchItems(
    libshm_media_handle_t h,
    void *userCtx,
    libshmmedia_item_checking_fn_t fn
);
```

Searches items using a user callback. Callback returns `1` when found, `0` to continue. On found, read index is set to the matched item.

### 6.12 Direct Buffer Access (Zero-Copy Write)

```c
// Apply (allocate) a buffer slot
uint8_t* LibViShmMediaItemApplyBuffer(libshm_media_handle_t h, unsigned int nlen);

// Commit the written data
int LibViShmMediaItemCommitBuffer(libshm_media_handle_t h, uint8_t *pItemAddr, unsigned int nlen);

// Write buffer with media header
int LibViShmMediaItemWriteBuffer(libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi,
    uint8_t *pItemAddr);

int LibViShmMediaItemWriteBufferIgnoreInternalCopy(libshm_media_handle_t h,
    const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi,
    uint8_t *pItemAddr);

// Get buffer layout for reading
int LibViShmMediaItemPreGetReadBufferLayout(libshm_media_handle_t h,
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    const uint8_t *pItemAddr);

// Get buffer layout for writing
int LibViShmMediaItemPreGetWriteBufferLayout(libshm_media_handle_t h,
    const libshm_media_item_param_t *pmi, uint8_t *pItemAddr,
    libshm_media_item_addr_layout_t *pLayout);
```

### 6.13 Remove SHM

```c
int LibViShmMediaRemoveShmFromSystem(const char *pMemoryName);
```

---

## 7. Protocol APIs

*Header: `libshm_media_protocol.h`*

Low-level protocol APIs for direct buffer manipulation.

### 7.1 Item Buffer Operations

```c
// Get total data length of item parameters
uint32_t LibShmMediaProGetItemParamDataLen(const libshm_media_item_param_t *pvi);

// Get head version from raw header buffer
unsigned int LibShmMediaProtoGetHeadVersion(const uint8_t *headBuf);

// Read item buffer layout (parse raw buffer into structs)
int LibShmMediaProtoReadItemBufferLayout(
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    const uint8_t *pItemAddr, uint32_t nItem);

int LibShmMediaProtoReadItemBufferLayoutWithHeadVer(
    libshm_media_head_param_t *pmh, libshm_media_item_param_t *pmi,
    const uint8_t *pItemAddr, uint32_t headVer);

// Get write buffer layout
int LibShmMediaProtoGetWriteItemBufferLayout(
    const libshm_media_item_param_t *pmi,
    uint8_t *pItemAddr, uint32_t itemLen,
    libshm_media_item_addr_layout_t *playout);

// Compute required buffer length for writing
int LibShmMediaProtoRequireWriteItemBufferLength(const libshm_media_item_param_t *pmi);

// Write item header + data to buffer
int LibShmMediaProtoWriteItemBuffer(
    const libshm_media_head_param_t *pmh,
    const libshm_media_item_param_t *pmi,
    uint8_t *pItemAddr);
```

---

## 8. Extension Data APIs

*Header: `libshm_media_extension_protocol.h`*

### 8.1 Media Type Enum

```c
typedef enum {
    LIBSHM_MEDIA_TYPE_INVALID                = 0,
    LIBSHM_MEDIA_TYPE_TVU_UID                = 1,
    LIBSHM_MEDIA_TYPE_TVU_INTERLACE_FLAG     = 2,
    LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA        = 3,
    LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2     = 4,
    LIBSHM_MEDIA_TYPE_TVULIVE_AUDIO          = FOURCC('T','L','V','A'),
    LIBSHM_MEDIA_TYPE_TVULIVE_HEADER         = FOURCC('T','L','V','H'),
    LIBSHM_MEDIA_TYPE_TVULIVE_VIDEO          = FOURCC('T','L','V','V'),
    LIBSHM_MEDIA_TYPE_TVULIVE_DATA           = FOURCC('T','L','V','D'),
    LIBSHM_MEDIA_TYPE_ENCODING_DATA          = FOURCC('T','E','N','C'),
    LIBSHM_MEDIA_TYPE_CONTROL_DATA           = FOURCC('T','C','T','L'),
    LIBSHM_MEDIA_TYPE_MPEG_TS_DATA           = FOURCC('T','M','T','S'),
    LIBSHM_MEDIA_TYPE_TVU_OTHER_MEDIA_INFO   = 0xffffffff,
} libshm_media_type_t;
```

### 8.2 Reading Extension Data

```c
// Parse extension data (data pointers reference internal buffer of v2DataCtx)
int LibShmMediaReadExtendData(
    libshmmedia_extend_data_info_t *pExtendData,
    const uint8_t *pShmUserData, int dataSize,
    int userDataType, libshmmedia_extended_data_context_t v2DataCtx);

// Parse extension data (data pointers reference pShmUserData directly)
int LibShmMeidaParseExtendData(
    libshmmedia_extend_data_info_t *pExtendData,
    const uint8_t *pShmUserData, int dataSize, int userDataType);

int LibShmMeidaParseExtendDataV2(
    libshmmedia_extend_data_info_t *pExtendData,
    const uint8_t *pShmUserData, int dataSize);
```

### 8.3 Writing Extension Data

```c
// Estimate required buffer size
int LibShmMediaEstimateExtendDataSize(const libshmmedia_extend_data_info_t *pExtendData);

// Write extension data to buffer
int LibShmMediaWriteExtendData(
    uint8_t dataBuffer[], int bufferSize,
    const libshmmedia_extend_data_info_t *pExtendData);
```

### 8.4 Extended Data Context

```c
libshmmedia_extended_data_context_t LibshmMediaExtDataCreateHandle();
void LibshmMediaExtDataResetEntry(libshmmedia_extended_data_context_t h);
unsigned int LibshmMediaExtDataGetEntryBuffSize(libshmmedia_extended_data_context_t h);
int LibshmMediaExtDataParseBuff(libshmmedia_extended_data_context_t h, const uint8_t *pbuf, uint32_t ibuflen);
unsigned int LibshmMediaExtDataGetEntryCounts(libshmmedia_extended_data_context_t h);
void LibshmMediaExtDataDestroyHandle(libshmmedia_extended_data_context_t *ph);
```

---

## 9. Audio Channel Layout APIs

*Header: `libshm_media_audio_track_channel_protocol.h`*

```c
// Create / Destroy
libshmmedia_audio_channel_layout_object_t *LibshmmediaAudioChannelLayoutCreate();
void LibshmmediaAudioChannelLayoutDestroy(libshmmedia_audio_channel_layout_object_t *pctx);

// Query
bool     LibshmmediaAudioChannelLayoutIsPlanar(const libshmmedia_audio_channel_layout_object_t *pctx);
uint16_t LibshmmediaAudioChannelLayoutGetChannelNum(const libshmmedia_audio_channel_layout_object_t *pctx);
bool     LibshmmediaAudioChannelLayoutGetChannelArr(const libshmmedia_audio_channel_layout_object_t *pctx,
             const uint16_t **ppDest, uint16_t *pnDest);
bool     LibshmmediaAudioChannelLayoutGetBinaryAddr(const libshmmedia_audio_channel_layout_object_t *pctx,
             const uint8_t **ppDest, uint32_t *pnDest);

// Serialize / Parse
bool LibshmmediaAudioChannelLayoutSerializeToBinary(
    libshmmedia_audio_channel_layout_object_t *pctx,
    const uint16_t *pChan, uint16_t nChan, bool bPlanar);

bool LibshmmediaAudioChannelLayoutParseFromBinary(
    libshmmedia_audio_channel_layout_object_t *pctx,
    const uint8_t *pBin, uint32_t nBin);

// Compare / Copy
int LibshmmediaAudioChannelLayoutCompare(
    const libshmmedia_audio_channel_layout_object_t *p1,
    const libshmmedia_audio_channel_layout_object_t *p2);

int LibshmmediaAudioChannelLayoutCopy(
    libshmmedia_audio_channel_layout_object_t *pdst,
    const libshmmedia_audio_channel_layout_object_t *psrc);
```

---

## 10. Binary Concat Protocol APIs

*Header: `libshm_media_bin_concat_protocol.h`*

Used to concatenate/split multiple binary segments into a single buffer.

```c
typedef struct {
    uint32_t       nSeg;
    const uint8_t *pSeg;
} libshmmedia_bin_concat_proto_seg_t;

// Create / Destroy
libshmmedia_bin_concat_proto_handle_t LibshmmediaBinConcatProtoCreate();
void LibshmmediaBinConcatProtoDestroy(libshmmedia_bin_concat_proto_handle_t pctx);
bool LibshmmediaBinConcatProtoReset(libshmmedia_bin_concat_proto_handle_t pctx);

// Concat segments into binary
bool LibshmmediaBinConcatProtoConcatSegment(
    libshmmedia_bin_concat_proto_handle_t pctx,
    const libshmmedia_bin_concat_proto_seg_t *pSeg);

bool LibshmmediaBinConcatProtoFlushBinary(
    libshmmedia_bin_concat_proto_handle_t pctx,
    const uint8_t **ppBin, uint32_t *pnBin);

// Split binary into segments
bool LibshmmediaBinConcatProtoSplitBinary(
    libshmmedia_bin_concat_proto_handle_t pctx,
    const uint8_t *pBin, uint32_t nBin, bool bCreateBuffer,
    libshmmedia_bin_concat_proto_seg_t **ppData, uint32_t *pnData);

bool LibshmmediaBinConcatProtoParseBinary(
    libshmmedia_bin_concat_proto_handle_t pctx,
    const uint8_t *pBin, uint32_t nBin,
    libshmmedia_bin_concat_proto_seg_t **ppData, uint32_t *pnData);
```

---

## 11. Subtitle Private Protocol APIs

*Header: `libshm_media_subtitle_private_protocol.h`*

```c
typedef struct {
    uint32_t       strucSize;
    uint32_t       type;        // Subtitle type (see ELibShmMediaExtenstionSubtitleType)
    uint64_t       timestamp;
    uint32_t       duration;
    uint32_t       dataLen;
    const uint8_t *data;
} libshmmedia_subtitle_private_proto_entry_item_t;

typedef struct {
    libshmmedia_subtitle_private_proto_entry_head_t head;   // { int counts; }
    libshmmedia_subtitle_private_proto_entry_item_t entries[8];
} libshmmedia_subtitle_private_proto_entries_t;

bool LibshmmediaSubtitlePrivateProtoIsCorrectType(uint32_t type);
int  LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(const libshmmedia_subtitle_private_proto_entries_t *);
int  LibshmmediaSubtitlePrivateProtoWriteBufferSize(const libshmmedia_subtitle_private_proto_entries_t *, uint8_t *buffer, int bufferSize);
int  LibshmmediaSubtitlePrivateProtoParseBufferSize(libshmmedia_subtitle_private_proto_entries_t *, const uint8_t *buffer, int bufferSize);
int  LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(uint8_t *buffer, int bufferSize, int offsetMs);
```

Supported subtitle types:

| Enum | Value | Format |
|------|-------|--------|
| `..._DVB_TELETEXT` | `0x10001` | DVB Teletext |
| `..._DVB_SUBTITLE` | `0x10002` | DVB Subtitle |
| `..._DVD_SUBTITLE` | `0x10003` | DVD Subtitle |
| `..._WEBVTT` | `0x10004` | WebVTT |
| `..._SRT` | `0x10005` | SubRip with timing |
| `..._SUBRIP` | `0x10006` | SubRip |
| `..._RAW_TEXT` | `0x10007` | Raw UTF-8 text |
| `..._TTML` | `0x10008` | Timed Text Markup |
| `..._EIA608` | `0x1000a` | EIA-608 CC |
| `..._SSA` | `0x1000b` | SubStation Alpha |
| `..._ASS` | `0x1000c` | Advanced SSA |

---

## 12. FourCC Definitions

*Header: `libtvu_media_fourcc.h`*

### 12.1 Video Pixel Formats (`ETVUPixfmtVideoFourCC`)

Common values:

| Constant | FourCC | Description |
|----------|--------|-------------|
| `K_TVU_PIXFMT_VIDEO_FOURCC_YUV420P` | `I420` | YUV 4:2:0 planar |
| `K_TVU_PIXFMT_VIDEO_FOURCC_NV12` | `NV12` | YUV 4:2:0 semi-planar |
| `K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422` | `UYVY` | YUV 4:2:2 packed |
| `K_TVU_PIXFMT_VIDEO_FOURCC_YUYV422` | `V422` | YUV 4:2:2 packed |
| `K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P` | `I422` | YUV 4:2:2 planar |
| `K_TVU_PIXFMT_VIDEO_FOURCC_YUV444P` | `I444` | YUV 4:4:4 planar |
| `K_TVU_PIXFMT_VIDEO_FOURCC_RGB24` | `RGB24` | RGB 24-bit |
| `K_TVU_PIXFMT_VIDEO_FOURCC_BGR24` | `BGR24` | BGR 24-bit |
| `K_TVU_PIXFMT_VIDEO_FOURCC_RGBA` | `RGBA` | RGBA 32-bit |
| `K_TVU_PIXFMT_VIDEO_FOURCC_BGRA` | `BGRA` | BGRA 32-bit |
| `K_TVU_PIXFMT_VIDEO_FOURCC_P010LE` | `P010` | 10-bit 4:2:0 |
| `K_TVU_PIXFMT_VIDEO_FOURCC_YUV422P10LE` | — | 10-bit 4:2:2 planar |
| `K_TVU_PIXFMT_VIDEO_FOURCC_JPEG` | `JPEG` | JPEG encoded |

### 12.2 Audio Formats (`ETVUPixfmtAudioFourCC`)

| Constant | Description |
|----------|-------------|
| `K_TVU_AUDIO_FOURCC_WAVE_48K_16` | PCM signed 16-bit, 48kHz |

### 12.3 Codec Tags (`ETvuCodecTagFourCC`)

| Constant | Codec |
|----------|-------|
| `K_TVU_CODEC_TAG_H264` | H.264/AVC |
| `K_TVU_CODEC_TAG_HEVC` | H.265/HEVC |
| `K_TVU_CODEC_TAG_VP8` | VP8 |
| `K_TVU_CODEC_TAG_VP9` | VP9 |
| `K_TVU_CODEC_TAG_AAC` | AAC audio |
| `K_TVU_CODEC_TAG_AC3` | Dolby AC-3 |
| `K_TVU_CODEC_TAG_OPUS` | Opus audio |
| `K_TVU_CODEC_TAG_FLAC` | FLAC audio |
| `K_TVU_CODEC_TAG_MP3` | MP3 audio |

---

## 13. Sample Code

### 13.1 Writer Example

The writer creates a shared memory segment, sets up media format parameters, and writes video+audio frames in a loop.

```cpp
#include "libshmmedia.h"
#include "libtvu_media_fourcc.h"

int main()
{
    // Initialize structures
    libshm_media_head_param_t ohp;
    LibShmMediaHeadParamInit(&ohp, sizeof(ohp));
    libshm_media_item_param_t ohi;
    LibShmMediaItemParamInit(&ohi, sizeof(ohi));

    // Set media format
    ohp.i_dstw        = 1920;
    ohp.i_dsth        = 1080;
    ohp.u_videofourcc = K_TVU_PIXFMT_VIDEO_FOURCC_UYVY422;
    ohp.i_duration    = 1000;    // FPS den
    ohp.i_scale       = 25000;   // FPS num -> 25fps
    ohp.u_audiofourcc = K_TVU_AUDIO_FOURCC_WAVE_48K_16;
    ohp.i_channels    = 0x22;    // 2 tracks stereo
    ohp.i_depth       = 16;
    ohp.i_samplerate  = 48000;

    uint32_t videoFrameSize = 2 * 1920 * 1080;  // UYVY
    uint32_t itemSize = videoFrameSize + audioFrameSize + 2048;

    // Create SHM
    libshm_media_handle_t h = LibShmMediaCreate(
        "/my_video_shm", 1024, 10, itemSize);

    if (!h) return -1;

    // Write loop
    while (running) {
        // Prepare video/audio data in ohi...
        ohi.p_vData  = videoFrame;
        ohi.i_vLen   = videoFrameSize;
        ohi.i64_vpts = current_pts;
        ohi.p_aData  = audioFrame;
        ohi.i_aLen   = audioFrameSize;
        ohi.i64_apts = current_pts;

        int ret = LibShmMediaPollSendable(h, 0);
        if (ret > 0) {
            LibShmMediaSendData(h, &ohp, &ohi);
        } else if (ret < 0) {
            break;  // Error
        }
    }

    LibShmMediaDestroy(h);
    return 0;
}
```

### 13.2 Reader Example

The reader opens an existing shared memory segment and reads frames.

```cpp
#include "libshmmedia.h"

int main()
{
    libshm_media_head_param_t ohp = {0};

    // Open SHM for reading
    libshm_media_handle_t h = LibShmMediaOpen("/my_video_shm", NULL, NULL);
    if (!h) return -1;

    // Read loop
    while (running) {
        libshm_media_item_param_t ohi = {0};
        int ret = LibShmMediaPollReadData(h, &ohp, &ohi, 3000);

        if (ret > 0) {
            // Process video
            if (ohi.p_vData && ohi.i_vLen > 0) {
                // Video frame at ohi.p_vData, length ohi.i_vLen
                // PTS: ohi.i64_vpts
            }
            // Process audio
            if (ohi.p_aData && ohi.i_aLen > 0) {
                // Audio frame at ohi.p_aData, length ohi.i_aLen
            }
            // Process extension data
            if (ohi.p_userData && ohi.i_userDataLen > 0
                && ohi.i_userDataType == LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2) {
                libshmmedia_extend_data_info_t ext;
                memset(&ext, 0, sizeof(ext));
                LibShmMeidaParseExtendData(&ext, ohi.p_userData,
                    ohi.i_userDataLen, ohi.i_userDataType);
            }
        } else if (ret < 0) {
            break;  // Error
        }
    }

    LibShmMediaDestroy(h);
    return 0;
}
```

### 13.3 Writing Extension Data

```cpp
libshmmedia_extend_data_info_t myExt;
memset(&myExt, 0, sizeof(myExt));

// Set fields
myExt.p_uuid_data      = (const uint8_t *)uuid_str;
myExt.i_uuid_length    = strlen(uuid_str);
myExt.p_cc608_cdp_data = cc608_data;
myExt.i_cc608_cdp_length = cc608_len;
myExt.p_timecode       = (const uint8_t *)&timecode;
myExt.i_timecode       = sizeof(uint32_t);
myExt.p_metaDataPts    = (const uint8_t *)&pts;  // 64-bit LE
myExt.i_metaDataPts    = sizeof(int64_t);

// Serialize
int bufSize = LibShmMediaEstimateExtendDataSize(&myExt);
uint8_t *buf = (uint8_t *)malloc(bufSize);
int written = LibShmMediaWriteExtendData(buf, bufSize, &myExt);

// Attach to item
ohi.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
ohi.p_userData     = buf;
ohi.i_userDataLen  = written;
```

---

## 14. Appendix

### 14.1 Return Code Conventions

Most read/write APIs follow this convention:

| Return | Meaning |
|--------|---------|
| `> 0` | Success |
| `0` | Not ready / no data (try again) |
| `< 0` | Error (handle may need to be destroyed and recreated) |

### 14.2 Thread Safety

- A single `libshm_media_handle_t` should be accessed from one thread at a time.
- The writer and reader processes access the same SHM segment concurrently — this is safe by design (ring buffer with separate read/write indices).
- Multiple readers can open the same SHM independently.

### 14.3 SHM Naming Convention

POSIX shared memory names must start with `/` (e.g., `"/my_video_shm"`). The SDK handles this internally if the leading `/` is provided.

### 14.4 Writer/Reader URL Format

The sample code supports a URL format for specifying multiple SHM names:

```
tvushm://0?v=<video_shm>&a=<audio_shm>&d=<data_shm>
```

Example: `tvushm://0?v=vx1&a=ax1&d=dx1`
