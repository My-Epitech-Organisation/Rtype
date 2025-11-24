#!/usr/bin/env bash
# Boost.Asio PoC Test Script with Benchmarking
# Tests the UDP server and client examples

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║       Boost.Asio PoC Test & Benchmark Script        ║${NC}"
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

# Build Boost.Asio PoC
echo -e "${YELLOW}═══ Building Boost.Asio PoC ═══${NC}"

# Clean build
rm -rf build-boost-poc
mkdir -p build-boost-poc
cd build-boost-poc

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
if [ ! -f "boost_udp_server" ]; then
    echo -e "${RED}✗ Server binary not found at project root${NC}"
    echo "Looking in build directory..."
    find build-boost-poc -name "boost_udp_server" -type f
    exit 1
fi

if [ ! -f "boost_udp_client" ]; then
    echo -e "${RED}✗ Client binary not found at project root${NC}"
    echo "Looking in build directory..."
    find build-boost-poc -name "boost_udp_client" -type f
    exit 1
fi

# Measure binary sizes
server_size=$(stat -f%z "boost_udp_server" 2>/dev/null || stat -c%s "boost_udp_server" 2>/dev/null || echo "0")
client_size=$(stat -f%z "boost_udp_client" 2>/dev/null || stat -c%s "boost_udp_client" 2>/dev/null || echo "0")

# Convert to KB
server_size_kb=$(echo "scale=2; $server_size / 1024" | bc)
client_size_kb=$(echo "scale=2; $client_size / 1024" | bc)

# Display benchmark results
echo ""
echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║           BOOST.ASIO BENCHMARK RESULTS               ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "  ${CYAN}Config time:${NC}  ${config_time}s"
echo -e "  ${CYAN}Build time:${NC}   ${build_time}s"
echo -e "  ${CYAN}Server size:${NC}  ${server_size_kb} KB (${server_size} bytes)"
echo -e "  ${CYAN}Client size:${NC}  ${client_size_kb} KB (${client_size} bytes)"

# Save results to CSV
echo "Metric,Value" > benchmark_results.csv
echo "Config Time (s),${config_time}" >> benchmark_results.csv
echo "Build Time (s),${build_time}" >> benchmark_results.csv
echo "Server Size (KB),${server_size_kb}" >> benchmark_results.csv
echo "Client Size (KB),${client_size_kb}" >> benchmark_results.csv
echo "Server Size (bytes),${server_size}" >> benchmark_results.csv
echo "Client Size (bytes),${client_size}" >> benchmark_results.csv

echo ""
echo -e "${YELLOW}═══ Functional Test: Boost.Asio ═══${NC}"

# Check if binaries exist
if [ ! -f "./boost_udp_server" ] || [ ! -f "./boost_udp_client" ]; then
    echo -e "${RED}Error: Binaries not found after build!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Binaries found${NC}"

# Start server in background
echo -e "${YELLOW}Starting UDP server on port 4242...${NC}"
./boost_udp_server 4242 > server.log 2>&1 &
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
if ./boost_udp_client 127.0.0.1 4242 > client.log 2>&1; then
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

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ Boost.Asio PoC Tests PASSED!                   ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${CYAN}📊 Benchmark results saved to: benchmark_results.csv${NC}"
    echo -e "${CYAN}📁 Build logs available in: build-boost-poc/${NC}"
else
    echo -e "${RED}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ Boost.Asio PoC Tests FAILED!                   ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════╝${NC}"
fi

exit $TEST_RESULT
