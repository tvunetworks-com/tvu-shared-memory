cmake_minimum_required(VERSION 2.6.0)

project(libshmmedia)

if(ENABLE_FEATURE_GCOVER)
    set(CMAKE_BUILD_TYPE "Debug")
    #add_compile_options(--coverage)
    #add_link_options(--coverage)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -O3")
endif()

add_subdirectory(../../prj prj)
add_subdirectory(../../prj/unitTest unitTest)

