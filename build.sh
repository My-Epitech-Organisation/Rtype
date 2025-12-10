#!/usr/bin/env bash
# build.sh - Build R-Type project for release (Linux)
#
# This script:
#   1. Checks/sets up vcpkg
#   2. Configures CMake with release preset
#   3. Builds the project
#   4. Copies executables to repository root
#
# Usage:
#   ./build.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           R-Type - Release Build Script (Linux)          ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Step 1: Check/setup vcpkg
echo "→ Step 1/4: Checking vcpkg installation..."

VCPKG_DIR="$PROJECT_ROOT/external/vcpkg"

if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/vcpkg" ]; then
    echo "✓ Using personal vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
elif [ -f "$VCPKG_DIR/vcpkg" ]; then
    echo "✓ vcpkg found in project: $VCPKG_DIR"
else
    echo "→ vcpkg not found, running setup..."
    bash "$PROJECT_ROOT/scripts/setup-vcpkg.sh"
fi

echo ""

# Step 2: Configure CMake
echo "→ Step 2/4: Configuring CMake (linux-release preset)..."
cd "$PROJECT_ROOT"

if [ -d "build" ]; then
    echo "  Cleaning existing build directory..."
    rm -rf build
fi

cmake --preset "linux-release"
echo "✓ CMake configuration complete"
echo ""

# Step 3: Build
echo "→ Step 3/4: Building project..."
cmake --build --preset "linux-release" -- -j$(nproc 2>/dev/null || echo 4) --target r-type_client r-type_server
echo "✓ Build complete"
echo ""

# Step 4: Copy executables to root
echo "→ Step 4/4: Copying executables to repository root..."

CLIENT_EXE="build/src/client/r-type_client"
SERVER_EXE="build/src/server/r-type_server"

if [ -f "$CLIENT_EXE" ]; then
    cp "$CLIENT_EXE" "$PROJECT_ROOT/"
    echo "  ✓ Copied r-type_client"
else
    echo "  ⚠ Warning: Client executable not found at $CLIENT_EXE"
fi

if [ -f "$SERVER_EXE" ]; then
    cp "$SERVER_EXE" "$PROJECT_ROOT/"
    echo "  ✓ Copied r-type_server"
else
    echo "  ⚠ Warning: Server executable not found at $SERVER_EXE"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                   Build Complete!                         ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Executables are now in the repository root:"
ls -lh "$PROJECT_ROOT"/r-type_* 2>/dev/null || echo "  (No executables found)"
echo ""
