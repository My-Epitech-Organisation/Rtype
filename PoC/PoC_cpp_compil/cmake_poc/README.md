# Mini PoC - CMake with Strict Flags

Minimal proof-of-concept demonstrating CMake configuration with strict warning flags.

## Quick Test

```bash
chmod +x test.sh
./test.sh
```

## Manual Build

```bash
# With GCC
cmake -B build-gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build-gcc
./build-gcc/bin/hello_strict

# With Clang
cmake -B build-clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-clang
./build-clang/bin/hello_strict
```

## What it tests

- C++20 standard
- Strict flags: `-Wall -Wextra -Werror -Wpedantic`
- Clean compilation (0 warnings)
- Basic C++20 features (designated initializers)

## Files

- `CMakeLists.txt` - Minimal CMake configuration
- `hello.cpp` - Simple C++20 program
- `test.sh` - Quick test script
