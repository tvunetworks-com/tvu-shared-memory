// Unit tests for KeyValParam
#include <gtest/gtest.h>
#include "libshm_key_value.h"
#include "buffer_controller.h"

using namespace tvushm;

TEST(KeyValParam, EmptyClearHasParameter) {
    KeyValParam kv;
    EXPECT_TRUE(kv.IsEmpty());
    EXPECT_FALSE(kv.HasParameter(1));
    kv.SetParamAsU8(1, 0x12);
    EXPECT_FALSE(kv.IsEmpty());
    EXPECT_TRUE(kv.HasParameter(1));
    kv.Clear();
    EXPECT_TRUE(kv.IsEmpty());
}

TEST(KeyValParam, AppendExtract_PrimitiveTypes) {
    KeyValParam kv;
    kv.SetParamAsU8(1, 0x10);
    kv.SetParamAsU16(2, 0x1234);
    kv.SetParamAsU32(3, 0x89ABCDEF);
    kv.SetParamAsU64(4, 0x1122334455667788ULL);

    BufferController_t buf; BufferCtrlInit(&buf);
    int enc = kv.AppendToBuffer(buf);
    ASSERT_GT(enc, 0);

    KeyValParam kv2;
    BufferCtrlRewind(&buf);
    int dec = kv2.ExtractFromBuffer(buf, false);
    ASSERT_GT(dec, 0);

    EXPECT_TRUE(kv2.HasParameter(1));
    EXPECT_EQ(kv2.GetParameter(1).GetAsUint32(), 0x10u);
    EXPECT_EQ(kv2.GetParameter(2).GetAsUint32(), 0x1234u);
    EXPECT_EQ(kv2.GetParameter(3).GetAsUint32(), 0x89ABCDEFu);
    EXPECT_EQ(kv2.GetParameter(4).GetAsUint64(), 0x1122334455667788ULL);
}

TEST(KeyValParam, AppendExtract_StringAndBytesAndUint128) {
    KeyValParam kv;
    kv.SetParamAsString(10, std::string("hello world"));

    const uint8_t raw[] = {1,2,3,4,5};
    kv.SetParamAsBytes(11, raw, sizeof(raw));

    tvushm::uint128_t v128; memset(&v128, 0, sizeof(v128));
    v128.namedQwords.lowQword = 0xAAAABBBBCCCCDDDDULL;
    v128.namedQwords.highQword = 0x1111222233334444ULL;
    kv.SetParamAsU128(12, v128);

    BufferController_t buf; BufferCtrlInit(&buf);
    int enc = kv.AppendToBuffer(buf);
    ASSERT_GT(enc, 0);

    KeyValParam kv2;
    BufferCtrlRewind(&buf);
    int dec = kv2.ExtractFromBuffer(buf, false);
    ASSERT_GT(dec, 0);

    EXPECT_TRUE(kv2.HasParameter(10));
    EXPECT_EQ(kv2.GetParameter(10).GetAsString(), std::string("hello world"));

    EXPECT_TRUE(kv2.HasParameter(11));
    const Bytes &b = kv2.GetParameter(11).GetAsBytes();
    EXPECT_EQ(b.GetBufLen(), (uint32_t)sizeof(raw));
    EXPECT_EQ(0, memcmp(b.GetBufAddr(), raw, sizeof(raw)));

    EXPECT_TRUE(kv2.HasParameter(12));
    const Uint128 &r128 = kv2.GetParameter(12).GetAsUint128();
    EXPECT_EQ(r128.namedQwords.lowQword, v128.namedQwords.lowQword);
    EXPECT_EQ(r128.namedQwords.highQword, v128.namedQwords.highQword);
}

TEST(KeyValParam, GetCompactBytesNumNeeded) {
    KeyValParam kv;
    kv.SetParamAsU32(1, 0x123456);
    kv.SetParamAsString(2, "abc");
    int need = kv.GetCompactBytesNumNeeded();
    EXPECT_GT(need, 0);
}

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
