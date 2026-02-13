// Unit test for libshm_media_key_value_proto
#include <gtest/gtest.h>
#include "libshm_media_key_value_proto_internal.h"
#include "libshm_media_audio_track_channel_protocol.h"
#include <string.h>

using namespace tvushm;

TEST(MediaKeyValueProto, AppendAndExtractChannelLayout) {
    // create a channel layout
    libshmmedia_audio_channel_layout_object_t *h = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h, nullptr);

    // prepare channel array
    uint16_t channels[] = {1, 2, 3, 4};
    bool ser = LibshmmediaAudioChannelLayoutSerializeToBinary(h, channels, 4, false);
    ASSERT_TRUE(ser);

    // append to buffer via keyValueProtoAppendToBuffer
    BufferController_t buf; BufferCtrlInit(&buf);
    int appended = keyValueProtoAppendToBuffer(buf, h);
    EXPECT_GT(appended, 0);

    // extract from buffer
    const uint8_t *pout = BufferCtrlGetOrigPtr(&buf);
    uint32_t nout = BufferCtrlGetBufLen(&buf);

    libshmmedia_audio_channel_layout_object_t *h2 = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h2, nullptr);

    int extracted = keyValueProtoExtractFromBuffer(h2, pout, nout);
    EXPECT_GT(extracted, 0);

    // compare using provided compare function
    int cmp = LibshmmediaAudioChannelLayoutCompare(h, h2);
    EXPECT_EQ(cmp, 0);

    LibshmmediaAudioChannelLayoutDestroy(h);
    LibshmmediaAudioChannelLayoutDestroy(h2);
}

TEST(MediaKeyValueProto, AppendReturnsErrorOnNullChannel) {
    BufferController_t buf; BufferCtrlInit(&buf);
    int appended = keyValueProtoAppendToBuffer(buf, nullptr);
    EXPECT_LT(appended, 0);
}

TEST(MediaKeyValueProto, ExtractReturnsZeroOnInvalidBuffer) {
    libshmmedia_audio_channel_layout_object_t *h = LibshmmediaAudioChannelLayoutCreate();
    ASSERT_NE(h, nullptr);

    // null data
    int ret = keyValueProtoExtractFromBuffer(h, nullptr, 0);
    EXPECT_EQ(ret, 0);

    LibshmmediaAudioChannelLayoutDestroy(h);
}

