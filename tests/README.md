# TVU Shared Memory Test Suite

This directory contains comprehensive test coverage for the TVU Shared Memory SDK APIs.

## Overview

The test suite validates the functionality of the open APIs provided by the tvu-shared-memory library, ensuring correctness, reliability, and proper integration between components.

## Test Structure

```
tests/
├── unit/                           # Unit tests for individual APIs
│   ├── test_libshm_media.cpp      # Tests for libshm_media APIs
│   └── test_libshm_media_protocol.cpp  # Tests for protocol APIs
├── integration/                    # Integration tests
│   └── test_read_write_integration.cpp # Read/write workflow tests
├── fixtures/                       # Test fixtures and utilities
├── CMakeLists.txt                 # CMake build configuration
├── Makefile                       # Make build configuration
└── README.md                      # This file
```

## Test Categories

### Unit Tests

#### test_libshm_media.cpp
Tests for core shared memory management APIs:
- `LibShmMediaCreate()` - Shared memory creation
- `LibShmMediaOpen()` - Opening existing shared memory
- `LibShmMediaDestroy()` - Cleanup and destruction
- `LibShmMediaGetVersion()` - Version information
- `LibShmMediaGetWriteIndex()` - Write index management
- `LibShmMediaGetReadIndex()` - Read index management
- `LibShmMediaSeekReadIndex()` - Reader positioning
- `LibShmMediaGetItemDataAddr()` - Data address retrieval
- `LibShmMediaGeHeadAddr()` - Header address retrieval

#### test_libshm_media_protocol.cpp
Tests for protocol and data structure APIs:
- `LibShmMediaHeadParamInit()` - Head parameter initialization
- `LibShmMediaItemParamInit()` - Item parameter initialization
- `LibShmMediaRawDataParamInit()` - Raw data parameter initialization
- `LibShmMediaProGetItemParamDataLen()` - Data length calculation
- `LibShmMediaProtoGetHeadVersion()` - Protocol version detection
- `LibShmMediaProtoWriteItemBuffer()` - Buffer writing
- `LibShmMediaProtoReadItemBufferLayout()` - Buffer reading
- `LibShmMediaProtoGetWriteItemBufferLayout()` - Layout retrieval
- Macro definitions and constants validation

### Integration Tests

#### test_read_write_integration.cpp
End-to-end tests for realistic workflows:
- Basic write and read operations
- Multiple frame sequences
- Combined audio and video data
- Reader seeking and positioning
- Timeout behavior on empty buffers
- Multi-threaded producer/consumer scenarios

## Prerequisites

### Required Dependencies

1. **Google Test Framework**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libgtest-dev

   # macOS
   brew install googletest

   # Or build from source
   git clone https://github.com/google/googletest.git
   cd googletest
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

2. **TVU Shared Memory Libraries**
   - Ensure the shared memory libraries are available in `../lib/`
   - Header files should be in the appropriate source directories

3. **Build Tools**
   - CMake 3.10 or higher (for CMake build)
   - Make (for Makefile build)
   - g++ or clang++ with C++11 support

## Building the Tests

### Option 1: Using Make

```bash
cd tests
make all
```

This will compile all test executables:
- `unit/test_libshm_media`
- `unit/test_libshm_media_protocol`
- `integration/test_read_write_integration`

### Option 2: Using CMake

```bash
cd tests
mkdir build
cd build
cmake ..
make
```

## Running the Tests

### Run All Tests

```bash
# Using Make
make test

# Using CMake/CTest
cd build
ctest --output-on-failure

# Or using custom target
make run_tests
```

### Run Specific Test Suites

```bash
# Unit tests only
make unit_tests

# Integration tests only
make integration_tests

# Individual test executable
./unit/test_libshm_media
./unit/test_libshm_media_protocol
./integration/test_read_write_integration
```

### Run with Verbose Output

```bash
# Google Test verbose mode
./unit/test_libshm_media --gtest_verbose

# Show all test names
./unit/test_libshm_media --gtest_list_tests

# Run specific test
./unit/test_libshm_media --gtest_filter=LibShmMediaTest.CreateSharedMemory
```

## Test Coverage

### API Coverage

The test suite covers the following open APIs:

**Core Memory Management:**
- ✅ LibShmMediaCreate
- ✅ LibShmMediaOpen
- ✅ LibShmMediaDestroy
- ✅ LibShmMediaGetVersion
- ✅ LibShmMediaGetHeadVersion
- ✅ LibShmMediaGetWriteIndex
- ✅ LibShmMediaGetReadIndex
- ✅ LibShmMediaSeekReadIndex
- ✅ LibShmMediaSeekReadIndexToWriteIndex
- ✅ LibShmMediaSeekReadIndexToRingStart
- ✅ LibShmMediaGetItemDataAddr
- ✅ LibShmMediaGeHeadAddr

**Protocol APIs:**
- ✅ LibShmMediaHeadParamInit
- ✅ LibShmMediaHeadParamRelease
- ✅ LibShmMediaItemParamInit
- ✅ LibShmMediaItemParamRelease
- ✅ LibShmMediaRawDataParamInit
- ✅ LibShmMediaRawHeadParamInit
- ✅ LibShmMediaProGetItemParamDataLen
- ✅ LibShmMediaProtoGetHeadVersion
- ✅ LibShmMediaProtoWriteItemBuffer
- ✅ LibShmMediaProtoReadItemBufferLayout
- ✅ LibShmMediaProtoReadItemBufferLayoutWithHeadVer
- ✅ LibShmMediaProtoGetWriteItemBufferLayout
- ✅ LibShmMediaProtoRequireWriteItemBufferLength

**Data I/O (Integration):**
- ✅ LibShmMediaWriteData
- ✅ LibShmMediaPollReadData
- ✅ LibShmMediaUpdateHead

### Test Scenarios

1. **Positive Tests:** Valid usage scenarios
2. **Negative Tests:** Invalid parameters and error conditions
3. **Boundary Tests:** Edge cases and limits
4. **Integration Tests:** Multi-component workflows
5. **Concurrency Tests:** Multi-threaded access patterns

## Interpreting Results

### Success Output
```
[==========] Running 15 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 15 tests from LibShmMediaTest
[ RUN      ] LibShmMediaTest.CreateSharedMemory
[       OK ] LibShmMediaTest.CreateSharedMemory (1 ms)
...
[==========] 15 tests from 1 test suite ran. (123 ms total)
[  PASSED  ] 15 tests.
```

### Failure Output
```
[ RUN      ] LibShmMediaTest.SomeTest
/path/to/test.cpp:42: Failure
Expected: (result) > (0), actual: -1 vs 0
[  FAILED  ] LibShmMediaTest.SomeTest (2 ms)
```

## Troubleshooting

### Common Issues

1. **Library Not Found**
   ```
   error while loading shared libraries: libshmmediawrap.so
   ```
   **Solution:** Set LD_LIBRARY_PATH or install libraries to system path
   ```bash
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../lib
   ```

2. **Google Test Not Found**
   ```
   fatal error: gtest/gtest.h: No such file or directory
   ```
   **Solution:** Install Google Test (see Prerequisites)

3. **Permission Denied on Shared Memory**
   ```
   Failed to create shared memory
   ```
   **Solution:** Check permissions or run with appropriate privileges

4. **Tests Timeout**
   - Increase timeout values in test code
   - Check system resources and shared memory limits
   ```bash
   # Check shared memory limits
   ipcs -l
   ```

## Extending the Tests

### Adding New Unit Tests

1. Create test file in `unit/` directory
2. Include necessary headers
3. Use Google Test macros (TEST, TEST_F, ASSERT_*, EXPECT_*)
4. Update CMakeLists.txt and Makefile
5. Run and verify

Example:
```cpp
#include <gtest/gtest.h>
#include "libshmmedia.h"

TEST(MyNewTest, TestSomething) {
    // Arrange
    // Act
    // Assert
    EXPECT_EQ(expected, actual);
}
```

### Adding Integration Tests

Follow the same pattern as existing integration tests, focusing on:
- Multi-component interactions
- Realistic usage scenarios
- Error handling and recovery
- Performance characteristics

## Continuous Integration

The test suite can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
test:
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v2
    - name: Install dependencies
      run: sudo apt-get install -y libgtest-dev
    - name: Build tests
      run: cd tests && make all
    - name: Run tests
      run: cd tests && make test
```

## Contributing

When contributing new tests:
1. Follow existing code style
2. Add both positive and negative test cases
3. Include meaningful test names and descriptions
4. Update this README with new test documentation
5. Ensure all tests pass before submitting

## License

Copyright 2025 TVU Networks

Licensed under the Apache License, Version 2.0.
See the LICENSE file in the root directory for details.

## Contact

For issues or questions about the test suite, please open an issue in the repository.
