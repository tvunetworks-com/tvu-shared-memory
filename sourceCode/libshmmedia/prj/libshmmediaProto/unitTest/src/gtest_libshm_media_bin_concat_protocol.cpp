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
 * @file gtest_libshm_media_bin_concat_protocol.cpp
 * @brief Unit tests for libshm_media_bin_concat_protocol.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "libshm_media_bin_concat_protocol.h"

class LibShmMediaBinConcatProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        handle_ = nullptr;
    }

    void TearDown() override {
        if (handle_) {
            LibshmmediaBinConcatProtoDestroy(handle_);
            handle_ = nullptr;
        }
    }

    libshmmedia_bin_concat_proto_handle_t handle_;
};

// Test LibshmmediaBinConcatProtoCreate
TEST_F(LibShmMediaBinConcatProtocolTest, Create_ReturnsValidHandle) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    EXPECT_NE(handle_, nullptr);
}

// Test LibshmmediaBinConcatProtoDestroy
TEST_F(LibShmMediaBinConcatProtocolTest, Destroy_NullHandle) {
    // Should not crash
    LibshmmediaBinConcatProtoDestroy(nullptr);
    SUCCEED();
}

TEST_F(LibShmMediaBinConcatProtocolTest, Destroy_ValidHandle) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    LibshmmediaBinConcatProtoDestroy(handle_);
    handle_ = nullptr;  // Prevent double destroy
    SUCCEED();
}

// Test LibshmmediaBinConcatProtoReset
TEST_F(LibShmMediaBinConcatProtocolTest, Reset_ValidHandle) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    bool ret = LibshmmediaBinConcatProtoReset(handle_);
    EXPECT_TRUE(ret);
}

TEST_F(LibShmMediaBinConcatProtocolTest, Reset_NullHandle) {
    bool ret = LibshmmediaBinConcatProtoReset(nullptr);
    EXPECT_TRUE(ret);  // Returns true even for null
}

// Test LibshmmediaBinConcatProtoConcatSegment
TEST_F(LibShmMediaBinConcatProtocolTest, ConcatSegment_NullSegment) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    bool ret = LibshmmediaBinConcatProtoConcatSegment(handle_, nullptr);
    EXPECT_FALSE(ret);
}

TEST_F(LibShmMediaBinConcatProtocolTest, ConcatSegment_EmptySegment) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = nullptr;
    seg.nSeg = 0;

    bool ret = LibshmmediaBinConcatProtoConcatSegment(handle_, &seg);
    EXPECT_FALSE(ret);
}

TEST_F(LibShmMediaBinConcatProtocolTest, ConcatSegment_ValidSegment) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = data;
    seg.nSeg = sizeof(data);

    bool ret = LibshmmediaBinConcatProtoConcatSegment(handle_, &seg);
    EXPECT_TRUE(ret);
}

// Test LibshmmediaBinConcatProtoFlushBinary
TEST_F(LibShmMediaBinConcatProtocolTest, FlushBinary_AfterConcat) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = data;
    seg.nSeg = sizeof(data);

    bool concatRet = LibshmmediaBinConcatProtoConcatSegment(handle_, &seg);
    ASSERT_TRUE(concatRet);

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;

    bool flushRet = LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin);
    EXPECT_TRUE(flushRet);
    EXPECT_NE(pBin, nullptr);
    EXPECT_GT(nBin, 0u);
}

// Test concat multiple segments and flush
TEST_F(LibShmMediaBinConcatProtocolTest, ConcatMultipleSegments) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data1[] = {0x01, 0x02, 0x03};
    uint8_t data2[] = {0x04, 0x05, 0x06, 0x07};
    uint8_t data3[] = {0x08, 0x09};

    libshmmedia_bin_concat_proto_seg_t seg1 = {sizeof(data1), data1};
    libshmmedia_bin_concat_proto_seg_t seg2 = {sizeof(data2), data2};
    libshmmedia_bin_concat_proto_seg_t seg3 = {sizeof(data3), data3};

    EXPECT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg1));
    EXPECT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg2));
    EXPECT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg3));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;

    bool flushRet = LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin);
    EXPECT_TRUE(flushRet);
    EXPECT_NE(pBin, nullptr);
    EXPECT_GT(nBin, sizeof(data1) + sizeof(data2) + sizeof(data3));
}

// Test LibshmmediaBinConcatProtoSplitBinary
TEST_F(LibShmMediaBinConcatProtocolTest, SplitBinary_SingleSegment) {
    // First create and concat
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0xCA, 0xFE, 0xBA, 0xBE};
    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = data;
    seg.nSeg = sizeof(data);

    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Now split with a new handle
    libshmmedia_bin_concat_proto_handle_t splitHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(splitHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool splitRet = LibshmmediaBinConcatProtoSplitBinary(splitHandle, pBin, nBin, true, &pSegs, &nSegs);
    EXPECT_TRUE(splitRet);
    EXPECT_NE(pSegs, nullptr);
    EXPECT_EQ(nSegs, 1u);

    if (nSegs > 0 && pSegs) {
        EXPECT_EQ(pSegs[0].nSeg, sizeof(data));
        EXPECT_EQ(memcmp(pSegs[0].pSeg, data, sizeof(data)), 0);
    }

    LibshmmediaBinConcatProtoDestroy(splitHandle);
}

TEST_F(LibShmMediaBinConcatProtocolTest, SplitBinary_MultipleSegments) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data1[] = {0x11, 0x22};
    uint8_t data2[] = {0x33, 0x44, 0x55};
    uint8_t data3[] = {0x66};

    libshmmedia_bin_concat_proto_seg_t seg1 = {sizeof(data1), data1};
    libshmmedia_bin_concat_proto_seg_t seg2 = {sizeof(data2), data2};
    libshmmedia_bin_concat_proto_seg_t seg3 = {sizeof(data3), data3};

    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg1));
    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg2));
    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg3));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Split
    libshmmedia_bin_concat_proto_handle_t splitHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(splitHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool splitRet = LibshmmediaBinConcatProtoSplitBinary(splitHandle, pBin, nBin, true, &pSegs, &nSegs);
    EXPECT_TRUE(splitRet);
    EXPECT_EQ(nSegs, 3u);

    if (nSegs >= 3 && pSegs) {
        EXPECT_EQ(pSegs[0].nSeg, sizeof(data1));
        EXPECT_EQ(memcmp(pSegs[0].pSeg, data1, sizeof(data1)), 0);

        EXPECT_EQ(pSegs[1].nSeg, sizeof(data2));
        EXPECT_EQ(memcmp(pSegs[1].pSeg, data2, sizeof(data2)), 0);

        EXPECT_EQ(pSegs[2].nSeg, sizeof(data3));
        EXPECT_EQ(memcmp(pSegs[2].pSeg, data3, sizeof(data3)), 0);
    }

    LibshmmediaBinConcatProtoDestroy(splitHandle);
}

// Test LibshmmediaBinConcatProtoParseBinary (same as SplitBinary with bCreateBuffer=false)
TEST_F(LibShmMediaBinConcatProtocolTest, ParseBinary_SingleSegment) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0xAB, 0xCD, 0xEF};
    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = data;
    seg.nSeg = sizeof(data);

    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Copy to ensure we have mutable buffer for Parse
    std::vector<uint8_t> buffer(pBin, pBin + nBin);

    libshmmedia_bin_concat_proto_handle_t parseHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(parseHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool parseRet = LibshmmediaBinConcatProtoParseBinary(parseHandle, buffer.data(), buffer.size(), &pSegs, &nSegs);
    EXPECT_TRUE(parseRet);
    EXPECT_EQ(nSegs, 1u);

    LibshmmediaBinConcatProtoDestroy(parseHandle);
}

// Test reset and reuse
TEST_F(LibShmMediaBinConcatProtocolTest, ResetAndReuse) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    // First use
    uint8_t data1[] = {0x01, 0x02};
    libshmmedia_bin_concat_proto_seg_t seg1 = {sizeof(data1), data1};
    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg1));

    const uint8_t* pBin1 = nullptr;
    uint32_t nBin1 = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin1, &nBin1));

    // Reset
    EXPECT_TRUE(LibshmmediaBinConcatProtoReset(handle_));

    // Second use
    uint8_t data2[] = {0x03, 0x04, 0x05};
    libshmmedia_bin_concat_proto_seg_t seg2 = {sizeof(data2), data2};
    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg2));

    const uint8_t* pBin2 = nullptr;
    uint32_t nBin2 = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin2, &nBin2));

    // Verify they are different
    EXPECT_NE(nBin1, nBin2);
}

// Test large segment
TEST_F(LibShmMediaBinConcatProtocolTest, LargeSegment) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    // Create 64KB segment
    std::vector<uint8_t> largeData(64 * 1024);
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i & 0xFF);
    }

    libshmmedia_bin_concat_proto_seg_t seg;
    seg.pSeg = largeData.data();
    seg.nSeg = largeData.size();

    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Split and verify
    libshmmedia_bin_concat_proto_handle_t splitHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(splitHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool splitRet = LibshmmediaBinConcatProtoSplitBinary(splitHandle, pBin, nBin, true, &pSegs, &nSegs);
    EXPECT_TRUE(splitRet);
    EXPECT_EQ(nSegs, 1u);

    if (nSegs > 0 && pSegs) {
        EXPECT_EQ(pSegs[0].nSeg, largeData.size());
        EXPECT_EQ(memcmp(pSegs[0].pSeg, largeData.data(), largeData.size()), 0);
    }

    LibshmmediaBinConcatProtoDestroy(splitHandle);
}

// Test many segments
TEST_F(LibShmMediaBinConcatProtocolTest, ManySegments) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    const int numSegments = 100;
    std::vector<std::vector<uint8_t>> allData(numSegments);

    for (int i = 0; i < numSegments; ++i) {
        allData[i].resize(i + 1);
        for (size_t j = 0; j < allData[i].size(); ++j) {
            allData[i][j] = static_cast<uint8_t>((i + j) & 0xFF);
        }

        libshmmedia_bin_concat_proto_seg_t seg;
        seg.pSeg = allData[i].data();
        seg.nSeg = allData[i].size();

        ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));
    }

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Split
    libshmmedia_bin_concat_proto_handle_t splitHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(splitHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool splitRet = LibshmmediaBinConcatProtoSplitBinary(splitHandle, pBin, nBin, true, &pSegs, &nSegs);
    EXPECT_TRUE(splitRet);
    EXPECT_EQ(nSegs, (uint32_t)numSegments);

    // Verify some segments
    if (pSegs && nSegs == (uint32_t)numSegments) {
        EXPECT_EQ(pSegs[0].nSeg, 1u);
        EXPECT_EQ(pSegs[49].nSeg, 50u);
        EXPECT_EQ(pSegs[99].nSeg, 100u);
    }

    LibshmmediaBinConcatProtoDestroy(splitHandle);
}

// Test segment structure
TEST_F(LibShmMediaBinConcatProtocolTest, SegmentStructure) {
    libshmmedia_bin_concat_proto_seg_t seg;
    memset(&seg, 0, sizeof(seg));

    EXPECT_EQ(seg.nSeg, 0u);
    EXPECT_EQ(seg.pSeg, nullptr);

    uint8_t data[] = {0x01, 0x02, 0x03};
    seg.nSeg = sizeof(data);
    seg.pSeg = data;

    EXPECT_EQ(seg.nSeg, 3u);
    EXPECT_EQ(seg.pSeg, data);
}

// Test split without creating buffer
TEST_F(LibShmMediaBinConcatProtocolTest, SplitBinary_WithoutCreatingBuffer) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0xAA, 0xBB, 0xCC};
    libshmmedia_bin_concat_proto_seg_t seg = {sizeof(data), data};

    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));

    const uint8_t* pBin = nullptr;
    uint32_t nBin = 0;
    ASSERT_TRUE(LibshmmediaBinConcatProtoFlushBinary(handle_, &pBin, &nBin));

    // Split without creating buffer (bCreateBuffer = false)
    libshmmedia_bin_concat_proto_handle_t splitHandle = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(splitHandle, nullptr);

    libshmmedia_bin_concat_proto_seg_t* pSegs = nullptr;
    uint32_t nSegs = 0;

    bool splitRet = LibshmmediaBinConcatProtoSplitBinary(splitHandle, pBin, nBin, false, &pSegs, &nSegs);
    EXPECT_TRUE(splitRet);
    EXPECT_EQ(nSegs, 1u);

    LibshmmediaBinConcatProtoDestroy(splitHandle);
}

// Test flush with null output pointers
TEST_F(LibShmMediaBinConcatProtocolTest, FlushBinary_NullOutputPointers) {
    handle_ = LibshmmediaBinConcatProtoCreate();
    ASSERT_NE(handle_, nullptr);

    uint8_t data[] = {0x01};
    libshmmedia_bin_concat_proto_seg_t seg = {sizeof(data), data};
    ASSERT_TRUE(LibshmmediaBinConcatProtoConcatSegment(handle_, &seg));

    // Pass null pointers - should still succeed
    bool ret = LibshmmediaBinConcatProtoFlushBinary(handle_, nullptr, nullptr);
    EXPECT_TRUE(ret);
}
