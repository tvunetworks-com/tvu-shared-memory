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
#include <string>
#include <cstring>
#include <unistd.h>

class LibShmMediaTest : public ::testing::Test {
protected:
    void SetUp() override {
        shmname = "test_shm_" + std::to_string(getpid());
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

// Test LibShmMediaCreate
TEST_F(LibShmMediaTest, CreateSharedMemory) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr) << "Failed to create shared memory";
}

// Test LibShmMediaCreate with invalid parameters
TEST_F(LibShmMediaTest, CreateSharedMemoryInvalidParams) {
    h_write = LibShmMediaCreate(nullptr, 4096, 32, 10240);
    EXPECT_EQ(h_write, nullptr) << "Should fail with null name";
}

// Test LibShmMediaOpen
TEST_F(LibShmMediaTest, OpenSharedMemory) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr) << "Failed to open shared memory";
}

// Test LibShmMediaOpen without create
TEST_F(LibShmMediaTest, OpenNonExistentSharedMemory) {
    h_read = LibShmMediaOpen("non_existent_shm_test", nullptr, nullptr);
    EXPECT_EQ(h_read, nullptr) << "Should fail to open non-existent shared memory";
}

// Test LibShmMediaGetVersion
TEST_F(LibShmMediaTest, GetVersion) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    uint32_t version = LibShmMediaGetVersion(h_write);
    EXPECT_GT(version, 0u) << "Version should be greater than 0";
}

// Test LibShmMediaGetHeadVersion
TEST_F(LibShmMediaTest, GetHeadVersion) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    uint32_t headVersion = LibShmMediaGetHeadVersion(h_write);
    EXPECT_GT(headVersion, 0u) << "Head version should be greater than 0";
}

// Test LibShmMediaGetWriteIndex
TEST_F(LibShmMediaTest, GetWriteIndex) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    unsigned int writeIndex = LibShmMediaGetWriteIndex(h_write);
    EXPECT_GE(writeIndex, 0u) << "Write index should be valid";
}

// Test LibShmMediaGetReadIndex
TEST_F(LibShmMediaTest, GetReadIndex) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    unsigned int readIndex = LibShmMediaGetReadIndex(h_read);
    EXPECT_GE(readIndex, 0u) << "Read index should be valid";
}

// Test LibShmMediaSeekReadIndexToWriteIndex
TEST_F(LibShmMediaTest, SeekReadIndexToWriteIndex) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    unsigned int writeIndex = LibShmMediaGetWriteIndex(h_write);
    unsigned int newReadIndex = LibShmMediaSeekReadIndexToWriteIndex(h_read);

    EXPECT_EQ(newReadIndex, writeIndex) << "Read index should match write index after seek";
}

// Test LibShmMediaSeekReadIndex
TEST_F(LibShmMediaTest, SeekReadIndex) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    h_read = LibShmMediaOpen(shmname.c_str(), nullptr, nullptr);
    ASSERT_NE(h_read, nullptr);

    unsigned int targetIndex = 5;
    unsigned int resultIndex = LibShmMediaSeekReadIndex(h_read, targetIndex);

    EXPECT_EQ(resultIndex, targetIndex) << "Read index should be set to target";
}

// Test LibShmMediaGetItemDataAddr
TEST_F(LibShmMediaTest, GetItemDataAddr) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    uint8_t* dataAddr = LibShmMediaGetItemDataAddr(h_write, 0);
    EXPECT_NE(dataAddr, nullptr) << "Item data address should not be null";
}

// Test LibShmMediaGetItemDataAddr with invalid index
TEST_F(LibShmMediaTest, GetItemDataAddrInvalidIndex) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    uint8_t* dataAddr = LibShmMediaGetItemDataAddr(h_write, 999);
    EXPECT_EQ(dataAddr, nullptr) << "Should return null for invalid index";
}

// Test LibShmMediaGeHeadAddr
TEST_F(LibShmMediaTest, GetHeadAddr) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    const uint8_t* headAddr = LibShmMediaGeHeadAddr(h_write);
    EXPECT_NE(headAddr, nullptr) << "Head address should not be null";
}

// Test LibShmMediaDestroy
TEST_F(LibShmMediaTest, DestroySharedMemory) {
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    ASSERT_NE(h_write, nullptr);

    LibShmMediaDestroy(h_write);
    h_write = nullptr;

    // Verify can't access after destroy by trying to create again
    h_write = LibShmMediaCreate(shmname.c_str(), 4096, 32, 10240);
    EXPECT_NE(h_write, nullptr) << "Should be able to recreate after destroy";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
