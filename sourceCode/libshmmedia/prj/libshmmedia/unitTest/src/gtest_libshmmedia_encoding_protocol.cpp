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
 * @file gtest_libshmmedia_encoding_protocol.cpp
 * @brief Unit tests for libshmmedia_encoding_protocol.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "libshmmedia_encoding_protocol.h"

class LibShmmediaEncodingProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&encodingData_, 0, sizeof(encodingData_));
        memset(&readData_, 0, sizeof(readData_));
    }

    void TearDown() override {
    }

    libshmmedia_encoding_data_t encodingData_;
    libshmmedia_encoding_data_t readData_;
};

// Test LibShmmediaEncodingDataGetBufferSize
TEST_F(LibShmmediaEncodingProtocolTest, GetBufferSize_ZeroDataLen) {
    uint32_t size = LibShmmediaEncodingDataGetBufferSize(0);
    EXPECT_GT(size, 0u);  // Should return at least header size
}

TEST_F(LibShmmediaEncodingProtocolTest, GetBufferSize_NonZeroDataLen) {
    uint32_t dataLen = 1024;
    uint32_t size = LibShmmediaEncodingDataGetBufferSize(dataLen);
    EXPECT_GT(size, dataLen);  // Should be header + data
}

TEST_F(LibShmmediaEncodingProtocolTest, GetBufferSize_LargeDataLen) {
    uint32_t dataLen = 1024 * 1024;  // 1MB
    uint32_t size = LibShmmediaEncodingDataGetBufferSize(dataLen);
    EXPECT_GT(size, dataLen);
}

// Test LibTvuMediaEncodingDataGetBufferSize (alias function)
TEST_F(LibShmmediaEncodingProtocolTest, TvuMediaGetBufferSize_MatchesLibShmmedia) {
    uint32_t dataLen = 512;
    uint32_t size1 = LibShmmediaEncodingDataGetBufferSize(dataLen);
    uint32_t size2 = LibTvuMediaEncodingDataGetBufferSize(dataLen);
    EXPECT_EQ(size1, size2);
}

// Test LibShmmediaEncodingDataWrite with null buffer
TEST_F(LibShmmediaEncodingProtocolTest, Write_NullBuffer) {
    encodingData_.u_codec_tag = 0x31637661;  // 'avc1'
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 1;
    encodingData_.i64_pts = 1000;
    encodingData_.i64_dts = 900;
    encodingData_.i_data = 100;
    encodingData_.p_data = nullptr;

    int ret = LibShmmediaEncodingDataWrite(&encodingData_, nullptr, 1024);
    EXPECT_EQ(ret, -1);
}

TEST_F(LibShmmediaEncodingProtocolTest, Write_ZeroBufferSize) {
    encodingData_.u_codec_tag = 0x31637661;
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 1;
    encodingData_.i64_pts = 1000;
    encodingData_.i64_dts = 900;
    encodingData_.i_data = 100;

    std::vector<uint8_t> buffer(1024);
    int ret = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), 0);
    EXPECT_EQ(ret, -1);
}

TEST_F(LibShmmediaEncodingProtocolTest, Write_BufferTooSmall) {
    uint8_t testData[] = {0x00, 0x01, 0x02, 0x03};
    encodingData_.u_codec_tag = 0x31637661;
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 1;
    encodingData_.i64_pts = 1000;
    encodingData_.i64_dts = 900;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    std::vector<uint8_t> buffer(4);  // Too small
    int ret = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_EQ(ret, -1);
}

TEST_F(LibShmmediaEncodingProtocolTest, Write_ValidData) {
    uint8_t testData[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    encodingData_.u_codec_tag = 0x31637661;  // 'avc1'
    encodingData_.u_stream_index = 1;
    encodingData_.u_frame_index = 100;
    encodingData_.i64_pts = 90000;
    encodingData_.i64_dts = 89000;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int ret = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(ret, 0);
    EXPECT_EQ((uint32_t)ret, bufferSize);
}

// Test LibShmmediaEncodingDataRead with null buffer
TEST_F(LibShmmediaEncodingProtocolTest, Read_NullBuffer) {
    int ret = LibShmmediaEncodingDataRead(&readData_, nullptr, 1024);
    EXPECT_EQ(ret, -1);
}

TEST_F(LibShmmediaEncodingProtocolTest, Read_ZeroBufferLen) {
    std::vector<uint8_t> buffer(1024);
    int ret = LibShmmediaEncodingDataRead(&readData_, buffer.data(), 0);
    EXPECT_EQ(ret, -1);
}

// Test Write/Read roundtrip
TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_BasicData) {
    uint8_t testData[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    encodingData_.u_codec_tag = 0x31637661;  // 'avc1'
    encodingData_.u_stream_index = 2;
    encodingData_.u_frame_index = 50;
    encodingData_.i64_pts = 123456789;
    encodingData_.i64_dts = 123456000;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    // Write
    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    // Read
    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);
    EXPECT_EQ(writeRet, readRet);

    // Verify data
    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
    EXPECT_EQ(readData_.i_data, encodingData_.i_data);
    EXPECT_NE(readData_.p_data, nullptr);
    EXPECT_EQ(memcmp(readData_.p_data, testData, sizeof(testData)), 0);
}

TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_ZeroLengthData) {
    encodingData_.u_codec_tag = 0x63706D69;  // 'impc'
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 0;
    encodingData_.i64_pts = 0;
    encodingData_.i64_dts = 0;
    encodingData_.i_data = 0;
    encodingData_.p_data = nullptr;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
    EXPECT_EQ(readData_.i_data, 0u);
}

TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_LargeData) {
    std::vector<uint8_t> largeData(64 * 1024);  // 64KB
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i & 0xFF);
    }

    encodingData_.u_codec_tag = 0x35367668;  // 'hv65' HEVC
    encodingData_.u_stream_index = 3;
    encodingData_.u_frame_index = 9999;
    encodingData_.i64_pts = INT64_MAX;
    encodingData_.i64_dts = INT64_MAX - 1000;
    encodingData_.i_data = largeData.size();
    encodingData_.p_data = largeData.data();

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
    EXPECT_EQ(readData_.i_data, encodingData_.i_data);
    EXPECT_EQ(memcmp(readData_.p_data, largeData.data(), largeData.size()), 0);
}

TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_NegativePtsDts) {
    uint8_t testData[] = {0x01, 0x02, 0x03, 0x04};
    encodingData_.u_codec_tag = 0x34363268;  // 'h264'
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 1;
    encodingData_.i64_pts = -1000;
    encodingData_.i64_dts = -2000;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
}

// Test LibTvuMediaEncodingDataWrite and LibTvuMediaEncodingDataRead (alias functions)
TEST_F(LibShmmediaEncodingProtocolTest, TvuMediaWriteRead_MatchesLibShmmedia) {
    uint8_t testData[] = {0xAA, 0xBB, 0xCC, 0xDD};
    encodingData_.u_codec_tag = 0x31637661;
    encodingData_.u_stream_index = 1;
    encodingData_.u_frame_index = 10;
    encodingData_.i64_pts = 5000;
    encodingData_.i64_dts = 4500;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    // Use Tvu* alias functions
    int writeRet = LibTvuMediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
    EXPECT_EQ(readData_.i_data, encodingData_.i_data);
}

// Test boundary values
TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_MaxValues) {
    uint8_t testData[] = {0xFF};
    encodingData_.u_codec_tag = UINT32_MAX;
    encodingData_.u_stream_index = UINT32_MAX;
    encodingData_.u_frame_index = UINT16_MAX;
    encodingData_.i64_pts = INT64_MAX;
    encodingData_.i64_dts = INT64_MAX;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
}

TEST_F(LibShmmediaEncodingProtocolTest, WriteReadRoundtrip_MinValues) {
    uint8_t testData[] = {0x00};
    encodingData_.u_codec_tag = 0;
    encodingData_.u_stream_index = 0;
    encodingData_.u_frame_index = 0;
    encodingData_.i64_pts = INT64_MIN;
    encodingData_.i64_dts = INT64_MIN;
    encodingData_.i_data = sizeof(testData);
    encodingData_.p_data = testData;

    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(encodingData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
    EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
    EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
    EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
    EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
}

// Test multiple write/read cycles
TEST_F(LibShmmediaEncodingProtocolTest, MultipleWriteReadCycles) {
    uint32_t bufferSize = LibShmmediaEncodingDataGetBufferSize(16);
    std::vector<uint8_t> buffer(bufferSize);

    for (int i = 0; i < 100; ++i) {
        uint8_t testData[16];
        for (int j = 0; j < 16; ++j) {
            testData[j] = static_cast<uint8_t>((i + j) & 0xFF);
        }

        encodingData_.u_codec_tag = 0x31637661 + i;
        encodingData_.u_stream_index = i % 8;
        encodingData_.u_frame_index = i;
        encodingData_.i64_pts = 1000 * i;
        encodingData_.i64_dts = 1000 * i - 100;
        encodingData_.i_data = sizeof(testData);
        encodingData_.p_data = testData;

        int writeRet = LibShmmediaEncodingDataWrite(&encodingData_, buffer.data(), buffer.size());
        EXPECT_GT(writeRet, 0);

        memset(&readData_, 0, sizeof(readData_));
        int readRet = LibShmmediaEncodingDataRead(&readData_, buffer.data(), writeRet);
        EXPECT_GT(readRet, 0);

        EXPECT_EQ(readData_.u_codec_tag, encodingData_.u_codec_tag);
        EXPECT_EQ(readData_.u_stream_index, encodingData_.u_stream_index);
        EXPECT_EQ(readData_.u_frame_index, encodingData_.u_frame_index);
        EXPECT_EQ(readData_.i64_pts, encodingData_.i64_pts);
        EXPECT_EQ(readData_.i64_dts, encodingData_.i64_dts);
        EXPECT_EQ(readData_.i_data, encodingData_.i_data);
        EXPECT_EQ(memcmp(readData_.p_data, testData, sizeof(testData)), 0);
    }
}
