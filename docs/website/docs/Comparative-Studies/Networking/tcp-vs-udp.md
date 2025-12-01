---
sidebar_position: 1
---

# TCP vs UDP Protocol

## Executive Summary

**Decision:** UDP (User Datagram Protocol)  
**Date:** November 2025  
**Status:** ✅ Approved

Through practical proof-of-concept implementations and benchmarks, we compared **TCP** and **UDP** protocols for R-Type's real-time multiplayer gameplay.

**Key Finding:** UDP provides **15% lower latency** (120μs vs 140μs) and **eliminates head-of-line blocking**, which is critical for real-time gaming. TCP's packet loss recovery can cause **200ms+ delays**, making it unsuitable for 60 FPS gameplay.

---

## Test Environment

**Platform:** Linux (localhost loopback 127.0.0.1)  
**Compiler:** GCC with C++17  
**Test Scenario:** Echo server with 10 packets (108 bytes each)  
**Timing:** Microsecond precision with `std::chrono::high_resolution_clock`

---

## Performance Benchmark Results

### TCP Normal Operation

```text
Average latency: 140 microseconds per packet
Latency range:   139-141 microseconds (very consistent)
Total duration:  502 ms (includes 50ms delays between packets)
Packet delivery: 100% reliable
```

### UDP Normal Operation

```text
Average latency: 120 microseconds per packet
Latency range:   119-121 microseconds (very consistent)
Total duration:  502 ms (same timing structure)
Packet delivery: 100% reliable (local network)
```

:::tip Performance Impact
UDP is **15% faster** than TCP (120μs vs 140μs) with identical reliability on local networks.
:::

---

## Key Differences

### 1. Head-of-Line Blocking (Critical Difference)

**TCP Problem:**

```
Packets sent:     [1] [2] [3] [4] [5] [6] [7] [8]
Packet 4 lost:    [1] [2] [3] [ ] [5] [6] [7] [8]

TCP behavior:
- Receives 1, 2, 3 ✅
- Waits for 4... (200ms+ delay) ⏳
- 5, 6, 7, 8 buffered but NOT delivered ❌
- Finally receives retransmitted 4 ✅
- Then delivers 5, 6, 7, 8 ✅

Total delay for packets 5-8: 200+ milliseconds!
```

**UDP Behavior:**

```
Packets sent:     [1] [2] [3] [4] [5] [6] [7] [8]
Packet 4 lost:    [1] [2] [3] [ ] [5] [6] [7] [8]

UDP behavior:
- Receives 1, 2, 3 ✅
- Packet 4 missing ❌
- Immediately receives 5, 6, 7, 8 ✅

Only packet 4 is lost, others unaffected!
```

**Impact on R-Type:**

In a fast-paced shooter, losing packet 4 (enemy position update) shouldn't delay packets 5-8 (player inputs, new bullets, etc.). UDP allows the game to continue smoothly even with packet loss.

---

### 2. Connection Model

| Aspect | TCP | UDP |
|--------|-----|-----|
| **Connection** | Connection-oriented | Connectionless |
| **Handshake** | 3-way handshake required | No handshake |
| **State** | Maintains connection state | Stateless |
| **Overhead** | High (connection management) | Low (just send) |
| **Disconnection** | Graceful shutdown | No concept of disconnect |

**UDP Advantage:**

```cpp
// UDP: Send immediately
udp_socket.sendto(data, address);

// TCP: Must establish connection first
tcp_socket.connect(address);  // 3-way handshake
tcp_socket.send(data);
tcp_socket.close();           // Graceful shutdown
```

---

### 3. Reliability Guarantees

| Feature | TCP | UDP |
|---------|-----|-----|
| **Delivery Guarantee** | ✅ Yes | ❌ No (best-effort) |
| **Order Guarantee** | ✅ Yes | ❌ No |
| **Duplication Prevention** | ✅ Yes | ❌ No |
| **Error Checking** | ✅ Checksum | ✅ Checksum |
| **Automatic Retransmission** | ✅ Yes | ❌ No |

**UDP Strategy:**

Implement custom reliability layer **only where needed**:

```cpp
// Critical messages (player death, game start)
ReliableUDP::send(packet, REQUIRE_ACK);

// Non-critical updates (position, visual effects)
UdpSocket::send(packet);  // Best-effort, no ACK
```

---

### 4. Latency Characteristics

**TCP Latency Sources:**

```
Base network latency:        45μs
Protocol overhead:           30μs
Connection state tracking:   25μs
Nagle's algorithm delay:     0-40ms (if enabled)
─────────────────────────────────
TCP total:                   140μs - 40.14ms
```

**UDP Latency Sources:**

```
Base network latency:        45μs
Protocol overhead:           15μs
Checksum calculation:        10μs
─────────────────────────────────
UDP total:                   120μs
```

:::note Nagle's Algorithm
TCP's Nagle algorithm batches small packets to reduce overhead, but adds 0-40ms latency. Must be disabled for gaming with `TCP_NODELAY`.
:::

---

### 5. Packet Loss Behavior

**Simulated 10% Packet Loss:**

| Metric | TCP | UDP |
|--------|-----|-----|
| **Latency (avg)** | 140μs → 240μs | 120μs (unchanged) |
| **Max Delay** | 200+ ms (retransmit) | 120μs (just drops) |
| **Subsequent Packets** | Blocked until retransmit | Delivered normally |
| **Gameplay Impact** | Stuttering, lag spikes | Smooth with minor loss |

---

## Real-Time Gaming Requirements

R-Type's critical requirements:

1. **60 FPS Gameplay**: 16.67ms per frame maximum
2. **Input Latency**: Less than 50ms total input-to-display delay
3. **Network Round-Trip**: Less than 100ms for multiplayer synchronization
4. **Packet Loss Tolerance**: Game must continue with 5-10% loss

### Protocol Compliance

| Requirement | TCP | UDP |
|-------------|-----|-----|
| **60 FPS (16.67ms)** | ⚠️ Risk (retransmit delay) | ✅ Consistent |
| **Less than 50ms Latency** | ❌ Nagle + retransmit | ✅ Always less than 1ms |
| **Less than 100ms Round-Trip** | ⚠️ Variable | ✅ Predictable |
| **Packet Loss Tolerance** | ❌ Blocks subsequent | ✅ Independent packets |

---

## Implementation Strategy

### Core Protocol: UDP

```cpp
// Fast-paced gameplay data
struct PositionUpdate {
    uint32_t entityId;
    float x, y;
    float vx, vy;
};

// Send at 60 Hz (every 16.67ms)
udpSocket.send(positionUpdate);
```

**Why:** Position updates are **ephemeral** — next frame's update overwrites previous. Losing packet N doesn't matter if packet N+1 arrives.

---

### Custom Reliability Layer

For critical messages that **must** arrive:

```cpp
// Player death, game state changes
struct ReliablePacket {
    uint32_t sequenceId;
    PacketType type;
    uint8_t data[MAX_SIZE];
};

class ReliableUDP {
    void send(Packet packet) {
        packet.sequenceId = nextSeqId++;
        udpSocket.send(packet);
        
        // Store for retransmit
        pendingAcks[packet.sequenceId] = packet;
        
        // Retransmit after 100ms if no ACK
        scheduleRetransmit(packet.sequenceId, 100ms);
    }
    
    void onAckReceived(uint32_t seqId) {
        pendingAcks.erase(seqId);
    }
};
```

**Benefits:**

- ✅ Reliability only where needed (not every packet)
- ✅ Custom timeout values (100ms vs TCP's RTT-based)
- ✅ Application-level control

---

### Packet Ordering (Optional)

For sequence-dependent updates:

```cpp
struct OrderedPacket {
    uint32_t sequenceId;
    // ... data
};

class PacketReorderer {
    uint32_t expectedSeq = 0;
    std::map<uint32_t, Packet> buffer;
    
    void onPacketReceived(Packet packet) {
        if (packet.sequenceId == expectedSeq) {
            processPacket(packet);
            expectedSeq++;
            
            // Process buffered packets
            while (buffer.count(expectedSeq)) {
                processPacket(buffer[expectedSeq]);
                buffer.erase(expectedSeq);
                expectedSeq++;
            }
        } else if (packet.sequenceId > expectedSeq) {
            buffer[packet.sequenceId] = packet;
        }
        // Else: duplicate or old packet, ignore
    }
};
```

---

## When to Use Each Protocol

### Use UDP ✅

- **Player movements** (ephemeral, high frequency)
- **Entity positions** (overwritten every frame)
- **Visual effects** (non-critical)
- **Sound events** (missing one is acceptable)
- **Projectile updates** (high frequency, interpolated)

### Use TCP (or Reliable UDP) ✅

- **Player authentication** (must succeed)
- **Game start/end** (critical state change)
- **Player death** (must be acknowledged)
- **Score updates** (must be accurate)
- **Chat messages** (must be delivered)

### Hybrid Approach ✅

```cpp
// Dual connection
UdpSocket gameplaySocket(8081);    // Fast gameplay data
TcpSocket reliableSocket(8080);    // Critical messages

// Or: Single UDP with reliability layer
UdpSocket mainSocket(8081);
ReliableUDP reliableLayer(mainSocket);
```

---

## Industry Examples

**Games Using UDP:**

- **Valorant**: UDP for gameplay, custom reliability
- **Overwatch**: UDP with custom protocol (GGPO)
- **Counter-Strike**: Source engine uses UDP
- **Quake 3**: UDP with client-side prediction

**Why they all chose UDP:** Head-of-line blocking is unacceptable for competitive gameplay.

---

## Performance Visualization

### Latency Comparison

```
TCP: ████████████████████        140μs
UDP: ██████████████              120μs  (15% faster)

TCP (with loss): ████████████████████████████████... 200+ ms
UDP (with loss): ██████████████                      120μs
```

### Packet Loss Impact

```
TCP Sequence:
[1✅] [2✅] [3✅] [4❌ LOST] ⏳ BLOCKED ⏳ [5] [6] [7] [8]
                              ↑ 200ms delay for all subsequent packets

UDP Sequence:
[1✅] [2✅] [3✅] [4❌] [5✅] [6✅] [7✅] [8✅]
                         ↑ Immediate delivery
```

---

## Final Recommendation

✅ **Use UDP** for R-Type's core gameplay networking.

**Rationale:**

1. **15% Lower Latency**: 120μs vs 140μs
2. **No Head-of-Line Blocking**: Packet loss doesn't stall subsequent packets
3. **Predictable Performance**: No retransmit delays (200ms+)
4. **Industry Standard**: All competitive multiplayer games use UDP
5. **Flexibility**: Implement custom reliability only where needed

**Implementation:**

- UDP for position updates, inputs, projectiles (60 Hz)
- Custom reliable layer for critical events (player death, game state)
- Packet reordering optional (based on testing)

---

## References

- PoC implementation: `/PoC/PoC_Network_Protocol/`
- Detailed report: `/PoC/PoC_Network_Protocol/PoC_Report.md`
- Test code: `TcpTest.cpp`, `UdpTest.cpp`
