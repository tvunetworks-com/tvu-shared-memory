// Unit tests for buffer_ctrl using Google Test
#include <gtest/gtest.h>
#include "buffer_controller.h"
#include <errno.h>

using namespace tvushm;

TEST(BufferCtrl, CompactEncodeDecodeU32) {
    BufferController_t o;
    BufferCtrlInit(&o);

    uint32_t in = 0x123456;
    int enc = BufferCtrlCompactEncodeValueU32(&o, in);
    ASSERT_GT(enc, 0);

    BufferController_t r;
    BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

    uint32_t out = 0;
    int dec = BufferCtrlCompactDecodeValueU32(&r, out);
    EXPECT_GT(dec, 0);
    EXPECT_EQ(out, in);
}

TEST(BufferCtrl, CompactEncodeDecodeU64) {
    BufferController_t o;
    BufferCtrlInit(&o);

    uint64_t in = 0x1122334455667788ULL;
    int enc = BufferCtrlCompactEncodeValueU64(&o, in);
    ASSERT_GT(enc, 0);

    BufferController_t r;
    BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

    uint64_t out = 0;
    int dec = BufferCtrlCompactDecodeValueU64(&r, out);
    EXPECT_GT(dec, 0);
    EXPECT_EQ(out, in);
}

TEST(BufferCtrl, WriteLe32ReadLe32) {
    BufferController_t o;
    BufferCtrlInit(&o);

    uint32_t in = 0xAABBCCDD;
    int w = BufferCtrlWLe32(&o, in);
    ASSERT_EQ(w, 4);

    BufferController_t r;
    BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

    uint32_t out = BufferCtrlRLe32(&r);
    EXPECT_EQ(out, in);
}

TEST(BufferCtrl, W8_Push_Pop_Seek_Reset) {
    BufferController_t o;
    BufferCtrlInit(&o);

    // write bytes
    BufferCtrlW8(&o, 0x11);
    BufferCtrlW8(&o, 0x22);
    BufferCtrlW8(&o, 0x33);

    // pop via external reader
    BufferController_t r;
    BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

    uint8_t out[3] = {0};
    int popped = BufferCtrlPopData(&r, out, 3);
    EXPECT_EQ(popped, 3);
    EXPECT_EQ(out[0], 0x11);
    EXPECT_EQ(out[1], 0x22);
    EXPECT_EQ(out[2], 0x33);

    // test seek/reset/tell
    BufferCtrlRewind(&r);
    EXPECT_EQ(BufferCtrlTellCurPos(&r), 0);
    BufferCtrlSeek(&r, 1, SEEK_SET);
    EXPECT_EQ(BufferCtrlTellCurPos(&r), 1);
    BufferCtrlReset(&r);
    EXPECT_EQ(BufferCtrlTellCurPos(&r), 0);
}

TEST(BufferCtrl, BE_LE_16_32_64) {
    // 16-bit
    {
        BufferController_t o; BufferCtrlInit(&o);
        BufferCtrlWBe16(&o, 0x1234);
        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
        uint32_t v = BufferCtrlRBe16(&r);
        EXPECT_EQ(v, 0x1234);
    }
    // 32-bit
    {
        BufferController_t o; BufferCtrlInit(&o);
        BufferCtrlWLe32(&o, 0x89ABCDEF);
        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
        uint32_t v = BufferCtrlRLe32(&r);
        EXPECT_EQ(v, 0x89ABCDEF);
    }
    // 64-bit
    {
        BufferController_t o; BufferCtrlInit(&o);
        BufferCtrlWBe64(&o, 0x0102030405060708ULL);
        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
        uint64_t v = BufferCtrlRBe64(&r);
        EXPECT_EQ(v, 0x0102030405060708ULL);
    }
}

TEST(BufferCtrl, LeBe128_ReadWrite) {
    // LE 128
    {
        BufferController_t o; BufferCtrlInit(&o);
        uint128_t vin; memset(&vin, 0, sizeof(vin));
        vin.namedQwords.lowQword = 0x1111111122222222ULL;
        vin.namedQwords.highQword = 0x3333333344444444ULL;
        BufferCtrlWriteRawDataLeU128(&o, vin);

        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

        uint128_t vout; memset(&vout, 0, sizeof(vout));
        int read = BufferCtrlReadRawDataLeU128(&r, vout);
        EXPECT_EQ(read, 16);
        EXPECT_EQ(vout.namedQwords.lowQword, vin.namedQwords.lowQword);
        EXPECT_EQ(vout.namedQwords.highQword, vin.namedQwords.highQword);
    }

    // BE 128
    {
        BufferController_t o; BufferCtrlInit(&o);
        uint128_t vin; memset(&vin, 0, sizeof(vin));
        vin.namedQwords.lowQword = 0xAAAABBBBCCCCDDDDULL;
        vin.namedQwords.highQword = 0x1111222233334444ULL;
        BufferCtrlWriteRawDataBeU128(&o, vin);

        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

        uint128_t vout; memset(&vout, 0, sizeof(vout));
        int read = BufferCtrlReadRawDataBeU128(&r, vout);
        EXPECT_EQ(read, 16);
        EXPECT_EQ(vout.namedQwords.lowQword, vin.namedQwords.lowQword);
        EXPECT_EQ(vout.namedQwords.highQword, vin.namedQwords.highQword);
    }
}

TEST(BufferCtrl, CompactGetBytesCountU32_Boundaries) {
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32(0x7F), 1);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32(0x80), 2);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32((1<<14)-1), 2);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32(1<<14), 3);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32((1<<21)-1), 3);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32(1<<21), 4);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32((1<<28)-1), 4);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU32(1<<28), 5);
}

TEST(BufferCtrl, CompactEncodeDecodeU32_Boundaries) {
    uint32_t values[] = {0, 0x7F, 0x80, 0x3FFF, 0x4000, 0x1FFFFF, 0x200000, 0x0FFFFFFF, 0x10000000};
    for (uint32_t v : values) {
        BufferController_t o; BufferCtrlInit(&o);
        int enc = BufferCtrlCompactEncodeValueU32(&o, v);
        ASSERT_GT(enc, 0);

        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
        uint32_t out = 0;
        int dec = BufferCtrlCompactDecodeValueU32(&r, out);
        EXPECT_GT(dec, 0);
        EXPECT_EQ(out, v);
    }
}

TEST(BufferCtrl, CompactGetBytesCountU64_Boundaries) {
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(0x7F), 1);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(0x80), 2);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64((1ULL<<14)-1), 2);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(1ULL<<14), 3);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64((1ULL<<21)-1), 3);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(1ULL<<21), 4);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64((1ULL<<28)-1), 4);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(1ULL<<28), 5);
    EXPECT_EQ(BufferCtrlCompactGetBytesCountU64(1ULL<<35), 6);
}

TEST(BufferCtrl, CompactEncodeDecodeU64_Boundaries) {
    uint64_t values[] = {0ULL, 0x7FULL, 0x80ULL, (1ULL<<14)-1, (1ULL<<14), (1ULL<<21)-1, (1ULL<<21), (1ULL<<35)};
    for (uint64_t v : values) {
        BufferController_t o; BufferCtrlInit(&o);
        int enc = BufferCtrlCompactEncodeValueU64(&o, v);
        ASSERT_GT(enc, 0);

        BufferController_t r; BufferCtrlInit(&r);
        BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
        uint64_t out = 0;
        int dec = BufferCtrlCompactDecodeValueU64(&r, out);
        EXPECT_GT(dec, 0);
        EXPECT_EQ(out, v);
    }
}

TEST(BufferCtrl, ExternalBufferRejectsPush) {
    BufferController_t o; BufferCtrlInit(&o);
    uint8_t data[] = {1,2,3,4};
    BufferCtrlPushData(&o, data, sizeof(data));

    BufferController_t r; BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));

    // attempts to push into an external buffer should return a negative error
    int ret = BufferCtrlPushData(&r, data, sizeof(data));
    EXPECT_LT(ret, 0);
}

TEST(BufferCtrl, MemMoveAndExtendAlloc) {
    BufferController_t o; BufferCtrlInit(&o);
    uint8_t seq[] = {10,20,30,40,50};
    BufferCtrlPushData(&o, seq, sizeof(seq));
    // move first 4 bytes one position to the right
    int moved = BufferCtrlMemMove(&o, 1, 0, 4);
    EXPECT_EQ(moved, 4);

    BufferController_t r; BufferCtrlInit(&r);
    BufferCtrlAttachExternalReadBuffer(&r, BufferCtrlGetOrigPtr(&o), BufferCtrlGetBufLen(&o));
    uint8_t out[6] = {0};
    int popped = BufferCtrlPopData(&r, out, 5);
    EXPECT_EQ(popped, 5);
    // expected layout: original first byte remains at 0, then copied: {10,10,20,30,40}
    EXPECT_EQ(out[0], 10);
    EXPECT_EQ(out[1], 10);
    EXPECT_EQ(out[2], 20);
    EXPECT_EQ(out[3], 30);
    EXPECT_EQ(out[4], 40);

    // extend size should succeed (non-negative)
    BufferCtrlInit(&o);
    int ext = BufferCtrlExtendSize(&o, 1024);
    EXPECT_GE(ext, 0);
    EXPECT_GE(BufferCtrlGetAllocSize(&o), 1024);
}

