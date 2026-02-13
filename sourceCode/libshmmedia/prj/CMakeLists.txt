cmake_minimum_required(VERSION 2.6.0)

project(libshmmedia)

if(ENABLE_FEATURE_GCOVER)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

add_subdirectory(libtvufourcc)
add_subdirectory(libshmUtil)
add_subdirectory(libsharememory)
add_subdirectory(libvaItemSharedMemory)
add_subdirectory(libshmmediaProto)
add_subdirectory(libshmmedia)
add_subdirectory(libshmmediawrap)
#add_subdirectory(unitTest)

