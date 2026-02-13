#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "TvuShmSharedCompactRingBuffer.h"

using namespace tvushm;

static std::string make_shm_name()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/gtest_va_%d_%u", (int)getpid(), (unsigned)rand());
    return std::string(buf);
}

TEST(SharedCompactRingBuffer, CreateWriteReadDestroy)
{
    std::string name = make_shm_name();
    SharedCompactRingBuffer r;

    // create
    bool ok = r.Create(name.c_str(), 128, 4096, 8);
    ASSERT_TRUE(ok);
    EXPECT_TRUE(r.IsValid());
    EXPECT_TRUE(r.IsActive());
    EXPECT_GE(r.GetPayloadSize(), (uint64_t)4096);
    EXPECT_GE(r.GetMaxItemNum(), (uint64_t)8);

    // write one payload
    const char *msg = "hello-shm";
    size_t len = strlen(msg) + 1;
    bool w = r.Write(msg, len);
    EXPECT_TRUE(w);

    // read back
    size_t rlen = 0;
    void *pdata = r.Read(&rlen);
    ASSERT_NE(pdata, (void*)NULL);
    EXPECT_EQ(rlen, len);
    EXPECT_STREQ((const char*)pdata, msg);

    // destroy and close
    r.Close();
    r.Destroy();
}
