#!/bin/bash

# Integration Test Runner Script
# This script automates running integration tests for the Rtype project

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=========================================="
echo "Rtype Integration Test Runner"
echo "=========================================="
echo "Project Root: $PROJECT_ROOT"
echo "Build Directory: $BUILD_DIR"
echo ""

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Build directory not found: $BUILD_DIR"
    echo "Please run build first:"
    echo "  mkdir -p build && cd build && cmake .. && make -j$(nproc)"
    exit 1
fi

cd "$BUILD_DIR"

if [ ! -f "tests/integration/test_integration" ]; then
    echo "❌ Integration test executable not found"
    echo "Building integration tests..."
    if ! cmake --build . --target test_integration -j$(nproc); then
        echo "❌ Failed to build integration tests"
        exit 1
    fi
fi

echo "🚀 Running Integration Tests..."
echo ""

if ./tests/integration/test_integration --gtest_output=xml:test_results_integration.xml; then
    echo ""
    echo "✅ All integration tests passed!"
    echo "📊 Test results saved to: test_results_integration.xml"
else
    echo ""
    echo "❌ Some integration tests failed!"
    echo "📊 Check test_results_integration.xml for details"
    exit 1
fi

echo ""
echo "=========================================="
echo "Integration Test Summary:"
echo "- Movement System Integration ✅"
echo "- SafeQueue Thread Safety ✅"
echo "- Component State Synchronization ✅"
echo "- ServerApp Initialization ✅"
echo "- ServerApp Client Management ✅"
echo "- ServerApp Metrics Integration ✅"
echo "- ServerApp Stop Functionality ✅"
echo "- ServerApp Client Timeout Handling ✅"
echo "- ServerApp Multiple Clients ✅"
echo "- Full Server Lifecycle ✅"
echo "- Performance Tests ✅"
echo "=========================================="