// Unit tests for libshm_zlib_wrap
#include <gtest/gtest.h>
#include "libshm_zlib_wrap_internal.h"
#include <zlib.h>
#include <string.h>

using namespace tvushm;

TEST(ZlibWrap, EstimateCompressSize) {
    unsigned long n = 1024;
    unsigned long est = ZlibWrapEstimateCompressSize(n);
    EXPECT_GE(est, n);

    est = ZlibWrapEstimateCompressSize(0);
    EXPECT_GE(est, 0u);
}

TEST(ZlibWrap, CompressUncompressRoundTrip) {
    const char *text = "The quick brown fox jumps over the lazy dog. Repeat. Repeat. Repeat.";
    unsigned long srcLen = (unsigned long)strlen(text) + 1;

    unsigned long destCap = ZlibWrapEstimateCompressSize(srcLen);
    std::vector<uint8_t> dest(destCap);

    unsigned long destLen = destCap;
    int ret = ZlibWrapCompress(dest.data(), &destLen, (const uint8_t*)text, srcLen);
    EXPECT_EQ(ret, Z_OK);
    EXPECT_GT(destLen, 0u);

    std::vector<uint8_t> out(srcLen);
    unsigned long outLen = srcLen;
    ret = ZlibWrapUnCompress(out.data(), &outLen, dest.data(), destLen);
    EXPECT_EQ(ret, Z_OK);
    EXPECT_EQ(outLen, srcLen);
    EXPECT_EQ(0, memcmp(out.data(), text, srcLen));
}

TEST(ZlibWrap, BadArgs) {
    const char *text = "data";
    unsigned long srcLen = (unsigned long)strlen(text);

    unsigned long destCap = ZlibWrapEstimateCompressSize(srcLen);
    std::vector<uint8_t> dest(destCap);

    unsigned long zeroLen = 0;
    // destLen zero -> error
    int ret = ZlibWrapCompress(dest.data(), &zeroLen, (const uint8_t*)text, srcLen);
    EXPECT_EQ(ret, Z_ERRNO);

    // null dest -> error
    unsigned long dlen = destCap;
    ret = ZlibWrapCompress(NULL, &dlen, (const uint8_t*)text, srcLen);
    EXPECT_EQ(ret, Z_ERRNO);

    // null source -> error
    dlen = destCap;
    ret = ZlibWrapCompress(dest.data(), &dlen, NULL, srcLen);
    EXPECT_EQ(ret, Z_ERRNO);

    // uncompress: null destLen
    ret = ZlibWrapUnCompress(dest.data(), &zeroLen, dest.data(), dlen);
    EXPECT_EQ(ret, Z_ERRNO);
}

TEST(ZlibWrap, SmallDestBufferProducesBufError) {
    const char *text = "This is some test data that will be compressed.";
    unsigned long srcLen = (unsigned long)strlen(text) + 1;
    unsigned long destCap = ZlibWrapEstimateCompressSize(srcLen);

    // intentionally make dest smaller than compressBound
    unsigned long smallCap = destCap > 10 ? destCap/10 : destCap/2;
    std::vector<uint8_t> dest(smallCap);
    unsigned long destLen = smallCap;
    int ret = ZlibWrapCompress(dest.data(), &destLen, (const uint8_t*)text, srcLen);
    // expected not to be Z_OK; often Z_BUF_ERROR
    EXPECT_NE(ret, Z_OK);
}

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
