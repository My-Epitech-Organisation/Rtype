#!/usr/bin/env bash
# build.sh - Build R-Type project for release
#
# This script:
#   1. Checks/sets up vcpkg
#   2. Configures CMake with release preset
#   3. Builds the project
#   4. Copies executables to repository root
#
# Usage:
#   ./build.sh [linux|windows]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Determine platform
PLATFORM="${1:-linux}"

if [[ "$PLATFORM" != "linux" && "$PLATFORM" != "windows" ]]; then
    echo "Error: Platform must be 'linux' or 'windows'"
    echo "Usage: ./build.sh [linux|windows]"
    exit 1
fi

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           R-Type - Release Build Script                  ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Platform: $PLATFORM"
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
echo "→ Step 2/4: Configuring CMake (${PLATFORM}-release preset)..."
cd "$PROJECT_ROOT"

if [ -d "build" ]; then
    echo "  Cleaning existing build directory..."
    rm -rf build
fi

cmake --preset "${PLATFORM}-release"
echo "✓ CMake configuration complete"
echo ""

# Step 3: Build
echo "→ Step 3/4: Building project..."
cmake --build --preset "${PLATFORM}-release" -- -j$(nproc 2>/dev/null || echo 4)
echo "✓ Build complete"
echo ""

# Step 4: Copy executables to root
echo "→ Step 4/4: Copying executables to repository root..."

if [ "$PLATFORM" = "linux" ]; then
    CLIENT_EXE="build/src/client/r-type_client"
    SERVER_EXE="build/src/server/r-type_server"
elif [ "$PLATFORM" = "windows" ]; then
    CLIENT_EXE="build/src/client/r-type_client.exe"
    SERVER_EXE="build/src/server/r-type_server.exe"
fi

if [ -f "$CLIENT_EXE" ]; then
    cp "$CLIENT_EXE" "$PROJECT_ROOT/"
    echo "  ✓ Copied $(basename "$CLIENT_EXE")"
else
    echo "  ⚠ Warning: Client executable not found at $CLIENT_EXE"
fi

if [ -f "$SERVER_EXE" ]; then
    cp "$SERVER_EXE" "$PROJECT_ROOT/"
    echo "  ✓ Copied $(basename "$SERVER_EXE")"
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
