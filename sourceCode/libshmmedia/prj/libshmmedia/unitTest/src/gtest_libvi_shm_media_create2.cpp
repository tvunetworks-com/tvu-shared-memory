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
 * @file gtest_libvi_shm_media_create2.cpp
 * @brief Unit tests for LibViShmMediaCreate2 (variable item layer)
 */

#include <gtest/gtest.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include "libshm_media_variable_item.h"

static std::string make_vi_create2_shm_name(const char *suffix)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "test_vi_c2_%s_%d", suffix, (int)getpid());
    return std::string(buf);
}

class LibViShmMediaCreate2Test : public ::testing::Test {
protected:
    void SetUp() override {
        creatorHandle_ = nullptr;
        readerHandle_ = nullptr;
    }

    void TearDown() override {
        if (readerHandle_) {
            LibViShmMediaDestroy(readerHandle_);
            readerHandle_ = nullptr;
        }
        if (creatorHandle_) {
            LibViShmMediaDestroy(creatorHandle_);
            creatorHandle_ = nullptr;
        }
        if (!shmName_.empty()) {
            LibViShmMediaRemoveShmFromSystem(shmName_.c_str());
        }
        if (!shmName2_.empty()) {
            LibViShmMediaRemoveShmFromSystem(shmName2_.c_str());
        }
    }

    static constexpr uint32_t kTestHeaderLen = 1024;
    static constexpr uint32_t kTestItemCount = 10;
    static constexpr uint64_t kTestTotalSize = 1024 * 1024;  // 1MB

    libshm_media_handle_t creatorHandle_;
    libshm_media_handle_t readerHandle_;
    std::string shmName_;
    std::string shmName2_;
};

constexpr uint32_t LibViShmMediaCreate2Test::kTestHeaderLen;
constexpr uint32_t LibViShmMediaCreate2Test::kTestItemCount;
constexpr uint64_t LibViShmMediaCreate2Test::kTestTotalSize;

// Test 1: Create with owner-only permissions, verify handle and basic queries
TEST_F(LibViShmMediaCreate2Test, ViCreate2_DefaultMode)
{
    shmName_ = make_vi_create2_shm_name("defmode");
    mode_t mode = S_IRUSR | S_IWUSR;

    creatorHandle_ = LibViShmMediaCreate2(shmName_.c_str(), kTestHeaderLen, kTestItemCount, kTestTotalSize, mode);
    ASSERT_NE(creatorHandle_, (libshm_media_handle_t)NULL);

    // Verify creator flag
    EXPECT_NE(LibViShmMediaIsCreator(creatorHandle_), 0);

    // Verify name
    const char *nm = LibViShmMediaGetName(creatorHandle_);
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STREQ(nm, shmName_.c_str());

    // Verify version is non-zero
    uint32_t ver = LibViShmMediaGetVersion(creatorHandle_);
    EXPECT_GT(ver, 0u);

    // Verify item counts
    unsigned int counts = LibViShmMediaGetItemCounts(creatorHandle_);
    EXPECT_EQ(counts, kTestItemCount);

    // Verify head length
    unsigned int headLen = LibViShmMediaGetHeadLen(creatorHandle_);
    EXPECT_EQ(headLen, kTestHeaderLen);
}

// Test 2: Create with group read/write permissions, verify works
TEST_F(LibViShmMediaCreate2Test, ViCreate2_GroupMode)
{
    shmName_ = make_vi_create2_shm_name("grpmode");
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    creatorHandle_ = LibViShmMediaCreate2(shmName_.c_str(), kTestHeaderLen, kTestItemCount, kTestTotalSize, mode);
    ASSERT_NE(creatorHandle_, (libshm_media_handle_t)NULL);

    // Verify creator flag
    EXPECT_NE(LibViShmMediaIsCreator(creatorHandle_), 0);

    // Verify name
    const char *nm = LibViShmMediaGetName(creatorHandle_);
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STREQ(nm, shmName_.c_str());

    // Verify item counts
    unsigned int counts = LibViShmMediaGetItemCounts(creatorHandle_);
    EXPECT_EQ(counts, kTestItemCount);

    // Verify head length
    unsigned int headLen = LibViShmMediaGetHeadLen(creatorHandle_);
    EXPECT_EQ(headLen, kTestHeaderLen);
}

// Test 3: Compare Create2 with original Create to verify equivalent behavior
TEST_F(LibViShmMediaCreate2Test, ViCreate2_CompareWithCreate)
{
    shmName_ = make_vi_create2_shm_name("cmp_orig");
    shmName2_ = make_vi_create2_shm_name("cmp_new");
    mode_t defaultMode = S_IRUSR | S_IWUSR;

    // Create via original API
    libshm_media_handle_t h1 = LibViShmMediaCreate(shmName_.c_str(), kTestHeaderLen, kTestItemCount, kTestTotalSize);
    ASSERT_NE(h1, (libshm_media_handle_t)NULL);

    // Create via Create2 with equivalent default mode
    libshm_media_handle_t h2 = LibViShmMediaCreate2(shmName2_.c_str(), kTestHeaderLen, kTestItemCount, kTestTotalSize, defaultMode);
    ASSERT_NE(h2, (libshm_media_handle_t)NULL);

    // Both should be creators
    EXPECT_NE(LibViShmMediaIsCreator(h1), 0);
    EXPECT_NE(LibViShmMediaIsCreator(h2), 0);

    // Same version
    EXPECT_EQ(LibViShmMediaGetVersion(h1), LibViShmMediaGetVersion(h2));

    // Same item counts
    EXPECT_EQ(LibViShmMediaGetItemCounts(h1), LibViShmMediaGetItemCounts(h2));

    // Same head length
    EXPECT_EQ(LibViShmMediaGetHeadLen(h1), LibViShmMediaGetHeadLen(h2));

    // Same total payload size
    EXPECT_EQ(LibViShmMediaGetTotalPayloadSize(h1), LibViShmMediaGetTotalPayloadSize(h2));

    // Same item offset
    EXPECT_EQ(LibViShmMediaGetItemOffset(h1), LibViShmMediaGetItemOffset(h2));

    // Both should have same initial write index
    EXPECT_EQ(LibViShmMediaGetWriteIndex(h1), LibViShmMediaGetWriteIndex(h2));

    // Verify both can send data equivalently
    libshm_media_head_param_t head;
    libshm_media_item_param_t item;
    memset(&head, 0, sizeof(head));
    memset(&item, 0, sizeof(item));

    uint8_t testData[64];
    memset(testData, 0xCD, sizeof(testData));
    item.p_userData = testData;
    item.i_userDataLen = sizeof(testData);
    item.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;

    int ret1 = LibViShmMediaSendData(h1, &head, &item);
    int ret2 = LibViShmMediaSendData(h2, &head, &item);
    EXPECT_GT(ret1, 0);
    EXPECT_GT(ret2, 0);

    // After one send, write indices should be equal
    EXPECT_EQ(LibViShmMediaGetWriteIndex(h1), LibViShmMediaGetWriteIndex(h2));

    // Store as member handles for TearDown cleanup
    creatorHandle_ = h1;
    readerHandle_ = h2;
}

// Test 4: Create with Create2, then open with LibViShmMediaOpen
TEST_F(LibViShmMediaCreate2Test, ViCreate2_ThenOpen)
{
    shmName_ = make_vi_create2_shm_name("thenopen");
    mode_t mode = S_IRUSR | S_IWUSR;

    // Create as writer
    creatorHandle_ = LibViShmMediaCreate2(shmName_.c_str(), kTestHeaderLen, kTestItemCount, kTestTotalSize, mode);
    ASSERT_NE(creatorHandle_, (libshm_media_handle_t)NULL);

    // Send some data so the reader has something
    libshm_media_head_param_t writeHead;
    libshm_media_item_param_t writeItem;
    memset(&writeHead, 0, sizeof(writeHead));
    memset(&writeItem, 0, sizeof(writeItem));

    uint8_t testData[128];
    for (int i = 0; i < 128; ++i) {
        testData[i] = static_cast<uint8_t>(i);
    }
    writeItem.p_userData = testData;
    writeItem.i_userDataLen = sizeof(testData);
    writeItem.i_userDataType = LIBSHM_MEDIA_TYPE_TVULIVE_DATA;

    int sendRet = LibViShmMediaSendData(creatorHandle_, &writeHead, &writeItem);
    ASSERT_GT(sendRet, 0);

    // Open as reader
    readerHandle_ = LibViShmMediaOpen(shmName_.c_str(), nullptr, nullptr);
    ASSERT_NE(readerHandle_, (libshm_media_handle_t)NULL);

    // Reader should not be creator
    EXPECT_EQ(LibViShmMediaIsCreator(readerHandle_), 0);

    // Reader should see same name and counts
    const char *rName = LibViShmMediaGetName(readerHandle_);
    ASSERT_NE(rName, (const char *)NULL);
    EXPECT_STREQ(rName, shmName_.c_str());

    unsigned int rCounts = LibViShmMediaGetItemCounts(readerHandle_);
    EXPECT_EQ(rCounts, kTestItemCount);

    // Seek reader to beginning since data was written before open
    LibViShmMediaSeekReadIndexToZero(readerHandle_);

    // Read the data back
    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    memset(&readHead, 0, sizeof(readHead));
    memset(&readItem, 0, sizeof(readItem));

    int readRet = LibViShmMediaPollReadData(readerHandle_, &readHead, &readItem, 100);
    EXPECT_GT(readRet, 0);

    if (readRet > 0) {
        EXPECT_EQ(readItem.i_userDataType, LIBSHM_MEDIA_TYPE_TVULIVE_DATA);
        EXPECT_EQ((size_t)readItem.i_userDataLen, sizeof(testData));
        EXPECT_NE(readItem.p_userData, nullptr);
    }
}
