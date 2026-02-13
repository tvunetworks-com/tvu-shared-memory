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
 * @file gtest_libshmmedia_tvulive_protocol.cpp
 * @brief Unit tests for libshmmedia_tvulive_protocol.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "libshmmedia_tvulive_protocol.h"

class LibShmmediaTvuliveProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&tvuliveData_, 0, sizeof(tvuliveData_));
        memset(&readData_, 0, sizeof(readData_));
        tvuliveData_.u_struct_size = sizeof(libtvumedia_tvulive_data_v2_t);
        readData_.u_struct_size = sizeof(libtvumedia_tvulive_data_v2_t);
    }

    void TearDown() override {
    }

    libtvumedia_tvulive_data_t tvuliveData_;
    libtvumedia_tvulive_data_t readData_;
};

// Test LibTvuMediaTvulivePreferBufferSize
TEST_F(LibShmmediaTvuliveProtocolTest, PreferBufferSize_ZeroDataLen) {
    uint32_t size = LibTvuMediaTvulivePreferBufferSize(0);
    EXPECT_GT(size, 0u);  // Should return at least header size
}

TEST_F(LibShmmediaTvuliveProtocolTest, PreferBufferSize_NonZeroDataLen) {
    uint32_t dataLen = 1024;
    uint32_t size = LibTvuMediaTvulivePreferBufferSize(dataLen);
    EXPECT_GT(size, dataLen);  // Should be header + data
}

TEST_F(LibShmmediaTvuliveProtocolTest, PreferBufferSize_LargeDataLen) {
    uint32_t dataLen = 1024 * 1024;  // 1MB
    uint32_t size = LibTvuMediaTvulivePreferBufferSize(dataLen);
    EXPECT_GT(size, dataLen);
}

// Test LibTvuMediaTvulivePreferHeadSize
TEST_F(LibShmmediaTvuliveProtocolTest, PreferHeadSize_ReturnsPositive) {
    uint32_t headSize = LibTvuMediaTvulivePreferHeadSize();
    EXPECT_GT(headSize, 0u);
}

TEST_F(LibShmmediaTvuliveProtocolTest, PreferHeadSize_MatchesBufferSizeWithZeroData) {
    uint32_t headSize = LibTvuMediaTvulivePreferHeadSize();
    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(0);
    EXPECT_EQ(headSize, bufferSize);
}

// Test LibTvuMediaTvuliveWriteData with null buffer
TEST_F(LibShmmediaTvuliveProtocolTest, WriteData_NullBuffer) {
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.i_data = 100;

    int ret = LibTvuMediaTvuliveWriteData(&tvuliveData_, nullptr, 1024);
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteData_ZeroBufferSize) {
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.i_data = 100;

    std::vector<uint8_t> buffer(1024);
    int ret = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), 0);
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteData_NullInfo) {
    std::vector<uint8_t> buffer(1024);
    int ret = LibTvuMediaTvuliveWriteData(nullptr, buffer.data(), buffer.size());
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteData_BufferTooSmall) {
    uint8_t testData[] = {0x00, 0x01, 0x02, 0x03};
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    std::vector<uint8_t> buffer(4);  // Too small
    int ret = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteData_ValidVideoData) {
    uint8_t testData[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1f};  // H.264 NAL unit
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 100;
    tvuliveData_.o_info.u_frame_timestamp_ms = 1234567890;
    tvuliveData_.u_createTime = 9876543210;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int ret = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(ret, 0);
}

// Test LibTvuMediaTvuliveWriteHead
TEST_F(LibShmmediaTvuliveProtocolTest, WriteHead_NullInfo) {
    std::vector<uint8_t> buffer(1024);
    int ret = LibTvuMediaTvuliveWriteHead(nullptr, buffer.data(), buffer.size());
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteHead_ValidData) {
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 1;
    tvuliveData_.o_info.u_frame_index = 50;
    tvuliveData_.i_data = 1024;  // Data length for header

    uint32_t headSize = LibTvuMediaTvulivePreferHeadSize();
    std::vector<uint8_t> buffer(headSize);

    int ret = LibTvuMediaTvuliveWriteHead(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(ret, 0);
    EXPECT_EQ((uint32_t)ret, headSize);
}

// Test LibTvuMediaTvuliveReadData with null buffer
TEST_F(LibShmmediaTvuliveProtocolTest, ReadData_NullBuffer) {
    int ret = LibTvuMediaTvuliveReadData(&readData_, nullptr, 1024);
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, ReadData_ZeroBufferLen) {
    std::vector<uint8_t> buffer(1024);
    int ret = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), 0);
    EXPECT_LT(ret, 0);
}

TEST_F(LibShmmediaTvuliveProtocolTest, ReadData_NullInfo) {
    std::vector<uint8_t> buffer(1024);
    int ret = LibTvuMediaTvuliveReadData(nullptr, buffer.data(), buffer.size());
    EXPECT_LT(ret, 0);
}

// Test Write/Read roundtrip for video data
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_VideoData) {
    uint8_t testData[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1f, 0xDE, 0xAD, 0xBE, 0xEF};
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 2;
    tvuliveData_.o_info.u_program_index = 1;
    tvuliveData_.o_info.u_frame_index = 999;
    tvuliveData_.o_info.u_frame_timestamp_ms = 123456789012345ULL;
    tvuliveData_.u_createTime = 987654321098765ULL;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    // Write
    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    // Read
    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);
    EXPECT_EQ(writeRet, readRet);

    // Verify data
    EXPECT_EQ(readData_.o_info.i_type, tvuliveData_.o_info.i_type);
    EXPECT_EQ(readData_.o_info.u_stream_index, tvuliveData_.o_info.u_stream_index);
    EXPECT_EQ(readData_.o_info.u_program_index, tvuliveData_.o_info.u_program_index);
    EXPECT_EQ(readData_.o_info.u_frame_index, tvuliveData_.o_info.u_frame_index);
    EXPECT_EQ(readData_.o_info.u_frame_timestamp_ms, tvuliveData_.o_info.u_frame_timestamp_ms);
    EXPECT_EQ(readData_.i_data, tvuliveData_.i_data);
    EXPECT_NE(readData_.p_data, nullptr);
    EXPECT_EQ(memcmp(readData_.p_data, testData, sizeof(testData)), 0);
}

// Test Write/Read roundtrip for audio data
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_AudioData) {
    uint8_t testData[] = {0xFF, 0xFB, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00};  // MP3 frame header
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_AUDIO;
    tvuliveData_.o_info.u_stream_index = 1;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 500;
    tvuliveData_.o_info.u_frame_timestamp_ms = 5000;
    tvuliveData_.u_createTime = 6000;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaTvuliveDataType_AUDIO);
    EXPECT_EQ(readData_.i_data, tvuliveData_.i_data);
    EXPECT_EQ(memcmp(readData_.p_data, testData, sizeof(testData)), 0);
}

// Test Write/Read roundtrip for header data
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_HeaderData) {
    uint8_t testData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_HEADER;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 0;
    tvuliveData_.o_info.u_frame_timestamp_ms = 0;
    tvuliveData_.u_createTime = 1000;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaTvuliveDataType_HEADER);
}

// Test Write/Read roundtrip for metadata
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_MetadataData) {
    uint8_t testData[] = {0xAA, 0xBB, 0xCC, 0xDD};
    tvuliveData_.o_info.i_type = kLibShmMediaDataTypeTvuliveMetadata;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 10;
    tvuliveData_.o_info.u_frame_timestamp_ms = 2000;
    tvuliveData_.u_createTime = 3000;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaDataTypeTvuliveMetadata);
}

// Test Write/Read roundtrip with zero-length data
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_ZeroLengthData) {
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 0;
    tvuliveData_.o_info.u_frame_timestamp_ms = 0;
    tvuliveData_.u_createTime = 0;
    tvuliveData_.i_data = 0;
    tvuliveData_.p_data = nullptr;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(0);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.i_data, 0u);
}

// Test Write/Read roundtrip with large data
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_LargeData) {
    std::vector<uint8_t> largeData(64 * 1024);  // 64KB
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i & 0xFF);
    }

    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 1000;
    tvuliveData_.o_info.u_frame_timestamp_ms = UINT64_MAX;
    tvuliveData_.u_createTime = UINT64_MAX - 1;
    tvuliveData_.i_data = largeData.size();
    tvuliveData_.p_data = largeData.data();

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.i_data, tvuliveData_.i_data);
    EXPECT_EQ(memcmp(readData_.p_data, largeData.data(), largeData.size()), 0);
}

// Test Write/Read roundtrip with GOP POC extension
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_WithGopPoc) {
    uint8_t testData[] = {0x01, 0x02, 0x03, 0x04};
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_program_index = 0;
    tvuliveData_.o_info.u_frame_index = 50;
    tvuliveData_.o_info.u_frame_timestamp_ms = 12345;
    tvuliveData_.o_info.ext.u_gop_poc = 0x80000001;  // With exist flag and value
    tvuliveData_.u_createTime = 67890;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.ext.u_gop_poc, tvuliveData_.o_info.ext.u_gop_poc);
}

// Test boundary values
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_MaxStreamIndex) {
    uint8_t testData[] = {0x01};
    tvuliveData_.o_info.i_type = kLibShmMediaTvuliveDataType_VIDEO;
    tvuliveData_.o_info.u_stream_index = UINT16_MAX;
    tvuliveData_.o_info.u_program_index = UINT16_MAX;
    tvuliveData_.o_info.u_frame_index = UINT16_MAX;
    tvuliveData_.o_info.u_frame_timestamp_ms = UINT64_MAX;
    tvuliveData_.u_createTime = UINT64_MAX;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.u_stream_index, tvuliveData_.o_info.u_stream_index);
    EXPECT_EQ(readData_.o_info.u_program_index, tvuliveData_.o_info.u_program_index);
    EXPECT_EQ(readData_.o_info.u_frame_index, tvuliveData_.o_info.u_frame_index);
}

// Test multiple write/read cycles
TEST_F(LibShmmediaTvuliveProtocolTest, MultipleWriteReadCycles) {
    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(16);
    std::vector<uint8_t> buffer(bufferSize);

    for (int i = 0; i < 100; ++i) {
        uint8_t testData[16];
        for (int j = 0; j < 16; ++j) {
            testData[j] = static_cast<uint8_t>((i + j) & 0xFF);
        }

        tvuliveData_.o_info.i_type = (i % 3 == 0) ? kLibShmMediaTvuliveDataType_VIDEO :
                                     (i % 3 == 1) ? kLibShmMediaTvuliveDataType_AUDIO :
                                                    kLibShmMediaTvuliveDataType_HEADER;
        tvuliveData_.o_info.u_stream_index = i % 4;
        tvuliveData_.o_info.u_program_index = i % 2;
        tvuliveData_.o_info.u_frame_index = i;
        tvuliveData_.o_info.u_frame_timestamp_ms = 1000ULL * i;
        tvuliveData_.u_createTime = 2000ULL * i;
        tvuliveData_.i_data = sizeof(testData);
        tvuliveData_.p_data = testData;

        int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
        EXPECT_GT(writeRet, 0);

        memset(&readData_, 0, sizeof(readData_));
        readData_.u_struct_size = sizeof(libtvumedia_tvulive_data_v2_t);
        int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
        EXPECT_GT(readRet, 0);

        EXPECT_EQ(readData_.o_info.i_type, tvuliveData_.o_info.i_type);
        EXPECT_EQ(readData_.o_info.u_stream_index, tvuliveData_.o_info.u_stream_index);
        EXPECT_EQ(readData_.o_info.u_program_index, tvuliveData_.o_info.u_program_index);
        EXPECT_EQ(readData_.o_info.u_frame_index, tvuliveData_.o_info.u_frame_index);
        EXPECT_EQ(readData_.o_info.u_frame_timestamp_ms, tvuliveData_.o_info.u_frame_timestamp_ms);
        EXPECT_EQ(readData_.i_data, tvuliveData_.i_data);
        EXPECT_EQ(memcmp(readData_.p_data, testData, sizeof(testData)), 0);
    }
}

// Test data type enums
TEST_F(LibShmmediaTvuliveProtocolTest, DataTypeEnums_Values) {
    EXPECT_EQ(kLibShmMediaTvuliveDataType_VIDEO, 'v');
    EXPECT_EQ(kLibShmMediaTvuliveDataType_AUDIO, 'a');
    EXPECT_EQ(kLibShmMediaTvuliveDataType_HEADER, 'h');
    EXPECT_EQ(kLibShmMediaDataTypeTvuliveMetadata, 'd');
    EXPECT_EQ(kLibShmMediaDataTypeTvuliveSubtile, 's');

    // Verify aliases
    EXPECT_EQ(kLibShmMediaDataTypeTvuliveVideo, kLibShmMediaTvuliveDataType_VIDEO);
    EXPECT_EQ(kLibShmMediaDataTypeTvuliveAudio, kLibShmMediaTvuliveDataType_AUDIO);
    EXPECT_EQ(kLibShmMediaDataTypeTvuliveHead, kLibShmMediaTvuliveDataType_HEADER);
}

// Test different data types (raw jpeg, audio volume value)
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_RawJpegType) {
    uint8_t testData[] = {0xFF, 0xD8, 0xFF, 0xE0};  // JPEG header
    tvuliveData_.o_info.i_type = kLibShmMediaDataTypeRawJpeg;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaDataTypeRawJpeg);
}

TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_AudioVolumeType) {
    // Audio volume value format: type(8bit) + reserve(24bit) + counts(32bit) + volume values
    uint8_t testData[] = {0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00};
    tvuliveData_.o_info.i_type = kLibShmMediaDataTypeRawAudioVolumeValue;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.i_data = sizeof(testData);
    tvuliveData_.p_data = testData;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaDataTypeRawAudioVolumeValue);
}

// Test subtitle type
TEST_F(LibShmmediaTvuliveProtocolTest, WriteReadRoundtrip_SubtitleType) {
    const char* subtitleText = "Test subtitle";
    tvuliveData_.o_info.i_type = kLibShmMediaDataTypeTvuliveSubtile;
    tvuliveData_.o_info.u_stream_index = 0;
    tvuliveData_.o_info.u_frame_index = 1;
    tvuliveData_.o_info.u_frame_timestamp_ms = 5000;
    tvuliveData_.i_data = strlen(subtitleText);
    tvuliveData_.p_data = (const uint8_t*)subtitleText;

    uint32_t bufferSize = LibTvuMediaTvulivePreferBufferSize(tvuliveData_.i_data);
    std::vector<uint8_t> buffer(bufferSize);

    int writeRet = LibTvuMediaTvuliveWriteData(&tvuliveData_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int readRet = LibTvuMediaTvuliveReadData(&readData_, buffer.data(), writeRet);
    EXPECT_GT(readRet, 0);

    EXPECT_EQ(readData_.o_info.i_type, kLibShmMediaDataTypeTvuliveSubtile);
    EXPECT_EQ(readData_.i_data, strlen(subtitleText));
}
