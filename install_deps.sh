#!/bin/bash
set -e

mkdir -p deps
cd deps
if [ ! -d "crc32c" ]; then git clone https://github.com/google/crc32c.git; fi
cd crc32c
git submodule update --init --recursive
mkdir -p build
cd build
cmake -DBUILD_SHARED_LIBS=OFF -DCRC32C_BUILD_TESTS=0 -DCRC32C_BUILD_BENCHMARKS=0 .. && make -j all install
cd ../../../

mkdir -p src/logging/build && cd src/logging/build && cmake ../ && make -j install && cd -
mkdir -p src/skvhttpclient/build && cd src/skvhttpclient/build && cmake ../ && make -j install && cd -
pip install --break-system-packages requests msgpack
