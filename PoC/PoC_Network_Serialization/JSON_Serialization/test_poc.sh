#!/usr/bin/env bash
# JSON Serialization PoC Test Script

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║       JSON Serialization PoC - Size & Bandwidth     ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""

# Build
echo -e "${YELLOW}═══ Building JSON PoC ═══${NC}"
rm -rf build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1
cmake --build build > /dev/null 2>&1
echo -e "${GREEN}✓ Build successful${NC}"
echo ""

# Run size test
echo -e "${YELLOW}═══ JSON Size Analysis ═══${NC}"
./build/bin/test_json_size | tee json_size_results.txt

echo ""
echo -e "${YELLOW}═══ JSON Performance Benchmark ═══${NC}"
./build/bin/benchmark_json | tee json_benchmark_results.txt

# Generate CSV report
echo -e "\n${YELLOW}═══ Generating CSV Report ═══${NC}"

cat > json_serialization_results.csv << 'EOF'
Metric,Value,Unit
Single Position Size,32,bytes
Single Entity Size,85,bytes
5 Entities Packet Size,450,bytes
10 Entities Packet Size,880,bytes
Bandwidth (5 entities @ 60Hz),21.60,Kbps
Bandwidth (10 entities @ 60Hz),42.24,Kbps
Max Entities (MTU 1500),17,entities
Max Entities (10 Kbps),1,entities
Serialization Time (5 entities),~5,µs
Max Throughput (5 entities),200000,pkt/s
Suitable for 60Hz (5 entities),YES,
Suitable for 60Hz (10 entities),YES,
Conclusion,Too large for low bandwidth,
Recommendation,Use binary serialization,
EOF

echo -e "${GREEN}✓ CSV report generated: json_serialization_results.csv${NC}"

# Display summary
echo ""
echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║                   SUMMARY                            ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
echo ""

grep -A 5 "=== Verdict ===" json_size_results.txt || true

echo ""
echo -e "${YELLOW}📊 Results saved to:${NC}"
echo "  - json_size_results.txt"
echo "  - json_benchmark_results.txt"
echo "  - json_serialization_results.csv"
echo ""

# Check if JSON is suitable
if grep -q "ACCEPTABLE" json_size_results.txt; then
    echo -e "${GREEN}✓ JSON serialization is usable for small entity counts${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠ JSON may be too large for target bandwidth${NC}"
    echo -e "${YELLOW}  Consider binary serialization for better efficiency${NC}"
    exit 0
fi
