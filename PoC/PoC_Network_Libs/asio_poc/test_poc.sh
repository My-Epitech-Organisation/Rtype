#!/usr/bin/env bash
# ASIO PoC Test Script
# Tests the UDP server and client examples

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== ASIO Standalone PoC Test Script ===${NC}"

# Build the project first
echo -e "${YELLOW}Building ASIO PoC...${NC}"
mkdir -p build-poc
cd build-poc

if cmake .. -DBUILD_EXAMPLES=ON > /dev/null 2>&1; then
    echo -e "${GREEN}✓ CMake configuration successful${NC}"
else
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
fi

if cmake --build . -- -j > /dev/null 2>&1; then
    echo -e "${GREEN}✓ Build successful${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

cd ..

# Check if binaries exist
if [ ! -f "./asio_udp_server" ] || [ ! -f "./asio_udp_client" ]; then
    echo -e "${RED}Error: Binaries not found after build!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Binaries found${NC}"

# Start server in background
echo -e "${YELLOW}Starting UDP server on port 4242...${NC}"
./asio_udp_server 4242 > server.log 2>&1 &
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
if ./asio_udp_client 127.0.0.1 4242 > client.log 2>&1; then
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

# Cleanup
echo ""
echo -e "${YELLOW}Stopping server...${NC}"
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

# Cleanup log files
rm -f server.log client.log

echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ ASIO PoC Tests PASSED!         ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════╝${NC}"
else
    echo -e "${RED}╔════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ ASIO PoC Tests FAILED!         ║${NC}"
    echo -e "${RED}╚════════════════════════════════════╝${NC}"
fi

exit $TEST_RESULT
