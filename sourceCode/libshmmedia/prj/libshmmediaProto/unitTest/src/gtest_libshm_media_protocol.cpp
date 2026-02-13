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

#include <gtest/gtest.h>
#include "libshm_media_protocol.h"
#include <cstring>
#include <vector>

class LibShmMediaProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize head param
        LibShmMediaHeadParamInit(&headParam, sizeof(headParam));

        // Set some test values
        headParam.i_srcw = 1920;
        headParam.i_srch = 1080;
        headParam.i_dstw = 1920;
        headParam.i_dsth = 1080;
        headParam.u_videofourcc = 0x56595559; // UYVY
        headParam.i_duration = 1001;
        headParam.i_scale = 30000;
        headParam.u_audiofourcc = 0x50434D20; // PCM
        headParam.i_channels = 2;
        headParam.i_depth = 16;
        headParam.i_samplerate = 48000;
    }

    void TearDown() override {
        LibShmMediaHeadParamRelease(&headParam);
        LibShmMediaItemParamRelease(&itemParam);
    }

    libshm_media_head_param_t headParam;
    libshm_media_item_param_t itemParam;
};

// Test LibShmMediaHeadParamInit
TEST_F(LibShmMediaProtocolTest, HeadParamInit) {
    libshm_media_head_param_t testParam;
    int result = LibShmMediaHeadParamInit(&testParam, sizeof(testParam));
    EXPECT_EQ(result, 0) << "HeadParamInit should return 0";
    LibShmMediaHeadParamRelease(&testParam);
}

// Test LibShmMediaHeadParamInit with null pointer
TEST_F(LibShmMediaProtocolTest, HeadParamInitNullPointer) {
    int result = LibShmMediaHeadParamInit(nullptr, sizeof(libshm_media_head_param_t));
    EXPECT_NE(result, 0) << "HeadParamInit should fail with null pointer";
}

// Test LibShmMediaItemParamInit
TEST_F(LibShmMediaProtocolTest, ItemParamInit) {
    libshm_media_item_param_t testParam;
    int result = LibShmMediaItemParamInit(&testParam, sizeof(testParam));
    EXPECT_EQ(result, 0) << "ItemParamInit should return 0";
    LibShmMediaItemParamRelease(&testParam);
}

// Test LibShmMediaItemParamInit with null pointer
TEST_F(LibShmMediaProtocolTest, ItemParamInitNullPointer) {
    int result = LibShmMediaItemParamInit(nullptr, sizeof(libshm_media_item_param_t));
    EXPECT_NE(result, 0) << "ItemParamInit should fail with null pointer";
}

// Test LibShmMediaRawDataParamInit
TEST_F(LibShmMediaProtocolTest, RawDataParamInit) {
    libshmmedia_raw_data_param_t rawDataParam;
    int result = LibShmMediaRawDataParamInit(&rawDataParam, sizeof(rawDataParam));
    EXPECT_EQ(result, 0) << "RawDataParamInit should return 0";
}

// Test LibShmMediaRawHeadParamInit
TEST_F(LibShmMediaProtocolTest, RawHeadParamInit) {
    libshmmedia_raw_head_param_t rawHeadParam;
    int result = LibShmMediaRawHeadParamInit(&rawHeadParam, sizeof(rawHeadParam));
    EXPECT_EQ(result, 0) << "RawHeadParamInit should return 0";
}

// Test LibShmMediaProGetItemParamDataLen
TEST_F(LibShmMediaProtocolTest, GetItemParamDataLen) {
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));

    // Set up some test data
    const int videoLen = 1920 * 1080 * 2; // UYVY format
    const int audioLen = 1920;
    std::vector<uint8_t> videoData(videoLen);
    std::vector<uint8_t> audioData(audioLen);

    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.p_aData = audioData.data();
    itemParam.i_aLen = audioLen;

    uint32_t totalLen = LibShmMediaProGetItemParamDataLen(&itemParam);
    EXPECT_GT(totalLen, 0u) << "Total data length should be greater than 0";
    EXPECT_GE(totalLen, static_cast<uint32_t>(videoLen + audioLen))
        << "Total length should at least include video and audio data";
}

// Test LibShmMediaProtoGetHeadVersion
TEST_F(LibShmMediaProtocolTest, GetHeadVersion) {
    std::vector<uint8_t> buffer(4096);

    // Write a simple item to buffer
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));
    itemParam.i_totalLen = 100;

    int written = LibShmMediaProtoWriteItemBuffer(&headParam, &itemParam, buffer.data());

    if (written > 0) {
        unsigned int version = LibShmMediaProtoGetHeadVersion(buffer.data());
        EXPECT_GT(version, 0u) << "Head version should be valid";
    }
}

// Test LibShmMediaProtoRequireWriteItemBufferLength
TEST_F(LibShmMediaProtocolTest, RequireWriteItemBufferLength) {
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));

    const int videoLen = 1024;
    const int audioLen = 512;
    std::vector<uint8_t> videoData(videoLen);
    std::vector<uint8_t> audioData(audioLen);

    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.p_aData = audioData.data();
    itemParam.i_aLen = audioLen;

    int requiredLen = LibShmMediaProtoRequireWriteItemBufferLength(&itemParam);
    EXPECT_GT(requiredLen, 0) << "Required buffer length should be positive";
    EXPECT_GE(requiredLen, videoLen + audioLen)
        << "Required length should at least include data sizes";
}

// Test LibShmMediaProtoWriteItemBuffer
TEST_F(LibShmMediaProtocolTest, WriteItemBuffer) {
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));

    const int videoLen = 1024;
    std::vector<uint8_t> videoData(videoLen, 0x55);
    std::vector<uint8_t> buffer(10240);

    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.i64_vpts = 1000;
    itemParam.i64_vdts = 1000;
    itemParam.u_frameType = 1; // I-frame

    int written = LibShmMediaProtoWriteItemBuffer(&headParam, &itemParam, buffer.data());
    EXPECT_GT(written, 0) << "Should write data successfully";
    EXPECT_LE(written, static_cast<int>(buffer.size()))
        << "Written bytes should not exceed buffer size";
}

// Test LibShmMediaProtoReadItemBufferLayout
TEST_F(LibShmMediaProtocolTest, ReadWriteItemBufferRoundtrip) {
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));

    // Prepare test data
    const int videoLen = 100;
    const int audioLen = 50;
    std::vector<uint8_t> videoData(videoLen, 0xAA);
    std::vector<uint8_t> audioData(audioLen, 0xBB);
    std::vector<uint8_t> buffer(10240);

    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.i64_vpts = 2000;
    itemParam.i64_vdts = 2000;
    itemParam.p_aData = audioData.data();
    itemParam.i_aLen = audioLen;
    itemParam.i64_apts = 2000;
    itemParam.i64_adts = 2000;

    // Write to buffer
    int written = LibShmMediaProtoWriteItemBuffer(&headParam, &itemParam, buffer.data());
    ASSERT_GT(written, 0) << "Should write data successfully";

    // Read back from buffer
    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    LibShmMediaHeadParamInit(&readHead, sizeof(readHead));
    LibShmMediaItemParamInit(&readItem, sizeof(readItem));

    int readLen = LibShmMediaProtoReadItemBufferLayout(&readHead, &readItem,
                                                       buffer.data(), written);
    ASSERT_GT(readLen, 0) << "Should read data successfully";

    // Verify data
    EXPECT_EQ(readItem.i_vLen, videoLen) << "Video length should match";
    EXPECT_EQ(readItem.i_aLen, audioLen) << "Audio length should match";
    EXPECT_EQ(readItem.i64_vpts, 2000) << "Video PTS should match";
    EXPECT_EQ(readItem.i64_apts, 2000) << "Audio PTS should match";

    LibShmMediaHeadParamRelease(&readHead);
    LibShmMediaItemParamRelease(&readItem);
}

// Test LibShmMediaProtoGetWriteItemBufferLayout
TEST_F(LibShmMediaProtocolTest, GetWriteItemBufferLayout) {
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));

    const int videoLen = 1024;
    const int audioLen = 512;

    itemParam.i_vLen = videoLen;
    itemParam.i_aLen = audioLen;

    std::vector<uint8_t> buffer(10240);
    libshm_media_item_addr_layout_t layout;

    int result = LibShmMediaProtoGetWriteItemBufferLayout(&itemParam, buffer.data(),
                                                          buffer.size(), &layout);

    if (result > 0) {
        EXPECT_NE(layout.p_vData, nullptr) << "Video data pointer should be set";
        EXPECT_NE(layout.p_aData, nullptr) << "Audio data pointer should be set";
        EXPECT_GT(layout.i_totalLen, 0u) << "Total length should be positive";
    }
}

// Test interlace type macros
TEST_F(LibShmMediaProtocolTest, InterlaceTypeMacros) {
    EXPECT_EQ(LIBSHM_MEDIA_INTERLACE_TYPE_UNKNOWN, 0x00);
    EXPECT_EQ(LIBSHM_MEDIA_INTERLACE_TYPE_PROGRESSIVE, 0x01);
    EXPECT_EQ(LIBSHM_MEDIA_INTERLACE_TYPE_INTERLACE, 0x02);
    EXPECT_EQ(LIBSHM_MEDIA_INTERLACE_TYPE_RESERVED, 0x03);
}

// Test picture type macros
TEST_F(LibShmMediaProtocolTest, PictureTypeMacros) {
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_VIDEO, 0);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_VIDEO, 1);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_NORMAL_AUDIO, 0 << 8);
    EXPECT_EQ(LIBSHM_MEDIA_PICTURE_TYPE_TVU_LOGO_AUDIO, 1 << 8);
}

// Test copied flags macros
TEST_F(LibShmMediaProtocolTest, CopiedFlagsMacros) {
    EXPECT_EQ(LIBSHM_MEDIA_VIDEO_COPIED_FLAG, 0x01);
    EXPECT_EQ(LIBSHM_MEDIA_AUDIO_COPIED_FLAG, 0x02);
    EXPECT_EQ(LIBSHM_MEDIA_SUBTITLE_COPIED_FLAG, 0x04);
    EXPECT_EQ(LIBSHM_MEDIA_CLOSED_CAPTION_COPIED_FLAG, 0x08);
    EXPECT_EQ(LIBSHM_MEDIA_USER_DATA_COPIED_FLAG, 0x10);
}

//int main(int argc, char **argv) {
//    ::testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}
