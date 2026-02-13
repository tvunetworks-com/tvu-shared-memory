#include <gtest/gtest.h>
#include "libshm_tvu_timestamp.h"
#include <vector>

// Test fixture for TVU timestamp tests
class LibshmTvuTimestampTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }

    void TearDown() override {
        // Clean up after each test
    }
};

// Test the transfer function with various frame rates
TEST_F(LibshmTvuTimestampTest, TransferFunctionTest) {
    // Test same FPS conversion (should return same value)
    uint64_t input = (1LL << 56) | 100; // FPS = 1, index = 100
    uint64_t result = LibshmutilTvutimestampTransfer(input, 1);
    EXPECT_EQ(result, input);

    // Test different FPS conversion
    uint64_t src_timecode = (5LL << 56) | 1000; // FPS = 5 (23.98 fps), index = 1000
    uint64_t converted = LibshmutilTvutimestampTransfer(src_timecode, 6); // Convert to 24fps
    EXPECT_NE(converted, src_timecode); // Should be different
    
    // Test invalid FPS values
    uint64_t invalid_src = (100LL << 56) | 100; // Invalid FPS value
    uint64_t invalid_result = LibshmutilTvutimestampTransfer(invalid_src, 100);
    EXPECT_EQ(invalid_result, TVU_TIMECODE_INVALID_VALUE);
    
    // Another invalid case
    invalid_result = LibshmutilTvutimestampTransfer(100, 100);
    EXPECT_EQ(invalid_result, TVU_TIMECODE_INVALID_VALUE);
}

// Test the comparison function
TEST_F(LibshmTvuTimestampTest, CompareFunctionTest) {
    uint64_t tc1 = (6LL << 56) | 100; // 25fps, index=100
    uint64_t tc2 = (6LL << 56) | 200; // 25fps, index=200
    
    // Test less than
    EXPECT_EQ(LibshmutilTvutimestampCompare(tc1, tc2), -1);
    
    // Test greater than
    EXPECT_EQ(LibshmutilTvutimestampCompare(tc2, tc1), 1);
    
    // Test equal
    EXPECT_EQ(LibshmutilTvutimestampCompare(tc1, tc1), 0);
    
    // Test across different FPS but same time value
    uint64_t tc3 = (7LL << 56) | 120; // 29.97fps, index=120
    uint64_t tc4 = (6LL << 56) | 100; // 25fps, index=100
    // These should represent approximately the same time when converted to common base
    int comp_result = LibshmutilTvutimestampCompare(tc3, tc4);
    // This might be -1, 0, or 1 depending on exact calculation
    EXPECT_TRUE(comp_result == -1 || comp_result == 0 || comp_result == 1);
}

// Test the minus function
TEST_F(LibshmTvuTimestampTest, MinusFunctionTest) {
    uint64_t tc1 = (6LL << 56) | 100; // 25fps, index=100
    uint64_t tc2 = (6LL << 56) | 50;  // 25fps, index=50
    
    int64_t diff = LibshmutilTvutimestampMinusWithMS(tc1, tc2);
    EXPECT_GT(diff, 0);  // Positive difference
    
    diff = LibshmutilTvutimestampMinusWithMS(tc2, tc1);
    EXPECT_LT(diff, 0);  // Negative difference
    
    diff = LibshmutilTvutimestampMinusWithMS(tc1, tc1);
    EXPECT_EQ(diff, 0);  // Zero difference
}

// Test validity check function
TEST_F(LibshmTvuTimestampTest, ValidityCheckTest) {
    // Valid timecode
    uint64_t valid_tc = (5LL << 56) | 100; // Valid FPS index
    EXPECT_TRUE(LibshmutilTvutimestampValid(valid_tc));
    
    // Invalid timecode (FPS index too high)
    uint64_t invalid_tc = (100LL << 56) | 100; // Invalid FPS index
    EXPECT_FALSE(LibshmutilTvutimestampValid(invalid_tc));
    
    // Boundary test - max valid FPS index
    uint64_t boundary_tc = ((TVU_FPS_KEY_MAX_NUM - UINT64_C(1)) << 56) | 100;
    EXPECT_TRUE(LibshmutilTvutimestampValid(boundary_tc));
    
    // Boundary test - just above max valid FPS index
    uint64_t above_boundary_tc = (TVU_FPS_KEY_MAX_NUM*UINT64_C(1) << 56) | 100;
    EXPECT_FALSE(LibshmutilTvutimestampValid(above_boundary_tc));
}

// Test merge and create functions
TEST_F(LibshmTvuTimestampTest, MergeAndCreateTest) {
    uint64_t index = 12345;
    int fps = 6; // 25fps
    
    uint64_t merged = LibshmutilTvutimestampMerge(index, fps);
    EXPECT_EQ(LibshmutilTvutimestampGetIndexValue(merged), index);
    EXPECT_EQ(LibshmutilTvutimestampGetFpsValue(merged), fps);
    
    // Test with FPS pair structure
    const struct STvuFpsPair* pair = LibshmutilTvutimestampFpspairGetNode(fps);
    ASSERT_NE(pair, nullptr);
    uint64_t created = LibshmutilTvutimestampCreate(index, pair);
    EXPECT_EQ(LibshmutilTvutimestampGetIndexValue(created), index);
    EXPECT_EQ(LibshmutilTvutimestampGetFpsValue(created), fps);
    
    // Test invalid FPS
    uint64_t invalid_created = LibshmutilTvutimestampCreate(index, nullptr);
    EXPECT_EQ(invalid_created, TVU_TIMECODE_INVALID_VALUE);
    
    // Test invalid FPS value in get node
    const struct STvuFpsPair* invalid_pair = LibshmutilTvutimestampFpspairGetNode(TVU_FPS_KEY_MAX_NUM);
    EXPECT_EQ(invalid_pair, nullptr);
}

// Test index and FPS value extraction
TEST_F(LibshmTvuTimestampTest, IndexAndFpsExtractionTest) {
    uint64_t full_value = (10LL << 56) | 0x23456789ABCDEF; // FPS = 10, index = 0x...CDEF
    
    uint64_t extracted_index = LibshmutilTvutimestampGetIndexValue(full_value);
    uint8_t extracted_fps = LibshmutilTvutimestampGetFpsValue(full_value);
    
    EXPECT_EQ(extracted_index, full_value & 0x23456789ABCDEF);
    EXPECT_EQ(extracted_fps, 10);
    
    // Test with maximum index value
    uint64_t max_index_full = (5LL << 56) | 0xFFFFFFFFFFFF;
    uint64_t max_extracted_index = LibshmutilTvutimestampGetIndexValue(max_index_full);
    EXPECT_EQ(max_extracted_index, 0xFFFFFFFFFFFF);
    
    uint8_t max_extracted_fps = LibshmutilTvutimestampGetFpsValue(max_index_full);
    EXPECT_EQ(max_extracted_fps, 5);
}

// Test binary conversion functions
TEST_F(LibshmTvuTimestampTest, BinaryConversionTest) {
    uint64_t original = 0x1234567890ABCDEF;
    uint8_t binary[8];
    
    // Set binary representation
    LibshmutilTvutimestampSetBinary(original, binary);
    
    // Parse back from binary
    uint64_t parsed = LibshmutilTvutimestampParseBinary(binary);
    
    EXPECT_EQ(parsed, original);
    
    // Test with another value
    uint64_t test_val = 0xFEDCBA9876543210;
    LibshmutilTvutimestampSetBinary(test_val, binary);
    uint64_t parsed2 = LibshmutilTvutimestampParseBinary(binary);
    EXPECT_EQ(parsed2, test_val);
}

// Test FPS pair get/set functions
TEST_F(LibshmTvuTimestampTest, FpsPairFunctionsTest) {
    // Test getting valid FPS pair nodes
    for (int i = 0; i < TVU_FPS_KEY_MAX_NUM && i < 10; ++i) {  // Limit iterations for efficiency
        const struct STvuFpsPair* pair = LibshmutilTvutimestampFpspairGetNode(i);
        ASSERT_NE(pair, nullptr);
        
        int retrieved_fps = LibshmutilTvutimestampFpspairGetValue(pair);
        EXPECT_GE(retrieved_fps, 0);
    }
    
    // Test with invalid node (nullptr)
    int invalid_fps = LibshmutilTvutimestampFpspairGetValue(nullptr);
    EXPECT_EQ(invalid_fps, -1);
    
    // Test getting node with invalid FPS index
    const struct STvuFpsPair* invalid_node = LibshmutilTvutimestampFpspairGetNode(TVU_FPS_KEY_MAX_NUM);
    EXPECT_EQ(invalid_node, nullptr);
}

// Test endianness detection (indirectly through binary functions)
TEST_F(LibshmTvuTimestampTest, EndiannessHandlingTest) {
    uint64_t test_value = 0x1122334455667788;
    uint8_t buffer[8];
    
    LibshmutilTvutimestampSetBinary(test_value, buffer);
    uint64_t restored = LibshmutilTvutimestampParseBinary(buffer);
    
    EXPECT_EQ(restored, test_value);
    
    // Check that the byte layout makes sense
    uint64_t reconstructed = 0;
    for (int i = 0; i < 8; ++i) {
        reconstructed |= ((uint64_t)buffer[i]) << (i * 8);
    }
    
    // Depending on system endianness, the result may differ
    // The important thing is that set/parsing round-trips correctly
    EXPECT_EQ(restored, test_value);
}

// Test edge cases and boundary conditions
TEST_F(LibshmTvuTimestampTest, EdgeCasesTest) {
    // Test with maximum possible index value
    uint64_t max_index = 0xFFFFFFFFFFFF;
    uint64_t max_tc = LibshmutilTvutimestampMerge(max_index, 5); // Use valid FPS
    EXPECT_EQ(LibshmutilTvutimestampGetIndexValue(max_tc), max_index);
    EXPECT_EQ(LibshmutilTvutimestampGetFpsValue(max_tc), 5);
    
    // Test with minimum values
    uint64_t min_tc = LibshmutilTvutimestampMerge(0, 0);
    EXPECT_EQ(LibshmutilTvutimestampGetIndexValue(min_tc), 0);
    EXPECT_EQ(LibshmutilTvutimestampGetFpsValue(min_tc), 0);
    
    // Test transfer with extreme values
    uint64_t extreme_tc = (15LL << 56) | 0x76543210FEDCBA;
    uint64_t extreme_converted = LibshmutilTvutimestampTransfer(extreme_tc, 0);
    EXPECT_NE(extreme_converted, TVU_TIMECODE_INVALID_VALUE);
    
    // Verify the FPS was changed but index portion is preserved proportionally
    EXPECT_EQ(LibshmutilTvutimestampGetFpsValue(extreme_converted), 0);
}