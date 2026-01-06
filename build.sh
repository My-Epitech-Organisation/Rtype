#!/usr/bin/env bash
# build.sh - Build R-Type project for release (Linux)
#
# This script:
#   1. Attempts to build with vcpkg
#   2. Falls back to CPM if vcpkg is unavailable
#   3. Configures CMake with the appropriate preset
#   4. Builds the project
#   5. Copies executables to repository root
#
# Usage:
#   ./build.sh              # Full rebuild (clean + build, client and server only)
#   ./build.sh -t           # Full rebuild with tests
#   ./build.sh -r           # Incremental build (reuse existing build)
#   ./build.sh -r -t        # Incremental build with tests
#   ./build.sh -c           # Use CPM even if vcpkg is available
#   ./build.sh -c -r        # Incremental build with CPM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Parse arguments
BUILD_TESTS=false
INCREMENTAL_BUILD=false
FORCE_CPM=false
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--tests)
            BUILD_TESTS=true
            shift
            ;;
        -r|--reuse)
            INCREMENTAL_BUILD=true
            shift
            ;;
        -c|--cpm)
            FORCE_CPM=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [-t|--tests] [-r|--reuse] [-c|--cpm]"
            echo "  -t, --tests    Build with unit tests"
            echo "  -r, --reuse    Incremental build (reuse existing build directory)"
            echo "  -c, --cpm      Force CPM (skip vcpkg even if available)"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           R-Type - Release Build Script (Linux)          ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

if [ "$BUILD_TESTS" = true ]; then
    echo "→ Build mode: Client + Server + Tests"
else
    echo "→ Build mode: Client + Server"
fi
if [ "$INCREMENTAL_BUILD" = true ]; then
    echo "→ Build type: Incremental"
else
    echo "→ Build type: Full rebuild"
fi
echo ""

# ========================================================================
# Step 1: Determine dependency strategy (vcpkg first, CPM fallback)
# ========================================================================
echo "→ Step 1/4: Determining dependency strategy..."
echo ""

VCPKG_AVAILABLE=false
VCPKG_DIR="$PROJECT_ROOT/external/vcpkg"
CMAKE_PRESET="linux-release"
BUILD_DIR="build"

verify_vcpkg_ready() {
    local vcpkg_path="$1"

    if [ ! -f "$vcpkg_path/vcpkg" ]; then
        return 1
    fi

    local vcpkg_output
    if ! vcpkg_output=$("$vcpkg_path/vcpkg" version 2>&1); then
        printf '%s\n' "$vcpkg_output" > "$PROJECT_ROOT/vcpkg_version_error.log"
        return 1
    fi

    return 0
}

if [ "$FORCE_CPM" = false ]; then
    # Check if vcpkg submodule is initialized
    if [ ! -d "$VCPKG_DIR" ] || [ ! -f "$VCPKG_DIR/.git" ]; then
        echo "  ⚠ vcpkg submodule not initialized."
        echo "    Initializing vcpkg submodule..."
        if git submodule update --init --recursive external/vcpkg; then
            echo "    ✓ vcpkg submodule initialized"
        else
            echo "  ⚠ Failed to initialize vcpkg submodule"
            echo "  → Falling back to CPM..."
            VCPKG_AVAILABLE=false
            FORCE_CPM=true
        fi
    fi
    
    if [ "$FORCE_CPM" = false ]; then
        if [ -n "$VCPKG_ROOT" ] && verify_vcpkg_ready "$VCPKG_ROOT"; then
            echo "  ✓ Using personal vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
            VCPKG_AVAILABLE=true
        elif verify_vcpkg_ready "$VCPKG_DIR"; then
            echo "  ✓ vcpkg found and bootstrapped in project: $VCPKG_DIR"
            VCPKG_AVAILABLE=true
        elif [ -d "$VCPKG_DIR" ] && [ ! -f "$VCPKG_DIR/vcpkg" ]; then
            echo "  ⚠ vcpkg directory exists but not bootstrapped."
            echo "    Attempting to bootstrap vcpkg..."
            if cd "$VCPKG_DIR" && ./bootstrap-vcpkg.sh; then
                echo "    ✓ vcpkg bootstrapped successfully"
                if verify_vcpkg_ready "$VCPKG_DIR"; then
                    echo "  ✓ vcpkg ready for use"
                    VCPKG_AVAILABLE=true
                else
                    echo "  ⚠ vcpkg bootstrap failed verification"
                    echo "  → Falling back to CPM..."
                    VCPKG_AVAILABLE=false
                fi
            else
                echo "  ⚠ vcpkg bootstrap failed"
                echo "  → Falling back to CPM..."
                VCPKG_AVAILABLE=false
            fi
        elif [ -f "$VCPKG_DIR/vcpkg" ] && ! verify_vcpkg_ready "$VCPKG_DIR"; then
            echo "  ⚠ vcpkg executable found but failed to run (not properly bootstrapped)."
            echo "    Attempting to re-bootstrap vcpkg..."
            if cd "$VCPKG_DIR" && ./bootstrap-vcpkg.sh; then
                echo "    ✓ vcpkg re-bootstrapped successfully"
                if verify_vcpkg_ready "$VCPKG_DIR"; then
                    echo "  ✓ vcpkg ready for use"
                    VCPKG_AVAILABLE=true
                else
                    echo "  ⚠ vcpkg re-bootstrap failed verification"
                    echo "  → Falling back to CPM..."
                    VCPKG_AVAILABLE=false
                fi
            else
                echo "  ⚠ vcpkg re-bootstrap failed"
                echo "  → Falling back to CPM..."
                VCPKG_AVAILABLE=false
            fi
        else
            echo "  ⚠ vcpkg not found. Will use CPM as fallback."
            VCPKG_AVAILABLE=false
        fi
    fi
else
    echo "  → CPM forced via -c flag (skipping vcpkg check)"
    VCPKG_AVAILABLE=false
fi

if [ "$VCPKG_AVAILABLE" = true ]; then
    echo "  → Using vcpkg-based build"
    CMAKE_PRESET="linux-release"
    BUILD_DIR="build"
else
    echo "  → Falling back to CPM for dependency management"
    CMAKE_PRESET="linux-release-cpm"
    BUILD_DIR="build-cpm"
fi

echo ""

# ========================================================================
# Step 2: Configure CMake
# ========================================================================
echo "→ Step 2/4: Configuring CMake ($CMAKE_PRESET preset)..."
cd "$PROJECT_ROOT"

if [ "$INCREMENTAL_BUILD" = false ] && [ -d "$BUILD_DIR" ]; then
    echo "  Cleaning existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

if [ ! -d "$BUILD_DIR" ]; then
    cmake --preset "$CMAKE_PRESET"
    echo "✓ CMake configuration complete"
else
    echo "✓ Using existing build configuration (incremental build)"
fi
echo ""

# ========================================================================
# Step 3: Build
# ========================================================================
echo "→ Step 3/4: Building project..."
if [ "$BUILD_TESTS" = true ]; then
    cmake --build --preset "$CMAKE_PRESET" -- -j$(nproc 2>/dev/null || echo 4)
else
    cmake --build --preset "$CMAKE_PRESET" --target r-type_client r-type_server -- -j$(nproc 2>/dev/null || echo 4)
fi
echo "✓ Build complete"
echo ""

# ========================================================================
# Step 4: Copy executables to root
# ========================================================================
echo "→ Step 4/4: Copying executables to repository root..."

CLIENT_EXE="$BUILD_DIR/src/client/r-type_client"
SERVER_EXE="$BUILD_DIR/src/server/r-type_server"

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
echo "║                   Build Complete!                        ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Executables are now in the repository root:"
ls -lh "$PROJECT_ROOT"/r-type_* 2>/dev/null || echo "  (No executables found)"
echo ""
echo "Dependency strategy used: $([ "$VCPKG_AVAILABLE" = true ] && echo "vcpkg" || echo "CPM")"
echo "Build directory: $BUILD_DIR"
echo ""
