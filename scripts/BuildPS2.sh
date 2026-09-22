#!/usr/bin/env bash
pushd "$(dirname "$(readlink -f "$0")")/.."
    mkdir build
    pushd build
        cmake .. -DCMAKE_BUILD_TYPE=$1 -DCMAKE_TOOLCHAIN_FILE=$PS2SDK/ps2dev.cmake -DPLATFORM=PS2
        make
    popd
popd