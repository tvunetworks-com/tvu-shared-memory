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
 * @file gtest_libshm_media_media_head_protocol.cpp
 * @brief Unit tests for libshm_media_media_head_protocol.cpp
 *
 * Note: The internal functions _mediaHeadSetChannelLayout and _mediaHeadGetChannelLayout
 * are not publicly exposed. These tests verify the related audio channel layout APIs
 * that are used by the media head protocol implementation.
 */

#include <gtest/gtest.h>
#include <cstring>
#include "libshm_media_media_head_protocol.h"
#include "libshm_media_audio_track_channel_protocol.h"

class LibShmMediaMediaHeadProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        channelLayoutHandle_ = nullptr;
    }

    void TearDown() override {
        if (channelLayoutHandle_) {
            LibshmmediaAudioChannelLayoutDestroy(channelLayoutHandle_);
            channelLayoutHandle_ = nullptr;
        }
    }

    libshmmedia_audio_channel_layout_object_t *channelLayoutHandle_;
};

// Test channel layout create and destroy
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayout_CreateDestroy) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);
    // Destroy is handled in TearDown
}

// Test channel layout serialize and parse roundtrip with stereo (2 channels)
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayoutRoundtrip_Stereo) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    // Set stereo layout (2 channels: left=0, right=1)
    uint16_t stereoChannels[] = {0, 1};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(
        channelLayoutHandle_,
        stereoChannels,
        2,
        false  // not planar
    );
    EXPECT_TRUE(ret);

    // Get binary representation
    const uint8_t *pBin = nullptr;
    uint32_t nBin = 0;
    ret = LibshmmediaAudioChannelLayoutGetBinaryAddr(channelLayoutHandle_, &pBin, &nBin);
    EXPECT_TRUE(ret);
    EXPECT_NE(pBin, nullptr);
    EXPECT_GT(nBin, 0u);

    // Parse from binary and verify
    libshmmedia_audio_channel_layout_object_t *readHandle = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(readHandle, nullptr);

    ret = LibshmmediaAudioChannelLayoutParseFromBinary(readHandle, pBin, nBin);
    EXPECT_TRUE(ret);

    // Verify channel count
    uint16_t channelCount = LibshmmediaAudioChannelLayoutGetChannelNum(readHandle);
    EXPECT_EQ(channelCount, 2u);  // Stereo has 2 channels

    // Verify planar setting
    EXPECT_FALSE(LibshmmediaAudioChannelLayoutIsPlanar(readHandle));

    LibshmmediaAudioChannelLayoutDestroy(readHandle);
}

// Test channel layout serialize and parse roundtrip with 5.1 surround (6 channels)
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayoutRoundtrip_Surround51) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    // Set 5.1 layout (6 channels)
    uint16_t surroundChannels[] = {0, 1, 2, 3, 4, 5};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(
        channelLayoutHandle_,
        surroundChannels,
        6,
        true  // planar
    );
    EXPECT_TRUE(ret);

    const uint8_t *pBin = nullptr;
    uint32_t nBin = 0;
    ret = LibshmmediaAudioChannelLayoutGetBinaryAddr(channelLayoutHandle_, &pBin, &nBin);
    EXPECT_TRUE(ret);

    libshmmedia_audio_channel_layout_object_t *readHandle = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(readHandle, nullptr);

    ret = LibshmmediaAudioChannelLayoutParseFromBinary(readHandle, pBin, nBin);
    EXPECT_TRUE(ret);

    uint16_t channelCount = LibshmmediaAudioChannelLayoutGetChannelNum(readHandle);
    EXPECT_EQ(channelCount, 6u);  // 5.1 has 6 channels

    EXPECT_TRUE(LibshmmediaAudioChannelLayoutIsPlanar(readHandle));

    LibshmmediaAudioChannelLayoutDestroy(readHandle);
}

// Test channel layout serialize and parse roundtrip with mono (1 channel)
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayoutRoundtrip_Mono) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    // Set mono layout (1 channel)
    uint16_t monoChannels[] = {0};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(
        channelLayoutHandle_,
        monoChannels,
        1,
        false
    );
    EXPECT_TRUE(ret);

    const uint8_t *pBin = nullptr;
    uint32_t nBin = 0;
    ret = LibshmmediaAudioChannelLayoutGetBinaryAddr(channelLayoutHandle_, &pBin, &nBin);
    EXPECT_TRUE(ret);

    libshmmedia_audio_channel_layout_object_t *readHandle = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(readHandle, nullptr);

    ret = LibshmmediaAudioChannelLayoutParseFromBinary(readHandle, pBin, nBin);
    EXPECT_TRUE(ret);

    uint16_t channelCount = LibshmmediaAudioChannelLayoutGetChannelNum(readHandle);
    EXPECT_EQ(channelCount, 1u);  // Mono has 1 channel

    LibshmmediaAudioChannelLayoutDestroy(readHandle);
}

// Test with invalid binary data
TEST_F(LibShmMediaMediaHeadProtocolTest, ParseChannelLayout_InvalidBinary) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    // Invalid binary data
    uint8_t invalidBin[] = {0xFF, 0xFF, 0xFF, 0xFF};
    bool ret = LibshmmediaAudioChannelLayoutParseFromBinary(
        channelLayoutHandle_,
        invalidBin,
        sizeof(invalidBin)
    );
    // Should fail
    EXPECT_FALSE(ret);
}

// Test with empty binary data
TEST_F(LibShmMediaMediaHeadProtocolTest, ParseChannelLayout_EmptyBinary) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    bool ret = LibshmmediaAudioChannelLayoutParseFromBinary(
        channelLayoutHandle_,
        nullptr,
        0
    );
    EXPECT_FALSE(ret);
}

// Test channel layout compare
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayout_Compare) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    libshmmedia_audio_channel_layout_object_t *handle2 = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(handle2, nullptr);

    // Serialize same channels to both
    uint16_t channels[] = {0, 1, 2};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(channelLayoutHandle_, channels, 3, false);
    EXPECT_TRUE(ret);

    ret = LibshmmediaAudioChannelLayoutSerializeToBinary(handle2, channels, 3, false);
    EXPECT_TRUE(ret);

    // Compare - should be equal
    int cmp = LibshmmediaAudioChannelLayoutCompare(channelLayoutHandle_, handle2);
    EXPECT_EQ(cmp, 0);

    LibshmmediaAudioChannelLayoutDestroy(handle2);
}

// Test channel layout copy
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayout_Copy) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    libshmmedia_audio_channel_layout_object_t *handle2 = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(handle2, nullptr);

    // Serialize channels to first handle
    uint16_t channels[] = {0, 1, 2, 3};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(channelLayoutHandle_, channels, 4, true);
    EXPECT_TRUE(ret);

    // Copy to second handle
    int copyRet = LibshmmediaAudioChannelLayoutCopy(handle2, channelLayoutHandle_);
    EXPECT_GE(copyRet, 0);

    // Compare - should be equal
    int cmp = LibshmmediaAudioChannelLayoutCompare(channelLayoutHandle_, handle2);
    EXPECT_EQ(cmp, 0);

    // Verify channel count in copy
    uint16_t channelCount = LibshmmediaAudioChannelLayoutGetChannelNum(handle2);
    EXPECT_EQ(channelCount, 4u);

    LibshmmediaAudioChannelLayoutDestroy(handle2);
}

// Test get channel array
TEST_F(LibShmMediaMediaHeadProtocolTest, ChannelLayout_GetChannelArray) {
    channelLayoutHandle_ = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(channelLayoutHandle_, nullptr);

    uint16_t channels[] = {0, 1, 2, 3, 4};
    bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(channelLayoutHandle_, channels, 5, false);
    EXPECT_TRUE(ret);

    const uint16_t *pArr = nullptr;
    uint16_t arrLen = 0;
    ret = LibshmmediaAudioChannelLayoutGetChannelArr(channelLayoutHandle_, &pArr, &arrLen);
    EXPECT_TRUE(ret);
    EXPECT_NE(pArr, nullptr);
    EXPECT_EQ(arrLen, 5u);

    if (ret && pArr) {
        for (uint16_t i = 0; i < arrLen; ++i) {
            EXPECT_EQ(pArr[i], channels[i]);
        }
    }
}

// Test multiple channel layouts with different configurations
TEST_F(LibShmMediaMediaHeadProtocolTest, MultipleChannelLayouts) {
    struct TestCase {
        uint16_t channels[8];
        uint16_t numChannels;
        bool planar;
    };

    TestCase testCases[] = {
        {{0}, 1, false},                          // Mono
        {{0, 1}, 2, false},                       // Stereo
        {{0, 1, 2}, 3, true},                     // 3 channels planar
        {{0, 1, 2, 3}, 4, false},                 // Quad
        {{0, 1, 2, 3, 4, 5}, 6, true},            // 5.1 planar
        {{0, 1, 2, 3, 4, 5, 6, 7}, 8, false}      // 7.1
    };

    for (size_t i = 0; i < sizeof(testCases) / sizeof(testCases[0]); ++i) {
        libshmmedia_audio_channel_layout_object_t *handle = LibshmmediaAudioChannelLayoutCreate();
        ASSERT_NE(handle, nullptr);

        bool ret = LibshmmediaAudioChannelLayoutSerializeToBinary(
            handle,
            testCases[i].channels,
            testCases[i].numChannels,
            testCases[i].planar
        );
        EXPECT_TRUE(ret);

        const uint8_t *pBin = nullptr;
        uint32_t nBin = 0;
        ret = LibshmmediaAudioChannelLayoutGetBinaryAddr(handle, &pBin, &nBin);
        EXPECT_TRUE(ret);

        libshmmedia_audio_channel_layout_object_t *readHandle = LibshmmediaAudioChannelLayoutCreate();
        ASSERT_NE(readHandle, nullptr);

        ret = LibshmmediaAudioChannelLayoutParseFromBinary(readHandle, pBin, nBin);
        EXPECT_TRUE(ret);

        uint16_t channelCount = LibshmmediaAudioChannelLayoutGetChannelNum(readHandle);
        EXPECT_EQ(channelCount, testCases[i].numChannels);

        bool isPlanar = LibshmmediaAudioChannelLayoutIsPlanar(readHandle);
        EXPECT_EQ(isPlanar, testCases[i].planar);

        LibshmmediaAudioChannelLayoutDestroy(readHandle);
        LibshmmediaAudioChannelLayoutDestroy(handle);
    }
}
