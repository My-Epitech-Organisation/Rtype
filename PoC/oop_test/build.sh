#!/bin/bash

# Build script for OOP PoC
# Compiles the traditional OOP approach for R-Type

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     Building OOP PoC for R-Type                     ║${NC}"
echo -e "${BLUE}║     Traditional OOP Inheritance Approach            ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════╝${NC}"
echo ""

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT="$BUILD_DIR/oop_poc"

# Compiler settings
CXX="${CXX:-g++}"
CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic"

# Debug or Release
if [ "$1" == "release" ]; then
    echo -e "${GREEN}Building in RELEASE mode${NC}"
    CXXFLAGS="$CXXFLAGS -O3 -DNDEBUG"
else
    echo -e "${YELLOW}Building in DEBUG mode${NC}"
    CXXFLAGS="$CXXFLAGS -g -O0"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Source files
SOURCES=(
    "$SRC_DIR/GameObject.cpp"
    "$SRC_DIR/Movable.cpp"
    "$SRC_DIR/Player.cpp"
    "$SRC_DIR/Enemy.cpp"
    "$SRC_DIR/DiamondProblem.cpp"
    "$SRC_DIR/main.cpp"
)

# Compile
echo -e "${BLUE}Compiling source files...${NC}"
OBJECTS=()
for src in "${SOURCES[@]}"; do
    filename=$(basename "$src" .cpp)
    obj="$BUILD_DIR/${filename}.o"
    OBJECTS+=("$obj")
    
    echo -e "  ${YELLOW}►${NC} Compiling $filename.cpp"
    $CXX $CXXFLAGS -I"$SRC_DIR" -c "$src" -o "$obj"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Compilation failed for $src${NC}"
        exit 1
    fi
done

# Link
echo -e "${BLUE}Linking executable...${NC}"
$CXX $CXXFLAGS -o "$OUTPUT" "${OBJECTS[@]}" -lm

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo -e "${GREEN}✓ Executable: $OUTPUT${NC}"
    echo ""
    echo -e "${BLUE}To run the PoC:${NC}"
    echo -e "  ${YELLOW}$OUTPUT${NC}"
    echo ""
    echo -e "${BLUE}Or use:${NC}"
    echo -e "  ${YELLOW}./build.sh && ./build/oop_poc${NC}"
    echo ""
else
    echo -e "${RED}✗ Linking failed${NC}"
    exit 1
fi

# Display info
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Build Information:${NC}"
echo -e "  Compiler: $CXX"
echo -e "  Flags: $CXXFLAGS"
echo -e "  Source files: ${#SOURCES[@]}"
echo -e "  Output: $OUTPUT"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
