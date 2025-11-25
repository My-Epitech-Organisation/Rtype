# JSON Serialization PoC

## Objective

Determine if JSON serialization is suitable for network packets at 60 packets/second in R-Type game.

**Question:** Is JSON too big for 60 packets/sec?

## Scope

- Serialize game entity positions to JSON strings
- Measure byte size per packet
- Calculate bandwidth usage
- Evaluate performance overhead

## Usage

### Quick Test

```bash
chmod +x test_poc.sh
./test_poc.sh
```

### Manual Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/test_json_size
./build/bin/benchmark_json
```

## Test Cases

### 1. Size Analysis (`test_json_size`)

Measures JSON packet sizes for different entity counts:
- Single position (x, y, rotation)
- Single entity (id, position, velocity, health, team)
- Game state packet (5 entities)
- Large packet (10 entities)

### 2. Performance Benchmark (`benchmark_json`)

Measures serialization speed:
- Iterations: 10,000 per configuration
- Entity counts: 1, 2, 5, 10, 20, 50, 100
- Target: 60 Hz (16.67ms per packet)

## Data Structures

```cpp
struct Position {
    float x, y, rotation;
};

struct EntityState {
    uint32_t id;
    Position position;
    float velocity_x, velocity_y;
    uint8_t health, team;
};

struct GameStatePacket {
    uint32_t timestamp;
    vector<EntityState> entities;
};
```

## Expected Results

### Size Metrics

| Content | JSON Size | Bandwidth @ 60Hz |
|---------|-----------|------------------|
| Position | ~32 bytes | 1.9 Kbps |
| Entity | ~85 bytes | 5.1 Kbps |
| 5 Entities | ~450 bytes | 27 Kbps |
| 10 Entities | ~880 bytes | 53 Kbps |

### Performance

- Single entity: ~2 µs (500,000 pkt/s) ✓
- 5 entities: ~5 µs (200,000 pkt/s) ✓
- 10 entities: ~10 µs (100,000 pkt/s) ✓
- All configurations exceed 60 Hz target

## Bandwidth Analysis

**Target:** < 10 Kbps for low-bandwidth scenarios

**Results:**
- 5 entities @ 60 Hz = **27 Kbps** ⚠️ (exceeds target)
- 10 entities @ 60 Hz = **53 Kbps** ⚠️ (exceeds target)

**Maximum entities for 10 Kbps:** ~1 entity

## Exit Criteria

✅ **Bandwidth usage calculated**
- Single position: 1.9 Kbps ✓
- 5 entities packet: 27 Kbps ✓
- 10 entities packet: 53 Kbps ✓

## Conclusion

### Performance ✓
JSON serialization is **fast enough** for 60 Hz:
- Can handle 100+ entities without bottleneck
- Negligible CPU overhead (~5 µs for 5 entities)

### Bandwidth ✗
JSON is **too large** for low-bandwidth scenarios:
- 5 entities = 27 Kbps (2.7x over 10 Kbps target)
- High overhead from text format and field names
- Not suitable for mobile/limited bandwidth

### Recommendation

**Use Binary Serialization** instead:
- Expected size: ~20-30 bytes for 5 entities (vs 450 bytes JSON)
- Bandwidth: ~1.5 Kbps @ 60 Hz (vs 27 Kbps)
- Trade-off: Less human-readable, but essential for network efficiency

JSON is acceptable for:
- Debug/development tools
- Configuration files
- Low-frequency updates (< 10 Hz)

Binary serialization required for:
- Real-time game state (60 Hz)
- Low-bandwidth connections
- Production network protocol

## Dependencies

- nlohmann/json 3.11.3 (via CPM)
- CMake 3.15+
- C++20 compiler

## Related

- Issue: [Spike] PoC: JSON Serialization
- Parent: [Spike] [Main] Network Serialization PoC #61
- Timebox: 28/11/2025 - 29/11/2025
- Next: Binary Serialization PoC (Protobuf/FlatBuffers/Custom)
