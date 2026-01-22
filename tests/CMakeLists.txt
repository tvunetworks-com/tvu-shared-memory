cmake_minimum_required(VERSION 3.10)
project(tvu_shared_memory_tests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile definitions
add_definitions(-DTVU_LINUX)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libshmmedia/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libshmmediaProto/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libvaItemSharedMemory/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link directories - adjust based on where your libraries are
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libshmmedia/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libshmmediaProto/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libsharememory/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libshmUtil/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libtvufourcc/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../sourceCode/libshmmedia/prj/libvaItemSharedMemory/lib
)

# Set library names (adjust based on your actual libraries)
# Order matters for static libraries - dependencies should come after libraries that depend on them
set(SHM_MEDIA_LIBS shmmedia shmmediaProto sharememory vaItemSharedMemory shmUtil tvuMediaInfo z)

# Unit tests
add_executable(test_libshm_media
    unit/test_libshm_media.cpp
)
target_link_libraries(test_libshm_media
    ${SHM_MEDIA_LIBS}
    ${GTEST_LIBRARIES}
    ${PLATFORM_LIBS}
)

add_executable(test_libshm_media_protocol
    unit/test_libshm_media_protocol.cpp
)
target_link_libraries(test_libshm_media_protocol
    ${SHM_MEDIA_LIBS}
    ${GTEST_LIBRARIES}
    ${PLATFORM_LIBS}
)

# Integration tests
add_executable(test_read_write_integration
    integration/test_read_write_integration.cpp
)
target_link_libraries(test_read_write_integration
    ${SHM_MEDIA_LIBS}
    ${GTEST_LIBRARIES}
    ${PLATFORM_LIBS}
)

# Platform-specific libraries
if(APPLE)
    set(PLATFORM_LIBS pthread)
else()
    set(PLATFORM_LIBS pthread rt)
endif()

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME LibShmMediaTest COMMAND test_libshm_media)
add_test(NAME LibShmMediaProtocolTest COMMAND test_libshm_media_protocol)
add_test(NAME ReadWriteIntegrationTest COMMAND test_read_write_integration)

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_libshm_media test_libshm_media_protocol test_read_write_integration
    COMMENT "Running all tests"
)
