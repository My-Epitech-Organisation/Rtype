#!/bin/bash

# Comprehensive Test Automation Script for Rtype
# This script runs all test suites and generates detailed reports

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
REPORTS_DIR="$BUILD_DIR/test_reports"

echo "=========================================="
echo "Rtype Comprehensive Test Suite"
echo "=========================================="
echo "Project Root: $PROJECT_ROOT"
echo "Build Directory: $BUILD_DIR"
echo "Reports Directory: $REPORTS_DIR"
echo ""

mkdir -p "$REPORTS_DIR"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    local status=$1
    local message=$2
    case $status in
        "success")
            echo -e "${GREEN}✅ $message${NC}"
            ;;
        "warning")
            echo -e "${YELLOW}⚠️  $message${NC}"
            ;;
        "error")
            echo -e "${RED}❌ $message${NC}"
            ;;
        "info")
            echo -e "${BLUE}ℹ️  $message${NC}"
            ;;
    esac
}

if [ ! -d "$BUILD_DIR" ]; then
    print_status "error" "Build directory not found: $BUILD_DIR"
    echo "Please run build first:"
    echo "  mkdir -p build && cd build && cmake .. && make -j$(nproc)"
    exit 1
fi

cd "$BUILD_DIR"

print_status "info" "Building all tests..."
if ! cmake --build . --target build_tests -j$(nproc) 2>/dev/null; then
    print_status "warning" "build_tests target not found, building entire project..."
    if ! cmake --build . -j$(nproc); then
        print_status "error" "Failed to build project"
        exit 1
    fi
fi

run_test_suite() {
    local test_name=$1
    local test_executable=$2
    local report_file="$REPORTS_DIR/${test_name}_results.xml"

    if [ ! -f "$test_executable" ]; then
        print_status "warning" "$test_name executable not found, skipping..."
        return 1
    fi

    print_status "info" "Running $test_name tests..."
    if $test_executable --gtest_output=xml:$report_file > /dev/null 2>&1; then
        print_status "success" "$test_name tests passed"
        return 0
    else
        print_status "error" "$test_name tests failed"
        return 1
    fi
}

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

print_status "info" "Running all unit tests with ctest..."
if ctest --output-on-failure --output-junit $REPORTS_DIR/unit_tests_results.xml > /dev/null 2>&1; then
    UNIT_TEST_COUNT=$(ctest -N 2>/dev/null | grep -E "Total Tests: [0-9]+" | sed 's/.*Total Tests: \([0-9]*\).*/\1/' || echo "0")
    if [ "$UNIT_TEST_COUNT" -gt 0 ]; then
        print_status "success" "Unit tests passed ($UNIT_TEST_COUNT tests)"
        TOTAL_TESTS=$((TOTAL_TESTS + UNIT_TEST_COUNT))
        PASSED_TESTS=$((PASSED_TESTS + UNIT_TEST_COUNT))
    else
        print_status "warning" "Could not determine unit test count"
    fi
else
    print_status "error" "Some unit tests failed"
    FAILED_TESTS=$((FAILED_TESTS + 1))
fi

if run_test_suite "integration" "tests/integration/test_integration"; then
    INTEGRATION_COUNT=$(grep -c "<testcase" "$REPORTS_DIR/integration_results.xml" 2>/dev/null || echo "0")
    TOTAL_TESTS=$((TOTAL_TESTS + INTEGRATION_COUNT))
    PASSED_TESTS=$((PASSED_TESTS + INTEGRATION_COUNT))
else
    FAILED_TESTS=$((FAILED_TESTS + 1))
fi

SUMMARY_FILE="$REPORTS_DIR/test_summary.txt"
cat > "$SUMMARY_FILE" << EOF
==========================================
Rtype Test Suite Summary Report
Generated: $(date)
==========================================

Test Results:
- Total Tests: $TOTAL_TESTS
- Passed: $PASSED_TESTS
- Failed: $FAILED_TESTS
- Success Rate: $((PASSED_TESTS * 100 / TOTAL_TESTS))%

Test Suites Executed:
EOF

if [ -f "$REPORTS_DIR/unit_tests_results.xml" ]; then
    echo "- Unit Tests: ✅ Executed" >> "$SUMMARY_FILE"
else
    echo "- Unit Tests: ❌ Not found" >> "$SUMMARY_FILE"
fi

if [ -f "$REPORTS_DIR/integration_results.xml" ]; then
    echo "- Integration Tests: ✅ Executed" >> "$SUMMARY_FILE"
else
    echo "- Integration Tests: ❌ Not found" >> "$SUMMARY_FILE"
fi

cat >> "$SUMMARY_FILE" << EOF

Reports Location: $REPORTS_DIR
- Unit Tests: unit_tests_results.xml
- Integration Tests: integration_results.xml
- Summary: test_summary.txt

==========================================
EOF

echo ""
print_status "info" "Test execution completed!"
echo "📊 Summary:"
echo "   Total Tests: $TOTAL_TESTS"
echo "   Passed: $PASSED_TESTS"
echo "   Failed: $FAILED_TESTS"
if [ $TOTAL_TESTS -gt 0 ]; then
    SUCCESS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    echo "   Success Rate: ${SUCCESS_RATE}%"
fi
echo ""
echo "📁 Detailed reports available in: $REPORTS_DIR"
echo "📄 Summary report: $SUMMARY_FILE"

if [ $FAILED_TESTS -eq 0 ]; then
    print_status "success" "All test suites completed successfully!"
    exit 0
else
    print_status "error" "Some test suites failed. Check reports for details."
    exit 1
fi