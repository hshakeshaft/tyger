#!/usr/bin/env sh
set -ex

if [ ! -d build ]; then
    mkdir build
fi

cmake -S . -B build
cmake --build build -j 8
./build/test_tyger
