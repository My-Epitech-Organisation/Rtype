---
sidebar_position: 3
---

# Serialization Methods

## Executive Summary

**Decision:** Custom Binary Packets  
**Date:** November 2025  
**Status:** ✅ Approved

Comparison of three serialization approaches: **Custom Binary Packets, Protocol Buffers (Protobuf), and JSON**. Custom binary provides the best bandwidth efficiency for real-time gameplay.

**Key Finding:** Binary packets are **317% smaller than JSON** and **39% smaller than Protobuf** for game state with 10 entities, using only **98.4 Kbps** at 60 Hz versus **410.88 Kbps** for JSON.

---

## Benchmark Results

| Metric | Binary Custom | Protobuf | JSON |
|--------|---------------|----------|------|
| **Position (bytes)** | 8 | 10 | 31 |
| **Entity (bytes)** | 20 | 26 | 95 |
| **GameState 5 entities** | 105 | 145 | 439 |
| **GameState 10 entities** | 205 | 285 | 856 |
| **Bandwidth @60Hz (5 entities)** | 50.40 Kbps | 69.60 Kbps | 210.72 Kbps |
| **Bandwidth @60Hz (10 entities)** | 98.40 Kbps | 136.80 Kbps | 410.88 Kbps |

:::tip Bandwidth Impact
Custom binary uses only **98.4 Kbps** for 10 entities at 60 Hz, versus **410.88 Kbps** for JSON — a **317% reduction**.
:::

---

## Detailed Analysis

### 1. Custom Binary Packets ✅

**Structure:**

```cpp
struct Position {
    float x;  // 4 bytes
    float y;  // 4 bytes
};  // Total: 8 bytes

struct Entity {
    uint32_t id;      // 4 bytes
    Position pos;     // 8 bytes
    Position vel;     // 8 bytes
};  // Total: 20 bytes

struct GameState {
    uint8_t entityCount;              // 1 byte
    Entity entities[MAX_ENTITIES];    // 20 * N bytes
};  // Total: 1 + 20N bytes
```

**Example (10 entities):**

```text
Header:         1 byte
Entities:       200 bytes (10 × 20)
────────────────────────
Total:          205 bytes
```

**Serialization:**

```cpp
// Write
void serialize(const GameState& state, uint8_t* buffer) {
    buffer[0] = state.entityCount;
    std::memcpy(buffer + 1, state.entities, state.entityCount * sizeof(Entity));
}

// Read
GameState deserialize(const uint8_t* buffer) {
    GameState state;
    state.entityCount = buffer[0];
    std::memcpy(state.entities, buffer + 1, state.entityCount * sizeof(Entity));
    return state;
}
```

**Advantages:**

- ✅ **Minimal Size**: 205 bytes for 10 entities
- ✅ **Zero Overhead**: No schema, no metadata
- ✅ **Maximum Speed**: Direct memory copy
- ✅ **No Dependencies**: Standard C++ only
- ✅ **Predictable**: Fixed size per entity

**Disadvantages:**

- ⚠️ **Manual Endianness**: Must handle big/little endian
- ⚠️ **No Versioning**: Schema changes break compatibility
- ⚠️ **Not Self-Describing**: Receiver must know structure

---

### 2. Protocol Buffers (Protobuf)

**Schema Definition:**

```protobuf
syntax = "proto3";

message Position {
    float x = 1;
    float y = 2;
}

message Entity {
    uint32 id = 1;
    Position pos = 2;
    Position vel = 3;
}

message GameState {
    repeated Entity entities = 1;
}
```

**Example (10 entities):**

```text
Field tags:     ~30 bytes
Entity data:    ~240 bytes
Varints:        ~15 bytes
────────────────────────
Total:          ~285 bytes
```

**Advantages:**

- ✅ **Schema Evolution**: Forward/backward compatibility
- ✅ **Cross-Language**: Works with C++, Python, Go, etc.
- ✅ **Validation**: Schema enforces structure
- ✅ **Tooling**: Code generation, debugging tools

**Disadvantages:**

- ⚠️ **39% Larger**: 285 bytes vs 205 for custom binary
- ⚠️ **Toolchain**: Requires protoc compiler
- ⚠️ **Complexity**: Build system integration
- ⚠️ **Overhead**: Field tags and varints

---

### 3. JSON

**Example:**

```json
{
  "entities": [
    {
      "id": 1,
      "pos": {"x": 100.5, "y": 200.3},
      "vel": {"x": 10.2, "y": -5.7}
    },
    {
      "id": 2,
      "pos": {"x": 150.0, "y": 300.0},
      "vel": {"x": 0.0, "y": 0.0}
    }
  ]
}
```

**Example (10 entities):**

```text
Keys & syntax:  ~400 bytes
Values:         ~300 bytes
Whitespace:     ~156 bytes
────────────────────────
Total:          ~856 bytes
```

**Advantages:**

- ✅ **Human Readable**: Easy to debug
- ✅ **Universal**: Every language supports JSON
- ✅ **Flexible**: Dynamic schema
- ✅ **Tooling**: Extensive ecosystem

**Disadvantages:**

- 🔴 **317% Larger**: 856 bytes vs 205 for binary
- 🔴 **Slow Parsing**: String conversion overhead
- 🔴 **High Bandwidth**: 410.88 Kbps @60Hz
- 🔴 **Not Suitable**: For real-time gameplay

---

## Bandwidth Analysis

### Network Requirements

R-Type target: **Less than 100 Kbps per client** for gameplay data

**At 60 Hz (60 packets/second):**

| Entities | Binary | Protobuf | JSON | Binary Target |
|----------|--------|----------|------|---------------|
| 5 | 50.40 Kbps ✅ | 69.60 Kbps ⚠️ | 210.72 Kbps ❌ | Less than 100 Kbps ✅ |
| 10 | 98.40 Kbps ✅ | 136.80 Kbps ❌ | 410.88 Kbps ❌ | Less than 100 Kbps ✅ |
| 20 | 197 Kbps ⚠️ | 274 Kbps ❌ | 821 Kbps ❌ | Less than 200 Kbps ⚠️ |

**Calculation:**

```text
Binary (10 entities):
205 bytes/packet × 60 packets/sec × 8 bits/byte = 98,400 bits/sec = 98.4 Kbps

JSON (10 entities):
856 bytes/packet × 60 packets/sec × 8 bits/byte = 410,880 bits/sec = 410.88 Kbps
```

:::tip Meeting Requirements
Only **custom binary** stays under 100 Kbps for 10 entities at 60 Hz with safe headroom.
:::

---

## Performance Comparison

### Serialization Speed

Benchmarked on AMD Ryzen 5 5600H:

| Method | Serialize (10 entities) | Deserialize | Total |
|--------|-------------------------|-------------|-------|
| **Binary** | **0.8 μs** | **0.6 μs** | **1.4 μs** |
| Protobuf | 12.5 μs | 15.3 μs | 27.8 μs |
| JSON | 45.2 μs | 67.8 μs | 113.0 μs |

**Binary is:**

- **20x faster** than Protobuf
- **80x faster** than JSON

---

### CPU Overhead

**Per frame (60 FPS = 16.67ms budget):**

| Method | Overhead | % of Frame Budget |
|--------|----------|-------------------|
| **Binary** | 1.4 μs | 0.008% ✅ |
| Protobuf | 27.8 μs | 0.17% ✅ |
| JSON | 113 μs | 0.68% ⚠️ |

All methods have acceptable CPU overhead, but binary is negligible.

---

## Use Case Recommendations

### Custom Binary ✅ RECOMMENDED

**Use for:**

- ✅ **Gameplay packets** (position, velocity, health)
- ✅ **High-frequency updates** (60 Hz)
- ✅ **Real-time data** (input, projectiles)
- ✅ **Bandwidth-critical** (mobile networks)

**Example:**

```cpp
struct PlayerUpdate {
    uint32_t playerId;
    float x, y;
    float vx, vy;
    uint8_t health;
};  // 21 bytes

// Send at 60 Hz
udpSocket.send(&update, sizeof(update));
```

---

### Protobuf ⚠️ OPTIONAL

**Use for:**

- ⚠️ **Cross-language tools** (Python admin dashboard)
- ⚠️ **Schema evolution** (long-term compatibility)
- ⚠️ **Complex nested data** (matchmaking, lobbies)
- ⚠️ **Low-frequency messages** (chat, events)

**Example:**

```protobuf
message MatchmakingRequest {
    string player_name = 1;
    uint32 skill_rating = 2;
    repeated string preferred_modes = 3;
}
```

---

### JSON ❌ NOT RECOMMENDED (Gameplay)

**Use for:**

- ✅ **Configuration files** (game settings)
- ✅ **Level data** (enemy spawns)
- ✅ **Debug logging** (error reports)
- ❌ **NOT for gameplay packets** (too large)

**Example:**

```json
{
  "window": {
    "width": 1920,
    "height": 1080,
    "fullscreen": true
  },
  "audio": {
    "master_volume": 0.8,
    "music_volume": 0.6
  }
}
```

---

## Hybrid Strategy ✅

**Recommended approach for R-Type:**

```cpp
// High-frequency gameplay: Binary
struct EntityUpdate {
    uint32_t id;
    float x, y, vx, vy;
};  // 20 bytes, sent at 60 Hz

// Low-frequency events: Protobuf or JSON
{
  "event": "player_killed",
  "victim_id": 42,
  "killer_id": 17
}  // Sent once per death

// Configuration: JSON
{
  "server": {
    "port": 8080,
    "max_players": 4
  }
}  // Loaded at startup
```

---

## Implementation Guide

### Binary Packet Design

```cpp
// Packet header
struct PacketHeader {
    uint16_t packetType;  // 2 bytes
    uint16_t payloadSize; // 2 bytes
};  // 4 bytes total

// Packet types
enum class PacketType : uint16_t {
    ENTITY_UPDATE = 1,
    PLAYER_INPUT = 2,
    SHOOT_BULLET = 3,
    PLAYER_DEATH = 4
};

// Example: Entity update packet
struct EntityUpdatePacket {
    PacketHeader header{PacketType::ENTITY_UPDATE, sizeof(entities)};
    uint8_t entityCount;
    Entity entities[10];
};
```

### Endianness Handling

```cpp
// Ensure cross-platform compatibility
uint32_t htonl_custom(uint32_t hostlong) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return __builtin_bswap32(hostlong);
    #else
        return hostlong;
    #endif
}

void serialize(uint32_t value, uint8_t* buffer) {
    uint32_t netValue = htonl_custom(value);
    std::memcpy(buffer, &netValue, sizeof(netValue));
}
```

---

## Final Recommendation

✅ **Use Custom Binary Packets** for R-Type gameplay networking.

**Rationale:**

1. **317% smaller** than JSON (205 bytes vs 856 bytes)
2. **39% smaller** than Protobuf (205 bytes vs 285 bytes)
3. **Meets bandwidth target** (98.4 Kbps < 100 Kbps @60Hz)
4. **80x faster** serialization than JSON
5. **Zero tooling overhead** (no protoc, no external libs)
6. **Maximum performance** for real-time gameplay

**Strategy:**

- Binary for gameplay packets (60 Hz updates)
- Protobuf for tooling and cross-language needs
- JSON for configuration and debugging only

---

## References

- PoC implementations: `/PoC/PoC_Network_Serialization/`
- Benchmark script: `/PoC/PoC_Network_Serialization/run_all_pocs.py`
- Results: `/PoC/PoC_Network_Serialization/result.md`
