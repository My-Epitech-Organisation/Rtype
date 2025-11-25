# Code Quality Tools - Documentation

This directory contains Proof of Concepts (PoCs) for evaluating static analysis and code formatting tools for the R-Type project.

## Overview

We evaluated three complementary tools to ensure code quality without budget constraints:

1. **Clang-Tidy**: Detects bugs, performance issues, and modernization opportunities
2. **Clang-Format**: Enforces consistent code style
3. **CppLint**: Checks Google C++ style compliance and header guards

## Tools Summary

| Tool | Purpose | Status | Location |
|------|---------|--------|----------|
| Clang-Tidy | Static analysis, bug detection, modernization | ✅ Recommended | `./clang-tidy/` |
| Clang-Format | Code style enforcement | ✅ Recommended | `./clang-format/` |
| CppLint | Style compliance, header guards | ✅ Recommended | `./cpplint/` |

## Quick Start

### Running Clang-Tidy

```bash
cd PoC/PoC_Code_Quality/clang-tidy
mkdir build && cd build
cmake ..
make
```

### Running Clang-Format

```bash
cd PoC/PoC_Code_Quality/clang-format
clang-format -i <file.cpp>
```

### Running CppLint

```bash
cd PoC/PoC_Code_Quality/cpplint
python3 cpplint.py <file.cpp>
```

## Individual PoC Details

- **[Clang-Tidy PoC](./clang-tidy/README.md)**: Catches modern C++ errors (raw pointers, narrowing conversions, etc.)
- **[Clang-Format PoC](./clang-format/README.md)**: Before/after demonstration of code formatting
- **[CppLint PoC](./cpplint/README.md)**: Header guard and style checking

## Recommendations for Project Integration

### 1. Compiler Warnings (Strict Mode)

Add strict compiler warnings to CMakeLists.txt to catch type and conversion issues:

```cmake
target_compile_options(your_target PRIVATE
    -Wall -Wextra -Wconversion -Wsign-conversion -Wdouble-promotion
)
```

**Why?**

- `-Wconversion`: Warns when a larger type is assigned to a smaller type
- `-Wsign-conversion`: Warns when switching between signed/unsigned
- Helps identify potential integer type mismatches

### 2. Clang-Tidy Integration

Recommended checks for modern C++ (C++20):

```cmake
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -checks=modernize-*,performance-*,readability-*,cppcoreguidelines-*
)
```

**Specifically useful checks:**

- `cppcoreguidelines-owning-memory`: Detects raw pointer usage
- `modernize-use-smart-ptr`: Suggests std::unique_ptr/std::shared_ptr
- `performance-unnecessary-copy-initialization`: Avoids unnecessary copies
- `readability-identifier-length`: Enforces meaningful variable names

### 3. LLVM Optimization Reports (Advanced)

To detect oversized integer types:

```cmake
target_compile_options(your_target PRIVATE
    -fsave-optimization-record
)
```

After building, check `.opt.yaml` files for lines like:

```text
Bitwidth of integer reduced from 32 to 16
```

This tells you LLVM could safely shrink that integer type.

### 4. Tool Division

| Use cpplint for: | Use clang-tidy for: | Use compiler warnings for: |
|------------------|-------------------|---------------------------|
| Naming style | Logical errors | Type mismatches |
| Header guards | Raw pointer usage | Conversion issues |
| Copyright notices | Performance issues | Signedness issues |
| Whitespace | C++ modernization | Implicit promotions |

## Benchmarks

All tools are **free and open-source**, backed by LLVM/Google.

This setup is comparable to expensive commercial tools like Polyspace or Astrée, but completely free for students.
