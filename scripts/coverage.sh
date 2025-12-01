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

if ! command -v jq &> /dev/null; then
    echo ">>> Installing jq for JSON parsing..."
    sudo apt-get update && sudo apt-get install -y jq
fi

if ! dpkg -s libstdc++-14-dev &> /dev/null 2>&1; then
    echo ">>> Installing libstdc++ development files..."
    sudo apt-get update && sudo apt-get install -y libstdc++-14-dev
fi

echo ">>> Configuring build with coverage..."
if [[ -d "$BUILD_DIR" && -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    echo ">>> Using cached build directory..."
    cd "$BUILD_DIR"
else
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    CMAKE_ARGS="-DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF"

    if [[ -n "$VCPKG_ROOT" ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DUSE_SFML=ON"
    fi

    cmake $CMAKE_ARGS "$PROJECT_ROOT"
fi

echo ""
echo ">>> Building project..."
make -j$(nproc)

echo ""
echo ">>> Running tests..."
ctest --output-on-failure

echo ""
echo ">>> Generating coverage data..."

if $GENERATE_HTML; then
    mkdir -p "$COVERAGE_DIR"

    LCOV_OPTS="--rc branch_coverage=1 --ignore-errors mismatch,gcov,inconsistent,negative,unused"

    lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" $LCOV_OPTS

    lcov --remove "$COVERAGE_DIR/coverage.info" \
        '/usr/*' \
        '*/build-coverage/_deps/*' \
        '*/tests/*' \
        '*/googletest/*' \
        --output-file "$COVERAGE_DIR/coverage.info" \
        $LCOV_OPTS

    echo ""
    echo ">>> Generating HTML report..."
    genhtml "$COVERAGE_DIR/coverage.info" \
        --output-directory "$COVERAGE_DIR/html" \
        --branch-coverage \
        --title "R-Type Code Coverage" \
        --legend \
        --ignore-errors inconsistent

    echo ""
    echo ">>> Checking coverage requirements..."

    COVERAGE_CONFIG="$SCRIPT_DIR/coverage_config.json"
    if [[ ! -f "$COVERAGE_CONFIG" ]]; then
        echo "Error: Coverage config file not found: $COVERAGE_CONFIG"
        exit 1
    fi

    LINES_THRESHOLD=$(jq -r '.lines' "$COVERAGE_CONFIG")
    FUNCTIONS_THRESHOLD=$(jq -r '.functions' "$COVERAGE_CONFIG")
    BRANCHES_THRESHOLD=$(jq -r '.branches' "$COVERAGE_CONFIG")

    if [[ "$LINES_THRESHOLD" == "null" || "$FUNCTIONS_THRESHOLD" == "null" || "$BRANCHES_THRESHOLD" == "null" ]]; then
        echo "Error: Invalid coverage config file"
        exit 1
    fi

    echo "Coverage thresholds: Lines ≥${LINES_THRESHOLD}%, Functions ≥${FUNCTIONS_THRESHOLD}%, Branches ≥${BRANCHES_THRESHOLD}%"

    COVERAGE_OUTPUT=$(lcov --summary "$COVERAGE_DIR/coverage.info" $LCOV_OPTS 2>/dev/null)
    echo "$COVERAGE_OUTPUT"

    LINES_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "lines......:" | sed -E 's/.*lines\.\.\.\.\.\.\.: ([0-9]+\.[0-9]+)%.*/\1/' | head -1)
    FUNCTIONS_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "functions..:" | sed -E 's/.*functions\.\.: ([0-9]+\.[0-9]+)%.*/\1/' | head -1)
    BRANCHES_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "branches....:" | sed -E 's/.*branches\.\.\.\.: ([0-9]+\.[0-9]+)%.*/\1/' | head -1)

    if [[ -z "$LINES_COVERAGE" || -z "$FUNCTIONS_COVERAGE" || -z "$BRANCHES_COVERAGE" ]]; then
        echo "Error: Could not parse coverage percentages"
        exit 1
    fi

    LINES_OK=$(echo "$LINES_COVERAGE >= $LINES_THRESHOLD" | awk '{if ($1) print "true"; else print "false"}')
    FUNCTIONS_OK=$(echo "$FUNCTIONS_COVERAGE >= $FUNCTIONS_THRESHOLD" | awk '{if ($1) print "true"; else print "false"}')
    BRANCHES_OK=$(echo "$BRANCHES_COVERAGE >= $BRANCHES_THRESHOLD" | awk '{if ($1) print "true"; else print "false"}')

    echo ""
    echo "Global line coverage: ${LINES_COVERAGE}%"
    echo "Global function coverage: ${FUNCTIONS_COVERAGE}%"
    echo "Global branch coverage: ${BRANCHES_COVERAGE}%"

    COVERAGE_MET=true
    if [[ "$LINES_OK" == "true" ]]; then
        echo "✓ Line coverage requirement met (≥${LINES_THRESHOLD}%)"
    else
        echo "✗ Line coverage requirement NOT met (≥${LINES_THRESHOLD}%)"
        COVERAGE_MET=false
    fi

    if [[ "$FUNCTIONS_OK" == "true" ]]; then
        echo "✓ Function coverage requirement met (≥${FUNCTIONS_THRESHOLD}%)"
    else
        echo "✗ Function coverage requirement NOT met (≥${FUNCTIONS_THRESHOLD}%)"
        COVERAGE_MET=false
    fi

    if [[ "$BRANCHES_OK" == "true" ]]; then
        echo "✓ Branch coverage requirement met (≥${BRANCHES_THRESHOLD}%)"
    else
        echo "✗ Branch coverage requirement NOT met (≥${BRANCHES_THRESHOLD}%)"
        COVERAGE_MET=false
    fi

    if [[ "$COVERAGE_MET" == "false" ]]; then
        echo "Please improve test coverage to meet the configured thresholds."
        exit 1
    fi

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
    echo ""
    echo "=== Coverage Summary ==="
    echo "For detailed HTML report, run: $0 --html"
    echo ""

    find . -name "*.gcda" -exec gcov {} \; 2>/dev/null | grep -E "^File|^Lines" | head -40
fi

echo ""
echo "Done!"
