#!/usr/bin/env bash
# ACE PoC Test Script with Benchmarking and Evaluation
# Tests the UDP server and client examples

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║     ACE PoC Test, Benchmark & Evaluation Script     ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""

# Cleanup function
cleanup() {
    if [ ! -z "${SERVER_PID:-}" ]; then
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
    rm -f server.log client.log
}

trap cleanup EXIT

# Build ACE PoC
echo -e "${YELLOW}═══ Building ACE PoC ═══${NC}"

# Clean build
rm -rf build-ace-poc
mkdir -p build-ace-poc
cd build-ace-poc

# Measure compilation time
echo -e "${BLUE}Configuring...${NC}"
config_start=$(date +%s.%N)
if cmake .. -DBUILD_EXAMPLES=ON -DBUILD_TESTS=OFF > cmake_config.log 2>&1; then
    config_end=$(date +%s.%N)
    config_time=$(echo "$config_end - $config_start" | bc)
    echo -e "${GREEN}✓ CMake configuration successful${NC}"
else
    echo -e "${RED}✗ CMake configuration failed${NC}"
    cat cmake_config.log
    exit 1
fi

echo -e "${BLUE}Building...${NC}"
build_start=$(date +%s.%N)
if cmake --build . -- -j > build.log 2>&1; then
    build_end=$(date +%s.%N)
    build_time=$(echo "$build_end - $build_start" | bc)
    echo -e "${GREEN}✓ Build successful${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    cat build.log
    exit 1
fi

cd ..

# Wait for binaries to be available
sleep 1

# Check if binaries exist
if [ ! -f "ace_udp_server" ]; then
    echo -e "${RED}✗ Server binary not found at project root${NC}"
    echo "Looking in build directory..."
    find build-ace-poc -name "ace_udp_server" -type f
    exit 1
fi

if [ ! -f "ace_udp_client" ]; then
    echo -e "${RED}✗ Client binary not found at project root${NC}"
    echo "Looking in build directory..."
    find build-ace-poc -name "ace_udp_client" -type f
    exit 1
fi

# Measure binary sizes
server_size=$(stat -f%z "ace_udp_server" 2>/dev/null || stat -c%s "ace_udp_server" 2>/dev/null || echo "0")
client_size=$(stat -f%z "ace_udp_client" 2>/dev/null || stat -c%s "ace_udp_client" 2>/dev/null || echo "0")

# Convert to KB
server_size_kb=$(echo "scale=2; $server_size / 1024" | bc)
client_size_kb=$(echo "scale=2; $client_size / 1024" | bc)

# Display benchmark results
echo ""
echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║              ACE BENCHMARK RESULTS                   ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "  ${CYAN}Config time:${NC}  ${config_time}s"
echo -e "  ${CYAN}Build time:${NC}   ${build_time}s"
echo -e "  ${CYAN}Server size:${NC}  ${server_size_kb} KB (${server_size} bytes)"
echo -e "  ${CYAN}Client size:${NC}  ${client_size_kb} KB (${client_size} bytes)"

echo ""
echo -e "${YELLOW}═══ Functional Test: ACE ═══${NC}"

echo -e "${GREEN}✓ Binaries found${NC}"

# Start server in background
echo -e "${YELLOW}Starting UDP server on port 4242...${NC}"
./ace_udp_server 4242 > server.log 2>&1 &
SERVER_PID=$!

# Give server time to start
sleep 1

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Server failed to start${NC}"
    cat server.log
    exit 1
fi

echo -e "${GREEN}✓ Server started (PID: $SERVER_PID)${NC}"

# Run client
echo -e "${YELLOW}Running client tests...${NC}"
if ./ace_udp_client 127.0.0.1 4242 > client.log 2>&1; then
    echo -e "${GREEN}✓ Client tests passed${NC}"
    TEST_RESULT=0
else
    echo -e "${RED}✗ Client tests failed${NC}"
    TEST_RESULT=1
fi

# Display logs
echo ""
echo -e "${YELLOW}=== Server Log ===${NC}"
cat server.log

echo ""
echo -e "${YELLOW}=== Client Log ===${NC}"
cat client.log

# ACE Evaluation
echo ""
echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║              ACE EVALUATION NOTES                    ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""

# Save evaluation to CSV
echo "Metric,Value,Notes" > ace_evaluation.csv
echo "Config Time (s),${config_time},CMake configuration time" >> ace_evaluation.csv
echo "Build Time (s),${build_time},Compilation time" >> ace_evaluation.csv
echo "Server Size (KB),${server_size_kb},Binary size" >> ace_evaluation.csv
echo "Client Size (KB),${client_size_kb},Binary size" >> ace_evaluation.csv
echo "Server Size (bytes),${server_size},-" >> ace_evaluation.csv
echo "Client Size (bytes),${client_size},-" >> ace_evaluation.csv
echo "Documentation Quality,Poor,Outdated and complex" >> ace_evaluation.csv
echo "Modern C++ Support,Limited,Designed for C++98/03" >> ace_evaluation.csv
echo "Build Complexity,Very High,Requires custom build system" >> ace_evaluation.csv
echo "Learning Curve,Steep,Complex patterns and abstractions" >> ace_evaluation.csv
echo "Community Activity,Low,Declining maintenance" >> ace_evaluation.csv
echo "Recommendation,Not Recommended,Too complex for modern C++20 project" >> ace_evaluation.csv

# Display evaluation summary
echo -e "${YELLOW}📊 Evaluation Summary:${NC}"
echo -e "  ${RED}✗ Documentation:${NC}       Outdated, difficult to navigate"
echo -e "  ${RED}✗ Modern C++:${NC}          Limited C++20 support"
echo -e "  ${RED}✗ Build System:${NC}        Complex, non-standard"
echo -e "  ${RED}✗ Learning Curve:${NC}      Very steep"
echo -e "  ${YELLOW}⚠ Community:${NC}            Declining activity"

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ ACE PoC Tests PASSED!                          ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${CYAN}📊 Evaluation results saved to: ace_evaluation.csv${NC}"
    echo -e "${CYAN}📁 Build logs available in: build-ace-poc/${NC}"
    echo ""
else
    echo -e "${RED}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ ACE PoC Tests FAILED!                          ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════╝${NC}"
fi

exit $TEST_RESULT
