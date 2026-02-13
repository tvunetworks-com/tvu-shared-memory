#include <gtest/gtest.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "libshm_media.h"
}

static std::string make_shm_name()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/gtest_libshm_media_%d_%u", (int)getpid(), (unsigned)rand());
    return std::string(buf);
}

TEST(LibShmMediaBasic, CreateAndQuery)
{
    std::string name = make_shm_name();
    libshm_media_handle_t h = LibShmMediaCreate(name.c_str(), 1024, 4, 4096);
    ASSERT_NE(h, (libshm_media_handle_t)NULL);

    EXPECT_EQ(LibShmMediaIsCreator(h), 1);

    unsigned int itemLen = LibShmMediaGetItemLength(h);
    EXPECT_GE(itemLen, 4096u);

    unsigned int counts = LibShmMediaGetItemCounts(h);
    EXPECT_GE(counts, 4u);

    const char *nm = LibShmMediaGetName(h);
    ASSERT_NE(nm, (const char *)NULL);
    EXPECT_STRNE(nm, "");

    unsigned int widx = LibShmMediaGetWriteIndex(h);
    unsigned int r = LibShmMediaSeekReadIndexToWriteIndex(h);
    EXPECT_EQ(r, widx);

    uint8_t *addr = LibShmMediaGetItemDataAddr(h, 0);
    EXPECT_NE(addr, (uint8_t *)NULL);

    // Try calling simple polling APIs (non-blocking)
    int sendable = LibShmMediaPollSendable(h, 0);
    EXPECT_GE(sendable, 0);

    int readable = LibShmMediaPollReadable(h, 0);
    EXPECT_LE(readable, 0);

    // cleanup
    LibShmMediaDestroy(h);

#if defined(TVU_LINUX)
    // remove system shm if exists
    LibShmMediaRemoveShmidFromSystem(name.c_str());
#endif
}

TEST(LibShmMediaBasic, SendDataNoPayload)
{
    std::string name = make_shm_name();
    libshm_media_handle_t h = LibShmMediaCreate(name.c_str(), 1024, 2, 4096);
    ASSERT_NE(h, (libshm_media_handle_t)NULL);

    libshm_media_head_param_t head;
    memset(&head, 0, sizeof(head));
    LibShmMediaPollReadHead(h, &head, 0); // should be safe (no-op)

    libshm_media_item_param_t item;
    memset(&item, 0, sizeof(item));

    int ret = LibShmMediaSendData(h, &head, &item);
    // sending zero-size item may return 0 (no-op) or non-negative; assert not negative
    EXPECT_GE(ret, 0);

    LibShmMediaDestroy(h);
#if defined(TVU_LINUX)
    LibShmMediaRemoveShmidFromSystem(name.c_str());
#endif
}

#include <gtest/gtest.h>
#include <memory>
#include "libshm_media.h"  // Main API header file
#include "libshm_media_internal.h"  // Internal structures if needed

// Mock or stub implementation for testing
class LibShmMediaSearchItemWithTvutimestampTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        // Initialize any common test setup
        shm_name = "test_shm_search_timestamp";
        header_len = 1024;
        item_count = 10;
        item_length = 4096;
    }

    virtual void TearDown() {
        // Cleanup after each test
        if (handle) {
            LibShmMediaDestroy(handle);
            handle = nullptr;
        }
    }

    const char* shm_name;
    uint32_t header_len;
    uint32_t item_count;
    uint32_t item_length;
    libshm_media_handle_t handle = nullptr;
};

// Helper function to initialize media head parameters
void InitMediaHeadParam(libshm_media_head_param_t* head_param) {
    memset(head_param, 0, sizeof(libshm_media_head_param_t));
    head_param->i_dstw = 1920;
    head_param->i_dsth = 1080;
    head_param->u_videofourcc = 0x31637661;  // avc1
    head_param->i_duration = 1;
    head_param->i_scale = 30;
    head_param->u_audiofourcc = 0x6169766f;  // ovia
    head_param->i_channels = 2;
    head_param->i_depth = 16;
    head_param->i_samplerate = 48000;
}

// Helper function to initialize media item parameters
void InitMediaItemParam(libshm_media_item_param_t* item_param, int64_t pts, uint64_t timestamp,uint8_t aExtBuff[256]) {
    memset(item_param, 0, sizeof(libshm_media_item_param_t));
    item_param->i64_vpts = pts;
    item_param->i64_apts = pts + 100;
    item_param->i64_spts = pts + 200;
    
    // Simulate timestamp data in user data section
#if 1
        uint32_t iExtBuffSize = 0;
        uint64_t tvutimestamp = timestamp;


        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.bGotTvutimestamp = true;
            myExt.u64Tvutimestamp = tvutimestamp;
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        item_param->i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        item_param->p_userData = aExtBuff;
        item_param->i_userDataLen = iExtBuffSize;
#endif

}

// Test successful search with valid timestamp
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, SuccessfulSearch) {
    // Create shared memory
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);

    // Prepare media head and item params
    libshm_media_head_param_t head_param;
    InitMediaHeadParam(&head_param);
    
    libshm_media_item_param_t item_param;
    uint64_t test_timestamp = (static_cast<uint64_t>(1) << 56) | 1000; // FPS index = 1, value = 1000
    uint8_t aExtBuff[256] = {0};
    InitMediaItemParam(&item_param, 1000, test_timestamp, aExtBuff);
    uint8_t vdata[256] = {0};
    memset(vdata, 0xAB, sizeof(vdata));
    item_param.p_vData = vdata;
    item_param.i_vLen = 16;
    item_param.p_aData = vdata+16;
    item_param.i_aLen = 16;
    
    // Send data to shared memory
    int send_result = LibShmMediaSendData(handle, &head_param, &item_param);
    ASSERT_GT(send_result, 0);
    
    // Wait a bit for data to be available
    usleep(10000); // 10ms delay
    
    // Search for item with timestamp


    /* test tvutimestamp search. */
    {
        libshm_media_handle_t hR = LibShmMediaOpen(shm_name, NULL, NULL);

        ASSERT_NE((void *)hR, nullptr);

        uint32_t write_index = LibShmMediaGetWriteIndex(hR);
        uint32_t shmCounts = LibShmMediaGetItemCounts(hR);

        uint32_t read_index = (write_index >= shmCounts) ? (write_index-shmCounts):0;

        LibShmMediaSeekReadIndex(hR, read_index);
        libshm_media_item_param_t ohi;
        {
            LibShmMediaItemParamInit(&ohi, sizeof(libshm_media_item_param_t));
        }
        libshm_media_item_param_t found_item;
        bool found = LibShmMediaSearchItemWithTvutimestamp(hR, test_timestamp, &found_item);
        EXPECT_TRUE(found);
        if (found) {
            EXPECT_EQ(found_item.i64_vpts, item_param.i64_vpts);
            EXPECT_EQ(found_item.i64_apts, item_param.i64_apts);
        }

        LibShmMediaDestroy(hR);
    }


}

// Test search with null handle
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, NullHandle) {
    libshm_media_item_param_t item_param;
    bool result = LibShmMediaSearchItemWithTvutimestamp(nullptr, 12345, &item_param);
    EXPECT_FALSE(result);
}

// Test search with null item parameter pointer
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, NullItemParam) {
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);
    
    bool result = LibShmMediaSearchItemWithTvutimestamp(handle, 12345, nullptr);
    EXPECT_FALSE(result);
}

// Test search with invalid timestamp
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, InvalidTimestamp1) {
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);

    libshm_media_head_param_t head_param;
    InitMediaHeadParam(&head_param);
    
    libshm_media_item_param_t item_param;
    uint64_t valid_timestamp = (static_cast<uint64_t>(1) << 56) | 1000;
    uint8_t aExtBuff[256] = {0};
    InitMediaItemParam(&item_param, 1000, valid_timestamp, aExtBuff);
    uint8_t vdata[256] = {0};
    memset(vdata, 0xAB, sizeof(vdata));
    item_param.p_vData = vdata;
    item_param.i_vLen = 16;
    item_param.p_aData = vdata+16;
    item_param.i_aLen = 16;
    
    int send_result = LibShmMediaSendData(handle, &head_param, &item_param);
    ASSERT_GT(send_result, 0);
    
    usleep(10000); // 10ms delay
    
    // Try searching with an invalid timestamp
    libshm_media_item_param_t found_item;
    uint64_t invalid_timestamp = 0xFFFFFFFFFFFFFFFF; // Definitely invalid
    bool found = LibShmMediaSearchItemWithTvutimestamp(handle, invalid_timestamp, &found_item);
    
    EXPECT_FALSE(found);
}

// Test search when item is not present in shared memory
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, ItemNotFound) {
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);

    libshm_media_head_param_t head_param;
    InitMediaHeadParam(&head_param);
    
    libshm_media_item_param_t item_param;
    uint64_t existing_timestamp = (static_cast<uint64_t>(1) << 56) | 1000;
    uint8_t aExtBuff[256] = {0};
    InitMediaItemParam(&item_param, 1000, existing_timestamp, aExtBuff);
    uint8_t vdata[256] = {0};
    memset(vdata, 0xAB, sizeof(vdata));
    item_param.p_vData = vdata;
    item_param.i_vLen = 16;
    item_param.p_aData = vdata+16;
    item_param.i_aLen = 16;
    
    int send_result = LibShmMediaSendData(handle, &head_param, &item_param);
    ASSERT_GT(send_result, 0);
    
    usleep(10000); // 10ms delay
    
    // Search for a timestamp that doesn't exist
    libshm_media_item_param_t found_item;
    uint64_t non_existing_timestamp = (static_cast<uint64_t>(1) << 56) | 9999;
    bool found = LibShmMediaSearchItemWithTvutimestamp(handle, non_existing_timestamp, &found_item);
    
    EXPECT_FALSE(found);
}

// Test with multiple items and search for middle one
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, MultipleItemsSearch) {
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);

    libshm_media_head_param_t head_param;
    InitMediaHeadParam(&head_param);
    
    // Add multiple items with different timestamps
    for (int i = 0; i < 5; ++i) {
        libshm_media_item_param_t item_param;
        uint64_t timestamp = (static_cast<uint64_t>(1) << 56) | (1000 + i * 100);
        uint8_t aExtBuff[256] = {0};
        InitMediaItemParam(&item_param, 1000+i*100, timestamp, aExtBuff);
        uint8_t vdata[256] = {0};
        memset(vdata, 0xAB, sizeof(vdata));
        item_param.p_vData = vdata;
        item_param.i_vLen = 16;
        item_param.p_aData = vdata+16;
        item_param.i_aLen = 16;
        
        int send_result = LibShmMediaSendData(handle, &head_param, &item_param);
        ASSERT_GT(send_result, 0);
    }
    
    usleep(50000); // 50ms delay to ensure all data is written

    /* test tvutimestamp search. */
    {
        libshm_media_handle_t hR = LibShmMediaOpen(shm_name, NULL, NULL);

        ASSERT_NE((void *)hR, nullptr);

 

        uint32_t write_index = LibShmMediaGetWriteIndex(hR);
        uint32_t shmCounts = LibShmMediaGetItemCounts(hR);

        uint32_t read_index = (write_index >= shmCounts) ? (write_index-shmCounts):0;

        LibShmMediaSeekReadIndex(hR, read_index);
        libshm_media_item_param_t ohi;
        {
            LibShmMediaItemParamInit(&ohi, sizeof(libshm_media_item_param_t));
        }
        // Search for the middle item
        libshm_media_item_param_t found_item;
        uint64_t target_timestamp = (static_cast<uint64_t>(1) << 56) | 1200; // Third item
        bool found = LibShmMediaSearchItemWithTvutimestamp(hR, target_timestamp, &found_item);
        
        EXPECT_TRUE(found);
        if (found) {
            // Verify we got the expected item (this depends on internal implementation)
            EXPECT_EQ(found_item.i64_vpts, 1200);
        }

        LibShmMediaDestroy(hR);
    }
}

// Test boundary condition with max timestamp value
TEST_F(LibShmMediaSearchItemWithTvutimestampTest, MaxTimestampValue) {
    handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
    ASSERT_NE(handle, nullptr);

    libshm_media_head_param_t head_param;
    InitMediaHeadParam(&head_param);
    
    // Create an item with maximum possible timestamp within valid FPS range
    libshm_media_item_param_t item_param;
    uint64_t max_valid_timestamp = (static_cast<uint64_t>(5) << 56) | 0xFFFFFFFFFFFFFF; // FPS index 5, max value
    uint8_t aExtBuff[256] = {0};
    InitMediaItemParam(&item_param, 1000, max_valid_timestamp, aExtBuff);
    
    int send_result = LibShmMediaSendData(handle, &head_param, &item_param);
    ASSERT_GT(send_result, 0);
    
    usleep(10000); // 10ms delay
    
    libshm_media_item_param_t found_item;
    bool found = LibShmMediaSearchItemWithTvutimestamp(handle, max_valid_timestamp, &found_item);
    
    EXPECT_TRUE(found);
}


#include <gtest/gtest.h>
#include "libshm_media.h"
#include "libshm_media_internal.h"
#include "libshm_tvu_timestamp.h"
#include <thread>
#include <chrono>
#include <atomic>

class LibShmMediaSearchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test parameters
        memory_name = "test_shm_search_tvu_timestamp";
        header_len = 1024;
        item_count = 10;
        item_length = 2048;
        
        // Clean up any existing shared memory
        #ifdef TVU_LINUX
        LibShmMediaRemoveShmidFromSystem(memory_name.c_str());
        #endif
        
        // Create shared memory instance for testing
        handle = LibShmMediaCreate(memory_name.c_str(), header_len, item_count, item_length);
        ASSERT_NE(handle, nullptr);
        
        // Initialize head parameters
        LibShmMediaHeadParamInit(&head_param, sizeof(head_param));
        head_param.i_dstw = 1920;
        head_param.i_dsth = 1080;
        head_param.u_videofourcc = 0x30323449; // 'I420'
        head_param.i_duration = 1;
        head_param.i_scale = 30;
        head_param.u_audiofourcc = 0x6C706D73; // 'smpl'
        head_param.i_channels = 2;
        head_param.i_depth = 16;
        head_param.i_samplerate = 48000;

        hReader = LibShmMediaOpen(memory_name.c_str(), NULL, NULL);
        ASSERT_NE((void *)hReader, nullptr);
    }
    
    void TearDown() override {
        if (handle) {
            LibShmMediaDestroy(handle);
        }
        if (hReader) {
            LibShmMediaDestroy(hReader);
        }
    }
    
    // Helper function to create a timecode with valid format
    uint64_t createTimecode(int fps_idx, uint64_t index) {
        return ((uint64_t)fps_idx << 56) | (index & 0xFFFFFFFFFFFFFF);
    }
    
    // Helper function to add sample data with timecodes
    void addSampleDataWithTimecode(uint64_t timecode) {
        libshm_media_item_param_t item_param = {};
        item_param.i64_vpts = 1000;
        item_param.i64_apts = 1000;
        item_param.i64_spts = 1000;
        
        // Create user data containing timecode
#if 1
        uint8_t aExtBuff[256] = {0};
        uint32_t iExtBuffSize = 0;
        uint64_t tvutimestamp = timecode;


        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.bGotTvutimestamp = true;
            myExt.u64Tvutimestamp = tvutimestamp;
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        item_param.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        item_param.p_userData = aExtBuff;
        item_param.i_userDataLen = iExtBuffSize;
        item_param.i64_userDataCT = 123456789;
#endif
        
        // Add some dummy video data
        uint8_t dummy_video[100];
        memset(dummy_video, 0xAB, sizeof(dummy_video));
        item_param.p_vData = dummy_video;
        item_param.i_vLen = sizeof(dummy_video);
        
        int result = LibShmMediaSendData(handle, &head_param, &item_param);
        ASSERT_GT(result, 0);
    }

    bool searchTimeCode(uint32_t from_index, uint64_t timecode, libshm_media_item_param_t* out_item) {
        LibShmMediaSeekReadIndex(hReader, from_index);
        return LibShmMediaSearchItemWithTvutimestamp(hReader, timecode, out_item);
    }

    void parseTimecodeFromItem(const libshm_media_item_param_t* item, uint64_t* out_timecode) {

        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        LibShmMeidaParseExtendDataV2(&myExt, item->p_userData, item->i_userDataLen);

        if (myExt.bGotTvutimestamp) {
            memcpy(out_timecode, &myExt.u64Tvutimestamp, sizeof(uint64_t));
        } else {
            *out_timecode = 0;
        }
    }
    
    std::string memory_name;
    uint32_t header_len;
    uint32_t item_count;
    uint32_t item_length;
    libshm_media_handle_t handle;
    libshm_media_handle_t hReader;
    libshm_media_head_param_t head_param;
};

// Test normal search operation with exact timestamp match
TEST_F(LibShmMediaSearchTest, SearchExactTimestampMatch) {
    // Add several items with different timecodes
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); // Ensure read index is at write index
    uint64_t target_timecode = createTimecode(6, 1000);  // 25fps, index 1000
    uint64_t timecode1 = createTimecode(6, 900);   // Before target
    uint64_t timecode2 = createTimecode(6, 1000);  // Exact match
    uint64_t timecode3 = createTimecode(6, 1100);  // After target
    
    addSampleDataWithTimecode(timecode1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Ensure timing order
    addSampleDataWithTimecode(timecode2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    addSampleDataWithTimecode(timecode3);
    
    // Wait a bit for data to be properly written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Perform search for the exact middle timestamp
    libshm_media_item_param_t result_item;
    bool found = searchTimeCode(write_index, target_timecode, &result_item);
    
    EXPECT_TRUE(found) << "Should find item with exact timestamp match";
    EXPECT_GT(result_item.i_userDataLen, sizeof(uint64_t));
    
    // Verify the found item has the expected timecode
    uint64_t found_timecode = 0;
    if (result_item.p_userData && result_item.i_userDataLen >= sizeof(uint64_t)) {
        parseTimecodeFromItem(&result_item, &found_timecode);
        EXPECT_EQ(found_timecode, target_timecode) << "Found item should have exact timestamp match";
    }
}

// Test search operation with non-existent timestamp
TEST_F(LibShmMediaSearchTest, SearchNonExistentTimestamp) {
    // Add only one item
    uint64_t existing_timecode = createTimecode(6, 1000);
    addSampleDataWithTimecode(existing_timecode);
    
    // Wait for data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to find a timestamp that doesn't exist
    uint64_t non_existent_timecode = createTimecode(6, 2000);
    libshm_media_item_param_t result_item;
    bool found = LibShmMediaSearchItemWithTvutimestamp(handle, non_existent_timecode, &result_item);
    
    // Note: Based on implementation, the function might return the closest item rather than exact match
    // For this test, we expect it to return false since our implementation shows it only returns true for exact matches
    EXPECT_FALSE(found) << "Should not find item with non-existent timestamp";
}

// Test search operation with invalid timestamp
TEST_F(LibShmMediaSearchTest, SearchInvalidTimestamp) {
    // Add a valid item first
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    uint64_t valid_timecode = createTimecode(6, 1000);
    addSampleDataWithTimecode(valid_timecode);
    
    // Wait for data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to search with an invalid timestamp (FPS index out of range)
    uint64_t invalid_timecode = createTimecode(100, 1000); // Invalid FPS index
    libshm_media_item_param_t result_item;
    bool found = searchTimeCode(write_index, invalid_timecode, &result_item);
    
    EXPECT_FALSE(found) << "Should not find anything with invalid timestamp";
    
    // Also test with TVU_TIMECODE_INVALID_VALUE if defined
    #ifdef TVU_TIMECODE_INVALID_VALUE
    bool found_invalid = searchTimeCode(write_index, TVU_TIMECODE_INVALID_VALUE, &result_item);
    EXPECT_FALSE(found_invalid) << "Should not find anything with TVU_TIMECODE_INVALID_VALUE";
    #endif
}

// Test search operation with null pointer parameter
TEST_F(LibShmMediaSearchTest, SearchNullPointerParameter) {
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    uint64_t timecode = createTimecode(6, 1000);
    
    // Try with null pointer for pmi parameter
    bool found = searchTimeCode(write_index, timecode, nullptr);
    
    EXPECT_FALSE(found) << "Should return false when pmi parameter is null";
}

// Test search operation with empty shared memory
TEST_F(LibShmMediaSearchTest, SearchEmptySharedMemory) {
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    uint64_t timecode = createTimecode(6, 1000);
    libshm_media_item_param_t result_item;
    bool found = searchTimeCode(write_index, timecode, &result_item);
    
    EXPECT_FALSE(found) << "Should not find anything in empty shared memory";
}

// Test search operation with multiple timestamps to verify binary search behavior
TEST_F(LibShmMediaSearchTest, SearchMultipleTimestamps) {
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    std::vector<uint64_t> timecodes;
    
    // Add multiple items with increasing timestamps
    for (int i = 1; i <= 8; i++) {
        uint64_t timecode = createTimecode(6, i * 100);
        timecodes.push_back(timecode);
        addSampleDataWithTimecode(timecode);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Wait for all data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test searching for each of the inserted timestamps
    for (size_t i = 0; i < timecodes.size(); i++) {
        libshm_media_item_param_t result_item;
        bool found = searchTimeCode(write_index, timecodes[i], &result_item);
        
        EXPECT_TRUE(found) << "Should find item for timestamp at position " << i;
        
        if (found && result_item.p_userData && result_item.i_userDataLen >= sizeof(uint64_t)) {
            uint64_t found_timecode;
            parseTimecodeFromItem(&result_item, &found_timecode);
            // Alternatively, directly extract from p_userData if format is known
            EXPECT_EQ(found_timecode, timecodes[i]) << "Found timestamp should match requested one";
        }
    }
}

// Test edge case with minimum and maximum timestamp values
TEST_F(LibShmMediaSearchTest, SearchTimestampEdgeCases) {
    // Test with minimum index value
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    uint64_t min_timecode = createTimecode(6, 1);  // Minimum practical index
    addSampleDataWithTimecode(min_timecode);
    
    // Test with maximum index value allowed
    uint64_t max_timecode = createTimecode(6, 0x00FFFFFFFFFFFE);  // Maximum index within mask
    addSampleDataWithTimecode(max_timecode);
    
    // Wait for data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to find the minimum timestamp
    libshm_media_item_param_t result_min;
    bool found_min = searchTimeCode(write_index, min_timecode, &result_min);
    EXPECT_TRUE(found_min) << "Should find item with minimum timestamp";
    
    // Try to find the maximum timestamp
    libshm_media_item_param_t result_max;
    bool found_max = searchTimeCode(write_index, max_timecode, &result_max);
    EXPECT_TRUE(found_max) << "Should find item with maximum timestamp";
}

// Test search with different FPS indices
TEST_F(LibShmMediaSearchTest, SearchDifferentFpsIndices) {
    // Add items with different FPS indices
    uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    uint64_t timecode_fps_5 = createTimecode(5, 1000);  // 23.98fps
    uint64_t timecode_fps_6 = createTimecode(6, 1000);  // 25fps
    uint64_t timecode_fps_7 = createTimecode(7, 1000);  // 29.97fps
    
    addSampleDataWithTimecode(timecode_fps_7);
    addSampleDataWithTimecode(timecode_fps_6);
    addSampleDataWithTimecode(timecode_fps_5);
    
    // Wait for data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Search for each FPS type specifically
    libshm_media_item_param_t result_5;
    bool found_5 = searchTimeCode(write_index, timecode_fps_5, &result_5);
    EXPECT_TRUE(found_5) << "Should find item with FPS index 5";
    
    libshm_media_item_param_t result_6;
    bool found_6 = searchTimeCode(write_index, timecode_fps_6, &result_6);
    EXPECT_TRUE(found_6) << "Should find item with FPS index 6";
    
    libshm_media_item_param_t result_7;
    bool found_7 = searchTimeCode(write_index, timecode_fps_7, &result_7);
    EXPECT_TRUE(found_7) << "Should find item with FPS index 7";
}

// Test concurrent access safety (basic check)
TEST_F(LibShmMediaSearchTest, ConcurrentAccessSafety) {
    // Add several items
    static uint32_t write_index = LibShmMediaGetWriteIndex(handle); 
    for (int i = 1; i <= 5; i++) {
        uint64_t timecode = createTimecode(6, i * 100);
        addSampleDataWithTimecode(timecode);
    }
    
    // Wait for data to be written
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to search concurrently from multiple threads
    std::vector<std::thread> threads;
    std::atomic<bool> all_success{true};
    
    for (int i = 0; i < 3; i++) {
        threads.emplace_back([this, &all_success]() {
            uint64_t target_timecode = createTimecode(6, 300); // Middle value
            libshm_media_item_param_t result_item;
            uint32_t writeIn = write_index;
            libshm_media_handle_t _h = LibShmMediaOpen(memory_name.c_str(), NULL, NULL);
            LibShmMediaSeekReadIndex(_h, writeIn);
            bool found = LibShmMediaSearchItemWithTvutimestamp(_h, target_timecode, &result_item);
            
            if (!found) {
                all_success.store(false);
            }

            LibShmMediaDestroy(_h);
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All searches should succeed (or at least not crash)
    EXPECT_TRUE(all_success.load()) << "Concurrent searches should succeed";
}


#include <gtest/gtest.h>
#include "libshm_media.h"
#include "libshm_tvu_timestamp.h"
#include <thread>
#include <chrono>
#include <vector>

// Test fixture for LibShmMediaReadItemWithTvutimestamp tests
class LibShmMediaReadItemWithTvutimestampTest : public ::testing::Test {
protected:
    void SetUp() override {
        shm_name = "test_shm_read_item_with_tvu_timestamp";
        header_len = 1024;
        item_count = 10;
        item_length = 2048;
        
        // Remove any existing shared memory with the same name
        #ifdef TVU_LINUX
        LibShmMediaRemoveShmidFromSystem(shm_name);
        #endif
        
        // Create shared memory for testing
        handle = LibShmMediaCreate(shm_name, header_len, item_count, item_length);
        ASSERT_NE(handle, nullptr);
        
        // Initialize media head parameters
        LibShmMediaHeadParamInit(&head_param, sizeof(head_param));
        head_param.i_dstw = 1920;
        head_param.i_dsth = 1080;
        head_param.u_videofourcc = 0x30323449; // 'I420'
        head_param.i_duration = 1;
        head_param.i_scale = 30;
        head_param.u_audiofourcc = 0x6C706D73; // 'smpl'
        head_param.i_channels = 2;
        head_param.i_depth = 16;
        head_param.i_samplerate = 48000;

        hReader = LibShmMediaOpen(shm_name, NULL, NULL);
        ASSERT_NE(handle, nullptr);
    }

    void TearDown() override {
        if (handle) {
            LibShmMediaDestroy(handle);
        }
    }

    // Helper function to create a valid TVU timestamp
    uint64_t createTimestamp(int fps_idx, uint64_t index) {
        return ((uint64_t)fps_idx << 56) | (index & 0xFFFFFFFFFFFFFF);
    }

    // Helper function to send data with timestamp
    void sendDataWithTimestamp(uint64_t timestamp) {
        libshm_media_item_param_t item_param = {};
        
        // Set up video data
        uint8_t video_data[100];
        memset(video_data, 0xAB, sizeof(video_data));
        item_param.p_vData = video_data;
        item_param.i_vLen = sizeof(video_data);
        item_param.i64_vpts = 1000;
        item_param.i64_vdts = 1000;
        
        // Set up audio data
        uint8_t audio_data[50];
        memset(audio_data, 0xCD, sizeof(audio_data));
        item_param.p_aData = audio_data;
        item_param.i_aLen = sizeof(audio_data);
        item_param.i64_apts = 1000;
        item_param.i64_adts = 1000;
        
        // Set up user data with timestamp
#if 1
        uint8_t aExtBuff[256] = {0};
        uint32_t iExtBuffSize = 0;
        uint64_t tvutimestamp = timestamp;


        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.bGotTvutimestamp = true;
            myExt.u64Tvutimestamp = tvutimestamp;
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        item_param.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        item_param.p_userData = aExtBuff;
        item_param.i_userDataLen = iExtBuffSize;
        item_param.i64_userDataCT = 123456789;
#endif
        
        int result = LibShmMediaSendData(handle, &head_param, &item_param);
        ASSERT_GT(result, 0);
    }

    void sendDataWithTimestamp2(uint64_t timestamp, uint64_t pts) {
        libshm_media_item_param_t item_param = {};

        // Set up video data
        uint8_t video_data[100];
        memset(video_data, 0xAB, sizeof(video_data));
        item_param.p_vData = video_data;
        item_param.i_vLen = sizeof(video_data);
        item_param.i64_vpts = pts;
        item_param.i64_vdts = pts;

        // Set up audio data
        uint8_t audio_data[50];
        memset(audio_data, 0xCD, sizeof(audio_data));
        item_param.p_aData = audio_data;
        item_param.i_aLen = sizeof(audio_data);
        item_param.i64_apts = 1000;
        item_param.i64_adts = 1000;

        // Set up user data with timestamp
#if 1
        uint8_t aExtBuff[256] = {0};
        uint32_t iExtBuffSize = 0;
        uint64_t tvutimestamp = timestamp;


        libshmmedia_extend_data_info_t myExt;
        {
            memset(&myExt, 0, sizeof (myExt));
        }

        {
            myExt.i_timecode_fps_index = 8;
            myExt.p_timecode_fps_index = (const uint8_t *)&tvutimestamp;
        }

        int iExtBuffSizeBeforeAlloc = LibShmMediaEstimateExtendDataSize(&myExt);
        iExtBuffSize = LibShmMediaWriteExtendData(aExtBuff, iExtBuffSizeBeforeAlloc, &myExt);

        item_param.i_userDataType = LIBSHM_MEDIA_TYPE_TVU_EXTEND_DATA_V2;
        item_param.p_userData = aExtBuff;
        item_param.i_userDataLen = iExtBuffSize;
        item_param.i64_userDataCT = 123456789;
#endif

        int result = LibShmMediaSendData(handle, &head_param, &item_param);
        ASSERT_GT(result, 0);
    }

    const char* shm_name;
    uint32_t header_len;
    uint32_t item_count;
    uint32_t item_length;
    libshm_media_handle_t handle;
    libshm_media_handle_t hReader;
    libshm_media_head_param_t head_param;
};

// Test successful reading of item with valid timestamp
TEST_F(LibShmMediaReadItemWithTvutimestampTest, SuccessWithValidTimestamp) {
    // Create and send data with timestamp
    uint64_t target_timestamp = createTimestamp(6, 1000); // 25fps, index 1000
    sendDataWithTimestamp(target_timestamp);
    
    // Wait a bit for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Attempt to read item with timestamp
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;
    
    int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
    
    EXPECT_GT(result, 0) << "Reading should succeed";
    EXPECT_TRUE(found) << "Item should be found";
    
    // Verify that the returned head parameters match
    EXPECT_EQ(read_head.i_dstw, head_param.i_dstw);
    EXPECT_EQ(read_head.i_dsth, head_param.i_dsth);
    EXPECT_EQ(read_head.u_videofourcc, head_param.u_videofourcc);
    EXPECT_EQ(read_head.i_duration, head_param.i_duration);
    EXPECT_EQ(read_head.i_scale, head_param.i_scale);
    EXPECT_EQ(read_head.u_audiofourcc, head_param.u_audiofourcc);
    EXPECT_EQ(read_head.i_channels, head_param.i_channels);
    EXPECT_EQ(read_head.i_depth, head_param.i_depth);
    EXPECT_EQ(read_head.i_samplerate, head_param.i_samplerate);
}

TEST_F(LibShmMediaReadItemWithTvutimestampTest, SuccessWithValidTimestampV2) {
    // Create and send data with timestamp
    uint64_t target_timestamp = createTimestamp(6, 1000); // 25fps, index 1000
    sendDataWithTimestamp(target_timestamp);
    
    // Wait a bit for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Attempt to read item with timestamp
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;

    libshmmedia_extend_data_info_t oext = {};
    
    int result = LibShmMediaReadItemWithTvutimestampV2(hReader, target_timestamp
        , 0, 0, &found, &foundPts, &read_head, &read_item, &oext);
    
    EXPECT_GT(result, 0) << "Reading should succeed";
    EXPECT_TRUE(found) << "Item should be found";
    EXPECT_TRUE(oext.bGotTvutimestamp);

    EXPECT_EQ(oext.u64Tvutimestamp, target_timestamp);
    
    // Verify that the returned head parameters match
    EXPECT_EQ(read_head.i_dstw, head_param.i_dstw);
    EXPECT_EQ(read_head.i_dsth, head_param.i_dsth);
    EXPECT_EQ(read_head.u_videofourcc, head_param.u_videofourcc);
    EXPECT_EQ(read_head.i_duration, head_param.i_duration);
    EXPECT_EQ(read_head.i_scale, head_param.i_scale);
    EXPECT_EQ(read_head.u_audiofourcc, head_param.u_audiofourcc);
    EXPECT_EQ(read_head.i_channels, head_param.i_channels);
    EXPECT_EQ(read_head.i_depth, head_param.i_depth);
    EXPECT_EQ(read_head.i_samplerate, head_param.i_samplerate);
}

TEST_F(LibShmMediaReadItemWithTvutimestampTest, SuccessWithValidPTS) {
    // Create and send data with timestamp
    uint64_t target_timestamp = createTimestamp(6, 1000); // 25fps, index 1000
    uint64_t target_timestamp2 = createTimestamp(6, 2000);
    sendDataWithTimestamp2(target_timestamp, 1000);
    sendDataWithTimestamp2(target_timestamp2, 2000);

    // Wait a bit for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Attempt to read item with timestamp
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;

    uint64_t target_timestamp1 = createTimestamp(6, 500);

    int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_timestamp1, 0, 1010, &found, &foundPts, &read_head, &read_item);

    EXPECT_GT(result, 0) << "Reading should succeed";
    EXPECT_FALSE(found) << "Item should be found";

    // Verify that the returned head parameters match
    EXPECT_EQ(read_head.i_dstw, head_param.i_dstw);
    EXPECT_EQ(read_head.i_dsth, head_param.i_dsth);
    EXPECT_EQ(read_head.u_videofourcc, head_param.u_videofourcc);
    EXPECT_EQ(read_head.i_duration, head_param.i_duration);
    EXPECT_EQ(read_head.i_scale, head_param.i_scale);
    EXPECT_EQ(read_head.u_audiofourcc, head_param.u_audiofourcc);
    EXPECT_EQ(read_head.i_channels, head_param.i_channels);
    EXPECT_EQ(read_head.i_depth, head_param.i_depth);
    EXPECT_EQ(read_head.i_samplerate, head_param.i_samplerate);
}

// Test reading with non-existent timestamp
TEST_F(LibShmMediaReadItemWithTvutimestampTest, NotFoundForNonExistentTimestamp) {
    // Send data with one timestamp
    uint64_t existing_timestamp = createTimestamp(6, 1000);
    sendDataWithTimestamp(existing_timestamp);
    
    // Wait for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to read with a different timestamp that doesn't exist
    uint64_t non_existent_timestamp = createTimestamp(6, 2000);
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = true; // Initialize to true to test if it gets set to false
    bool foundPts = false;
    
    int result = LibShmMediaReadItemWithTvutimestamp(hReader, non_existent_timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
    
    // The function should still return successfully but with found=false
    EXPECT_GT(result, 0) << "Function should return successfully even if item not found";
    EXPECT_FALSE(found) << "Item should not be found for non-existent timestamp";
}

// Test reading with invalid handle
TEST_F(LibShmMediaReadItemWithTvutimestampTest, InvalidHandle) {
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;
    uint64_t timestamp = createTimestamp(6, 1000);
    
    // Use null handle
    int result = LibShmMediaReadItemWithTvutimestamp(nullptr, timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
    
    EXPECT_LE(result, 0) << "Should fail with null handle";
}

// Test reading with null found parameter
TEST_F(LibShmMediaReadItemWithTvutimestampTest, NullFoundParameter) {
    uint64_t timestamp = createTimestamp(6, 1000);
    sendDataWithTimestamp(timestamp);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    
    // Pass null for found parameter
    int result = LibShmMediaReadItemWithTvutimestamp(handle, timestamp, 0, 0, nullptr, nullptr, &read_head, &read_item);
    
    // Expect failure due to null parameter
    EXPECT_LE(result, 0) << "Should fail with null found parameter";
}

// Test reading with null head parameter
TEST_F(LibShmMediaReadItemWithTvutimestampTest, NullHeadParameter) {
    uint64_t timestamp = createTimestamp(6, 1000);
    sendDataWithTimestamp(timestamp);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;
    
    // Pass null for head parameter
    int result = LibShmMediaReadItemWithTvutimestamp(handle, timestamp, 0, 0, &found, &foundPts, nullptr, &read_item);
    
    // Expect failure due to null parameter
    EXPECT_LE(result, 0) << "Should fail with null head parameter";
}

// Test reading with null item parameter
TEST_F(LibShmMediaReadItemWithTvutimestampTest, NullItemParameter) {
    uint64_t timestamp = createTimestamp(6, 1000);
    sendDataWithTimestamp(timestamp);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    libshm_media_head_param_t read_head;
    bool found = false;
    bool foundPts = false;
    
    // Pass null for item parameter
    int result = LibShmMediaReadItemWithTvutimestamp(handle, timestamp, 0, 0, &found, &foundPts, &read_head, nullptr);
    
    // Expect failure due to null parameter
    EXPECT_LE(result, 0) << "Should fail with null item parameter";
}

// Test reading with invalid timestamp
TEST_F(LibShmMediaReadItemWithTvutimestampTest, InvalidTimestamp2) {
    uint64_t invalid_timestamp = 0xFFFFFFFFFFFFFFFF; // Invalid timestamp
    libshm_media_head_param_t read_head;
    libshm_media_item_param_t read_item;
    bool found = false;
    bool foundPts = false;
    
    int result = LibShmMediaReadItemWithTvutimestamp(handle, invalid_timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
    
    // Function should handle invalid timestamp gracefully
    EXPECT_LE(result, 0) << "Should fail with invalid timestamp";
}

// Test reading with multiple timestamps in sequence
TEST_F(LibShmMediaReadItemWithTvutimestampTest, MultipleTimestampsSequential) {
    // Create and send multiple items with different timestamps
    std::vector<uint64_t> timestamps;
    for (int i = 1; i <= 5; ++i) {
        uint64_t ts = createTimestamp(6, i * 1000);
        timestamps.push_back(ts);
        sendDataWithTimestamp(ts);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small delay between sends
    }
    
    // Wait for all data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to read each timestamp individually
    for (uint64_t target_ts : timestamps) {
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_ts, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Reading should succeed for timestamp: " << target_ts;
        EXPECT_TRUE(found) << "Item should be found for timestamp: " << target_ts;
        
        // Verify head parameters
        EXPECT_EQ(read_head.i_dstw, head_param.i_dstw);
        EXPECT_EQ(read_head.i_dsth, head_param.i_dsth);
        EXPECT_EQ(read_head.u_videofourcc, head_param.u_videofourcc);
    }
}

TEST_F(LibShmMediaReadItemWithTvutimestampTest, ReverseReadingCheck) {
    // Create and send multiple items with different timestamps
    std::vector<uint64_t> timestamps;
    for (int i = 1; i <= 5; ++i) {
        uint64_t ts = createTimestamp(6, i * 1000);
        timestamps.push_back(ts);
        sendDataWithTimestamp(ts);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small delay between sends
    }
    
    // Wait for all data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to read each timestamp individually
    //for (uint64_t target_ts : timestamps) 
    {
        uint64_t target_ts = timestamps[1];
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_ts, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Reading should succeed for timestamp: " << target_ts;
        EXPECT_TRUE(found) << "Item should be found for timestamp: " << target_ts;
    }

    {
        uint64_t target_ts = timestamps[0];
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_ts, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Reading should succeed for timestamp: " << target_ts;
        EXPECT_FALSE(found) << "Item should be found for timestamp: " << target_ts;
    }

    {
        uint64_t target_ts = timestamps[3];
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_ts, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Reading should succeed for timestamp: " << target_ts;
        EXPECT_TRUE(found) << "Item should be found for timestamp: " << target_ts;
    }
}

// Test reading with different FPS indices
TEST_F(LibShmMediaReadItemWithTvutimestampTest, DifferentFpsIndices) {
    // Send items with different FPS indices
    std::vector<int> fps_indices = {7, 6, 5}; // 23.98fps, 25fps, 29.97fps
    std::vector<uint64_t> timestamps;
    
    for (int fps_idx : fps_indices) {
        uint64_t ts = createTimestamp(fps_idx, 1000);
        timestamps.push_back(ts);
        sendDataWithTimestamp(ts);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Wait for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to read each timestamp with different FPS index
    for (uint64_t target_ts : timestamps) {
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, target_ts, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Reading should succeed for timestamp: " << target_ts;
        EXPECT_TRUE(found) << "Item should be found for timestamp: " << target_ts;
        
        // Verify head parameters
        EXPECT_EQ(read_head.i_dstw, head_param.i_dstw);
        EXPECT_EQ(read_head.i_dsth, head_param.i_dsth);
        EXPECT_EQ(read_head.u_videofourcc, head_param.u_videofourcc);
    }
}

// Test concurrent access to the same shared memory
TEST_F(LibShmMediaReadItemWithTvutimestampTest, ConcurrentAccess) {
    // Send multiple items
    for (int i = 1; i <= 5; ++i) {
        uint64_t ts = createTimestamp(6, i * 1000);
        sendDataWithTimestamp(ts);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Wait for data to be available
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Create multiple threads to call the function concurrently
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);
    std::atomic<int> failure_count(0);
    
    for (int t = 0; t < 3; ++t) {
        threads.emplace_back([&]() {
            for (int i = 1; i <= 3; ++i) {
                uint64_t ts = createTimestamp(6, i * 1000);
                libshm_media_head_param_t read_head;
                libshm_media_item_param_t read_item;
                bool found = false;
                bool foundPts = false;
                
                libshm_media_handle_t _h = LibShmMediaOpen(shm_name, NULL, NULL);
                LibShmMediaSeekReadIndex(_h, 0);
                int result = LibShmMediaReadItemWithTvutimestamp(_h, ts, 0, 0, &found, &foundPts, &read_head, &read_item);

                if (result > 0) {
                    success_count++;
                } else {
                    failure_count++;
                }
                LibShmMediaDestroy(_h);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // At least some operations should succeed
    EXPECT_GT(success_count.load(), 0) << "Some read operations should succeed";
}

// Test boundary conditions with max/min timestamp values
TEST_F(LibShmMediaReadItemWithTvutimestampTest, TimestampBoundaryConditions) {
    // Test with minimum valid timestamp
    uint64_t min_timestamp = createTimestamp(1, 1);
    sendDataWithTimestamp(min_timestamp);
    
    // Test with maximum valid timestamp within bounds
    uint64_t max_timestamp = createTimestamp(15, 0xFFFFFFFFFFFFFE);
    sendDataWithTimestamp(max_timestamp);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Test reading minimum timestamp
    {
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, min_timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Should succeed reading minimum timestamp";
        EXPECT_TRUE(found) << "Minimum timestamp item should be found";
    }
    
    // Test reading maximum timestamp
    {
        libshm_media_head_param_t read_head;
        libshm_media_item_param_t read_item;
        bool found = false;
        bool foundPts = false;
        
        int result = LibShmMediaReadItemWithTvutimestamp(hReader, max_timestamp, 0, 0, &found, &foundPts, &read_head, &read_item);
        
        EXPECT_GT(result, 0) << "Should succeed reading maximum timestamp";
        EXPECT_TRUE(found) << "Maximum timestamp item should be found";
    }
}
