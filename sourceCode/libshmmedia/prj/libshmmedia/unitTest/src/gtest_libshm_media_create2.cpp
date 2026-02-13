#include <gtest/gtest.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

extern "C" {
#include "libshm_media.h"
}

static std::string make_create2_shm_name(const char *suffix)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "/gtest_create2_%s_%d", suffix, (int)getpid());
    return std::string(buf);
}

// Test 1: Create with owner-only permissions, verify handle and basic queries
TEST(LibShmMediaCreate2, Create2_DefaultMode)
{
    std::string name = make_create2_shm_name("defmode");
    mode_t mode = S_IRUSR | S_IWUSR;

    libshm_media_handle_t h = LibShmMediaCreate2(name.c_str(), 1024, 4, 4096, mode);
    ASSERT_NE(h, (libshm_media_handle_t)NULL);

    // Verify creator flag
    EXPECT_EQ(LibShmMediaIsCreator(h), 1);

    // Verify name
    const char *nm = LibShmMediaGetName(h);
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STREQ(nm, name.c_str());

    // Verify version is non-zero
    uint32_t ver = LibShmMediaGetVersion(h);
    EXPECT_GT(ver, 0u);

    // Verify item counts and length
    unsigned int counts = LibShmMediaGetItemCounts(h);
    EXPECT_GE(counts, 4u);

    unsigned int itemLen = LibShmMediaGetItemLength(h);
    EXPECT_GE(itemLen, 4096u);

    LibShmMediaDestroy(h);

#if defined(TVU_LINUX)
    LibShmMediaRemoveShmidFromSystem(name.c_str());
#endif
}

// Test 2: Create with group read/write permissions, verify works
TEST(LibShmMediaCreate2, Create2_GroupReadWriteMode)
{
    std::string name = make_create2_shm_name("grpmode");
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    libshm_media_handle_t h = LibShmMediaCreate2(name.c_str(), 1024, 8, 2048, mode);
    ASSERT_NE(h, (libshm_media_handle_t)NULL);

    EXPECT_EQ(LibShmMediaIsCreator(h), 1);

    const char *nm = LibShmMediaGetName(h);
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STREQ(nm, name.c_str());

    unsigned int counts = LibShmMediaGetItemCounts(h);
    EXPECT_GE(counts, 8u);

    unsigned int itemLen = LibShmMediaGetItemLength(h);
    EXPECT_GE(itemLen, 2048u);

    LibShmMediaDestroy(h);

#if defined(TVU_LINUX)
    LibShmMediaRemoveShmidFromSystem(name.c_str());
#endif
}

// Test 3: Create with Create2, then open with LibShmMediaOpen, verify reader works
TEST(LibShmMediaCreate2, Create2_ThenOpen)
{
    std::string name = make_create2_shm_name("thenopen");
    mode_t mode = S_IRUSR | S_IWUSR;

    // Create as writer
    libshm_media_handle_t hWriter = LibShmMediaCreate2(name.c_str(), 1024, 4, 4096, mode);
    ASSERT_NE(hWriter, (libshm_media_handle_t)NULL);

    // Send some data so the reader has something
    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));
    head.i_dstw = 1920;
    head.i_dsth = 1080;
    head.u_videofourcc = 0x31637661; // avc1
    head.i_duration = 1;
    head.i_scale = 30;

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    uint8_t vdata[64];
    memset(vdata, 0xAB, sizeof(vdata));
    item.p_vData = vdata;
    item.i_vLen = sizeof(vdata);
    item.i64_vpts = 1000;

    int sendRet = LibShmMediaSendData(hWriter, &head, &item);
    EXPECT_GT(sendRet, 0);

    // Open as reader
    libshm_media_handle_t hReader = LibShmMediaOpen(name.c_str(), NULL, NULL);
    ASSERT_NE(hReader, (libshm_media_handle_t)NULL);

    // Reader should not be creator
    EXPECT_EQ(LibShmMediaIsCreator(hReader), 0);

    // Reader should see same name and counts
    const char *rName = LibShmMediaGetName(hReader);
    ASSERT_NE(rName, (const char *)NULL);
    EXPECT_STREQ(rName, name.c_str());

    unsigned int rCounts = LibShmMediaGetItemCounts(hReader);
    EXPECT_GE(rCounts, 4u);

    // Seek reader to start and read the data back
    LibShmMediaSeekReadIndexToRingStart(hReader);
    libshm_media_head_param_t readHead;
    libshm_media_item_param_t readItem;
    memset(&readHead, 0, sizeof(readHead));
    memset(&readItem, 0, sizeof(readItem));

    int readRet = LibShmMediaReadData(hReader, &readHead, &readItem);
    EXPECT_GT(readRet, 0);

    if (readRet > 0) {
        EXPECT_EQ(readHead.i_dstw, 1920);
        EXPECT_EQ(readHead.i_dsth, 1080);
        EXPECT_EQ(readItem.i64_vpts, 1000);
    }

    LibShmMediaDestroy(hReader);
    LibShmMediaDestroy(hWriter);

#if defined(TVU_LINUX)
    LibShmMediaRemoveShmidFromSystem(name.c_str());
#endif
}

// Test 4: Verify Create2 with default mode produces equivalent results to LibShmMediaCreate
TEST(LibShmMediaCreate2, Create2_CompareWithCreate)
{
    std::string name1 = make_create2_shm_name("cmp_orig");
    std::string name2 = make_create2_shm_name("cmp_new");

    uint32_t headerLen = 1024;
    uint32_t itemCount = 4;
    uint32_t itemLength = 4096;
    mode_t defaultMode = S_IRUSR | S_IWUSR;

    // Create via original API
    libshm_media_handle_t h1 = LibShmMediaCreate(name1.c_str(), headerLen, itemCount, itemLength);
    ASSERT_NE(h1, (libshm_media_handle_t)NULL);

    // Create via Create2 with equivalent default mode
    libshm_media_handle_t h2 = LibShmMediaCreate2(name2.c_str(), headerLen, itemCount, itemLength, defaultMode);
    ASSERT_NE(h2, (libshm_media_handle_t)NULL);

    // Both should be creators
    EXPECT_EQ(LibShmMediaIsCreator(h1), 1);
    EXPECT_EQ(LibShmMediaIsCreator(h2), 1);

    // Same version
    EXPECT_EQ(LibShmMediaGetVersion(h1), LibShmMediaGetVersion(h2));

    // Same item counts
    EXPECT_EQ(LibShmMediaGetItemCounts(h1), LibShmMediaGetItemCounts(h2));

    // Same item length
    EXPECT_EQ(LibShmMediaGetItemLength(h1), LibShmMediaGetItemLength(h2));

    // Same head length
    EXPECT_EQ(LibShmMediaGeHeadLength(h1), LibShmMediaGeHeadLength(h2));

    // Same item offset
    EXPECT_EQ(LibShmMediaGetItemOffset(h1), LibShmMediaGetItemOffset(h2));

    // Both should have valid write index (starting at same value)
    EXPECT_EQ(LibShmMediaGetWriteIndex(h1), LibShmMediaGetWriteIndex(h2));

    // Verify both can send/receive data equivalently
    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));
    head.i_dstw = 640;
    head.i_dsth = 480;
    head.i_duration = 1;
    head.i_scale = 25;

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));
    uint8_t vdata[32];
    memset(vdata, 0xCD, sizeof(vdata));
    item.p_vData = vdata;
    item.i_vLen = sizeof(vdata);
    item.i64_vpts = 500;

    int ret1 = LibShmMediaSendData(h1, &head, &item);
    int ret2 = LibShmMediaSendData(h2, &head, &item);
    EXPECT_GT(ret1, 0);
    EXPECT_GT(ret2, 0);

    // After one send, write indices should be equal
    EXPECT_EQ(LibShmMediaGetWriteIndex(h1), LibShmMediaGetWriteIndex(h2));

    LibShmMediaDestroy(h1);
    LibShmMediaDestroy(h2);

#if defined(TVU_LINUX)
    LibShmMediaRemoveShmidFromSystem(name1.c_str());
    LibShmMediaRemoveShmidFromSystem(name2.c_str());
#endif
}
