# Protobuf Serialization PoC

## Goal

Evaluate whether Protocol Buffers justify their setup cost for the R-Type networking layer.

- ✅ Setup protoc via CMake FetchContent
- ✅ Serialize representative game state
- ✅ Measure size & performance vs JSON and binary custom packets

## Usage

```bash
chmod +x test_poc.sh
./test_poc.sh
```

Artifacts:

- `protobuf_size_results.txt`
- `protobuf_benchmark_results.txt`
- `comparison_results.txt`
- `protobuf_serialization_results.csv`

## Proto Schema

```proto
syntax = "proto3";
package rtype.serialization;

message Vec2 {
  float x = 1;
  float y = 2;
}

message EntityState {
  uint32 id = 1;
  Vec2 position = 2;
  Vec2 velocity = 3;
}

message GameState {
  uint32 timestamp = 1;
  repeated EntityState entities = 2;
}
```

## Tests

### Size (`test_protobuf_size`)

- Measures Vec2, EntityState, GameState w/5 & 10 entities
- Calculates bandwidth @ 60 Hz
- Compares with JSON and Binary packets

### Performance (`benchmark_protobuf`)

- 100k iterations per configuration
- Entities tested: 1, 2, 5, 10, 20, 50, 100
- Reports serialization-only and serialize+deserialize throughput

### JSON Comparison (`compare_serialization`)

- Dumps protobuf bytes in hex
- Prints equivalent JSON payload
- Computes % size savings

## Requirements

- CMake ≥ 3.20
- Ninja (recommended)
- C++20 compiler (Clang/GCC)
- Internet access to fetch `protocolbuffers/protobuf` (v28.2)

## Issue Link

- [Spike] PoC: Protobuf (Timebox 29-30/11/2025)
- Parent: [Spike] [Main] Network Serialization PoC #61
