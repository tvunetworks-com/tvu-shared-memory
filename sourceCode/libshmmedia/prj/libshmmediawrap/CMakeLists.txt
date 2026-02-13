cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0015 NEW)

project(shmmediawrap)
set(PROJECT_NAME shmmediawrap)

include(../../build/common.cmake)
#include(../../build/common.cmake OPTIONAL RESULT_VARIABLE THE_INCLUDE_RESULT)
#if(NOT THE_INCLUDE_RESULT)
#    set(CMAKE_CXX_FLAGS "-DTVU_LINUX")
#endif()

set(PLATFORM ${CMAKE_PLATFORM_NAME})
set(PRJ_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_SOURCE_DIR ${PRJ_PATH}/src)

set(DEP1_PATH ${PRJ_PATH}/../libshmUtil)
set(DEP2_PATH ${PRJ_PATH}/../libsharememory)
set(DEP3_PATH ${PRJ_PATH}/../libshmmediaProto)
set(DEP4_PATH ${PRJ_PATH}/../libvaItemSharedMemory)
set(DEP5_PATH ${PRJ_PATH}/../libtvufourcc)
set(DEP6_PATH ${PRJ_PATH}/../libshmmedia)

SET(INCLUDE_LIST
    ${PRJ_PATH}/../../include
    ${PRJ_PATH}/include
    ${PRJ_PATH}/src
    ${PRJ_PATH}/src/include
    ${DEP3_PATH}/include
    ${DEP6_PATH}/include
)

INCLUDE_DIRECTORIES(${INCLUDE_LIST})

SET(PATH_SRC ${PROJECT_SOURCE_DIR})

AUX_SOURCE_DIRECTORY(${PATH_SRC} SRC_LIST)

file (GLOB HEADERS "${PATH_SRC}/*.h" "${PROJECT_INCLUDE_DIR}/*.h")
source_group("Headers" FILES ${HEADERS})


SET(LIB_DIRECTORY_LIST
    ${DEP1_PATH}/lib/${PLATFORM}/${CMAKE_BUILD_TYPE}
    ${DEP2_PATH}/lib/${PLATFORM/${CMAKE_BUILD_TYPE}}
    ${DEP3_PATH}/lib/${PLATFORM/${CMAKE_BUILD_TYPE}}
    ${DEP4_PATH}/lib/${PLATFORM/${CMAKE_BUILD_TYPE}}
    ${DEP5_PATH}/lib/${PLATFORM/${CMAKE_BUILD_TYPE}}
    ${DEP6_PATH}/lib/${PLATFORM/${CMAKE_BUILD_TYPE}}
    )

link_directories(${PROJECT_NAME} ${LIB_DIRECTORY_LIST})

SET(LIBRARY_OUTPUT_PATH ${PRJ_PATH}/lib/${PLATFORM}/${CMAKE_BUILD_TYPE})
ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${SRC_LIST} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    "shmmedia"
    "shmmediaProto"
    "sharememory"
    "vaItemSharedMemory"
    "shmUtil"
    "tvuMediaInfo"
    )


