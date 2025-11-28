---
sidebar_position: 2
---

# Network Library Selection

## Executive Summary

**Decision:** ASIO Standalone 1.30.2  
**Date:** November 2025  
**Status:** ✅ Approved

Comprehensive benchmarking of four networking libraries: **ASIO Standalone, Boost.Asio, Qt Network, and ACE**. ASIO Standalone offers the best balance between performance, binary size, and zero external dependencies.

**Key Finding:** ASIO Standalone provides **header-only** architecture with **no dependencies**, **smaller binaries** than Boost (347 KB vs 391 KB), and **faster configuration** (17.7s vs 26.7s) without requiring framework overhead.

---

## Benchmark Results

| Library | Config Time | Build Time | Server Size | Client Size | Dependencies | Status |
|---------|-------------|------------|-------------|-------------|--------------|--------|
| **ASIO Standalone** | **17.7s** | **1.66s** | **347 KB** | **355 KB** | **None** | ✅ **SELECTED** |
| Boost.Asio | 26.7s | 1.30s | 391 KB | 455 KB | Full Boost | ⚠️ Slower config |
| Qt Network | 17.2s | 1.30s | 47 KB | 75 KB | Qt6 Framework | ❌ QCoreApplication required |
| ACE | N/A | N/A | N/A | N/A | ACE Framework | ❌ Config failed |

:::tip Performance Summary
ASIO Standalone: **50% faster configuration** than Boost, **13% smaller binaries**, and **zero dependencies**.
:::

---

## Decision Factors

### ✅ Why ASIO Standalone?

**1. Zero Dependencies**

```cmake
# Just download header files
CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-30-2
    OPTIONS "ASIO_STANDALONE ON"
)

# No Boost, no Qt, no external frameworks
target_include_directories(server PRIVATE ${asio_SOURCE_DIR}/asio/include)
```

**2. Header-Only Library**

- ✅ No linking required
- ✅ No binary compatibility issues
- ✅ Simplified deployment
- ✅ Faster incremental builds

**3. Smaller Binaries**

```text
Server Binary Sizes:
ASIO Standalone:  347 KB  ✅
Boost.Asio:       391 KB  (13% larger)
Qt Network:       47 KB   (requires Qt runtime!)
```

**4. Fast Configuration**

```text
CMake Configure Times:
ASIO Standalone:  17.7s  ✅
Qt Network:       17.2s
Boost.Asio:       26.7s  (50% slower)
```

**5. Modern C++20 Compatible**

```cpp
#include <asio.hpp>

asio::io_context io;
asio::ip::udp::socket socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 8080));

// Async operations with coroutines (C++20)
asio::co_spawn(io, 
    []() -> asio::awaitable<void> {
        co_await async_read(...);
    }, 
    asio::detached);
```

---

## Why NOT the Alternatives?

### ❌ Boost.Asio

**Problems:**

- 🔴 **Entire Boost Framework**: Requires full Boost installation (~300MB)
- 🔴 **Slower Configuration**: 26.7s vs 17.7s (50% slower)
- 🔴 **Larger Binaries**: 13% bigger (391 KB vs 347 KB)
- 🔴 **Dependency Overhead**: All of Boost for just networking

**When to use Boost.Asio:**

- Already using Boost for other features
- Need Boost-specific integrations

---

### ❌ Qt Network

**Problems:**

- 🔴 **Requires QCoreApplication**: Incompatible with game loop architecture
- 🔴 **Event Loop Conflict**: `QCoreApplication::exec()` blocks main thread
- 🔴 **Framework Dependency**: Needs entire Qt6 runtime
- 🔴 **Not Suitable for Games**: Designed for GUI applications

**Example of the problem:**

```cpp
// Qt Network requires event loop
QCoreApplication app(argc, argv);

QUdpSocket socket;
socket.bind(8080);

// ❌ This blocks the game loop!
return app.exec();
```

**Why it doesn't work for R-Type:**

```cpp
// Game loop structure
while (running) {
    processInput();        // 60 FPS loop
    updateGame(deltaTime); // Can't call app.exec() here!
    renderFrame();
}
```

---

### ❌ ACE (Adaptive Communication Environment)

**Problems:**

- 🔴 **Deprecated**: Not actively maintained for modern C++
- 🔴 **Configuration Failed**: Couldn't configure on test environment
- 🔴 **Legacy Design**: Pre-C++11 architecture
- 🔴 **Complex API**: Overly abstracted for simple use cases

---

## Technical Comparison

### API Simplicity

**ASIO Standalone (Clean):**

```cpp
#include <asio.hpp>

// UDP Server
asio::io_context io;
asio::ip::udp::socket socket(io, 
    asio::ip::udp::endpoint(asio::ip::udp::v4(), 8080));

char buffer[1024];
asio::ip::udp::endpoint remote;
socket.receive_from(asio::buffer(buffer), remote);
socket.send_to(asio::buffer("ACK"), remote);
```

**Boost.Asio (Identical API):**

```cpp
#include <boost/asio.hpp>

// Same API, just different namespace
boost::asio::io_context io;
// ... rest is identical
```

**Qt Network (Different Paradigm):**

```cpp
#include <QUdpSocket>
#include <QCoreApplication>

QCoreApplication app(argc, argv);
QUdpSocket socket;
socket.bind(QHostAddress::Any, 8080);

QObject::connect(&socket, &QUdpSocket::readyRead, [&]() {
    QByteArray buffer = socket.receiveDatagram().data();
    socket.writeDatagram("ACK", sender);
});

app.exec();  // ❌ Blocks game loop
```

---

### Performance Characteristics

**Async I/O Model:**

All three use asynchronous I/O, but:

| Library | I/O Model | Thread Model | Overhead |
|---------|-----------|--------------|----------|
| ASIO Standalone | Proactor | Single/Multi | Minimal |
| Boost.Asio | Proactor | Single/Multi | Minimal |
| Qt Network | Signals/Slots | Event-driven | Framework |
| ACE | Reactor/Proactor | Various | High |

**Memory Footprint:**

```text
Runtime Memory Usage (10,000 connections):
ASIO Standalone:  ~15 MB
Boost.Asio:       ~15 MB
Qt Network:       ~45 MB (Qt framework overhead)
```

---

### Cross-Platform Support

| Library | Windows | Linux | macOS | BSD |
|---------|---------|-------|-------|-----|
| ASIO Standalone | ✅ | ✅ | ✅ | ✅ |
| Boost.Asio | ✅ | ✅ | ✅ | ✅ |
| Qt Network | ✅ | ✅ | ✅ | ❌ |
| ACE | ⚠️ | ⚠️ | ⚠️ | ⚠️ |

---

## R-Type Requirements

**What we need:**

1. ✅ **UDP Support**: Real-time gameplay packets
2. ✅ **Async I/O**: Non-blocking operations
3. ✅ **Zero Dependencies**: Simplify build system
4. ✅ **Cross-Platform**: Linux + Windows
5. ✅ **Modern C++**: C++20 coroutines support
6. ✅ **Small Binaries**: Minimize distribution size

**ASIO Standalone compliance:**

| Requirement | Compliance |
|-------------|------------|
| UDP Support | ✅ Full support |
| Async I/O | ✅ io_context + coroutines |
| Zero Dependencies | ✅ Header-only |
| Cross-Platform | ✅ Windows/Linux/macOS |
| Modern C++20 | ✅ Coroutine support |
| Small Binaries | ✅ 347 KB server |

---

## Implementation Example

### Server (UDP Echo)

```cpp
#include <asio.hpp>
#include <iostream>

class UdpServer {
    asio::io_context io_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_;
    std::array<char, 1024> buffer_;

public:
    UdpServer(uint16_t port) 
        : socket_(io_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
        startReceive();
    }

    void run() {
        io_.run();
    }

private:
    void startReceive() {
        socket_.async_receive_from(
            asio::buffer(buffer_), 
            remote_,
            [this](std::error_code ec, std::size_t bytes) {
                if (!ec) {
                    handleReceive(bytes);
                }
                startReceive();  // Continue receiving
            });
    }

    void handleReceive(std::size_t bytes) {
        std::cout << "Received " << bytes << " bytes\n";
        socket_.send_to(asio::buffer(buffer_, bytes), remote_);
    }
};

int main() {
    UdpServer server(8080);
    server.run();
}
```

### Client (UDP Send)

```cpp
#include <asio.hpp>

class UdpClient {
    asio::io_context io_;
    asio::ip::udp::socket socket_;

public:
    UdpClient() : socket_(io_) {
        socket_.open(asio::ip::udp::v4());
    }

    void send(const std::string& message, const std::string& host, uint16_t port) {
        asio::ip::udp::endpoint endpoint(
            asio::ip::make_address(host), 
            port);
        
        socket_.send_to(asio::buffer(message), endpoint);
    }
};

int main() {
    UdpClient client;
    client.send("Hello Server", "127.0.0.1", 8080);
}
```

---

## Migration from Other Libraries

### From Boost.Asio → ASIO Standalone

```cpp
// Change namespace
- #include <boost/asio.hpp>
+ #include <asio.hpp>

- boost::asio::io_context io;
+ asio::io_context io;

// Rest of API is identical!
```

### From Qt Network → ASIO

More complex, requires architectural changes:

```cpp
// Qt (event-driven)
QUdpSocket socket;
QObject::connect(&socket, &QUdpSocket::readyRead, handleData);
app.exec();

// ASIO (async callbacks)
asio::io_context io;
asio::ip::udp::socket socket(io);
socket.async_receive_from(..., handleData);
io.run();
```

---

## Final Recommendation

✅ **Use ASIO Standalone** for all R-Type networking.

**Rationale:**

1. **Header-only**: No linking, simpler builds
2. **Zero dependencies**: No Boost/Qt framework overhead
3. **13% smaller binaries**: 347 KB vs 391 KB (Boost)
4. **50% faster configuration**: 17.7s vs 26.7s (Boost)
5. **Modern C++20**: Coroutine support for async code
6. **Industry proven**: Used in production systems worldwide
7. **Active maintenance**: Regular updates (1.30.2 latest)

**Implementation:**

- UDP socket for gameplay packets
- Async I/O with io_context
- Coroutines for clean async code (C++20)
- Header-only integration via CPM

---

## References

- PoC implementations: `/PoC/PoC_Network_Libs/`
- ASIO documentation: [https://think-async.com/Asio/](https://think-async.com/Asio/)
- Comparison report: `/PoC/PoC_Network_Libs/network_library_selection.md`
