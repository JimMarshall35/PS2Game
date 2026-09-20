#!/usr/bin/env bash

# Assumes script is in a ./scripts folder at the root of the repo, set repo root as cwd
pushd "$(dirname "$(readlink -f "$0")")/.."

    pushd tools/texture_maker
        mkdir -p build
        pushd build 
            cmake ..
            make 
        popd
    popd

    pushd tools/packer
        mkdir -p build
        pushd build 
            cmake -DCMAKE_BUILD_TYPE=Debug ..
            make 
        popd
    popd

popd