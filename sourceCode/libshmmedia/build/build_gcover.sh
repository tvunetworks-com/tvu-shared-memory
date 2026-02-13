#!/bin/bash

_main()
{
    build_dir=_debug_gcove_build
    mkdir -p ${build_dir}
    pushd $build_dir
    cp ../cmake_script/CMakeLists.txt ./

    #cmake_options=""
    #if [ "$ENABLE_FEATURE_SECURITY" = "ON" ]; then
    #    cmake_options="-DENABLE_FEATURE_SECURITY=ON"
    #    echo "Enabling security feature"
    #fi

    #cmake -DCMAKE_CXX_COMPILER="g++-11" -DCMAKE_C_COMPILER="gcc-11" ./ $cmake_options -DENABLE_FEATURE_GCOVER=on || exit 1
    cmake ./ $cmake_options -DENABLE_FEATURE_GCOVER=on || exit 1
    make VERBOSE=1 -j 4 || exit 1
    popd
}

pushd "$(dirname "$0")"
_main $*
popd


