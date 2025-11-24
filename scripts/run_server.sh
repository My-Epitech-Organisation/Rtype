#!/usr/bin/env bash
set -euo pipefail
./build-debug/r-type_server || ./build-release/r-type_server || echo "Build and run the server binary from build dir"
