#!/bin/bash

#
#FFMPEG_DIR=/cygdrive/d/linuxshare/package/cygwin/ffmpeg-1.0
#
#cd $FFMPEG_DIR
#bash build.sh && bash build.sh install
#
#cd -

if [ "_$1" = "_help" ];then
    echo -e "Usage:\n\
    [all | ]:build the solution \n\
    [install]:install program \n\
    [clean]:clean the solution \n\
    [CROSS_TOOLS=i686-pc-mingw32-]:cross tool choose\n
    [TARGET_OS=<MINGW|LINUX>:choose target os]
"
    exit 1
fi


CURRENT=$PWD
VERSION=${2:-1.0.0}
version_str=$VERSION
PLAT_FORM=${3:-linux64x}

if [[ -z "$PLATFORM" ]]; then
    PLATFORM=$PLAT_FORM
    export PLATFORM
fi


major=$(echo ${version_str} | awk 'BEGIN{FS=".";OFS=" "} {print $1}')
minor=$(echo ${version_str} | awk 'BEGIN{FS=".";OFS=" "} {print $2}')
rev=$(echo ${version_str} | awk 'BEGIN{FS=".";OFS=" "} {print $3}')
build_num=$(echo ${version_str} | awk 'BEGIN{FS=".";OFS=" "} {print $4}')
LIBRARY_SO_VER_STR="${major}.${minor}.${rev}"

if [ "_$build_num" != "_" ];then
    LIBRARY_SO_VER_STR="${major}.${minor}.${rev}.${build_num}"
fi

LIBSHMMEDIA_INCLUDE_VARIABLE_ITEM_SHM_M=${LIBSHMMEDIA_INCLUDE_VARIABLE_ITEM_SHM_M:-1}

BASE_PRJ_PATH=../prj
#DEPEND_BUILD_PATH=../dependlib/build

DEPEND_LIBTVUUTIL=$BASE_PRJ_PATH/libtvuutil
DEPEND_LIBTVUULOG=$BASE_PRJ_PATH/libtvulog
DEPEND_LIBVAITEM_SHM_PATH=$BASE_PRJ_PATH/libvaItemSharedMemory
DEPEND_LIBSHAREMEMORY=$BASE_PRJ_PATH/libsharememory
DEPEND_LIBSHMMEDIA_PROTO=$BASE_PRJ_PATH/libshmmediaProto
DEPEND_LIBSHMUTIL_PATH=$BASE_PRJ_PATH/libshmUtil
DEPEND_LIBSHMMEDIA=$BASE_PRJ_PATH/libshmmedia
DEPEND_LIBTVUFOURCC=$BASE_PRJ_PATH/libtvufourcc
#DEPEND_LIBTVUFOUNDATION=../dependlib/linux/libTvuFoundation
DEPEND_READ_SHM_PATH=$BASE_PRJ_PATH/read_shm
DEPEND_UNIT_TEST_PATH=$BASE_PRJ_PATH/unitTest

if [ "_$1" != "_install" ];
then

    if [ $LIBSHMMEDIA_INCLUDE_VARIABLE_ITEM_SHM_M -eq 1 ];then
        vishm_build_param=" VISHM_M=1"
    else
        vishm_build_param=""
    fi

    pushd $DEPEND_LIBSHMUTIL_PATH
    make $* $vishm_build_param || exit 1
    popd

    pushd $DEPEND_LIBVAITEM_SHM_PATH
    make $* $vishm_build_param || exit 1
    popd

    pushd $DEPEND_LIBSHAREMEMORY
    make $* $vishm_build_param || exit 1
    popd

    pushd $DEPEND_LIBSHMMEDIA_PROTO
    make $* $vishm_build_param VER="$VERSION" || exit 1
    popd

    pushd $DEPEND_LIBSHMMEDIA
    make $* -f Makefile_so $vishm_build_param VER="$VERSION" || exit 1
    popd

    pushd $DEPEND_LIBTVUFOURCC
    make $* || exit 1
    popd

    pushd $DEPEND_UNIT_TEST_PATH
    make $* -j 4 || exit 1
    popd
else

# install libshmmediawrap
    mkdir -p $CURRENT/libshmmediawrap
    mkdir -p $CURRENT/libshmmediawrap/lib
    mkdir -p $CURRENT/libshmmediawrap/include
    mkdir -p $CURRENT/libshmmediawrap/test
    mkdir -p $CURRENT/libshmmediawrap/bin
    cp -f $DEPEND_LIBVAITEM_SHM_PATH/lib/libvaItemSharedMemory.a $CURRENT/libshmmediawrap/lib
    cp -f $DEPEND_LIBSHMUTIL_PATH/lib/libshmUtil.a $CURRENT/libshmmediawrap/lib
    cp -f $DEPEND_LIBSHAREMEMORY/lib/libsharememory.a $CURRENT/libshmmediawrap/lib
    cp -f $DEPEND_LIBSHMMEDIA_PROTO/lib/*.a $CURRENT/libshmmediawrap/lib
    cp -f $DEPEND_LIBSHMMEDIA/lib/libshmmedia.a $CURRENT/libshmmediawrap/lib
    cp -f $DEPEND_LIBSHMMEDIA/lib/libshmmediawrap.so $CURRENT/libshmmediawrap/lib/libshmmediawrap.so.$LIBRARY_SO_VER_STR
    pushd $CURRENT/libshmmediawrap/lib
        ln -sf libshmmediawrap.so.$LIBRARY_SO_VER_STR libshmmediawrap.so.$major
        ln -sf libshmmediawrap.so.$major libshmmediawrap.so
    popd
    cp -f $DEPEND_LIBVAITEM_SHM_PATH/include/*.h $CURRENT/libshmmediawrap/include
    cp -f $DEPEND_LIBSHAREMEMORY/include/*.h $CURRENT/libshmmediawrap/include
    cp -f $DEPEND_LIBSHMMEDIA_PROTO/include/*.h $CURRENT/libshmmediawrap/include
    cp -f $DEPEND_LIBSHMMEDIA/include/*.h $CURRENT/libshmmediawrap/include
    cp -f $DEPEND_LIBSHMMEDIA/test_libshmmedia/test.cpp $CURRENT/libshmmediawrap/test
    cp -f $DEPEND_LIBSHMMEDIA/test_libshmmedia/Makefile_so $CURRENT/libshmmediawrap/test
    cp -f $DEPEND_LIBSHMMEDIA/test_libshmmedia/Makefile_ar $CURRENT/libshmmediawrap/test
    cp -rf $DEPEND_LIBSHMMEDIA/test_libshmmedia/variable_item_shm_test $CURRENT/libshmmediawrap/test
    cp -rf $DEPEND_LIBSHMMEDIA/test_libshmmedia/test_data_protocol $CURRENT/libshmmediawrap/test
    cp -rf $DEPEND_LIBSHMMEDIA/test_libshmmedia/test_raw_data $CURRENT/libshmmediawrap/test
    cp -rf $DEPEND_LIBSHMMEDIA/test_libshmmedia/control_data_test $CURRENT/libshmmediawrap/test
    cp -rf $DEPEND_LIBSHMMEDIA/test_libshmmedia/test_shm_tvulive $CURRENT/libshmmediawrap/test
    cp -f $DEPEND_LIBTVUFOURCC/include/*.h $CURRENT/libshmmediawrap/include
    cp -f $DEPEND_LIBTVUFOURCC/lib/*.a $CURRENT/libshmmediawrap/lib
    cp -rf $DEPEND_LIBTVUFOURCC/test $CURRENT/libshmmediawrap/test_libtvufourcc

    cp -f $DEPEND_UNIT_TEST_PATH/unitTest $CURRENT/libshmmediawrap/bin

    rm -rf `find $CURRENT/libshmmediawrap -name .svn`
    echo "$VERSION" >> $CURRENT/libshmmediawrap/version
    tar zcvf libshmmediawrap-$PLAT_FORM-$VERSION.tar.gz libshmmediawrap
fi

