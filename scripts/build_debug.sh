#!/usr/bin/env bash
set -euo pipefail
mkdir -p build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -- -j
