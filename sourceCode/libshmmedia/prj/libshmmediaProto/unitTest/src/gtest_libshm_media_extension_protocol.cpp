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
#include "libshm_media_extension_protocol.h"
#include "libshm_media_extension_protocol_internal.h"
#include <cstring>
#include <vector>

class LibShmMediaExtensionProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&extendData, 0, sizeof(extendData));
        ctx = LibshmMediaExtDataCreateHandle();
    }

    void TearDown() override {
        if (ctx) {
            LibshmMediaExtDataDestroyHandle(&ctx);
        }
    }

    libshmmedia_extend_data_info_t extendData;
    libshmmedia_extended_data_context_t ctx;
};

// ===================== Handle Creation/Destruction Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, CreateHandle) {
    libshmmedia_extended_data_context_t handle = LibshmMediaExtDataCreateHandle();
    ASSERT_NE(handle, nullptr) << "Handle creation should succeed";
    LibshmMediaExtDataDestroyHandle(&handle);
    EXPECT_EQ(handle, nullptr) << "Handle should be null after destruction";
}

TEST_F(LibShmMediaExtensionProtocolTest, DestroyNullHandle) {
    libshmmedia_extended_data_context_t handle = nullptr;
    LibshmMediaExtDataDestroyHandle(&handle);
    EXPECT_EQ(handle, nullptr) << "Destroying null handle should be safe";
}

TEST_F(LibShmMediaExtensionProtocolTest, DestroyNullPointer) {
    LibshmMediaExtDataDestroyHandle(nullptr);
    // Should not crash
}

// ===================== Estimate Size Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeEmptyData) {
    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_EQ(size, 0) << "Empty extend data should have zero size";
}

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeWithUUID) {
    const uint8_t uuid[] = "test-uuid-12345";
    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = sizeof(uuid);

    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_GT(size, 0) << "Extend data with UUID should have positive size";
    EXPECT_GE(size, (int)(sizeof(uuid) + 16)) << "Size should include UUID data plus header";
}

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeWithCC608CDP) {
    const uint8_t cc608Data[] = {0x01, 0x02, 0x03, 0x04};
    extendData.p_cc608_cdp_data = cc608Data;
    extendData.i_cc608_cdp_length = sizeof(cc608Data);

    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_GT(size, 0) << "Extend data with CC608 CDP should have positive size";
}

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeWithMultipleFields) {
    const uint8_t uuid[] = "uuid";
    const uint8_t cc608[] = {0x01, 0x02};
    const uint8_t caption[] = "caption text";

    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = sizeof(uuid);
    extendData.p_cc608_cdp_data = cc608;
    extendData.i_cc608_cdp_length = sizeof(cc608);
    extendData.p_caption_text = caption;
    extendData.i_caption_text_length = sizeof(caption);

    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_GT(size, 0) << "Extend data with multiple fields should have positive size";
    EXPECT_GE(size, (int)(sizeof(uuid) + sizeof(cc608) + sizeof(caption) + 16))
        << "Size should include all data plus header";
}

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeWithSCTE104) {
    const uint8_t scteData[] = {0xFC, 0x30, 0x11, 0x00};
    extendData.p_scte104_data = scteData;
    extendData.i_scte104_data_len = sizeof(scteData);

    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_GT(size, 0) << "Extend data with SCTE104 should have positive size";
}

TEST_F(LibShmMediaExtensionProtocolTest, EstimateSizeWithColorParams) {
    extendData.bHasColorPrimariesVal_ = true;
    extendData.uColorPrimariesVal_ = 1;
    extendData.bHasColorTransferCharacteristicVal_ = true;
    extendData.uColorTransferCharacteristicVal_ = 1;
    extendData.bHasColorSpaceVal_ = true;
    extendData.uColorSpaceVal_ = 1;

    int size = LibShmMediaEstimateExtendDataSize(&extendData);
    EXPECT_GT(size, 0) << "Extend data with color params should have positive size";
}

// ===================== Write and Parse Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, WriteExtendDataEmpty) {
    std::vector<uint8_t> buffer(1024);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    // Empty data may return 0 or small header
    EXPECT_GE(written, 0) << "Writing empty data should not fail";
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteExtendDataWithUUID) {
    const uint8_t uuid[] = "test-uuid-data-12345";
    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);

    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    EXPECT_GT(written, 0) << "Writing UUID data should succeed";
    EXPECT_LE(written, (int)buffer.size()) << "Written size should not exceed buffer";
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseExtendDataRoundtrip) {
    // Setup test data
    const uint8_t uuid[] = "roundtrip-uuid";
    const uint8_t cc608[] = {0xAA, 0xBB, 0xCC, 0xDD};
    const uint8_t caption[] = "Test Caption Text";

    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);
    extendData.p_cc608_cdp_data = cc608;
    extendData.i_cc608_cdp_length = sizeof(cc608);
    extendData.p_caption_text = caption;
    extendData.i_caption_text_length = strlen((const char*)caption);

    // Write
    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0) << "Write should succeed";

    // Parse
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0) << "Parse should succeed";

    // Verify
    EXPECT_EQ(parsedData.i_uuid_length, (int)strlen((const char*)uuid)) << "UUID length should match";
    EXPECT_EQ(parsedData.i_cc608_cdp_length, (int)sizeof(cc608)) << "CC608 length should match";
    EXPECT_EQ(parsedData.i_caption_text_length, (int)strlen((const char*)caption)) << "Caption length should match";

    if (parsedData.p_uuid_data) {
        EXPECT_EQ(memcmp(parsedData.p_uuid_data, uuid, parsedData.i_uuid_length), 0)
            << "UUID data should match";
    }
    if (parsedData.p_cc608_cdp_data) {
        EXPECT_EQ(memcmp(parsedData.p_cc608_cdp_data, cc608, parsedData.i_cc608_cdp_length), 0)
            << "CC608 data should match";
    }
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseWithContext) {
    // Setup test data
    const uint8_t uuid[] = "context-uuid";

    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);

    // Write
    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0) << "Write should succeed";

    // Parse with context
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = libShmReadExtendDataV2(&parsedData, buffer.data(), written,
                                        LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2, ctx);
    ASSERT_EQ(result, 0) << "Parse with context should succeed";

    EXPECT_EQ(parsedData.i_uuid_length, (int)strlen((const char*)uuid)) << "UUID length should match";
}

// ===================== Parse Invalid Data Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, ParseNullData) {
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = LibShmMeidaParseExtendData(&parsedData, nullptr, 100, LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2);
    EXPECT_LT(result, 0) << "Parse with null data should fail";
}

TEST_F(LibShmMediaExtensionProtocolTest, ParseZeroSize) {
    std::vector<uint8_t> buffer(100);
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = LibShmMeidaParseExtendData(&parsedData, buffer.data(), 0, LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2);
    EXPECT_LT(result, 0) << "Parse with zero size should fail";
}

TEST_F(LibShmMediaExtensionProtocolTest, ParseInvalidType) {
    std::vector<uint8_t> buffer(100);
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = LibShmMeidaParseExtendData(&parsedData, buffer.data(), buffer.size(), 999);
    EXPECT_LT(result, 0) << "Parse with invalid type should fail";
}

// ===================== Read Extend Data V2 Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, ReadExtendDataV2WithUIDType) {
    const uint8_t uuid[] = "simple-uid-data";
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = libShmReadExtendDataV2(&parsedData, uuid, sizeof(uuid),
                                        LIBSHM_MEDIA_TYPE_TVU_UID, ctx);
    ASSERT_EQ(result, 0) << "Read UID type should succeed";
    EXPECT_EQ(parsedData.i_uuid_length, (int)sizeof(uuid)) << "UUID length should match";
    EXPECT_EQ(parsedData.p_uuid_data, uuid) << "UUID pointer should point to original data";
}

TEST_F(LibShmMediaExtensionProtocolTest, ReadExtendDataV2NullContext) {
    std::vector<uint8_t> buffer(100);
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = libShmReadExtendDataV2(&parsedData, buffer.data(), buffer.size(),
                                        LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2, nullptr);
    EXPECT_LT(result, 0) << "Read with null context for V2 type should fail";
}

// ===================== Entry Management Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, ResetEntry) {
    ASSERT_NE(ctx, nullptr);
    LibshmMediaExtDataResetEntry(ctx);
    EXPECT_EQ(LibshmMediaExtDataGetEntryCounts(ctx), 0u) << "Entry count should be 0 after reset";
}

TEST_F(LibShmMediaExtensionProtocolTest, GetEntryCountsEmpty) {
    ASSERT_NE(ctx, nullptr);
    LibshmMediaExtDataResetEntry(ctx);
    unsigned int count = LibshmMediaExtDataGetEntryCounts(ctx);
    EXPECT_EQ(count, 0u) << "Empty context should have 0 entries";
}

TEST_F(LibShmMediaExtensionProtocolTest, AddAndGetEntry) {
    ASSERT_NE(ctx, nullptr);
    LibshmMediaExtDataResetEntry(ctx);

    // Add entry
    const uint8_t testData[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    libshmmedia_extended_entry_data_t entry;
    entry.p_data = testData;
    entry.u_len = sizeof(testData);
    entry.u_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID;

    std::vector<uint8_t> buffer(1024);
    int result = LibshmMediaExtDataAddOneEntry(ctx, &entry, buffer.data(), buffer.size());
    EXPECT_GT(result, 0) << "Adding entry should succeed";

    unsigned int bufSize = LibshmMediaExtDataGetEntryBuffSize(ctx);
    EXPECT_GT(bufSize, 0u) << "Buffer size should be positive after adding entry";
}

TEST_F(LibShmMediaExtensionProtocolTest, ParseBufferAndGetEntries) {
    // First write some data
    const uint8_t uuid[] = "parse-test-uuid";
    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0) << "Write should succeed";

    // Create new context for parsing
    libshmmedia_extended_data_context_t parseCtx = LibshmMediaExtDataCreateHandle();
    ASSERT_NE(parseCtx, nullptr);

    // Parse
    int entryCount = LibshmMediaExtDataParseBuff(parseCtx, buffer.data(), written);
    EXPECT_GT(entryCount, 0) << "Should have at least one entry";

    unsigned int count = LibshmMediaExtDataGetEntryCounts(parseCtx);
    EXPECT_EQ(count, (unsigned int)entryCount) << "Entry count should match";

    // Get entry
    libshmmedia_extended_entry_data_t entry;
    int getResult = LibshmMediaExtDataGetOneEntry(parseCtx, 0, &entry);
    EXPECT_EQ(getResult, 0) << "Getting first entry should succeed";
    EXPECT_EQ(entry.u_type, (uint32_t)LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID) << "Entry type should be UID";

    LibshmMediaExtDataDestroyHandle(&parseCtx);
}

TEST_F(LibShmMediaExtensionProtocolTest, ParseEmptyBuffer) {
    int entryCount = LibshmMediaExtDataParseBuff(ctx, nullptr, 0);
    EXPECT_EQ(entryCount, 0) << "Parsing empty buffer should return 0 entries";
}

// ===================== All Data Types Roundtrip Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseSCTE104) {
    const uint8_t scteData[] = {0xFC, 0x30, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
    extendData.p_scte104_data = scteData;
    extendData.i_scte104_data_len = sizeof(scteData);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_scte104_data_len, (int)sizeof(scteData));
    if (parsedData.p_scte104_data) {
        EXPECT_EQ(memcmp(parsedData.p_scte104_data, scteData, parsedData.i_scte104_data_len), 0);
    }
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseSCTE35) {
    const uint8_t scteData[] = {0xFC, 0x30, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xF0};
    extendData.p_scte35_data = scteData;
    extendData.i_scte35_data_len = sizeof(scteData);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_scte35_data_len, (int)sizeof(scteData));
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseHDRMetadata) {
    const uint8_t hdrData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    extendData.p_hdr_metadata = hdrData;
    extendData.i_hdr_metadata = sizeof(hdrData);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_hdr_metadata, (int)sizeof(hdrData));
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseTimecode) {
    const uint8_t timecodeData[] = "01:02:03:04";
    extendData.p_timecode = timecodeData;
    extendData.i_timecode = strlen((const char*)timecodeData);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_timecode, (int)strlen((const char*)timecodeData));
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseSourceTimestamp) {
    uint64_t timestamp = 0x0123456789ABCDEF;
    extendData.p_source_timestamp = (const uint8_t*)&timestamp;
    extendData.i_source_timestamp = sizeof(timestamp);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_source_timestamp, (int)sizeof(timestamp));
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseSubtitle) {
    const uint8_t subtitleData[] = "Test subtitle text";
    extendData.u_subtitle_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    extendData.p_subtitle = subtitleData;
    extendData.i_subtitle = strlen((const char*)subtitleData);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_EQ(parsedData.i_subtitle, (int)strlen((const char*)subtitleData));
    EXPECT_EQ(parsedData.u_subtitle_type, (uint32_t)LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT);
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseAllFields) {
    // Setup all fields
    const uint8_t uuid[] = "all-fields-uuid";
    const uint8_t cc608[] = {0x01, 0x02};
    const uint8_t caption[] = "caption";
    const uint8_t producer[] = "producer-info";
    const uint8_t receiver[] = "receiver-info";
    const uint8_t scte104[] = {0xFC, 0x30};
    const uint8_t scte35[] = {0xFC, 0x30, 0x25};
    const uint8_t timecodeIdx[] = {0x01, 0x02, 0x03, 0x04};
    const uint8_t startTimecode[] = "00:00:00:00";
    const uint8_t hdr[] = {0x01, 0x02, 0x03};
    const uint8_t timecodeFps[] = {0x01, 0x02};
    const uint8_t picStruct[] = {0x01};
    uint64_t srcTimestamp = 12345678;
    const uint8_t timecode[] = "01:02:03:04";
    uint64_t metaPts = 98765432;
    const uint8_t timebase[] = {0x01, 0x02, 0x03, 0x04};
    const uint8_t afd[] = {0x01, 0x02};
    uint64_t actionTimestamp = 11223344;
    const uint8_t vanc[] = {0x01, 0x02, 0x03};
    uint32_t gopPoc = 42;
    const uint8_t subtitle[] = "subtitle text";

    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);
    extendData.p_cc608_cdp_data = cc608;
    extendData.i_cc608_cdp_length = sizeof(cc608);
    extendData.p_caption_text = caption;
    extendData.i_caption_text_length = strlen((const char*)caption);
    extendData.p_producer_stream_info = producer;
    extendData.i_producer_stream_info_length = strlen((const char*)producer);
    extendData.p_receiver_info = receiver;
    extendData.i_receiver_info_length = strlen((const char*)receiver);
    extendData.p_scte104_data = scte104;
    extendData.i_scte104_data_len = sizeof(scte104);
    extendData.p_scte35_data = scte35;
    extendData.i_scte35_data_len = sizeof(scte35);
    extendData.p_timecode_index = timecodeIdx;
    extendData.i_timecode_index_length = sizeof(timecodeIdx);
    extendData.p_start_timecode = startTimecode;
    extendData.i_start_timecode_length = strlen((const char*)startTimecode);
    extendData.p_hdr_metadata = hdr;
    extendData.i_hdr_metadata = sizeof(hdr);
    extendData.p_timecode_fps_index = timecodeFps;
    extendData.i_timecode_fps_index = sizeof(timecodeFps);
    extendData.p_pic_struct = picStruct;
    extendData.i_pic_struct = sizeof(picStruct);
    extendData.p_source_timestamp = (const uint8_t*)&srcTimestamp;
    extendData.i_source_timestamp = sizeof(srcTimestamp);
    extendData.p_timecode = timecode;
    extendData.i_timecode = strlen((const char*)timecode);
    extendData.p_metaDataPts = (const uint8_t*)&metaPts;
    extendData.i_metaDataPts = sizeof(metaPts);
    extendData.p_source_timebase = timebase;
    extendData.i_source_timebase = sizeof(timebase);
    extendData.p_smpte_afd_data = afd;
    extendData.i_smpte_afd_data = sizeof(afd);
    extendData.p_source_action_timestamp = (const uint8_t*)&actionTimestamp;
    extendData.i_source_action_timestamp = sizeof(actionTimestamp);
    extendData.p_vanc_smpte2038 = vanc;
    extendData.i_vanc_smpte2038 = sizeof(vanc);
    extendData.p_gop_poc = (const uint8_t*)&gopPoc;
    extendData.i_gop_poc = sizeof(gopPoc);
    extendData.u_subtitle_type = LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT;
    extendData.p_subtitle = subtitle;
    extendData.i_subtitle = strlen((const char*)subtitle);

    // Write
    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 256);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0) << "Writing all fields should succeed";

    // Parse
    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0) << "Parsing all fields should succeed";

    // Verify key fields
    EXPECT_EQ(parsedData.i_uuid_length, (int)strlen((const char*)uuid));
    EXPECT_EQ(parsedData.i_cc608_cdp_length, (int)sizeof(cc608));
    EXPECT_EQ(parsedData.i_scte104_data_len, (int)sizeof(scte104));
    EXPECT_EQ(parsedData.i_subtitle, (int)strlen((const char*)subtitle));
}

// ===================== Color Parameters Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseColorParameters) {
    extendData.bHasColorPrimariesVal_ = true;
    extendData.uColorPrimariesVal_ = 1; // BT.709
    extendData.bHasColorTransferCharacteristicVal_ = true;
    extendData.uColorTransferCharacteristicVal_ = 1;
    extendData.bHasColorSpaceVal_ = true;
    extendData.uColorSpaceVal_ = 1;
    extendData.bHasVideoFullRangeFlagVal_ = true;
    extendData.uVideoFullRangeFlagVal_ = 0;
    extendData.bGotTvutimestamp = true;
    extendData.u64Tvutimestamp = 0x123456789ABCDEF0;

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));
    int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
    ASSERT_EQ(result, 0);

    EXPECT_TRUE(parsedData.bHasColorPrimariesVal_);
    EXPECT_EQ(parsedData.uColorPrimariesVal_, 1u);
    EXPECT_TRUE(parsedData.bHasColorTransferCharacteristicVal_);
    EXPECT_EQ(parsedData.uColorTransferCharacteristicVal_, 1u);
    EXPECT_TRUE(parsedData.bHasColorSpaceVal_);
    EXPECT_EQ(parsedData.uColorSpaceVal_, 1u);
    EXPECT_TRUE(parsedData.bHasVideoFullRangeFlagVal_);
    EXPECT_EQ(parsedData.uVideoFullRangeFlagVal_, 0u);
    EXPECT_TRUE(parsedData.bGotTvutimestamp);
    EXPECT_EQ(parsedData.u64Tvutimestamp, 0x123456789ABCDEF0ull);
}

// ===================== CLibShmMediaExtendedDataV2 Class Tests =====================

TEST(CLibShmMediaExtendedDataV2Test, ConstructorDestructor) {
    CLibShmMediaExtendedDataV2* obj = new CLibShmMediaExtendedDataV2();
    ASSERT_NE(obj, nullptr);
    delete obj;
}

TEST(CLibShmMediaExtendedDataV2Test, ResetEntry) {
    CLibShmMediaExtendedDataV2 obj;
    int result = obj.resetEntry();
    EXPECT_EQ(result, 0);
    EXPECT_EQ(obj.getEntryCouts(), 0u);
}

TEST(CLibShmMediaExtendedDataV2Test, AddEntryToBuff) {
    CLibShmMediaExtendedDataV2 obj;
    obj.resetEntry();

    const uint8_t testData[] = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> buffer(1024);

    int written = obj.addEntryToBuff(testData, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID, sizeof(testData),
                                      buffer.data(), buffer.size());
    EXPECT_GT(written, 0);
    EXPECT_GT(obj.getBufWriteSize(), 0u);
}

TEST(CLibShmMediaExtendedDataV2Test, ParseBuffAndGetEntries) {
    CLibShmMediaExtendedDataV2 writeObj;
    writeObj.resetEntry();

    // Add multiple entries
    const uint8_t data1[] = {0x01, 0x02};
    const uint8_t data2[] = {0x03, 0x04, 0x05};

    std::vector<uint8_t> buffer(1024);
    writeObj.addEntryToBuff(data1, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID, sizeof(data1),
                            buffer.data(), buffer.size());
    writeObj.addEntryToBuff(data2, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP, sizeof(data2),
                            buffer.data(), buffer.size());

    unsigned int bufSize = writeObj.getBufWriteSize();

    // Parse
    CLibShmMediaExtendedDataV2 readObj;
    int entryCount = readObj.parseBuff(buffer.data(), bufSize);
    EXPECT_EQ(entryCount, 2);
    EXPECT_EQ(readObj.getEntryCouts(), 2u);

    // Get entries
    EXPECT_EQ(readObj.getOneEntryType(0), (unsigned int)LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID);
    EXPECT_EQ(readObj.getOneEntryLen(0), (int)sizeof(data1));
    EXPECT_EQ(readObj.getOneEntryType(1), (unsigned int)LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_CC608_CDP);
    EXPECT_EQ(readObj.getOneEntryLen(1), (int)sizeof(data2));

    libshmmedia_extended_entry_data_t entry;
    int result = readObj.getOneEntry(0, &entry);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(entry.u_type, (uint32_t)LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID);
    EXPECT_EQ(entry.u_len, sizeof(data1));

    // Get entry data
    std::vector<uint8_t> entryData(sizeof(data1));
    int dataLen = readObj.getOneEntryData(0, entryData.data(), entryData.size());
    EXPECT_EQ(dataLen, (int)sizeof(data1));
    EXPECT_EQ(memcmp(entryData.data(), data1, sizeof(data1)), 0);
}

TEST(CLibShmMediaExtendedDataV2Test, GetOneEntryInvalidIndex) {
    CLibShmMediaExtendedDataV2 obj;
    obj.resetEntry();

    libshmmedia_extended_entry_data_t entry;
    int result = obj.getOneEntry(0, &entry);
    EXPECT_LT(result, 0) << "Getting entry from empty object should fail";
    EXPECT_EQ(entry.u_len, 0u);
    EXPECT_EQ(entry.u_type, 0u);
    EXPECT_EQ(entry.p_data, nullptr);
}

TEST(CLibShmMediaExtendedDataV2Test, SetUsingExternalBuf) {
    CLibShmMediaExtendedDataV2 writeObj;
    writeObj.resetEntry();

    const uint8_t data[] = {0x01, 0x02, 0x03};
    std::vector<uint8_t> buffer(1024);
    writeObj.addEntryToBuff(data, LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_UID, sizeof(data),
                            buffer.data(), buffer.size());
    unsigned int bufSize = writeObj.getBufWriteSize();

    CLibShmMediaExtendedDataV2 readObj;
    readObj.setUsingExternalBuf(true);
    int entryCount = readObj.parseBuff(buffer.data(), bufSize);
    EXPECT_EQ(entryCount, 1);

    libshmmedia_extended_entry_data_t entry;
    readObj.getOneEntry(0, &entry);
    // When using external buffer, p_data should point into the original buffer
    EXPECT_NE(entry.p_data, nullptr);
}

// ===================== Subtitle Type Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, WriteAndParseAllSubtitleTypes) {
    const uint32_t subtitleTypes[] = {
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_TELETEXT,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVB_SUBTITLE,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_DVD_SUBTITLE,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_WEBVTT,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SRT,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SUBRIP,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_RAW_TEXT,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_TTML,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_SSA,
        LIBSHMMEDIA_EXTEND_DATA_TYPE_V2_SUBTITLE_ASS,
    };

    const uint8_t subtitleData[] = "Test subtitle content";

    for (uint32_t subType : subtitleTypes) {
        memset(&extendData, 0, sizeof(extendData));
        extendData.u_subtitle_type = subType;
        extendData.p_subtitle = subtitleData;
        extendData.i_subtitle = strlen((const char*)subtitleData);

        int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
        std::vector<uint8_t> buffer(estimatedSize + 128);
        int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
        ASSERT_GT(written, 0) << "Write failed for subtitle type " << subType;

        libshmmedia_extend_data_info_t parsedData;
        memset(&parsedData, 0, sizeof(parsedData));
        int result = LibShmMeidaParseExtendDataV2(&parsedData, buffer.data(), written);
        ASSERT_EQ(result, 0) << "Parse failed for subtitle type " << subType;

        EXPECT_EQ(parsedData.u_subtitle_type, subType) << "Subtitle type mismatch";
        EXPECT_EQ(parsedData.i_subtitle, (int)strlen((const char*)subtitleData));
    }
}

// ===================== LibShmMediaReadExtendData API Tests =====================

TEST_F(LibShmMediaExtensionProtocolTest, LibShmMediaReadExtendDataAPI) {
    // Setup test data
    const uint8_t uuid[] = "api-test-uuid";
    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize + 128);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    ASSERT_GT(written, 0);

    libshmmedia_extend_data_info_t parsedData;
    memset(&parsedData, 0, sizeof(parsedData));

    int result = LibShmMediaReadExtendData(&parsedData, buffer.data(), written,
                                           LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2, ctx);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(parsedData.i_uuid_length, (int)strlen((const char*)uuid));
}

// ===================== Buffer Size Edge Cases =====================

TEST_F(LibShmMediaExtensionProtocolTest, WriteToSmallBuffer) {
    const uint8_t largeData[512] = {0};
    extendData.p_uuid_data = largeData;
    extendData.i_uuid_length = sizeof(largeData);

    std::vector<uint8_t> buffer(10); // Too small
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    // Either returns 0 or the actual size needed (implementation dependent)
    EXPECT_GE(written, 0);
}

TEST_F(LibShmMediaExtensionProtocolTest, WriteToExactBuffer) {
    const uint8_t uuid[] = "exact-test";
    extendData.p_uuid_data = uuid;
    extendData.i_uuid_length = strlen((const char*)uuid);

    int estimatedSize = LibShmMediaEstimateExtendDataSize(&extendData);
    std::vector<uint8_t> buffer(estimatedSize);
    int written = LibShmMediaWriteExtendData(buffer.data(), buffer.size(), &extendData);
    EXPECT_GT(written, 0);
    EXPECT_LE(written, estimatedSize);
}
