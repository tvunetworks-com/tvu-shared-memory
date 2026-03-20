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
#include "libshmmedia.h"
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/time.h>

static inline int64_t get_sys_ms64() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

class ReadWriteIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        shmname = "test_integration_" + std::to_string(getpid());
        h_write = nullptr;
        h_read = nullptr;
    }

    void TearDown() override {
        if (h_write) {
            LibShmMediaDestroy(h_write);
            h_write = nullptr;
        }
        if (h_read) {
            LibShmMediaDestroy(h_read);
            h_read = nullptr;
        }
    }

    std::string shmname;
    libshm_media_handle_t h_write;
    libshm_media_handle_t h_read;
};

// Test basic write and read
TEST_F(ReadWriteIntegrationTest, BasicWriteRead) {
    // Create writer
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    // Open reader
    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    // Prepare head parameters
    libshm_media_head_param_t headParam;
    LibShmMediaHeadParamInit(&headParam, sizeof(headParam));
    headParam.i_srcw = 1920;
    headParam.i_srch = 1080;
    headParam.i_dstw = 1920;
    headParam.i_dsth = 1080;
    headParam.u_videofourcc = 0x56595559; // UYVY
    headParam.i_duration = 1001;
    headParam.i_scale = 30000;

    // Update header
    int result = LibShmMediaUpdateHead(h_write, &headParam);
    EXPECT_GE(result, 0);

    // Prepare video data
    const int videoLen = 1024;
    std::vector<uint8_t> videoData(videoLen, 0xAA);

    libshm_media_item_param_t itemParam;
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));
    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.i64_vpts = 1000;
    itemParam.i64_vdts = 1000;
    itemParam.u_frameType = 1;

    // Write data
    result = LibShmMediaWriteData(h_write, &itemParam);
    EXPECT_GT(result, 0) << "Failed to write data";

    // Give reader time to detect data
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Read data
    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    LibShmMediaHeadParamInit(&readHead, sizeof(readHead));
    LibShmMediaItemParamInit(&readItem, sizeof(readItem));

    result = LibShmMediaPollReadData(h_read, &readHead, &readItem, 1000);
    EXPECT_GT(result, 0) << "Failed to read data";

    if (result > 0) {
        EXPECT_EQ(readItem.i_vLen, videoLen) << "Video length mismatch";
        EXPECT_EQ(readItem.i64_vpts, 1000) << "Video PTS mismatch";
        EXPECT_NE(readItem.p_vData, nullptr) << "Video data pointer should not be null";
    }

    LibShmMediaHeadParamRelease(&headParam);
    LibShmMediaItemParamRelease(&itemParam);
    LibShmMediaHeadParamRelease(&readHead);
    LibShmMediaItemParamRelease(&readItem);
}

// Test multiple frames write and read
TEST_F(ReadWriteIntegrationTest, MultipleFramesWriteRead) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    libshm_media_head_param_t headParam;
    LibShmMediaHeadParamInit(&headParam, sizeof(headParam));
    headParam.i_srcw = 1920;
    headParam.i_srch = 1080;
    headParam.u_videofourcc = 0x56595559;

    LibShmMediaUpdateHead(h_write, &headParam);

    const int numFrames = 10;
    const int videoLen = 512;
    std::vector<uint8_t> videoData(videoLen);

    // Write multiple frames
    for (int i = 0; i < numFrames; ++i) {
        std::fill(videoData.begin(), videoData.end(), static_cast<uint8_t>(i));

        libshm_media_item_param_t itemParam;
        LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));
        itemParam.p_vData = videoData.data();
        itemParam.i_vLen = videoLen;
        itemParam.i64_vpts = i * 1000;
        itemParam.i64_vdts = i * 1000;
        itemParam.u_frameType = (i % 10 == 0) ? 1 : 0; // I-frame every 10 frames

        int result = LibShmMediaWriteData(h_write, &itemParam);
        EXPECT_GT(result, 0) << "Failed to write frame " << i;

        LibShmMediaItemParamRelease(&itemParam);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Read frames
    int framesRead = 0;
    for (int i = 0; i < numFrames; ++i) {
        libshm_media_head_param_t readHead;
        libshm_media_item_param_t readItem;
        LibShmMediaHeadParamInit(&readHead, sizeof(readHead));
        LibShmMediaItemParamInit(&readItem, sizeof(readItem));

        int result = LibShmMediaPollReadData(h_read, &readHead, &readItem, 1000);

        if (result > 0) {
            framesRead++;
            EXPECT_EQ(readItem.i_vLen, videoLen) << "Video length mismatch for frame " << i;
        }

        LibShmMediaHeadParamRelease(&readHead);
        LibShmMediaItemParamRelease(&readItem);
    }

    EXPECT_GT(framesRead, 0) << "Should have read at least some frames";

    LibShmMediaHeadParamRelease(&headParam);
}

// Test audio and video together
TEST_F(ReadWriteIntegrationTest, AudioVideoWriteRead) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 20480);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    libshm_media_head_param_t headParam;
    LibShmMediaHeadParamInit(&headParam, sizeof(headParam));
    headParam.i_srcw = 1920;
    headParam.i_srch = 1080;
    headParam.u_videofourcc = 0x56595559;
    headParam.u_audiofourcc = 0x50434D20; // PCM
    headParam.i_channels = 2;
    headParam.i_depth = 16;
    headParam.i_samplerate = 48000;

    LibShmMediaUpdateHead(h_write, &headParam);

    const int videoLen = 1024;
    const int audioLen = 1920;
    std::vector<uint8_t> videoData(videoLen, 0xAA);
    std::vector<uint8_t> audioData(audioLen, 0xBB);

    libshm_media_item_param_t itemParam;
    LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));
    itemParam.p_vData = videoData.data();
    itemParam.i_vLen = videoLen;
    itemParam.i64_vpts = 1000;
    itemParam.i64_vdts = 1000;
    itemParam.p_aData = audioData.data();
    itemParam.i_aLen = audioLen;
    itemParam.i64_apts = 1000;
    itemParam.i64_adts = 1000;

    int result = LibShmMediaWriteData(h_write, &itemParam);
    EXPECT_GT(result, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    LibShmMediaHeadParamInit(&readHead, sizeof(readHead));
    LibShmMediaItemParamInit(&readItem, sizeof(readItem));

    result = LibShmMediaPollReadData(h_read, &readHead, &readItem, 1000);
    EXPECT_GT(result, 0);

    if (result > 0) {
        EXPECT_EQ(readItem.i_vLen, videoLen);
        EXPECT_EQ(readItem.i_aLen, audioLen);
        EXPECT_EQ(readItem.i64_vpts, 1000);
        EXPECT_EQ(readItem.i64_apts, 1000);
    }

    LibShmMediaHeadParamRelease(&headParam);
    LibShmMediaItemParamRelease(&itemParam);
    LibShmMediaHeadParamRelease(&readHead);
    LibShmMediaItemParamRelease(&readItem);
}

// Test reader seeking
TEST_F(ReadWriteIntegrationTest, ReaderSeeking) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    libshm_media_head_param_t headParam;
    LibShmMediaHeadParamInit(&headParam, sizeof(headParam));
    headParam.i_srcw = 1920;
    headParam.i_srch = 1080;
    headParam.u_videofourcc = 0x56595559;

    LibShmMediaUpdateHead(h_write, &headParam);

    // Write several frames
    const int numFrames = 5;
    const int videoLen = 512;
    std::vector<uint8_t> videoData(videoLen);

    for (int i = 0; i < numFrames; ++i) {
        libshm_media_item_param_t itemParam;
        LibShmMediaItemParamInit(&itemParam, sizeof(itemParam));
        itemParam.p_vData = videoData.data();
        itemParam.i_vLen = videoLen;
        itemParam.i64_vpts = i * 1000;

        LibShmMediaWriteData(h_write, &itemParam);
        LibShmMediaItemParamRelease(&itemParam);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Test seek to write index
    unsigned int writeIdx = LibShmMediaGetWriteIndex(h_write);
    unsigned int newReadIdx = LibShmMediaSeekReadIndexToWriteIndex(h_read);
    EXPECT_EQ(newReadIdx, writeIdx);

    // Test seek to specific index
    unsigned int targetIdx = 2;
    newReadIdx = LibShmMediaSeekReadIndex(h_read, targetIdx);
    EXPECT_EQ(newReadIdx, targetIdx);

    LibShmMediaHeadParamRelease(&headParam);
}

// Test timeout on empty shared memory
TEST_F(ReadWriteIntegrationTest, ReadTimeoutOnEmpty) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    LibShmMediaHeadParamInit(&readHead, sizeof(readHead));
    LibShmMediaItemParamInit(&readItem, sizeof(readItem));

    int64_t startTime = get_sys_ms64();
    int result = LibShmMediaPollReadData(h_read, &readHead, &readItem, 500);
    int64_t endTime = get_sys_ms64();

    EXPECT_EQ(result, 0) << "Should timeout with no data";
    EXPECT_GE(endTime - startTime, 400) << "Should wait for timeout period";

    LibShmMediaHeadParamRelease(&readHead);
    LibShmMediaItemParamRelease(&readItem);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
