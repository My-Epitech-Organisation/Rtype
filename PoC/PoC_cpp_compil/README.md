# Linux Development Environment PoC - GCC vs Clang

## Objective

Verify that our C++20 code compiles cleanly with strict warning flags (`-Wall -Wextra -Werror`) on Linux using both GCC and Clang compilers with Make and Ninja build systems.

## Scope

- **Compilers**: GCC 11+ and Clang 14+
- **Build Systems**: Make and Ninja generators
- **Strict Flags**: `-Wall -Wextra -Werror -Wpedantic`
- **C++ Standard**: C++20

## Test Cases

### 1. `test_cpp20_features.cpp`
Tests modern C++20 features:
- Concepts
- Ranges
- Span
- Compile-time checks

### 2. `test_strict_warnings.cpp`
Clean code that should compile without any warnings:
- Proper initialization
- No unused variables
- Correct const-correctness
- All parameters used

### 3. `test_edge_cases.cpp`
Potential warning sources properly handled:
- Signed/unsigned comparisons
- Switch statements with all cases
- String operations
- Proper struct initialization

## Usage

### Quick Test
```bash
chmod +x build_linux.sh
./build_linux.sh
```

### Manual Testing

#### With GCC + Make
```bash
cmake -B build-gcc -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++
cmake --build build-gcc
./build-gcc/bin/test_cpp20_features
./build-gcc/bin/test_strict_warnings
./build-gcc/bin/test_edge_cases
```

#### With Clang + Ninja
```bash
cmake -B build-clang -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build-clang
./build-clang/bin/test_cpp20_features
./build-clang/bin/test_strict_warnings
./build-clang/bin/test_edge_cases
```

## Expected Output

The script will:
1. Check compiler and build system availability
2. Test all combinations (GCC/Clang × Make/Ninja)
3. Measure configuration and build times
4. Verify clean compilation (0 warnings, 0 errors)
5. Execute all test binaries
6. Generate CSV report with results

## Results Format

CSV file: `compiler_comparison_results.csv`

| Compiler | Generator | Config Time | Build Time | Warnings | Errors | Status |
|----------|-----------|-------------|------------|----------|--------|--------|
| GCC      | Make      | 0.45s       | 1.23s      | 0        | 0      | PASSED |
| Clang    | Ninja     | 0.38s       | 0.89s      | 0        | 0      | PASSED |

## Exit Criteria

✅ Clean build with no warnings on Linux
✅ All test executables run successfully
✅ Both GCC and Clang support verified
✅ Both Make and Ninja generators work

## Recommendation

Based on test results, the script will recommend:
- **Clang**: Better diagnostics, clearer error messages, faster compilation
- **GCC**: Excellent optimization, widely available, mature toolchain

Both are suitable for the R-Type project.

## Requirements

- CMake 3.15+
- GCC 11+ or Clang 14+
- Make and/or Ninja
- bc (for time calculations)

## Related

- Issue: [Spike] PoC: Linux Development Environment (GCC/Clang)
- Parent: [Spike] [Main] OS Support & Cross-Platform PoC #25
- Timebox: 17/11/2025 - 18/11/2025
