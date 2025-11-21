# Network Public API

This directory contains the **public interface** for the R-Type networking layer.

## Available Interfaces

### `INetworkSocket.hpp`
Public interface for UDP network communication.

**Usage:**
```cpp
#include <rtype/network/INetworkSocket.hpp>

// Server
rtype::network::INetworkSocket& server = createSocket();
server.bind(4242);

// Client
rtype::network::INetworkSocket& client = createSocket();
client.connect("127.0.0.1", 4242);
```

**Key Features:**
- `bind()` - Bind socket to port (server)
- `connect()` - Connect to remote host (client)
- `send()` - Send data through socket
- `receive()` - Receive data from socket

### `IPacket.hpp`
Public interface for network packet handling.

**Usage:**
```cpp
#include <rtype/network/IPacket.hpp>

rtype::network::IPacket& packet = createPacket(
    rtype::network::PacketType::PlayerInput
);
packet.setData(inputData);
socket.send(packet.serialize());
```

**Packet Types:**
- `Unknown` - Invalid/uninitialized packet
- `PlayerInput` - Client → Server input commands
- `EntityUpdate` - Server → Client entity state
- `EntitySpawn` - Server → Client entity creation
- `EntityDestroy` - Server → Client entity removal

## Implementation Details

Implementation headers are located in `src/network/`.
This directory contains **only** abstract interfaces for public consumption.
