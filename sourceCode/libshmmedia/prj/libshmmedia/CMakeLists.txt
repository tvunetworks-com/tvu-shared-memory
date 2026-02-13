cmake_minimum_required(VERSION 2.9)

PROJECT(shmmedia)

include(../../build/common.cmake)
#include(../../build/common.cmake OPTIONAL RESULT_VARIABLE THE_INCLUDE_RESULT)
#if(NOT THE_INCLUDE_RESULT)
#    set(CMAKE_CXX_FLAGS "-DTVU_LINUX")
#endif()


set(PLATFORM ${CMAKE_PLATFORM_NAME})
set(PRJ_PATH ${shmmedia_SOURCE_DIR})
set(PRJ_NAME "shmmedia")
set(DEP1_PATH ${PRJ_PATH}/../libshmUtil)
set(DEP2_PATH ${PRJ_PATH}/../libsharememory)
set(DEP3_PATH ${PRJ_PATH}/../libshmmediaProto)
set(DEP4_PATH ${PRJ_PATH}/../libvaItemSharedMemory)
set(DEP5_PATH ${PRJ_PATH}/../libtvufourcc)

SET(INCLUDE_LIST 
    ${PRJ_PATH}/../../include
    ${PRJ_PATH}/include
    ${PRJ_PATH}/src
    ${PRJ_PATH}/src/include
    ${DEP1_PATH}/include
    ${DEP1_PATH}/src/include
    ${DEP1_PATH}/src
    ${DEP2_PATH}/include
    ${DEP2_PATH}/src/include
    ${DEP2_PATH}/src
    ${DEP3_PATH}/include
    ${DEP3_PATH}/src
    ${DEP4_PATH}/include
    ${DEP4_PATH}/src/include
    ${DEP5_PATH}/include
	) 


SET(PATH_SRC ${PRJ_PATH}/src)
AUX_SOURCE_DIRECTORY(${PATH_SRC} SRC_LIST)

INCLUDE_DIRECTORIES(${INCLUDE_LIST})

file (GLOB HEADERS "${PRJ_PATH}/src/*.h" "${PRJ_PATH}/include/*.h" "${PRJ_PATH}/src/include/*.h")
source_group("Headers" FILES ${HEADERS})

SET(LIBRARY_OUTPUT_PATH ${PRJ_PATH}/lib/${PLATFORM}/${CMAKE_BUILD_TYPE})

ADD_LIBRARY(${PRJ_NAME}  ${SRC_LIST} ${HEADERS})

