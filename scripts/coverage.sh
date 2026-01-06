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

# Detect package manager
detect_package_manager() {
    if command -v apt-get &> /dev/null; then
        echo "apt"
    elif command -v dnf &> /dev/null; then
        echo "dnf"
    elif command -v yum &> /dev/null; then
        echo "yum"
    else
        echo "unknown"
    fi
}

install_package() {
    local pkg_apt="$1"
    local pkg_dnf="$2"
    local pm=$(detect_package_manager)
    
    case $pm in
        apt)
            sudo apt-get update && sudo apt-get install -y "$pkg_apt"
            ;;
        dnf)
            sudo dnf install -y "$pkg_dnf"
            ;;
        yum)
            sudo yum install -y "$pkg_dnf"
            ;;
        *)
            echo "Warning: Unknown package manager, please install $pkg_apt manually"
            ;;
    esac
}

if $GENERATE_HTML; then
    if ! command -v lcov &> /dev/null; then
        echo "Error: lcov is required for HTML reports"
        echo "Install with: sudo apt-get install lcov (Ubuntu) or sudo dnf install lcov (Fedora)"
        exit 1
    fi
    if ! command -v genhtml &> /dev/null; then
        echo "Error: genhtml is required for HTML reports"
        echo "Install with: sudo apt-get install lcov (Ubuntu) or sudo dnf install lcov (Fedora)"
        exit 1
    fi
fi

if ! command -v jq &> /dev/null; then
    echo ">>> Installing jq for JSON parsing..."
    install_package "jq" "jq"
fi

echo ">>> Configuring build with coverage..."

# Check if build directory exists and is valid (has CMakeCache.txt and build.ninja)
if [[ -d "$BUILD_DIR" && -f "$BUILD_DIR/CMakeCache.txt" && -f "$BUILD_DIR/build.ninja" ]]; then
    echo ">>> Using cached build directory..."
    cd "$BUILD_DIR"
else
    # Clean up any partial build
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    CMAKE_ARGS="-G Ninja -DBUILD_GRAPHICAL_TESTS=OFF -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF -DDISABLE_ASAN=ON"

    # Try to find vcpkg installation in order of preference:
    # 1. Cached vcpkg_installed from CI build
    # 2. Submodule in external/vcpkg
    # 3. VCPKG_ROOT environment variable
    if [[ -d "$PROJECT_ROOT/build/vcpkg_installed" ]]; then
        echo ">>> Using cached vcpkg installation from build..."
        # Copy the cached vcpkg_installed to build-coverage to avoid rebuilding
        if [[ ! -d "$BUILD_DIR/vcpkg_installed" ]]; then
            echo ">>> Copying cached vcpkg packages to build-coverage..."
            cp -r "$PROJECT_ROOT/build/vcpkg_installed" "$BUILD_DIR/"
        fi
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$PROJECT_ROOT/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CMAKE_ARGS="$CMAKE_ARGS -DUSE_SFML=ON"
    elif [[ -d "$PROJECT_ROOT/external/vcpkg" ]]; then
        echo ">>> Using vcpkg submodule..."
        # Ensure submodule is initialized
        if [[ ! -f "$PROJECT_ROOT/external/vcpkg/bootstrap-vcpkg.sh" ]]; then
            echo ">>> Initializing vcpkg submodule..."
            git -C "$PROJECT_ROOT" submodule update --init external/vcpkg
        fi
        # Bootstrap vcpkg if needed
        if [[ ! -f "$PROJECT_ROOT/external/vcpkg/vcpkg" ]]; then
            echo ">>> Bootstrapping vcpkg..."
            "$PROJECT_ROOT/external/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
        fi
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$PROJECT_ROOT/external/vcpkg/scripts/buildsystems/vcpkg.cmake -DUSE_SFML=ON"
    elif [[ -n "$VCPKG_ROOT" ]]; then
        echo ">>> Using VCPKG_ROOT: $VCPKG_ROOT"
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DUSE_SFML=ON"
    else
        echo "Error: Could not find vcpkg installation"
        echo "Please either:"
        echo "  1. Run 'git submodule update --init external/vcpkg'"
        echo "  2. Set VCPKG_ROOT environment variable"
        exit 1
    fi

    cmake $CMAKE_ARGS "$PROJECT_ROOT"
fi

echo ""
echo ">>> Building project..."
ninja

echo ""
echo ">>> Running tests..."
ctest --output-on-failure

echo ""
echo ">>> Generating coverage data..."

if $GENERATE_HTML; then
    mkdir -p "$COVERAGE_DIR"

    LCOV_OPTS="--rc branch_coverage=1 --ignore-errors mismatch,gcov,inconsistent,negative,unused,source"

    lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" $LCOV_OPTS

    lcov --remove "$COVERAGE_DIR/coverage.info" \
        '/usr/*' \
        '*/build-coverage/_deps/*' \
        '*/build/_deps/*' \
        '*/external/*' \
        '*/tests/*' \
        '*/googletest/*' \
        '*/asio/*' \
        '*/boost/*' \
        '*/vcpkg_installed/*' \
        '*/build/*' \
        '*/build-coverage/*' \
        '*/docs/*' \
        '*/scripts/*' \
        '*/assets/*' \
        '*/dev_ressources/*' \
        '*/cmake/*' \
        '*/tools/*' \
        '*/saves/*' \
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

    LINES_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "lines" | sed -E 's/.* ([0-9]+\.[0-9]+)%.*/\1/' | head -1)
    FUNCTIONS_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "functions" | sed -E 's/.* ([0-9]+\.[0-9]+)%.*/\1/' | head -1)
    BRANCHES_COVERAGE=$(echo "$COVERAGE_OUTPUT" | grep "branches" | sed -E 's/.* ([0-9]+\.[0-9]+)%.*/\1/' | head -1)

    if [[ -z "$LINES_COVERAGE" || -z "$FUNCTIONS_COVERAGE" || -z "$BRANCHES_COVERAGE" ]]; then
        echo "Error: Could not parse coverage percentages"
        exit 1
    fi

    LINES_OK=$(awk "BEGIN {if ($LINES_COVERAGE >= $LINES_THRESHOLD) print \"true\"; else print \"false\"}")
    FUNCTIONS_OK=$(awk "BEGIN {if ($FUNCTIONS_COVERAGE >= $FUNCTIONS_THRESHOLD) print \"true\"; else print \"false\"}")
    BRANCHES_OK=$(awk "BEGIN {if ($BRANCHES_COVERAGE >= $BRANCHES_THRESHOLD) print \"true\"; else print \"false\"}")

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
    # Generate coverage summary using lcov (text mode)
    mkdir -p "$COVERAGE_DIR"

    LCOV_OPTS="--rc branch_coverage=1 --ignore-errors mismatch,gcov,inconsistent,negative,unused"

    echo ">>> Capturing coverage data..."
    lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" $LCOV_OPTS

    echo ">>> Filtering coverage data..."
    lcov --remove "$COVERAGE_DIR/coverage.info" \
        '/usr/*' \
        '*/build-coverage/_deps/*' \
        '*/build/_deps/*' \
        '*/external/*' \
        '*/tests/*' \
        '*/googletest/*' \
        '*/asio/*' \
        '*/boost/*' \
        '*/vcpkg_installed/*' \
        '*/build/*' \
        '*/build-coverage/*' \
        '*/docs/*' \
        '*/scripts/*' \
        '*/assets/*' \
        '*/dev_ressources/*' \
        '*/cmake/*' \
        '*/tools/*' \
        '*/saves/*' \
        --output-file "$COVERAGE_DIR/coverage.info" \
        $LCOV_OPTS

    echo ""
    echo "=== Coverage Summary ==="
    lcov --summary "$COVERAGE_DIR/coverage.info" $LCOV_OPTS 2>&1
    echo ""
    echo "For detailed HTML report, run: $0 --html"
fi

echo ""
echo "=== Coverage Results ==="
if $GENERATE_HTML; then
    echo "HTML report: $COVERAGE_DIR/html/index.html"
else
    echo "Coverage info: $COVERAGE_DIR/coverage.info"
    echo "Tip: Run with --html for a detailed HTML report"
fi
echo ""
echo "Done!"
