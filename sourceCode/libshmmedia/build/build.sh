#!/bin/bash

_main()
{
    build_dir=_release_build
    mkdir -p ${build_dir}
    pushd $build_dir
    cp ../cmake_script/CMakeLists.txt ./

    #cmake_options=""
    #if [ "$ENABLE_FEATURE_SECURITY" = "ON" ]; then
    #    cmake_options="-DENABLE_FEATURE_SECURITY=ON"
    #    echo "Enabling security feature"
    #fi

    cmake ./ $cmake_options || exit 1
    make -j 4 || exit 1
    popd
}

pushd "$(dirname "$0")"
_main $*
popd


