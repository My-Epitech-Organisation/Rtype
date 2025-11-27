#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BUILD_TYPE="Release"
BUILD_TESTS=OFF
RUN_TESTS=false
VERBOSE=false
CLEAN=false
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
INSTALL=false
INSTALL_PREFIX="/usr/local"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
VCPKG_DIR="$SCRIPT_DIR/vcpkg"

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

show_help() {
    cat << EOF
R-Type Build Script

Usage: ./build.sh [options]

Options:
  -h, --help          Show this help message
  -d, --debug         Build in Debug mode (default: Release)
  -r, --release       Build in Release mode
  -t, --test          Build and run tests
  --build-tests       Build tests without running them
  -v, --verbose       Enable verbose output
  -c, --clean         Clean build directory before building
  -j, --jobs N        Number of parallel jobs (default: auto-detect)
  --install           Install after building
  --prefix PATH       Installation prefix (default: /usr/local)
  --setup-vcpkg       Setup vcpkg if not present

Examples:
  ./build.sh                    # Release build
  ./build.sh -d                 # Debug build
  ./build.sh -t                 # Build and run tests
  ./build.sh -d -t -v           # Debug build with tests, verbose
  ./build.sh -c -d              # Clean debug build
  ./build.sh --setup-vcpkg      # Setup vcpkg first, then build

EOF
}

setup_vcpkg() {
    if [ -d "$VCPKG_DIR" ] && [ -f "$VCPKG_DIR/vcpkg" ]; then
        log_info "vcpkg already installed at $VCPKG_DIR"
        return 0
    fi

    log_info "Setting up vcpkg..."

    if [ -d "$VCPKG_DIR" ]; then
        log_warn "Removing incomplete vcpkg installation..."
        rm -rf "$VCPKG_DIR"
    fi

    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics

    log_success "vcpkg setup complete"
}

check_dependencies() {
    local missing=()

    if ! command -v cmake &> /dev/null; then
        missing+=("cmake")
    fi

    if ! command -v ninja &> /dev/null; then
        log_warn "Ninja not found, will use default generator"
    fi

    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        missing+=("g++ or clang++")
    fi

    if [ ${#missing[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing[*]}"
        log_info "Install with: sudo apt-get install ${missing[*]} ninja-build"
        exit 1
    fi

    if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
        log_warn "vcpkg not found at $VCPKG_DIR"
        log_info "Run with --setup-vcpkg to install vcpkg"
        log_info "Or manually: git clone https://github.com/microsoft/vcpkg.git && ./vcpkg/bootstrap-vcpkg.sh"
        exit 1
    fi
}

install_system_deps() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        log_info "Checking system dependencies for SFML..."
        local deps="libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libfreetype6-dev"

        local need_install=false
        for pkg in $deps; do
            if ! dpkg -s "${pkg}" &> /dev/null; then
                need_install=true
                break
            fi
        done

        if $need_install; then
            log_info "Installing system dependencies (requires sudo)..."
            sudo apt-get update
            sudo apt-get install -y $deps ninja-build
        fi
    fi
}

clean_build() {
    if [ -d "$BUILD_DIR" ]; then
        log_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        log_success "Build directory cleaned"
    fi
}

configure() {
    log_info "Configuring project (${BUILD_TYPE})..."

    local cmake_args=(
        -S "$SCRIPT_DIR"
        -B "$BUILD_DIR"
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"
        -DBUILD_TESTS="$BUILD_TESTS"
        -DBUILD_EXAMPLES=OFF
    )

    if command -v ninja &> /dev/null; then
        cmake_args+=(-G Ninja)
    fi

    if $INSTALL; then
        cmake_args+=(-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX")
    fi

    if $VERBOSE; then
        cmake_args+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
        cmake "${cmake_args[@]}"
    else
        cmake "${cmake_args[@]}" 2>&1 | grep -E "(^--|error|warning|vcpkg)" || true
    fi

    log_success "Configuration complete"
}

build() {
    log_info "Building project with $JOBS parallel jobs..."

    local build_args=(
        --build "$BUILD_DIR"
        --parallel "$JOBS"
    )

    if $VERBOSE; then
        build_args+=(--verbose)
    fi

    cmake "${build_args[@]}"

    log_success "Build complete"
}

run_tests() {
    log_info "Running tests..."

    cd "$BUILD_DIR"

    local ctest_args=(
        --output-on-failure
        --timeout 120
    )

    if $VERBOSE; then
        ctest_args+=(--verbose)
    fi

    ctest "${ctest_args[@]}"

    log_success "All tests passed"
}

install_project() {
    log_info "Installing to $INSTALL_PREFIX..."
    cmake --install "$BUILD_DIR"
    log_success "Installation complete"
}

print_summary() {
    echo ""
    echo "═══════════════════════════════════════════════════════════"
    echo "                    BUILD SUMMARY"
    echo "═══════════════════════════════════════════════════════════"
    echo "  Build Type:    $BUILD_TYPE"
    echo "  Build Dir:     $BUILD_DIR"
    echo "  Tests:         $BUILD_TESTS"
    echo "  Jobs:          $JOBS"
    echo "═══════════════════════════════════════════════════════════"
    echo ""

    if [ -f "$BUILD_DIR/src/server/r-type_server" ]; then
        log_success "Server binary: $BUILD_DIR/src/server/r-type_server"
    fi
    if [ -f "$BUILD_DIR/src/client/r-type_client" ]; then
        log_success "Client binary: $BUILD_DIR/src/client/r-type_client"
    fi
    echo ""
}

SETUP_VCPKG=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -t|--test)
            BUILD_TESTS=ON
            RUN_TESTS=true
            shift
            ;;
        --build-tests)
            BUILD_TESTS=ON
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --setup-vcpkg)
            SETUP_VCPKG=true
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

main() {
    echo ""
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║                   R-TYPE BUILD SYSTEM                     ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo ""

    cd "$SCRIPT_DIR"

    if $SETUP_VCPKG; then
        setup_vcpkg
    fi

    check_dependencies

    if $CLEAN; then
        clean_build
    fi

    configure
    build

    if $RUN_TESTS; then
        run_tests
    fi

    if $INSTALL; then
        install_project
    fi

    print_summary
}

main
