#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include <string.h>

#include "sharememory.h"

static std::string make_shm_name()
{
    char buf[128];
    snprintf(buf, sizeof(buf), "/gtest_shm_%d_%u", (int)getpid(), (unsigned)rand());
    return std::string(buf);
}

TEST(ShareMemoryBasic, CreateOpenClose)
{
    std::string name = make_shm_name();

    CTvuBaseShareMemory shm;
    uint8_t *hdr = shm.CreateOrOpen(name.c_str(), 1024, 4, 4096, nullptr);
    ASSERT_NE(hdr, (uint8_t *)NULL);

    const char *nm = shm.GetName();
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STRNE(nm, "");

    EXPECT_GE(shm.GetItemLength(), (uint32_t)4096);
    EXPECT_GE(shm.GetItemCounts(), (uint32_t)4);
    EXPECT_GT(shm.GetHeadLen(), (uint32_t)0);

    // check write/read indexes and flags (non-blocking checks)
    uint32_t widx = shm.GetWriteIndex();
    (void)widx; // just ensure call doesn't crash

    int sendable = shm.Sendable();
    EXPECT_GE(sendable, -1);

    int readable = shm.Readable(false);
    EXPECT_GE(readable, -1);

    // basic FinishWrite / FinishRead should not crash
    shm.FinishWrite();
    shm.FinishRead();

    // Close and remove
    shm.CloseMapFile();

#if defined(TVU_LINUX)
    CTvuBaseShareMemory::RemoveShmFromKernal(name.c_str());
#endif
}

#if !defined(GTEST_MAIN_ENTRANCE)
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    srand((unsigned)time(NULL) ^ getpid());
    return RUN_ALL_TESTS();
}
#endif
