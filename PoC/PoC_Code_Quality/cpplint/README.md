# CppLint PoC

This PoC evaluates Google's cpplint for C++ style checking.

## Setup

- `cpplint.py`: Downloaded from [cpplint repository](https://github.com/cpplint/cpplint).
- `no_guards.h`: Sample header without guards to test cpplint.

## Output

Running `python3 cpplint.py no_guards.h`:

```text
no_guards.h:0:  No copyright message found.  You should have a line: "Copyright [year] <Copyright Owner>"  [legal/copyright] [5]
no_guards.h:0:  No #ifndef header guard found, suggested CPP variable is: POC_POC_CODE_QUALITY_CPPLINT_NO_GUARDS_H_  [build/header_guard] [5]
no_guards.h:2:  public: should be indented +1 space inside class BadClass  [whitespace/indent] [3]
no_guards.h:4:  Could not find a newline character at the end of the file.  [whitespace/ending_newline] [5]
Done processing no_guards.h
Total errors found: 4
```

## Decision

**Keep CppLint**: It catches style issues (e.g., header guards, indentation, copyright) that clang-tidy doesn't focus on. Use alongside clang-tidy for comprehensive code quality.

## Exit Criteria

Verification of CppLint output utility: ✅ (catches header guard errors missed by clang-tidy).
