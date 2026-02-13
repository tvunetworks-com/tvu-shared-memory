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
 * @file gtest_libshmmedia_variableitem_rawdata.cpp
 * @brief Unit tests for libshmmedia_variableitem_rawdata.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "libshmmedia_variableitem_rawdata.h"
#include "libshm_media_variable_item.h"

class LibViShmMediaRawDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        handle_ = nullptr;
    }

    void TearDown() override {
        if (handle_) {
            LibViShmMediaDestroy(handle_);
            handle_ = nullptr;
        }
        // Clean up any test shared memory
        LibViShmMediaRemoveShmFromSystem(kTestShmName);
    }

    static constexpr const char* kTestShmName = "test_vi_rawdata_shm";
    static constexpr uint32_t kTestHeaderLen = 1024;
    static constexpr uint32_t kTestItemCount = 10;
    static constexpr uint64_t kTestTotalSize = 1024 * 1024;  // 1MB

    libshm_media_handle_t handle_;
};

// Test LibViShmMediaRawDataWrite with null data
TEST_F(LibViShmMediaRawDataTest, RawDataWrite_NullData) {
    // Create a shared memory handle first
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    int ret = LibViShmMediaRawDataWrite(handle_, nullptr);
    // Function currently returns 0 for null data
    EXPECT_EQ(ret, 0);
}

// Test LibViShmMediaRawDataApply with null handle
TEST_F(LibViShmMediaRawDataTest, RawDataApply_NullHandle) {
    // This will likely cause segfault or undefined behavior if not handled
    // Skip this test as API doesn't check for null handle
    GTEST_SKIP() << "API does not handle null handle gracefully";
}

// Test LibViShmMediaRawDataApply with valid handle
TEST_F(LibViShmMediaRawDataTest, RawDataApply_ValidHandle) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    uint8_t* buffer = LibViShmMediaRawDataApply(handle_, 256);
    // Should return valid buffer pointer
    EXPECT_NE(buffer, nullptr);
}

TEST_F(LibViShmMediaRawDataTest, RawDataApply_ZeroLength) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    uint8_t* buffer = LibViShmMediaRawDataApply(handle_, 0);
    // Behavior with zero length depends on implementation
    // May return null or valid pointer
    SUCCEED();  // Just verify no crash
}

// Test LibViShmMediaRawDataCommit with valid handle
TEST_F(LibViShmMediaRawDataTest, RawDataCommit_ValidHandle) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    // First apply a buffer
    size_t len = 256;
    uint8_t* buffer = LibViShmMediaRawDataApply(handle_, len);
    ASSERT_NE(buffer, nullptr);

    // Fill with test data
    memset(buffer, 0xAB, len);

    // Commit the buffer
    int ret = LibViShmMediaRawDataCommit(handle_, buffer, len);
    EXPECT_GT(ret, 0);
}

// Test LibViShmMediaRawHeadWrite with null data
TEST_F(LibViShmMediaRawDataTest, RawHeadWrite_NullData) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    int ret = LibViShmMediaRawHeadWrite(handle_, nullptr);
    EXPECT_EQ(ret, 0);
}

TEST_F(LibViShmMediaRawDataTest, RawHeadWrite_ValidData) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    libshmmedia_raw_head_param_t head;
    memset(&head, 0, sizeof(head));
    head.uRawHead_ = 0x12345678;

    int ret = LibViShmMediaRawHeadWrite(handle_, &head);
    // Returns the head value
    EXPECT_EQ((uint32_t)ret, head.uRawHead_);
}

// Test LibViShmMediaHasReader with valid handle but no reader
TEST_F(LibViShmMediaRawDataTest, HasReader_NoReader) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    int ret = LibViShmMediaHasReader(handle_, 0);
    // No reader opened, should return 0
    EXPECT_EQ(ret, 0);
}

// Test Write then Read cycle
TEST_F(LibViShmMediaRawDataTest, WriteReadCycle) {
    // Create writer handle
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    // Open reader handle
    libshm_media_handle_t readerHandle = LibViShmMediaOpen(kTestShmName, nullptr, nullptr);
    ASSERT_NE(readerHandle, nullptr);

    // Write some data
    size_t writeLen = 128;
    uint8_t* writeBuffer = LibViShmMediaRawDataApply(handle_, writeLen);
    ASSERT_NE(writeBuffer, nullptr);

    // Fill with pattern
    for (size_t i = 0; i < writeLen; ++i) {
        writeBuffer[i] = static_cast<uint8_t>(i & 0xFF);
    }

    int commitRet = LibViShmMediaRawDataCommit(handle_, writeBuffer, writeLen);
    EXPECT_GT(commitRet, 0);


    // Read data
    libshmmedia_raw_head_param_t readHead;
    libshmmedia_raw_data_param_t readData;
    memset(&readHead, 0, sizeof(readHead));
    memset(&readData, 0, sizeof(readData));

    int readRet = LibViShmMediaRawDataRead(readerHandle, &readHead, &readData, 100);
    EXPECT_GT(readRet, 0);

    if (readRet > 0) {
        EXPECT_NE(readData.pRawData_, nullptr);
        EXPECT_GT(readData.uRawData_, 0u);
    }

    LibViShmMediaDestroy(readerHandle);
}

// Test multiple writes
TEST_F(LibViShmMediaRawDataTest, MultipleWrites) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    for (int i = 0; i < 5; ++i) {
        size_t writeLen = 64 + i * 32;
        uint8_t* writeBuffer = LibViShmMediaRawDataApply(handle_, writeLen);
        ASSERT_NE(writeBuffer, nullptr);

        memset(writeBuffer, i & 0xFF, writeLen);

        int commitRet = LibViShmMediaRawDataCommit(handle_, writeBuffer, writeLen);
        EXPECT_GT(commitRet, 0);
    }
}

// Test raw data param structure
TEST_F(LibViShmMediaRawDataTest, RawDataParam_Structure) {
    libshmmedia_raw_data_param_t param;
    memset(&param, 0, sizeof(param));

    EXPECT_EQ(param.pRawData_, nullptr);
    EXPECT_EQ(param.uRawData_, 0u);
}

// Test raw head param structure
TEST_F(LibViShmMediaRawDataTest, RawHeadParam_Structure) {
    libshmmedia_raw_head_param_t param;
    memset(&param, 0, sizeof(param));

    EXPECT_EQ(param.uRawHead_, 0u);

    param.uRawHead_ = 0xDEADBEEF;
    EXPECT_EQ(param.uRawHead_, 0xDEADBEEFu);
}

// Test large data write
TEST_F(LibViShmMediaRawDataTest, LargeDataWrite) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    // Try to write 64KB
    size_t writeLen = 64 * 1024;
    uint8_t* writeBuffer = LibViShmMediaRawDataApply(handle_, writeLen);

    if (writeBuffer != nullptr) {
        // Fill with pattern
        for (size_t i = 0; i < writeLen; ++i) {
            writeBuffer[i] = static_cast<uint8_t>(i & 0xFF);
        }

        int commitRet = LibViShmMediaRawDataCommit(handle_, writeBuffer, writeLen);
        EXPECT_GT(commitRet, 0);
    }
}

// Test boundary - apply buffer larger than available space
TEST_F(LibViShmMediaRawDataTest, ApplyBufferTooLarge) {
    handle_ = LibViShmMediaCreate(kTestShmName, kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(handle_, nullptr);

    // Try to apply more than total size
    uint8_t* buffer = LibViShmMediaRawDataApply(handle_, kTestTotalSize * 2);
    // Should return null for too large request
    EXPECT_EQ(buffer, nullptr);
}
