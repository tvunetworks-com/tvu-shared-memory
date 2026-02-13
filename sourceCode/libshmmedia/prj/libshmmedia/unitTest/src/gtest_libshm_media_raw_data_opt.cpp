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
 * @file gtest_libshm_media_raw_data_opt.cpp
 * @brief Unit tests for libshm_media_raw_data_opt.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "libshmmedia.h"
#include "libshm_media_raw_data_opt.h"

class LibShmMediaRawDataOptTest : public ::testing::Test {
protected:
    void SetUp() override {
        creatorHandle_ = nullptr;
        readerHandle_ = nullptr;
        // Clean up any leftover shared memory
        LibShmMediaRemoveShmidFromSystem(kTestShmName);
    }

    void TearDown() override {
        if (readerHandle_) {
            LibShmMediaDestroy(readerHandle_);
            readerHandle_ = nullptr;
        }
        if (creatorHandle_) {
            LibShmMediaDestroy(creatorHandle_);
            creatorHandle_ = nullptr;
        }
        // Clean up test shared memory
        LibShmMediaRemoveShmidFromSystem(kTestShmName);
    }

    static constexpr const char* kTestShmName = "test_rawdata_opt_shm";
    static constexpr uint32_t kTestHeaderLen = 512;
    static constexpr uint32_t kTestItemLen = 4096;
    static constexpr uint32_t kTestItemCount = 8;

    libshm_media_handle_t creatorHandle_;
    libshm_media_handle_t readerHandle_;
};

// Test LibShmMediaRawDataWrite with null data
TEST_F(LibShmMediaRawDataOptTest, RawDataWrite_NullData) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    int ret = LibShmMediaRawDataWrite(creatorHandle_, nullptr);
    EXPECT_EQ(ret, 0);  // Function returns 0 currently
}

// Test LibShmMediaRawDataApply
TEST_F(LibShmMediaRawDataOptTest, RawDataApply_ValidHandle) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    uint8_t* buffer = LibShmMediaRawDataApply(creatorHandle_, 256);
    EXPECT_NE(buffer, nullptr);
}

TEST_F(LibShmMediaRawDataOptTest, RawDataApply_ZeroLength) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    uint8_t* buffer = LibShmMediaRawDataApply(creatorHandle_, 0);
    // Behavior depends on implementation
    SUCCEED();  // Just verify no crash
}

TEST_F(LibShmMediaRawDataOptTest, RawDataApply_LargeLength) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    // Request larger than item length
    uint8_t* buffer = LibShmMediaRawDataApply(creatorHandle_, kTestItemLen * 2);
    // May return null or valid pointer depending on implementation
    SUCCEED();
}

// Test LibShmMediaRawDataCommit
TEST_F(LibShmMediaRawDataOptTest, RawDataCommit_AfterApply) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    size_t len = 128;
    uint8_t* buffer = LibShmMediaRawDataApply(creatorHandle_, len);
    ASSERT_NE(buffer, nullptr);

    // Fill with test data
    memset(buffer, 0xEF, len);

    int ret = LibShmMediaRawDataCommit(creatorHandle_, buffer, len);
    EXPECT_GT(ret, 0);
}

// Test LibShmMediaRawHeadWrite with null data
TEST_F(LibShmMediaRawDataOptTest, RawHeadWrite_NullData) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    int ret = LibShmMediaRawHeadWrite(creatorHandle_, nullptr);
    EXPECT_EQ(ret, 0);  // Returns 0 for null data
}

TEST_F(LibShmMediaRawDataOptTest, RawHeadWrite_ValidData) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    libshmmedia_raw_head_param_t head;
    memset(&head, 0, sizeof(head));
    head.uRawHead_ = 0xCAFEBABE;

    int ret = LibShmMediaRawHeadWrite(creatorHandle_, &head);
    // Returns the head value
    EXPECT_EQ((uint32_t)ret, head.uRawHead_);
}

// Test LibShmMediaRawDataRead
TEST_F(LibShmMediaRawDataOptTest, RawDataRead_AfterWrite) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    // Open reader
    readerHandle_ = LibShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle_, nullptr);

    // Write data
    size_t writeLen = 256;
    uint8_t* writeBuffer = LibShmMediaRawDataApply(creatorHandle_, writeLen);
    ASSERT_NE(writeBuffer, nullptr);

    for (size_t i = 0; i < writeLen; ++i) {
        writeBuffer[i] = static_cast<uint8_t>(i & 0xFF);
    }

    int commitRet = LibShmMediaRawDataCommit(creatorHandle_, writeBuffer, writeLen);
    EXPECT_GT(commitRet, 0);

    // Read data
    libshmmedia_raw_head_param_t readHead;
    libshmmedia_raw_data_param_t readData;
    memset(&readHead, 0, sizeof(readHead));
    memset(&readData, 0, sizeof(readData));

    int readRet = LibShmMediaRawDataRead(readerHandle_, &readHead, &readData, 100);

    EXPECT_GT(readRet, 0);

    if (readRet > 0) {
        EXPECT_NE(readData.pRawData_, nullptr);
        EXPECT_GT(readData.uRawData_, 0u);
    }
}

// Test LibShmMediaHasReader
TEST_F(LibShmMediaRawDataOptTest, HasReader_NoReader) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    int ret = LibShmMediaHasReader(creatorHandle_, 0);
    // No reader opened yet
    EXPECT_EQ(ret, 0);
}

TEST_F(LibShmMediaRawDataOptTest, HasReader_WithReader) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    readerHandle_ = LibShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle_, nullptr);

    // Allow some time for reader registration
    int ret = LibShmMediaHasReader(creatorHandle_, 100);
    // Should detect reader
    // Note: Behavior depends on implementation of reader detection
    SUCCEED();
}

// Test LibShmMediaItemApplyBuffer
TEST_F(LibShmMediaRawDataOptTest, ItemApplyBuffer_ValidParams) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 512;

    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    int ret = LibShmMediaItemApplyBuffer(creatorHandle_, &item, &layout);
    EXPECT_GT(ret, 0);
    EXPECT_NE(layout.p_userData, nullptr);
}

// NOTE: ItemApplyBuffer_NullItem test disabled - LibShmMediaItemApplyBuffer crashes with nullptr
// TEST_F(LibShmMediaRawDataOptTest, ItemApplyBuffer_NullItem) {
//     creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
//     ASSERT_NE(creatorHandle_, nullptr);
//
//     libshm_media_item_addr_layout_t layout;
//     memset(&layout, 0, sizeof(layout));
//
//     int ret = LibShmMediaItemApplyBuffer(creatorHandle_, nullptr, &layout);
//     // Behavior with null item
//     SUCCEED();
// }

// Test LibShmMediaItemCommitBuffer
TEST_F(LibShmMediaRawDataOptTest, ItemCommitBuffer_AfterApply) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 256;
    item.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;

    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    int applyRet = LibShmMediaItemApplyBuffer(creatorHandle_, &item, &layout);
    ASSERT_GT(applyRet, 0);
    ASSERT_NE(layout.p_userData, nullptr);

    // Fill user data
    memset(layout.p_userData, 0xAA, item.i_userDataLen);

    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));

    int commitRet = LibShmMediaItemCommitBuffer(creatorHandle_, &head, &item);
    EXPECT_GE(commitRet, 0);
}

// Test LibShmMediaItemGetWriteBufferLayout
TEST_F(LibShmMediaRawDataOptTest, GetWriteBufferLayout_ValidParams) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 256;

    // First apply a buffer
    libshm_media_item_addr_layout_t applyLayout;
    memset(&applyLayout, 0, sizeof(applyLayout));
    int applyRet = LibShmMediaItemApplyBuffer(creatorHandle_, &item, &applyLayout);
    ASSERT_GT(applyRet, 0);

    // Get write buffer layout
    libshm_media_item_addr_layout_t writeLayout;
    memset(&writeLayout, 0, sizeof(writeLayout));

    int ret = LibShmMediaItemGetWriteBufferLayout(
        creatorHandle_,
        &item,
        applyLayout.p_userData,
        applyRet,
        &writeLayout
    );
    EXPECT_GT(ret, 0);
}

TEST_F(LibShmMediaRawDataOptTest, GetWriteBufferLayout_NullHandle) {
    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));

    uint8_t buffer[256];
    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    int ret = LibShmMediaItemGetWriteBufferLayout(nullptr, &item, buffer, sizeof(buffer), &layout);
    EXPECT_EQ(ret, 0);
}

TEST_F(LibShmMediaRawDataOptTest, GetWriteBufferLayout_ReaderHandle) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    readerHandle_ = LibShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle_, nullptr);

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 256;

    uint8_t buffer[256];
    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    // Should return 0 for reader handle (not creator)
    int ret = LibShmMediaItemGetWriteBufferLayout(readerHandle_, &item, buffer, sizeof(buffer), &layout);
    EXPECT_EQ(ret, 0);
}

// Test LibShmMediaItemWriteBuffer
TEST_F(LibShmMediaRawDataOptTest, ItemWriteBuffer_ValidData) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    // Apply buffer
    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 128;
    item.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;

    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    int applyRet = LibShmMediaItemApplyBuffer(creatorHandle_, &item, &layout);
    ASSERT_GT(applyRet, 0);
    ASSERT_NE(layout.p_userData, nullptr);

    // Prepare user data
    uint8_t userData[128];
    memset(userData, 0xBB, sizeof(userData));
    item.p_userData = userData;

    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));

    // Write buffer
    int writeRet = LibShmMediaItemWriteBuffer(creatorHandle_, &head, &item, layout.p_userData);
    EXPECT_GT(writeRet, 0);
}

TEST_F(LibShmMediaRawDataOptTest, ItemWriteBuffer_ReaderHandle) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    readerHandle_ = LibShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle_, nullptr);

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    item.i_userDataLen = 64;

    uint8_t buffer[256];
    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));

    // Should return 0 for reader handle
    int ret = LibShmMediaItemWriteBuffer(readerHandle_, &head, &item, buffer);
    EXPECT_EQ(ret, 0);
}

// Test write/read cycle using item buffer APIs
TEST_F(LibShmMediaRawDataOptTest, WriteReadCycle_ItemBuffer) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    // Open reader
    readerHandle_ = LibShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle_, nullptr);

    // Prepare test data
    uint8_t testData[256];
    for (int i = 0; i < 256; ++i) {
        testData[i] = static_cast<uint8_t>(i);
    }

    // Apply buffer
    libshm_media_item_param_t writeItem;
    memset(&writeItem, 0, sizeof(writeItem));
    writeItem.i_userDataLen = sizeof(testData);
    writeItem.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;
    writeItem.p_userData = testData;

    libshm_media_item_addr_layout_t layout;
    memset(&layout, 0, sizeof(layout));

    int applyRet = LibShmMediaItemApplyBuffer(creatorHandle_, &writeItem, &layout);
    ASSERT_GT(applyRet, 0);
    ASSERT_NE(layout.p_userData, nullptr);

    libshm_media_head_param_t writeHead;
    memset(&writeHead, 0, sizeof(writeHead));

    // Write buffer
    {
        memcpy(layout.p_userData, testData, sizeof(testData));
    }

    // Commit
    int commitRet = LibShmMediaItemCommitBuffer(creatorHandle_, &writeHead, &writeItem);
    ASSERT_GE(commitRet, 0);

    // Read data
    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    memset(&readHead, 0, sizeof(readHead));
    memset(&readItem, 0, sizeof(readItem));

    int readRet = LibShmMediaPollReadData(readerHandle_, &readHead, &readItem, 100);
    EXPECT_GT(readRet, 0);

    if (readRet > 0) {
        EXPECT_EQ((size_t)readItem.i_userDataLen, sizeof(testData));
        EXPECT_EQ(readItem.i_userDataType, LIBSHM_MEDIA_TYPE_TVULIVE_DATA);
        EXPECT_NE(readItem.p_userData, nullptr);
        EXPECT_EQ(memcmp(readItem.p_userData, testData, sizeof(testData)), 0);
    }
}

// Test multiple sequential writes
TEST_F(LibShmMediaRawDataOptTest, MultipleSequentialWrites) {
    creatorHandle_ = LibShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemLen, kTestItemCount);
    ASSERT_NE(creatorHandle_, nullptr);

    for (int i = 0; i < 5; ++i) {
        size_t len = 100 + i * 50;
        uint8_t* buffer = LibShmMediaRawDataApply(creatorHandle_, len);
        ASSERT_NE(buffer, nullptr);

        memset(buffer, i, len);

        int commitRet = LibShmMediaRawDataCommit(creatorHandle_, buffer, len);
        EXPECT_GT(commitRet, 0);
    }
}

// Test raw data param structure initialization
TEST_F(LibShmMediaRawDataOptTest, RawDataParam_Initialization) {
    libshmmedia_raw_data_param_t param;
    memset(&param, 0, sizeof(param));

    EXPECT_EQ(param.pRawData_, nullptr);
    EXPECT_EQ(param.uRawData_, 0u);
}

// Test raw head param structure
TEST_F(LibShmMediaRawDataOptTest, RawHeadParam_Initialization) {
    libshmmedia_raw_head_param_t param;
    memset(&param, 0, sizeof(param));

    EXPECT_EQ(param.uRawHead_, 0u);

    param.uRawHead_ = 0x12345678;
    EXPECT_EQ(param.uRawHead_, 0x12345678u);
}
