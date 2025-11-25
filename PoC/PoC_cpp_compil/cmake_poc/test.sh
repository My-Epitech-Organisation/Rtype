#!/usr/bin/env bash
# Quick test script for mini PoC

set -e

echo "=== Mini PoC: CMake Strict Compilation ==="
echo ""

# Test with current compiler
echo "Building with default compiler..."
cmake -B build -G Ninja
cmake --build build

echo ""
echo "Running executable..."
./build/bin/hello_strict

echo ""
echo "✓ Mini PoC successful!"
