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
 * @file gtest_libshm_media_item_info.cpp
 * @brief Unit tests for libshm_media_item_info.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "libshm_media_item_info.h"

class LibShmMediaItemInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        itemInfo_.Reset();
    }

    void TearDown() override {
    }

    tvushm::ItemInfo itemInfo_;
};

// Test default constructor and Reset
TEST_F(LibShmMediaItemInfoTest, DefaultConstructor_InitializesAllFields) {
    tvushm::ItemInfo info;
    EXPECT_EQ(info.readRet_, 0);
    EXPECT_EQ(info.itemIdex_, 0u);
    EXPECT_FALSE(info.bGotTvutimestamp_);
    EXPECT_EQ(info.tvutimestamp_, 0u);
    EXPECT_EQ(info.pts_, 0u);
}

TEST_F(LibShmMediaItemInfoTest, Reset_ClearsAllFields) {
    // Set some values
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 50;
    itemInfo_.bGotTvutimestamp_ = true;
    itemInfo_.tvutimestamp_ = 123456789;
    itemInfo_.pts_ = 987654321;

    // Reset
    itemInfo_.Reset();

    // Verify all cleared
    EXPECT_EQ(itemInfo_.readRet_, 0);
    EXPECT_EQ(itemInfo_.itemIdex_, 0u);
    EXPECT_FALSE(itemInfo_.bGotTvutimestamp_);
    EXPECT_EQ(itemInfo_.tvutimestamp_, 0u);
    EXPECT_EQ(itemInfo_.pts_, 0u);
}

// Test GetReadingRet
TEST_F(LibShmMediaItemInfoTest, GetReadingRet_ReturnsZeroAfterInit) {
    EXPECT_EQ(itemInfo_.GetReadingRet(), 0);
}

TEST_F(LibShmMediaItemInfoTest, GetReadingRet_ReturnsSetValue) {
    itemInfo_.readRet_ = 42;
    EXPECT_EQ(itemInfo_.GetReadingRet(), 42);
}

TEST_F(LibShmMediaItemInfoTest, GetReadingRet_ReturnsNegativeValue) {
    itemInfo_.readRet_ = -1;
    EXPECT_EQ(itemInfo_.GetReadingRet(), -1);
}

// Test IsReadingSuccess
TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_ReturnsFalseWhenReadRetIsZero) {
    itemInfo_.readRet_ = 0;
    itemInfo_.itemIdex_ = 5;
    EXPECT_FALSE(itemInfo_.IsReadingSuccess(5));
}

TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_ReturnsFalseWhenReadRetIsNegative) {
    itemInfo_.readRet_ = -1;
    itemInfo_.itemIdex_ = 5;
    EXPECT_FALSE(itemInfo_.IsReadingSuccess(5));
}

TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_ReturnsFalseWhenIndexMismatch) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    EXPECT_FALSE(itemInfo_.IsReadingSuccess(10));  // Different index
}

TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_ReturnsTrueWhenConditionsMet) {
    itemInfo_.readRet_ = 100;  // Positive
    itemInfo_.itemIdex_ = 5;
    EXPECT_TRUE(itemInfo_.IsReadingSuccess(5));  // Same index
}

// Test HasGottenTvutimestamp
TEST_F(LibShmMediaItemInfoTest, HasGottenTvutimestamp_ReturnsFalseWhenReadingFailed) {
    itemInfo_.readRet_ = 0;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = true;
    EXPECT_FALSE(itemInfo_.HasGottenTvutimestamp(5));
}

TEST_F(LibShmMediaItemInfoTest, HasGottenTvutimestamp_ReturnsFalseWhenNotGotten) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = false;
    EXPECT_FALSE(itemInfo_.HasGottenTvutimestamp(5));
}

TEST_F(LibShmMediaItemInfoTest, HasGottenTvutimestamp_ReturnsTrueWhenAllConditionsMet) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = true;
    EXPECT_TRUE(itemInfo_.HasGottenTvutimestamp(5));
}

TEST_F(LibShmMediaItemInfoTest, HasGottenTvutimestamp_ReturnsFalseWhenIndexMismatch) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = true;
    EXPECT_FALSE(itemInfo_.HasGottenTvutimestamp(10));  // Different index
}

// Test GetTvutimestamp
TEST_F(LibShmMediaItemInfoTest, GetTvutimestamp_ReturnsZeroAfterInit) {
    EXPECT_EQ(itemInfo_.GetTvutimestamp(), 0u);
}

TEST_F(LibShmMediaItemInfoTest, GetTvutimestamp_ReturnsSetValue) {
    itemInfo_.tvutimestamp_ = 123456789012345ULL;
    EXPECT_EQ(itemInfo_.GetTvutimestamp(), 123456789012345ULL);
}

TEST_F(LibShmMediaItemInfoTest, GetTvutimestamp_ReturnsMaxValue) {
    itemInfo_.tvutimestamp_ = UINT64_MAX;
    EXPECT_EQ(itemInfo_.GetTvutimestamp(), UINT64_MAX);
}

// Test GetSafeTvutimestamp
TEST_F(LibShmMediaItemInfoTest, GetSafeTvutimestamp_ReturnsMinusOneWhenNotGotten) {
    itemInfo_.readRet_ = 0;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.tvutimestamp_ = 12345;
    // Note: -1 cast to uint64_t is UINT64_MAX
    EXPECT_EQ(itemInfo_.GetSafeTvutimestamp(5), static_cast<uint64_t>(-1));
}

TEST_F(LibShmMediaItemInfoTest, GetSafeTvutimestamp_ReturnsValueWhenGotten) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = true;
    itemInfo_.tvutimestamp_ = 12345;
    EXPECT_EQ(itemInfo_.GetSafeTvutimestamp(5), 12345u);
}

TEST_F(LibShmMediaItemInfoTest, GetSafeTvutimestamp_ReturnsMinusOneWhenIndexMismatch) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    itemInfo_.bGotTvutimestamp_ = true;
    itemInfo_.tvutimestamp_ = 12345;
    EXPECT_EQ(itemInfo_.GetSafeTvutimestamp(10), static_cast<uint64_t>(-1));
}

// Test HasGottenPts
TEST_F(LibShmMediaItemInfoTest, HasGottenPts_SameAsIsReadingSuccess) {
    // HasGottenPts simply delegates to IsReadingSuccess
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 5;
    EXPECT_TRUE(itemInfo_.HasGottenPts(5));
    EXPECT_FALSE(itemInfo_.HasGottenPts(10));

    itemInfo_.readRet_ = 0;
    EXPECT_FALSE(itemInfo_.HasGottenPts(5));
}

// Test GetPts
TEST_F(LibShmMediaItemInfoTest, GetPts_ReturnsZeroAfterInit) {
    EXPECT_EQ(itemInfo_.GetPts(), 0u);
}

TEST_F(LibShmMediaItemInfoTest, GetPts_ReturnsSetValue) {
    itemInfo_.pts_ = 90000;
    EXPECT_EQ(itemInfo_.GetPts(), 90000u);
}

TEST_F(LibShmMediaItemInfoTest, GetPts_ReturnsMaxValue) {
    itemInfo_.pts_ = UINT64_MAX;
    EXPECT_EQ(itemInfo_.GetPts(), UINT64_MAX);
}

// Test Init and Release (they both call Reset internally)
TEST_F(LibShmMediaItemInfoTest, Init_ResetsAllFields) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 50;
    itemInfo_.bGotTvutimestamp_ = true;
    itemInfo_.tvutimestamp_ = 123456789;

    itemInfo_.Init();

    EXPECT_EQ(itemInfo_.readRet_, 0);
    EXPECT_EQ(itemInfo_.itemIdex_, 0u);
    EXPECT_FALSE(itemInfo_.bGotTvutimestamp_);
    EXPECT_EQ(itemInfo_.tvutimestamp_, 0u);
}

TEST_F(LibShmMediaItemInfoTest, Release_ResetsAllFields) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 50;
    itemInfo_.bGotTvutimestamp_ = true;
    itemInfo_.tvutimestamp_ = 123456789;

    itemInfo_.Release();

    EXPECT_EQ(itemInfo_.readRet_, 0);
    EXPECT_EQ(itemInfo_.itemIdex_, 0u);
    EXPECT_FALSE(itemInfo_.bGotTvutimestamp_);
    EXPECT_EQ(itemInfo_.tvutimestamp_, 0u);
}

// Test HookInit and HookRelease static methods
TEST_F(LibShmMediaItemInfoTest, HookInit_CallsInit) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 50;

    tvushm::ItemInfo::HookInit(itemInfo_);

    EXPECT_EQ(itemInfo_.readRet_, 0);
    EXPECT_EQ(itemInfo_.itemIdex_, 0u);
}

TEST_F(LibShmMediaItemInfoTest, HookRelease_CallsRelease) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 50;

    tvushm::ItemInfo::HookRelease(itemInfo_);

    EXPECT_EQ(itemInfo_.readRet_, 0);
    EXPECT_EQ(itemInfo_.itemIdex_, 0u);
}

// Test edge cases with zero index
TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_WorksWithZeroIndex) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 0;
    EXPECT_TRUE(itemInfo_.IsReadingSuccess(0));
}

TEST_F(LibShmMediaItemInfoTest, HasGottenTvutimestamp_WorksWithZeroIndex) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = 0;
    itemInfo_.bGotTvutimestamp_ = true;
    EXPECT_TRUE(itemInfo_.HasGottenTvutimestamp(0));
}

// Test with maximum index value
TEST_F(LibShmMediaItemInfoTest, IsReadingSuccess_WorksWithMaxIndex) {
    itemInfo_.readRet_ = 100;
    itemInfo_.itemIdex_ = UINT32_MAX;
    EXPECT_TRUE(itemInfo_.IsReadingSuccess(UINT32_MAX));
    EXPECT_FALSE(itemInfo_.IsReadingSuccess(0));
}

// Test destructor behavior (implicit through scope)
TEST_F(LibShmMediaItemInfoTest, Destructor_NoLeak) {
    // Create and destroy in scope - should not leak
    {
        tvushm::ItemInfo tempInfo;
        tempInfo.readRet_ = 100;
        tempInfo.tvutimestamp_ = 12345;
    }
    // No explicit check - just verify no crash/leak via sanitizers
    SUCCEED();
}

// Test shared_ptr usage
TEST_F(LibShmMediaItemInfoTest, SharedPtr_Works) {
    tvushm::ItemInfoSharePtr ptr = std::make_shared<tvushm::ItemInfo>();
    ptr->readRet_ = 42;
    ptr->itemIdex_ = 10;

    EXPECT_EQ(ptr->GetReadingRet(), 42);
    EXPECT_TRUE(ptr->IsReadingSuccess(10));
}

// Test multiple instances
TEST_F(LibShmMediaItemInfoTest, MultipleInstances_Independent) {
    tvushm::ItemInfo info1, info2;

    info1.readRet_ = 100;
    info1.itemIdex_ = 5;
    info1.tvutimestamp_ = 1000;

    info2.readRet_ = 200;
    info2.itemIdex_ = 10;
    info2.tvutimestamp_ = 2000;

    EXPECT_EQ(info1.GetReadingRet(), 100);
    EXPECT_EQ(info2.GetReadingRet(), 200);
    EXPECT_TRUE(info1.IsReadingSuccess(5));
    EXPECT_TRUE(info2.IsReadingSuccess(10));
    EXPECT_FALSE(info1.IsReadingSuccess(10));
    EXPECT_FALSE(info2.IsReadingSuccess(5));
}
