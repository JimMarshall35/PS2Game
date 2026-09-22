#!/usr/bin/env bash
pushd "$(dirname "$(readlink -f "$0")")/.."
    mkdir build
    pushd build
        cmake .. -DCMAKE_BUILD_TYPE=$1 -DPLATFORM=LINUX
        make
    popd
popd