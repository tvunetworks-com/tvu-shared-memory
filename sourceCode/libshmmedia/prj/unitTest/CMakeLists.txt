cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0015 NEW)

project(unitTest)
set(PROJECT_NAME unitTest)
set(PROJECT_DIR ${unitTest_SOURCE_DIR})
set(PRJ_PATH ${PROJECT_DIR})
set(DEPENDS_DIR ${PROJECT_DIR}/../../depends)

if(ENABLE_FEATURE_GCOVER)
set(CMAKE_CXX_STANDARD 11)
else()
set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ENABLE_FEATURE_GCOVER)
    set(CMAKE_BUILD_TYPE "Debug")
    add_link_options(--coverage)
endif()

if(NOT CMAKE_PLATFORM_NAME)
    set(CMAKE_PLATFORM_NAME "linux64x")
    add_definitions(-DTVU_LINUX)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -DGTEST_MAIN_ENTRANCE=1")

set(PLATFORM ${CMAKE_PLATFORM_NAME})

set(DEP1_PATH ${PRJ_PATH}/../libshmUtil)
set(DEP2_PATH ${PRJ_PATH}/../libsharememory)
set(DEP3_PATH ${PRJ_PATH}/../libshmmediaProto)
set(DEP4_PATH ${PRJ_PATH}/../libvaItemSharedMemory)
set(DEP5_PATH ${PRJ_PATH}/../libtvufourcc)
set(DEP6_PATH ${PRJ_PATH}/../libshmmedia)

SET(INCLUDE_LIST
    ${PROJECT_DIR}
    ${PROJECT_DIR}/../include
    ${PROJECT_DIR}/../src
    ${DEP1_PATH}/include
    ${DEP1_PATH}/src
    ${DEP1_PATH}/src/include
    ${DEP2_PATH}/include
    ${DEP2_PATH}/src
    ${DEP2_PATH}/src/include
    ${DEP3_PATH}/include
    ${DEP3_PATH}/src
    ${DEP3_PATH}/src/include
    ${DEP4_PATH}/include
    ${DEP4_PATH}/src
    ${DEP4_PATH}/src/include
    ${DEP5_PATH}/include
    ${DEP5_PATH}/src
    ${DEP5_PATH}/src/include
    ${DEP6_PATH}/include
    ${DEP6_PATH}/src
    ${DEP6_PATH}/src/include
)

INCLUDE_DIRECTORIES(${INCLUDE_LIST})

# SET(PATH_SRC 
#     ${PROJECT_DIR}/src
# )

#AUX_SOURCE_DIRECTORY(${PATH_SRC} SRC_LIST)

file(GLOB SRC_LIST
    "${PROJECT_DIR}/src/*.cpp"
    "${DEP1_PATH}/unitTest/src/*.cpp"
    "${DEP2_PATH}/unitTest/src/*.cpp"
    "${DEP3_PATH}/unitTest/src/*.cpp"
    "${DEP4_PATH}/unitTest/src/*.cpp"
    "${DEP5_PATH}/unitTest/src/*.cpp"
    "${DEP6_PATH}/unitTest/src/*.cpp"
)

file (GLOB HEADERS "${PATH_SRC}/*.h")
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

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_DIR}/bin/${CMAKE_BUILD_TYPE})
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADERS})

# setting runtimte path to origin exe file path
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-Wl,-rpath,'$ORIGIN'"
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "shmmedia"
    "shmmediaProto"
    "sharememory"
    "vaItemSharedMemory"
    "shmUtil"
    "tvuMediaInfo"
    "gtest"
    "gtest_main"
    "gcov"
    "z"
    "rt"
    "pthread;"
)

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
