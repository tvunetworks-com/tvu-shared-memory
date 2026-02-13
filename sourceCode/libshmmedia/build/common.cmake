
if(DEFINED LIBSHMMEDIA_COMMON_INCLUDED)
    return()
endif()
set(LIBSHMMEDIA_COMMON_INCLUDED TRUE)

set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_PLATFORM_NAME)
    set(CMAKE_PLATFORM_NAME "linux64x")
endif()

if(ENABLE_FEATURE_GCOVER)
    set(CMAKE_BUILD_TYPE "Debug")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)

if("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -O3")
endif()
if("${CMAKE_PLATFORM_NAME}" STREQUAL "linux64x")
    add_compile_definitions(TVU_LINUX)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-but-set-variable -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS -fPIC")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3")

