#!/bin/bash

# Run script for Colorblind Accessibility PoC

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
EXECUTABLE="${BUILD_DIR}/bin/colorblind_poc"

if [ ! -f "${EXECUTABLE}" ]; then
    echo "Executable not found. Building first..."
    "${SCRIPT_DIR}/build.sh"
fi

echo "=== Running Colorblind Accessibility PoC ==="
echo ""

"${EXECUTABLE}"
