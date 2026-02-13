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
 * @file gtest_libshm_media_subtitle_private_protocol.cpp
 * @brief Unit tests for libshm_media_subtitle_private_protocol_internal.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "libshm_media_subtitle_private_protocol.h"
#include "libshm_media_extension_protocol.h"

class LibShmMediaSubtitlePrivateProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&entries_, 0, sizeof(entries_));
        memset(&readEntries_, 0, sizeof(readEntries_));
    }

    void TearDown() override {
    }

    libshmmedia_subtitle_private_proto_entries_t entries_;
    libshmmedia_subtitle_private_proto_entries_t readEntries_;
};

// Test LibshmmediaSubtitlePrivateProtoIsCorrectType
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, IsCorrectType_ValidType) {
    bool ret = LibshmmediaSubtitlePrivateProtoIsCorrectType(
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_PRIVATE_PROTOCOL_EXTENTION_STRUCTURE
    );
    EXPECT_TRUE(ret);
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, IsCorrectType_InvalidType) {
    bool ret = LibshmmediaSubtitlePrivateProtoIsCorrectType(0);
    EXPECT_FALSE(ret);

    ret = LibshmmediaSubtitlePrivateProtoIsCorrectType(0x12345678);
    EXPECT_FALSE(ret);
}

// Test LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, PreEstimateBufferSize_EmptyEntries) {
    entries_.head.counts = 0;

    int size = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    EXPECT_GT(size, 0);  // Should have header overhead
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, PreEstimateBufferSize_SingleEntry) {
    const char* subtitleText = "Test subtitle";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 5000;
    entries_.entries[0].duration = 3000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    int size = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    EXPECT_GT(size, (int)strlen(subtitleText));
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, PreEstimateBufferSize_MultipleEntries) {
    const char* text1 = "First subtitle";
    const char* text2 = "Second subtitle";

    entries_.head.counts = 2;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 1000;
    entries_.entries[0].duration = 2000;
    entries_.entries[0].dataLen = strlen(text1);
    entries_.entries[0].data = (const uint8_t*)text1;

    entries_.entries[1].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[1].timestamp = 4000;
    entries_.entries[1].duration = 2000;
    entries_.entries[1].dataLen = strlen(text2);
    entries_.entries[1].data = (const uint8_t*)text2;

    int size = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    EXPECT_GT(size, (int)(strlen(text1) + strlen(text2)));
}

// Test LibshmmediaSubtitlePrivateProtoWriteBufferSize
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, WriteBufferSize_BufferTooSmall) {
    const char* subtitleText = "Test subtitle";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 5000;
    entries_.entries[0].duration = 3000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    std::vector<uint8_t> buffer(4);  // Too small
    int ret = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_LT(ret, 0);  // Should fail
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, WriteBufferSize_ValidWrite) {
    const char* subtitleText = "Test subtitle content";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 5000;
    entries_.entries[0].duration = 3000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int ret = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(ret, 0);
    EXPECT_EQ(ret, estimatedSize);
}

// Test Write/Parse roundtrip
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, WriteParseRoundtrip_SingleEntry) {
    const char* subtitleText = "Hello World Subtitle";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 12345;
    entries_.entries[0].duration = 5000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    // Write
    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    // Parse
    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);  // 0 means success

    // Verify
    EXPECT_EQ(readEntries_.head.counts, entries_.head.counts);
    EXPECT_EQ(readEntries_.entries[0].type, entries_.entries[0].type);
    EXPECT_EQ(readEntries_.entries[0].timestamp, entries_.entries[0].timestamp);
    EXPECT_EQ(readEntries_.entries[0].duration, entries_.entries[0].duration);
    EXPECT_EQ(readEntries_.entries[0].dataLen, entries_.entries[0].dataLen);
    EXPECT_NE(readEntries_.entries[0].data, nullptr);
    EXPECT_EQ(memcmp(readEntries_.entries[0].data, subtitleText, strlen(subtitleText)), 0);
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, WriteParseRoundtrip_MultipleEntries) {
    const char* text1 = "First subtitle line";
    const char* text2 = "Second subtitle line";
    const char* text3 = "Third subtitle line";

    entries_.head.counts = 3;

    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 1000;
    entries_.entries[0].duration = 2000;
    entries_.entries[0].dataLen = strlen(text1);
    entries_.entries[0].data = (const uint8_t*)text1;

    entries_.entries[1].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT;
    entries_.entries[1].timestamp = 4000;
    entries_.entries[1].duration = 2500;
    entries_.entries[1].dataLen = strlen(text2);
    entries_.entries[1].data = (const uint8_t*)text2;

    entries_.entries[2].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT;
    entries_.entries[2].timestamp = 8000;
    entries_.entries[2].duration = 3000;
    entries_.entries[2].dataLen = strlen(text3);
    entries_.entries[2].data = (const uint8_t*)text3;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    // Write
    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    // Parse
    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    // Verify
    EXPECT_EQ(readEntries_.head.counts, 3);

    EXPECT_EQ(readEntries_.entries[0].type, entries_.entries[0].type);
    EXPECT_EQ(readEntries_.entries[0].timestamp, entries_.entries[0].timestamp);
    EXPECT_EQ(readEntries_.entries[0].duration, entries_.entries[0].duration);
    EXPECT_EQ(memcmp(readEntries_.entries[0].data, text1, strlen(text1)), 0);

    EXPECT_EQ(readEntries_.entries[1].type, entries_.entries[1].type);
    EXPECT_EQ(readEntries_.entries[1].timestamp, entries_.entries[1].timestamp);
    EXPECT_EQ(memcmp(readEntries_.entries[1].data, text2, strlen(text2)), 0);

    EXPECT_EQ(readEntries_.entries[2].type, entries_.entries[2].type);
    EXPECT_EQ(readEntries_.entries[2].timestamp, entries_.entries[2].timestamp);
    EXPECT_EQ(memcmp(readEntries_.entries[2].data, text3, strlen(text3)), 0);
}

// Test LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, TuneTimestampOffset_PositiveOffset) {
    const char* subtitleText = "Test subtitle";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 5000;
    entries_.entries[0].duration = 3000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    ASSERT_GT(writeRet, 0);

    // Apply +1000ms offset
    int tuneRet = LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(buffer.data(), writeRet, 1000);
    EXPECT_EQ(tuneRet, 0);  // 0 means success

    // Parse and verify timestamp changed
    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].timestamp, 6000u);  // 5000 + 1000
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, TuneTimestampOffset_NegativeOffset) {
    const char* subtitleText = "Test subtitle";
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 5000;
    entries_.entries[0].duration = 3000;
    entries_.entries[0].dataLen = strlen(subtitleText);
    entries_.entries[0].data = (const uint8_t*)subtitleText;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    ASSERT_GT(writeRet, 0);

    // Apply -2000ms offset
    int tuneRet = LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(buffer.data(), writeRet, -2000);
    EXPECT_EQ(tuneRet, 0);

    // Parse and verify timestamp changed
    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].timestamp, 3000u);  // 5000 - 2000
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, TuneTimestampOffset_MultipleEntries) {
    const char* text1 = "First";
    const char* text2 = "Second";

    entries_.head.counts = 2;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[0].timestamp = 1000;
    entries_.entries[0].duration = 2000;
    entries_.entries[0].dataLen = strlen(text1);
    entries_.entries[0].data = (const uint8_t*)text1;

    entries_.entries[1].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    entries_.entries[1].timestamp = 4000;
    entries_.entries[1].duration = 2000;
    entries_.entries[1].dataLen = strlen(text2);
    entries_.entries[1].data = (const uint8_t*)text2;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    ASSERT_GT(writeRet, 0);

    // Apply +500ms offset
    int tuneRet = LibshmmediaSubtitlePrivateProtoTuneTimestampOffSet(buffer.data(), writeRet, 500);
    EXPECT_EQ(tuneRet, 0);

    // Parse and verify both timestamps changed
    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].timestamp, 1500u);  // 1000 + 500
    EXPECT_EQ(readEntries_.entries[1].timestamp, 4500u);  // 4000 + 500
}

// Test different subtitle types
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, SubtitleTypes_DVBTeletext) {
    uint8_t testData[] = {0x01, 0x02, 0x03, 0x04};
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT;
    entries_.entries[0].timestamp = 1000;
    entries_.entries[0].duration = 2000;
    entries_.entries[0].dataLen = sizeof(testData);
    entries_.entries[0].data = testData;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].type, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT);
}

TEST_F(LibShmMediaSubtitlePrivateProtocolTest, SubtitleTypes_EIA608) {
    uint8_t testData[] = {0x94, 0x2C};  // EIA-608 control code example
    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_EIA608;
    entries_.entries[0].timestamp = 1000;
    entries_.entries[0].duration = 100;
    entries_.entries[0].dataLen = sizeof(testData);
    entries_.entries[0].data = testData;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].type, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_EIA608);
}

// Test max entries count
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, MaxEntriesCount) {
    EXPECT_EQ(LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS, 8);

    // Fill all entries
    entries_.head.counts = LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS;
    for (int i = 0; i < LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS; ++i) {
        entries_.entries[i].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
        entries_.entries[i].timestamp = i * 1000;
        entries_.entries[i].duration = 500;
        entries_.entries[i].dataLen = 4;
        static const uint8_t data[] = "test";
        entries_.entries[i].data = data;
    }

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.head.counts, LIBSHMMEDIA_SUBTITLE_MAX_ENTRY_COUNTS);
}

// Test entry item structure
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, EntryItemStructure) {
    libshmmedia_subtitle_private_proto_entry_item_t item;
    memset(&item, 0, sizeof(item));

    item.type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA;
    item.timestamp = 123456789;
    item.duration = 5000;
    item.dataLen = 100;

    EXPECT_EQ(item.type, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA);
    EXPECT_EQ(item.timestamp, 123456789u);
    EXPECT_EQ(item.duration, 5000u);
    EXPECT_EQ(item.dataLen, 100u);
}

// Test with binary data (not just text)
TEST_F(LibShmMediaSubtitlePrivateProtocolTest, BinaryData) {
    // DVB subtitle binary data example
    uint8_t binaryData[64];
    for (int i = 0; i < 64; ++i) {
        binaryData[i] = static_cast<uint8_t>(i);
    }

    entries_.head.counts = 1;
    entries_.entries[0].type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_SUBTITLE;
    entries_.entries[0].timestamp = 10000;
    entries_.entries[0].duration = 1000;
    entries_.entries[0].dataLen = sizeof(binaryData);
    entries_.entries[0].data = binaryData;

    int estimatedSize = LibshmmediaSubtitlePrivateProtoPreEstimateBufferSize(&entries_);
    std::vector<uint8_t> buffer(estimatedSize);

    int writeRet = LibshmmediaSubtitlePrivateProtoWriteBufferSize(&entries_, buffer.data(), buffer.size());
    EXPECT_GT(writeRet, 0);

    int parseRet = LibshmmediaSubtitlePrivateProtoParseBufferSize(&readEntries_, buffer.data(), writeRet);
    EXPECT_EQ(parseRet, 0);

    EXPECT_EQ(readEntries_.entries[0].dataLen, sizeof(binaryData));
    EXPECT_EQ(memcmp(readEntries_.entries[0].data, binaryData, sizeof(binaryData)), 0);
}
