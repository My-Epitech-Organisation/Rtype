#!/usr/bin/env bash
set -euo pipefail
mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j
