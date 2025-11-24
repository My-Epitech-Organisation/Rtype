# TCP Head-of-Line Blocking Demonstration

This PoC demonstrates the head-of-line blocking issue with TCP, which is critical for understanding why UDP is preferred for real-time multiplayer games like R-Type.

## What is Head-of-Line Blocking?

In TCP, packets must be delivered in order. If one packet is lost or delayed, all subsequent packets are held in a queue until the missing packet arrives. This can cause significant latency spikes in real-time applications.

## How to Run the Test

### Prerequisites

- CMake 3.16+
- C++17 compiler
- Linux/macOS (or adjust for Windows)

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run Server

```bash
./tcptest server
```

### Run Client (Normal Operation)

In another terminal:

```bash
./tcptest client
```

This will send 10 packets and show normal TCP behavior.

### Run Client (Simulate Packet Drop)

```bash
./tcptest client simulate_drop
```

This simulates dropping packet 5, demonstrating head-of-line blocking.

## Expected Behavior

### Normal Operation

- All 10 packets are sent and received quickly
- Responses come back in order
- No significant delays

### With Packet Drop Simulation

**Current Implementation:**

- Packets 1-4 are processed normally
- Packet 5 is skipped (not sent by client)
- Packets 6-10 are sent and received normally (no blocking observed)
- Server renumbers received packets sequentially

**Limitation:**

The current simulation doesn't trigger actual TCP retransmission because TCP doesn't know packet 5 is missing - the sequence continues normally.

**For True Head-of-Line Blocking:**

Network-level packet dropping would be needed to trigger TCP's retransmission mechanism and demonstrate the blocking effect.

## Why This Matters for R-Type

In a fast-paced shooter like R-Type:

- Player inputs must be processed immediately (<50ms latency)
- Head-of-line blocking can cause 200ms+ delays
- This makes the game feel unresponsive and unplayable
- UDP avoids this by allowing out-of-order delivery

## Test Results

### Normal TCP (no drops)

- Average latency: ~140μs per packet
- All packets delivered in order
- Total test duration: 502ms

### TCP with simulated drop

- Packet 5 skipped by client
- Server received packets 6-10 as packets 5-9
- No head-of-line blocking observed (simulation limitation)
- Total test duration: 452ms

*Note: The current simulation skips sending packet 5 but doesn't trigger actual TCP retransmission. For true head-of-line blocking demonstration, network-level packet dropping would be needed.*

## Conclusion

TCP's reliability comes at the cost of potential latency spikes due to head-of-line blocking, making it unsuitable for real-time game input handling. UDP with custom reliability mechanisms is the better choice for R-Type's core gameplay loop.
