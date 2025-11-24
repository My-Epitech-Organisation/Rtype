#!/usr/bin/env bash
set -euo pipefail
./build-debug/r-type_client || ./build-release/r-type_client || echo "Build and run the client binary from build dir"
