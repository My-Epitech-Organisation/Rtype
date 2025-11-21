# Public API Headers

This directory contains **only public interfaces** for the R-Type project.

## Philosophy

Following modern C++ best practices, we separate:
- **Public API** (`include/rtype/`) - Abstract interfaces for external consumption
- **Implementation** (`src/`) - Concrete implementations with headers co-located

## Directory Structure

```
include/rtype/
├── engine/          # ECS and core engine interfaces
│   ├── IRegistry.hpp
│   └── README.md
├── network/         # Network communication interfaces
│   ├── INetworkSocket.hpp
│   ├── IPacket.hpp
│   └── README.md
└── games/rtype/     # Game-specific interfaces (future)
    ├── shared/      # Shared client/server interfaces
    ├── client/      # Client-specific interfaces
    └── server/      # Server-specific interfaces
```

## Available Public Interfaces

### Engine (`rtype/engine/`)
- **`IRegistry.hpp`** - ECS registry for entity management
  - Create, destroy, and manage entities
  - Core of the Entity Component System

### Network (`rtype/network/`)
- **`INetworkSocket.hpp`** - UDP socket communication
  - Client/server network operations
  - Send/receive data over UDP
  
- **`IPacket.hpp`** - Network packet handling
  - Packet types and serialization
  - Protocol definitions

## Usage

Include these headers in your client/server code:

```cpp
#include <rtype/engine/IRegistry.hpp>
#include <rtype/network/INetworkSocket.hpp>
#include <rtype/network/IPacket.hpp>

// Use the interfaces...
```

## Implementation Note

All concrete implementations are in `src/` with headers next to `.cpp` files.
This `include/` directory should **never** contain implementation details.

When adding new public API:
1. Create abstract interface in appropriate `include/rtype/` subdirectory
2. Implement concrete class in corresponding `src/` directory
3. Update the relevant README.md
