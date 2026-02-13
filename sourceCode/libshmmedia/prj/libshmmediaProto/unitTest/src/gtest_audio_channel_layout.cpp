// Unit tests for Audio Channel Layout protocol
#include <gtest/gtest.h>
#include "libshm_media_audio_track_channel_protocol.h"
#include <string.h>

TEST(AudioChannelLayout, SerializeParseRoundTrip) {
    libshmmedia_audio_channel_layout_object_t *h = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h, nullptr);

    uint16_t channels[] = {0, 1, 2, 3, 4};
    bool ok = LibshmmediaAudioChannelLayoutSerializeToBinary(h, channels, 5, true);
    EXPECT_TRUE(ok);

    // check channel number and planar
    uint16_t nchan = LibshmmediaAudioChannelLayoutGetChannelNum(h);
    EXPECT_EQ(nchan, (uint16_t)5);
    EXPECT_TRUE(LibshmmediaAudioChannelLayoutIsPlanar(h));

    const uint16_t *pArr = nullptr; uint16_t arrLen = 0;
    bool bgot = LibshmmediaAudioChannelLayoutGetChannelArr(h, &pArr, &arrLen);
    EXPECT_TRUE(bgot);
    EXPECT_EQ(arrLen, (uint16_t)5);
    if (bgot) {
        for (uint16_t i = 0; i < arrLen; ++i) {
            EXPECT_EQ(pArr[i], channels[i]);
        }
    }

    const uint8_t *pbin = nullptr; uint32_t nbin = 0;
    bool bbin = LibshmmediaAudioChannelLayoutGetBinaryAddr(h, &pbin, &nbin);
    EXPECT_TRUE(bbin);
    EXPECT_GT(nbin, 0u);

    // parse into new object
    libshmmedia_audio_channel_layout_object_t *h2 = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h2, nullptr);
    bool parsed = LibshmmediaAudioChannelLayoutParseFromBinary(h2, pbin, nbin);
    EXPECT_TRUE(parsed);

    // compare
    int cmp = LibshmmediaAudioChannelLayoutCompare(h, h2);
    EXPECT_EQ(cmp, 0);

    // copy
    libshmmedia_audio_channel_layout_object_t *h3 = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h3, nullptr);
    int copyRet = LibshmmediaAudioChannelLayoutCopy(h3, h2);
    EXPECT_GE(copyRet, 0);
    EXPECT_EQ(LibshmmediaAudioChannelLayoutCompare(h2, h3), 0);

    LibshmmediaAudioChannelLayoutDestroy(h);
    LibshmmediaAudioChannelLayoutDestroy(h2);
    LibshmmediaAudioChannelLayoutDestroy(h3);
}

TEST(AudioChannelLayout, ParseFailsOnBadData) {
    libshmmedia_audio_channel_layout_object_t *h = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h, nullptr);

    // invalid binary
    uint8_t bad[4] = {0xFF, 0xEE, 0xDD, 0xCC};
    bool parsed = LibshmmediaAudioChannelLayoutParseFromBinary(h, bad, sizeof(bad));
    EXPECT_FALSE(parsed);

    LibshmmediaAudioChannelLayoutDestroy(h);
}

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
