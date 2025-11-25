#!/usr/bin/env bash
# Protobuf PoC test runner
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

printf "%b\n" "${CYAN}╔═══════════════════════════════════════════╗${NC}"
printf "%b\n" "${CYAN}║        Protobuf Serialization PoC        ║${NC}"
printf "%b\n" "${CYAN}╚═══════════════════════════════════════════╝${NC}"
printf "\n"

printf "%b\n" "${YELLOW}═══ Building (FetchContent + Ninja) ═══${NC}"
rm -rf build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release > build_config.log
cmake --build build > build_compile.log
printf "%b\n\n" "${GREEN}✓ Build complete${NC}"

printf "%b\n" "${YELLOW}═══ Measuring Size ═══${NC}"
./build/bin/test_protobuf_size | tee protobuf_size_results.txt

printf "\n%b\n" "${YELLOW}═══ Benchmarking ═══${NC}"
./build/bin/benchmark_protobuf | tee protobuf_benchmark_results.txt

printf "\n%b\n" "${YELLOW}═══ Comparing with JSON ═══${NC}"
./build/bin/compare_serialization | tee comparison_results.txt

printf "\n%b\n" "${YELLOW}═══ Aggregating CSV ═══${NC}"
position_bytes=$(grep "Vec2 (position)" protobuf_size_results.txt | awk '{print $NF}')
entity_bytes=$(grep "EntityState" protobuf_size_results.txt | awk '{print $NF}')
packet5_bytes=$(grep "GameState x5" protobuf_size_results.txt | awk '{print $NF}')
packet10_bytes=$(grep "GameState x10" protobuf_size_results.txt | awk '{print $NF}')

bandwidth5=$(grep "5 entities:" protobuf_size_results.txt | awk '{print $(NF-1)}')
bandwidth10=$(grep "10 entities:" protobuf_size_results.txt | awk '{print $(NF-1)}')

cat > protobuf_serialization_results.csv <<EOF
Metric,Protobuf,Binary,JSON
Position Size (bytes),${position_bytes},8,31
Entity Size (bytes),${entity_bytes},20,95
GameState 5 Entities (bytes),${packet5_bytes},105,439
GameState 10 Entities (bytes),${packet10_bytes},205,856
Bandwidth 5 entities @60Hz (Kbps),${bandwidth5},49.2,205.8
Bandwidth 10 entities @60Hz (Kbps),${bandwidth10},96.1,401.2
EOF

printf "%b\n" "${GREEN}✓ Results saved to protobuf_serialization_results.csv${NC}"

printf "\n%b\n" "${YELLOW}Artifacts:${NC}"
printf "  • protobuf_size_results.txt\n  • protobuf_benchmark_results.txt\n  • comparison_results.txt\n  • protobuf_serialization_results.csv\n  • build_config.log\n  • build_compile.log\n"

printf "\n%b\n" "${GREEN}Protobuf PoC complete${NC}"
