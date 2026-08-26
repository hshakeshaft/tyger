#!/usr/bin/env sh
set -ex

if [ ! -d build ]; then
    cmake -S . -B build
fi

cmake --build build -j 4

./build/test_tyger
