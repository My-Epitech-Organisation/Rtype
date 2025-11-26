#!/bin/bash

################################################################################
# R-Type Code Quality Tools Runner
#
# This script runs all code quality checks on the R-Type project:
# - Compiler warnings (via CMake)
# - Clang-Tidy (static analysis)
# - Clang-Format (style checking)
# - CppLint (Google style compliance)
#
# Usage: ./run_quality_checks.sh [--fix] [--verbose]
################################################################################

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

FIX_MODE=0
VERBOSE=0
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
REPORT_FILE="/tmp/rtype_quality_report.txt"

while [[ $# -gt 0 ]]; do
    case $1 in
        --fix) FIX_MODE=1; shift ;;
        --verbose) VERBOSE=1; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[⚠]${NC} $1"
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
}

echo "Code Quality Check Report - $(date)" > "$REPORT_FILE"
echo "Project: R-Type" >> "$REPORT_FILE"
echo "Date: $(date)" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

log_info "Starting code quality checks for R-Type project..."
log_info "Project root: $PROJECT_ROOT"
echo ""

log_info "Checking for required tools..."
TOOLS_MISSING=0

if ! command -v clang-tidy &> /dev/null; then
    log_error "clang-tidy not found. Install with: sudo apt-get install clang-tidy"
    TOOLS_MISSING=1
fi

if ! command -v clang-format &> /dev/null; then
    log_error "clang-format not found. Install with: sudo apt-get install clang-format"
    TOOLS_MISSING=1
fi

if ! command -v python3 &> /dev/null; then
    log_error "python3 not found. Install with: sudo apt-get install python3"
    TOOLS_MISSING=1
fi

if [ $TOOLS_MISSING -eq 1 ]; then
    log_error "Some required tools are missing. Exiting."
    exit 1
fi

log_success "All required tools found"
echo ""

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  CLANG-FORMAT - Code Style Checking${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
log_info "Running Clang-Format checks..."
echo "" >> "$REPORT_FILE"
echo "1. CLANG-FORMAT RESULTS" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

CLANG_FORMAT_ERRORS=0
FORMAT_FAILED=""

CPP_FILES=$(find "$PROJECT_ROOT/src" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null)

CLANG_FORMAT_CONFIG="$PROJECT_ROOT/config/clang/.clang-format"

if [ -z "$CPP_FILES" ]; then
    log_warning "No C++ files found to check"
else
    if [ ! -f "$CLANG_FORMAT_CONFIG" ]; then
        log_error "Clang-Format config not found at $CLANG_FORMAT_CONFIG"
    else
        if [ $FIX_MODE -eq 1 ]; then
            log_info "Applying clang-format to all files (multiple passes for convergence)..."
            for pass in 1 2 3; do
                if [ $VERBOSE -eq 1 ]; then
                    log_info "Format pass $pass..."
                fi
                for file in $CPP_FILES; do
                    if ! clang-format -style=file:"$CLANG_FORMAT_CONFIG" -i "$file" 2>/dev/null; then
                        log_error "Failed to format: $file"
                    fi
                done
            done
        else
            log_info "Checking formatting (without applying changes)..."
        fi

        log_info "Checking for remaining formatting issues..."
        for file in $CPP_FILES; do
            REPLACEMENTS=$(clang-format -style=file:"$CLANG_FORMAT_CONFIG" -output-replacements-xml "$file" 2>/dev/null || true)
            if [[ "$REPLACEMENTS" == *"<replacement "* ]]; then
                CLANG_FORMAT_ERRORS=$((CLANG_FORMAT_ERRORS + 1))
                FORMAT_FAILED="${FORMAT_FAILED}  ✗ Needs formatting: $file\n"
                log_warning "Needs formatting: $file"
            fi
        done
    fi
fi

if [ $CLANG_FORMAT_ERRORS -eq 0 ]; then
    log_success "Clang-Format: All files formatted successfully ✓"
    echo "Status: PASS - All files formatted successfully" >> "$REPORT_FILE"
else
    log_error "Clang-Format: $CLANG_FORMAT_ERRORS file(s) still have formatting issues"
    echo "Status: FAIL - $CLANG_FORMAT_ERRORS file(s) still have formatting issues" >> "$REPORT_FILE"
    if [ -n "$FORMAT_FAILED" ]; then
        echo -e "Issues:\n$FORMAT_FAILED" >> "$REPORT_FILE"
    fi
fi

echo ""
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  CPPLINT - Google Style Compliance${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
log_info "Running CppLint checks..."
echo "" >> "$REPORT_FILE"
echo "2. CPPLINT RESULTS" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

CPPLINT_ERRORS=0
CPPLINT_SCRIPT="$PROJECT_ROOT/config/cpplint/cpplint.py"

if [ ! -f "$CPPLINT_SCRIPT" ]; then
    log_warning "CppLint script not found at $CPPLINT_SCRIPT"
    echo "Status: SKIPPED - cpplint.py not found" >> "$REPORT_FILE"
else
    CPPLINT_FILTERS="--filter=-whitespace/parens,-legal/copyright,-whitespace/indent,-whitespace/line_length,-build/include_subdir,-build/c++11,-runtime/references,-build/include_what_you_use"
    CPPLINT_OUTPUT=$(python3 "$CPPLINT_SCRIPT" $CPPLINT_FILTERS --linelength=100 $CPP_FILES 2>&1 | grep -v "^$" | grep -v "^Done processing" || true)

    if [ -z "$CPPLINT_OUTPUT" ]; then
        log_success "CppLint: No style violations found ✓"
        echo "Status: PASS - No style violations" >> "$REPORT_FILE"
    else
        CPPLINT_ERRORS=$(echo "$CPPLINT_OUTPUT" | grep "Total errors found:" | awk '{print $NF}' || echo "0")
        if [ -z "$CPPLINT_ERRORS" ] || [ "$CPPLINT_ERRORS" = "0" ]; then
            log_success "CppLint: No violations (after filtering) ✓"
            echo "Status: PASS - No violations after filtering" >> "$REPORT_FILE"
        else
            log_error "CppLint: Found $CPPLINT_ERRORS style violation(s)"
            echo "Status: FAIL - Style violations detected" >> "$REPORT_FILE"

            ERRORS_ONLY=$(echo "$CPPLINT_OUTPUT" | grep -v "^Total errors" || true)
            echo "" >> "$REPORT_FILE"
            echo "$ERRORS_ONLY" >> "$REPORT_FILE"

            if [ $VERBOSE -eq 1 ]; then
                echo "$ERRORS_ONLY"
            fi
        fi
    fi
fi

echo ""
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  CLANG-TIDY - Static Analysis${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
log_info "Running Clang-Tidy on source files..."
echo "" >> "$REPORT_FILE"
echo "3. CLANG-TIDY RESULTS" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

CLANG_TIDY_ERRORS=0
CLANG_TIDY_CONFIG="$PROJECT_ROOT/config/clang/.clang-tidy"

TIDY_FILES=$(find "$PROJECT_ROOT/src" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) 2>/dev/null)

if [ ! -f "$CLANG_TIDY_CONFIG" ]; then
    log_warning "Clang-Tidy config not found"
    echo "Status: SKIPPED - .clang-tidy config not found" >> "$REPORT_FILE"
else
    if [ -z "$TIDY_FILES" ]; then
        log_warning "No source files found to analyze"
        echo "Status: SKIPPED - No source files found" >> "$REPORT_FILE"
    else
        TIDY_OUTPUT=$(clang-tidy --config-file="$CLANG_TIDY_CONFIG" $TIDY_FILES -- -std=c++20 -Iinclude 2>&1 | grep "warning:" | head -50 || true)

        if [ -z "$TIDY_OUTPUT" ]; then
            CLANG_TIDY_ERRORS=0
        else
            CLANG_TIDY_ERRORS=$(echo "$TIDY_OUTPUT" | wc -l)
        fi

        if [ $CLANG_TIDY_ERRORS -eq 0 ]; then
            log_success "Clang-Tidy: No warnings found ✓"
            echo "Status: PASS - No analysis warnings" >> "$REPORT_FILE"
        else
            log_warning "Clang-Tidy: Found $CLANG_TIDY_ERRORS warning(s)"
            echo "Status: WARNINGS - Found $CLANG_TIDY_ERRORS issue(s)" >> "$REPORT_FILE"
            echo "" >> "$REPORT_FILE"
            echo "Warnings:" >> "$REPORT_FILE"
            echo "$TIDY_OUTPUT" >> "$REPORT_FILE"

            if [ $VERBOSE -eq 1 ]; then
                echo "Clang-Tidy Warnings:"
                echo "$TIDY_OUTPUT"
            fi
        fi
    fi
fi

echo ""
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  COMPILER WARNINGS - Type & Size Optimization${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
log_info "Checking for compiler optimization warnings..."
echo "" >> "$REPORT_FILE"
echo "4. COMPILER WARNINGS - OPTIMIZATION CHECKS" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

COMPILER_WARNINGS=0

COMPILER_OUTPUT=""
for file in $CPP_FILES; do
    FILE_WARNINGS=$(clang++ -Wall -Wextra -Wconversion -Wsign-conversion \
        -Wdouble-promotion -Wnarrowing -std=c++20 -I"$PROJECT_ROOT/include" \
        -fsyntax-only "$file" 2>&1 | grep -E "warning:|error:" || true)

    if [ -n "$FILE_WARNINGS" ]; then
        COMPILER_OUTPUT="${COMPILER_OUTPUT}${FILE_WARNINGS}\n"
    fi
done

COMPILER_WARNINGS=$(echo -e "$COMPILER_OUTPUT" | grep -c "warning:" || echo 0)

if [ "$COMPILER_WARNINGS" -eq 0 ]; then
    log_success "Compiler: No optimization warnings found ✓"
    echo "Status: PASS - No type/size optimization issues detected" >> "$REPORT_FILE"
else
    log_warning "Compiler: Found $COMPILER_WARNINGS optimization warning(s)"
    echo "Status: WARNINGS - Found $COMPILER_WARNINGS optimization suggestion(s)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "Optimization Suggestions:" >> "$REPORT_FILE"
    echo -e "$COMPILER_OUTPUT" >> "$REPORT_FILE"

    if [ $VERBOSE -eq 1 ]; then
        echo ""
        echo "Optimization warnings details:"
        echo -e "$COMPILER_OUTPUT" | head -20
    fi
fi

echo ""
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  SUMMARY${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
log_info "Generating summary report..."
echo ""
echo "======================================" >> "$REPORT_FILE"
echo "SUMMARY" >> "$REPORT_FILE"
echo "======================================" >> "$REPORT_FILE"

TOTAL_ERRORS=$((CLANG_FORMAT_ERRORS + CPPLINT_ERRORS))

echo "Clang-Format Issues: $CLANG_FORMAT_ERRORS" >> "$REPORT_FILE"
echo "CppLint Issues: $CPPLINT_ERRORS" >> "$REPORT_FILE"
echo "Clang-Tidy Warnings: $CLANG_TIDY_ERRORS" >> "$REPORT_FILE"
echo "Compiler Optimization Suggestions: $COMPILER_WARNINGS" >> "$REPORT_FILE"
echo "Total Issues (blocking): $TOTAL_ERRORS" >> "$REPORT_FILE"

if [ $TOTAL_ERRORS -eq 0 ]; then
    log_success "Code Quality Check PASSED ✓"
    echo "" >> "$REPORT_FILE"
    echo "RESULT: PASS ✓" >> "$REPORT_FILE"
    echo "All code quality checks passed successfully!" >> "$REPORT_FILE"
    EXIT_CODE=0
else
    log_error "Code Quality Check FAILED"
    echo "" >> "$REPORT_FILE"
    echo "RESULT: FAIL ✗" >> "$REPORT_FILE"
    echo "Found $TOTAL_ERRORS issue(s) that need to be fixed." >> "$REPORT_FILE"

    if [ $FIX_MODE -eq 1 ]; then
        echo "Auto-fix mode was enabled. Some issues may have been resolved." >> "$REPORT_FILE"
    else
        echo "Run with --fix flag to automatically fix formatting issues." >> "$REPORT_FILE"
    fi
    EXIT_CODE=1
fi

echo ""
echo "======================================"
echo "Report saved to: $REPORT_FILE"
echo "======================================"
echo ""

cat "$REPORT_FILE"

exit $EXIT_CODE
