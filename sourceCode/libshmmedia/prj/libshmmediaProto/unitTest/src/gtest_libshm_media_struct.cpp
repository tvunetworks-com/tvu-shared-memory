/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/
/**
 * @file gtest_libshm_media_struct.cpp
 * @brief Unit tests for libshm_media_struct.cpp and related structures
 */

#include <gtest/gtest.h>
#include <cstring>
#include <climits>
#include "libshm_media_protocol.h"

class LibShmMediaStructTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test head param initialization with LibShmMediaHeadParamInit
TEST_F(LibShmMediaStructTest, HeadParam_Init) {
    libshm_media_head_param_t head;
    int ret = LibShmMediaHeadParamInit(&head, sizeof(head));
    EXPECT_GE(ret, 0);

    // After init, values should be zeroed
    EXPECT_EQ(head.i_sarw, 0);
    EXPECT_EQ(head.i_sarh, 0);
    EXPECT_EQ(head.i_srcw, 0);
    EXPECT_EQ(head.i_srch, 0);
    EXPECT_EQ(head.i_dstw, 0);
    EXPECT_EQ(head.i_dsth, 0);

    LibShmMediaHeadParamRelease(&head);
}

// Test head param field assignments
TEST_F(LibShmMediaStructTest, HeadParam_FieldAssignments) {
    libshm_media_head_param_t head;
    LibShmMediaHeadParamInit(&head, sizeof(head));

    // Set SAR (Sample Aspect Ratio)
    head.i_sarw = 1;
    head.i_sarh = 1;
    EXPECT_EQ(head.i_sarw, 1);
    EXPECT_EQ(head.i_sarh, 1);

    // Set source resolution
    head.i_srcw = 1920;
    head.i_srch = 1080;
    EXPECT_EQ(head.i_srcw, 1920);
    EXPECT_EQ(head.i_srch, 1080);

    // Set destination resolution
    head.i_dstw = 1280;
    head.i_dsth = 720;
    EXPECT_EQ(head.i_dstw, 1280);
    EXPECT_EQ(head.i_dsth, 720);

    // Set video fourcc
    head.u_videofourcc = 0x31637661;  // 'avc1'
    EXPECT_EQ(head.u_videofourcc, 0x31637661u);

    // Set audio fourcc
    head.u_audiofourcc = 0x6134706D;  // 'mp4a'
    EXPECT_EQ(head.u_audiofourcc, 0x6134706Du);

    // Set audio parameters
    head.i_channels = 2;
    head.i_depth = 16;
    head.i_samplerate = 48000;
    EXPECT_EQ(head.i_channels, 2);
    EXPECT_EQ(head.i_depth, 16);
    EXPECT_EQ(head.i_samplerate, 48000);

    LibShmMediaHeadParamRelease(&head);
}

// Test item param initialization with LibShmMediaItemParamInit
TEST_F(LibShmMediaStructTest, ItemParam_Init) {
    libshm_media_item_param_t item;
    int ret = LibShmMediaItemParamInit(&item, sizeof(item));
    EXPECT_GE(ret, 0);

    // After init, values should be zeroed
    EXPECT_EQ(item.i_totalLen, 0);
    EXPECT_EQ(item.i_vLen, 0);
    EXPECT_EQ(item.i_aLen, 0);
    EXPECT_EQ(item.i_sLen, 0);
    EXPECT_EQ(item.i_CCLen, 0);
    EXPECT_EQ(item.i_timeCode, 0);

    EXPECT_EQ(item.p_vData, nullptr);
    EXPECT_EQ(item.p_aData, nullptr);
    EXPECT_EQ(item.p_sData, nullptr);
    EXPECT_EQ(item.p_CCData, nullptr);
    EXPECT_EQ(item.p_timeCode, nullptr);

    LibShmMediaItemParamRelease(&item);
}

// Test item param video fields
TEST_F(LibShmMediaStructTest, ItemParam_VideoFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t videoBuffer[4096];
    item.p_vData = videoBuffer;
    item.i_vLen = sizeof(videoBuffer);
    item.i64_vpts = 90000;
    item.i64_vdts = 89000;
    item.i64_vct = 1234567890;
    item.u_frameType = 1;  // I-frame

    EXPECT_EQ(item.p_vData, videoBuffer);
    EXPECT_EQ(item.i_vLen, (int)sizeof(videoBuffer));
    EXPECT_EQ(item.i64_vpts, 90000);
    EXPECT_EQ(item.i64_vdts, 89000);
    EXPECT_EQ(item.i64_vct, 1234567890);
    EXPECT_EQ(item.u_frameType, 1u);

    LibShmMediaItemParamRelease(&item);
}

// Test item param audio fields
TEST_F(LibShmMediaStructTest, ItemParam_AudioFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t audioBuffer[1024];
    item.p_aData = audioBuffer;
    item.i_aLen = sizeof(audioBuffer);
    item.i64_apts = 48000;
    item.i64_adts = 48000;
    item.i64_act = 1234567891;

    EXPECT_EQ(item.p_aData, audioBuffer);
    EXPECT_EQ(item.i_aLen, (int)sizeof(audioBuffer));
    EXPECT_EQ(item.i64_apts, 48000);
    EXPECT_EQ(item.i64_adts, 48000);
    EXPECT_EQ(item.i64_act, 1234567891);

    LibShmMediaItemParamRelease(&item);
}

// Test item param subtitle fields
TEST_F(LibShmMediaStructTest, ItemParam_SubtitleFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t subtBuffer[256];
    item.p_sData = subtBuffer;
    item.i_sLen = sizeof(subtBuffer);
    item.i64_spts = 100000;
    item.i64_sdts = 100000;
    item.i64_sct = 1234567892;

    EXPECT_EQ(item.p_sData, subtBuffer);
    EXPECT_EQ(item.i_sLen, (int)sizeof(subtBuffer));
    EXPECT_EQ(item.i64_spts, 100000);
    EXPECT_EQ(item.i64_sdts, 100000);
    EXPECT_EQ(item.i64_sct, 1234567892);

    LibShmMediaItemParamRelease(&item);
}

// Test item param closed caption fields
TEST_F(LibShmMediaStructTest, ItemParam_ClosedCaptionFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t ccBuffer[128];
    item.p_CCData = ccBuffer;
    item.i_CCLen = sizeof(ccBuffer);

    EXPECT_EQ(item.p_CCData, ccBuffer);
    EXPECT_EQ(item.i_CCLen, (int)sizeof(ccBuffer));

    LibShmMediaItemParamRelease(&item);
}

// Test item param timecode fields
TEST_F(LibShmMediaStructTest, ItemParam_TimecodeFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t timecodeBuffer[32];
    item.p_timeCode = timecodeBuffer;
    item.i_timeCode = sizeof(timecodeBuffer);

    EXPECT_EQ(item.p_timeCode, timecodeBuffer);
    EXPECT_EQ(item.i_timeCode, (int)sizeof(timecodeBuffer));

    LibShmMediaItemParamRelease(&item);
}

// Test item param user data fields
TEST_F(LibShmMediaStructTest, ItemParam_UserDataFields) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    uint8_t userBuffer[512];
    item.p_userData = userBuffer;
    item.i_userDataLen = sizeof(userBuffer);
    item.i64_userDataCT = 1234567893;
    item.i_userDataType = 1;

    EXPECT_EQ(item.p_userData, userBuffer);
    EXPECT_EQ(item.i_userDataLen, (int)sizeof(userBuffer));
    EXPECT_EQ(item.i64_userDataCT, 1234567893);
    EXPECT_EQ(item.i_userDataType, 1);

    LibShmMediaItemParamRelease(&item);
}

// Test item addr layout structure
TEST_F(LibShmMediaStructTest, ItemAddrLayout_Initialization) {
    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    EXPECT_EQ(layout.i_totalLen, 0u);
    EXPECT_EQ(layout.i_vOffset, 0u);
    EXPECT_EQ(layout.i_aOffset, 0u);
    EXPECT_EQ(layout.i_sOffset, 0u);
    EXPECT_EQ(layout.i_ccOffset, 0u);
    EXPECT_EQ(layout.i_timecodeOffset, 0u);
    EXPECT_EQ(layout.p_vData, nullptr);
    EXPECT_EQ(layout.p_aData, nullptr);
    EXPECT_EQ(layout.p_sData, nullptr);
    EXPECT_EQ(layout.p_CCData, nullptr);
    EXPECT_EQ(layout.p_timeCode, nullptr);
}

// Test raw data param initialization
TEST_F(LibShmMediaStructTest, RawDataParam_Init) {
    libshmmedia_raw_data_param_t param;
    int ret = LibShmMediaRawDataParamInit(&param, sizeof(param));
    EXPECT_GE(ret, 0);

    EXPECT_EQ(param.pRawData_, nullptr);
    EXPECT_EQ(param.uRawData_, 0u);
}

// Test raw head param initialization
TEST_F(LibShmMediaStructTest, RawHeadParam_Init) {
    libshmmedia_raw_head_param_t param;
    int ret = LibShmMediaRawHeadParamInit(&param, sizeof(param));
    EXPECT_GE(ret, 0);

    EXPECT_EQ(param.pRawHead_, nullptr);
    EXPECT_EQ(param.uRawHead_, 0u);
}

// Test structure copy - head param
TEST_F(LibShmMediaStructTest, StructureCopy_HeadParam) {
    libshm_media_head_param_t head1, head2;
    LibShmMediaHeadParamInit(&head1, sizeof(head1));
    LibShmMediaHeadParamInit(&head2, sizeof(head2));

    head1.i_srcw = 1920;
    head1.i_srch = 1080;
    head1.i_sarw = 1;
    head1.i_sarh = 1;

    memcpy(&head2, &head1, sizeof(head1));

    EXPECT_EQ(head2.i_srcw, head1.i_srcw);
    EXPECT_EQ(head2.i_srch, head1.i_srch);
    EXPECT_EQ(head2.i_sarw, head1.i_sarw);
    EXPECT_EQ(head2.i_sarh, head1.i_sarh);

    LibShmMediaHeadParamRelease(&head1);
    LibShmMediaHeadParamRelease(&head2);
}

// Test structure copy - item param
TEST_F(LibShmMediaStructTest, StructureCopy_ItemParam) {
    libshm_media_item_param_t item1, item2;
    LibShmMediaItemParamInit(&item1, sizeof(item1));
    LibShmMediaItemParamInit(&item2, sizeof(item2));

    item1.i_vLen = 4096;
    item1.i64_vpts = 90000;
    item1.u_frameType = 1;

    memcpy(&item2, &item1, sizeof(item1));

    EXPECT_EQ(item2.i_vLen, item1.i_vLen);
    EXPECT_EQ(item2.i64_vpts, item1.i64_vpts);
    EXPECT_EQ(item2.u_frameType, item1.u_frameType);

    LibShmMediaItemParamRelease(&item1);
    LibShmMediaItemParamRelease(&item2);
}

// Test boundary values - max values
TEST_F(LibShmMediaStructTest, BoundaryValues_MaxValues) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    item.i_totalLen = INT_MAX;
    item.i64_vpts = INT64_MAX;
    item.i64_vdts = INT64_MAX;

    EXPECT_EQ(item.i_totalLen, INT_MAX);
    EXPECT_EQ(item.i64_vpts, INT64_MAX);
    EXPECT_EQ(item.i64_vdts, INT64_MAX);

    LibShmMediaItemParamRelease(&item);
}

// Test boundary values - min values
TEST_F(LibShmMediaStructTest, BoundaryValues_MinValues) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    item.i64_vpts = INT64_MIN;
    item.i64_vdts = INT64_MIN;

    EXPECT_EQ(item.i64_vpts, INT64_MIN);
    EXPECT_EQ(item.i64_vdts, INT64_MIN);

    LibShmMediaItemParamRelease(&item);
}

// Test interlace flags
TEST_F(LibShmMediaStructTest, InterlaceFlags) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    // Test unknown
    item.i_interlaceFlag = LIBSHM_MEDIA_INTERLACE_TYPE_UNKNOWN;
    EXPECT_EQ(item.i_interlaceFlag, 0x00u);

    // Test progressive
    item.i_interlaceFlag = LIBSHM_MEDIA_INTERLACE_TYPE_PROGRESSIVE;
    EXPECT_EQ(item.i_interlaceFlag, 0x01u);

    // Test interlace
    item.i_interlaceFlag = LIBSHM_MEDIA_INTERLACE_TYPE_INTERLACE;
    EXPECT_EQ(item.i_interlaceFlag, 0x02u);

    LibShmMediaItemParamRelease(&item);
}

// Test picture type masks
TEST_F(LibShmMediaStructTest, PictureTypeMasks) {
    // Test video mask
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_VIDEO_MASK, 0x000000FFu);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_AUDIO_MASK, 0x0000FF00u);

    // Test normal video/audio
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO, 0u);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_AUDIO, 0u);

    // Test TVU logo video/audio
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO, 1u);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO, 0x0100u);
}

// Test copied flags
TEST_F(LibShmMediaStructTest, CopiedFlags) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    // Test individual flags
    item.u_copied_flags = LIBSHM_MEDIA_VIDEO_COPIED_FLAG;
    EXPECT_EQ(item.u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG, LIBSHM_MEDIA_VIDEO_COPIED_FLAG);

    item.u_copied_flags = LIBSHM_MEDIA_AUDIO_COPIED_FLAG;
    EXPECT_EQ(item.u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG, LIBSHM_MEDIA_AUDIO_COPIED_FLAG);

    item.u_copied_flags = LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG;
    EXPECT_EQ(item.u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG, LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG);

    // Test combined flags
    item.u_copied_flags = LIBSHM_MEDIA_VIDEO_COPIED_FLAG | LIBSHM_MEDIA_AUDIO_COPIED_FLAG;
    EXPECT_NE(item.u_copied_flags & LIBSHM_MEDIA_VIDEO_COPIED_FLAG, 0u);
    EXPECT_NE(item.u_copied_flags & LIBSHM_MEDIA_AUDIO_COPIED_FLAG, 0u);
    EXPECT_EQ(item.u_copied_flags & LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG, 0u);

    LibShmMediaItemParamRelease(&item);
}

// Test picture struct enum values
TEST_F(LibShmMediaStructTest, PictureStructEnum) {
    EXPECT_EQ(kLibshmmediaPicStructProgressive, 0);
    EXPECT_EQ(kLibshmmediaPicStructTop, 1);
    EXPECT_EQ(kLibshmmediaPicStructBot, 2);
    EXPECT_EQ(kLibshmmediaPicStructTopBot, 3);
    EXPECT_EQ(kLibshmmediaPicStructBotTop, 4);
    EXPECT_EQ(kLibshmmediaPicStructTopBotTop, 5);
    EXPECT_EQ(kLibshmmediaPicStructBotTopBot, 6);
    EXPECT_EQ(kLibshmmediaPicStructFrameDouble, 7);
    EXPECT_EQ(kLibshmmediaPicStructFrameTriple, 8);
}

// Test LibShmMediaProGetItemParamDataLen
TEST_F(LibShmMediaStructTest, GetItemParamDataLen) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    // Empty item should have minimal length
    uint32_t len = LibShmMediaProGetItemParamDataLen(&item);
    EXPECT_EQ(len, 0u);

    // Set some data lengths
    item.i_vLen = 1000;
    item.i_aLen = 500;
    len = LibShmMediaProGetItemParamDataLen(&item);
    EXPECT_EQ(len, 1500u);

    LibShmMediaItemParamRelease(&item);
}

// Test pic type field
TEST_F(LibShmMediaStructTest, ItemParam_PicType) {
    libshm_media_item_param_t item;
    LibShmMediaItemParamInit(&item, sizeof(item));

    // Normal video and audio
    item.u_picType = LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO | LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_AUDIO;
    EXPECT_EQ(item.u_picType & LIBSHM_MEDIA_PICTURE_TYPE_VIDEO_MASK, LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO);

    // TVU logo
    item.u_picType = LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO | LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO;
    EXPECT_EQ(item.u_picType & LIBSHM_MEDIA_PICTURE_TYPE_VIDEO_MASK, LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO);
    EXPECT_EQ(item.u_picType & LIBSHM_MEDIA_PICTURE_TYPE_AUDIO_MASK, LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO);

    LibShmMediaItemParamRelease(&item);
}

// Test structure sizes (for compatibility)
TEST_F(LibShmMediaStructTest, StructureSizes_Reasonable) {
    // Basic sanity checks on structure sizes
    EXPECT_GT(sizeof(libshm_media_head_param_t), 0u);
    EXPECT_GT(sizeof(libshm_media_item_param_t), 0u);
    EXPECT_GT(sizeof(libshm_media_item_addr_layout_t), 0u);
    EXPECT_GT(sizeof(libshmmedia_raw_data_param_t), 0u);
    EXPECT_GT(sizeof(libshmmedia_raw_head_param_t), 0u);
}

// Test head param with channel layout
TEST_F(LibShmMediaStructTest, HeadParam_ChannelLayout) {
    libshm_media_head_param_t head;
    LibShmMediaHeadParamInit(&head, sizeof(head));

    // Channel layout pointer should be null after init
    EXPECT_EQ(head.h_channel, nullptr);

    // Create a channel layout and assign
    libshmmedia_audio_channel_layout_object_t *channelHandle = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelHandle, nullptr);

    head.h_channel = channelHandle;
    EXPECT_EQ(head.h_channel, channelHandle);

    LibshmmediaAudioChannelLayoutDestroy(channelHandle);
    LibShmMediaHeadParamRelease(&head);
}
