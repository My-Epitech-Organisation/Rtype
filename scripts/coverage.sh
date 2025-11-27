#!/bin/bash
# ============================================================================
# R-Type - Coverage Report Generator
# ============================================================================
# Usage: ./scripts/coverage.sh [--html] [--open]
#   --html  Generate HTML report (requires lcov)
#   --open  Open HTML report in browser
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-coverage"
COVERAGE_DIR="$PROJECT_ROOT/coverage"

# Parse arguments
GENERATE_HTML=false
OPEN_REPORT=false

for arg in "$@"; do
    case $arg in
        --html)
            GENERATE_HTML=true
            ;;
        --open)
            OPEN_REPORT=true
            GENERATE_HTML=true
            ;;
    esac
done

echo "=== R-Type Coverage Report ==="
echo ""

# Check for required tools
if $GENERATE_HTML; then
    if ! command -v lcov &> /dev/null; then
        echo "Error: lcov is required for HTML reports"
        echo "Install with: sudo apt-get install lcov"
        exit 1
    fi
    if ! command -v genhtml &> /dev/null; then
        echo "Error: genhtml is required for HTML reports"
        echo "Install with: sudo apt-get install lcov"
        exit 1
    fi
fi

# Check for libstdc++ (required for clang++ linking)
if ! dpkg -s libstdc++-14-dev &> /dev/null 2>&1; then
    echo ">>> Installing libstdc++ development files..."
    sudo apt-get update && sudo apt-get install -y libstdc++-14-dev
fi

# Clean and create build directory
echo ">>> Configuring build with coverage..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with coverage enabled (force GCC for better coverage support)
cmake -DENABLE_COVERAGE=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_C_COMPILER=gcc \
      "$PROJECT_ROOT"

# Build
echo ""
echo ">>> Building project..."
make -j$(nproc)

# Run tests
echo ""
echo ">>> Running tests..."
ctest --output-on-failure

# Generate coverage report
echo ""
echo ">>> Generating coverage data..."

if $GENERATE_HTML; then
    mkdir -p "$COVERAGE_DIR"

    # lcov 2.0+ options for compatibility with GCC 13+
    LCOV_OPTS="--rc branch_coverage=1 --ignore-errors mismatch,gcov,inconsistent,negative,unused"

    # Capture coverage data
    lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" $LCOV_OPTS

    # Remove external libraries and test files from coverage
    lcov --remove "$COVERAGE_DIR/coverage.info" \
        '/usr/*' \
        '*/build-coverage/_deps/*' \
        '*/tests/*' \
        '*/googletest/*' \
        --output-file "$COVERAGE_DIR/coverage.info" \
        $LCOV_OPTS

    # Generate HTML report
    echo ""
    echo ">>> Generating HTML report..."
    genhtml "$COVERAGE_DIR/coverage.info" \
        --output-directory "$COVERAGE_DIR/html" \
        --branch-coverage \
        --title "R-Type Code Coverage" \
        --legend \
        --ignore-errors inconsistent

    echo ""
    echo "=== Coverage Report Generated ==="
    echo "HTML Report: $COVERAGE_DIR/html/index.html"

    # Summary
    lcov --summary "$COVERAGE_DIR/coverage.info" $LCOV_OPTS

    if $OPEN_REPORT; then
        if command -v xdg-open &> /dev/null; then
            xdg-open "$COVERAGE_DIR/html/index.html"
        elif command -v open &> /dev/null; then
            open "$COVERAGE_DIR/html/index.html"
        else
            echo "Could not open browser automatically"
        fi
    fi
else
    # Just show gcov summary
    echo ""
    echo "=== Coverage Summary ==="
    echo "For detailed HTML report, run: $0 --html"
    echo ""

    # Find and process gcda files
    find . -name "*.gcda" -exec gcov {} \; 2>/dev/null | grep -E "^File|^Lines" | head -40
fi

echo ""
echo "Done!"
