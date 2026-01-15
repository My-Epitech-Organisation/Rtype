---
sidebar_position: 1
---

# Global Architecture

This document provides a high-level overview of the R-Type project architecture, explaining how the different layers and components interact with each other.

## Architecture Diagram

![R-Type Architecture Diagram](/img/ArchiRTYPe.png)

## Layered Architecture Overview

The R-Type project follows a **layered architecture** pattern with clear separation of concerns. The system is divided into three main layers:

1. **Application Layer** - Game-specific executables (Server & Client)
2. **Engine Layer** - Reusable game engine libraries (ECS, Network)
3. **Data Layer** - Configuration and assets

## Components

### Application Layer

#### R-Type Server (Logic & Auth)

The server is the **authoritative source of truth** for the game state. It handles:

- Game logic execution
- Player authentication and session management
- Entity spawning, updating, and destruction
- Broadcasting state updates to all connected clients
- Anti-cheat validation

**Dependencies:**
- Config Files → Reads server configuration (ports, tick rate, etc.)
- Common Lib → Uses shared protocol definitions and data structures
- ECS Lib → Manages game entities using the Entity Component System
- Network Lib → Handles UDP socket communication

#### R-Type Client (Render & Input)

The client provides the **player-facing interface**. It handles:

- Rendering the game world
- Capturing and sending player input
- Local prediction for responsive gameplay
- State interpolation for smooth visuals
- Audio playback

**Dependencies:**
- Config Files → Reads client configuration (controls, video settings)
- Asset Files → Loads textures, sounds, fonts
- Common Lib → Uses shared protocol definitions and data structures
- ECS Lib → Local entity management for rendering
- Network Lib → Communicates with server via UDP

### Engine Layer

#### ECS Lib (Core System)

The Entity Component System library provides the **core game engine functionality**:

- Entity creation and destruction
- Component storage and retrieval (using sparse sets)
- System registration and execution
- Entity queries and iteration

This library is **game-agnostic** and can be reused for other projects.

#### Network Lib (Sockets)

The network library provides **low-level networking primitives**:

- UDP socket abstraction
- Cross-platform socket implementation
- Send/receive operations
- Connection management

This library handles raw bytes and is **protocol-agnostic**.

#### Common Lib (Protocol & Structs)

The common library contains **shared definitions** used by both client and server:

- RTGP protocol packet definitions
- Serialization/deserialization utilities
- Shared data structures (components, enums)
- Constants and configuration types

This ensures **consistency** between client and server implementations.

### Data Layer

#### Config Files (JSON/TOML Rules)

Configuration files define **runtime behavior** without recompilation:

```
config/
├── client/
│   ├── controls.json    # Key bindings, gamepad mappings
│   └── video.toml       # Resolution, fullscreen, vsync
└── server/
    ├── server.toml      # Port, tick rate, max players
    └── gameplay.toml    # Difficulty, wave settings
```

#### Asset Files (Media)

Asset files contain **game resources**:

```
assets/
├── img/         # Sprites, backgrounds
├── audio/       # Music, sound effects
├── fonts/       # UI fonts
└── shaders/     # GLSL fragment shaders
```

## Library Structure

The engine libraries are located in the `lib/` directory:

```
lib/
├── ecs/         # Entity Component System core
├── common/      # Shared utilities (Logger, ArgParser, Serializer)
├── network/     # Network socket abstraction
├── display/     # Graphics rendering with SFML
├── audio/       # Sound system with SDL2
├── background/  # Parallax scrolling backgrounds
└── engine/      # Game engine abstraction layer
```

## Dependency Flow

The architecture enforces a **unidirectional dependency flow**:

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                        │
│  ┌─────────────────┐              ┌─────────────────┐       │
│  │     Server      │              │     Client      │       │
│  └────────┬────────┘              └────────┬────────┘       │
│           │                                │                │
└───────────┼────────────────────────────────┼────────────────┘
            │         depends on             │
            ▼                                ▼
┌─────────────────────────────────────────────────────────────┐
│                      ENGINE LAYER                           │
│  ┌──────────┐    ┌──────────────┐    ┌──────────────┐       │
│  │   ECS    │    │    Common    │    │   Network    │       │
│  │   Lib    │    │     Lib      │    │     Lib      │       │
│  └──────────┘    └──────────────┘    └──────────────┘       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
            ▲                                ▲
            │          references            │
            │                                │
┌───────────┼────────────────────────────────┼────────────────┐
│           │        DATA LAYER              │                │
│  ┌────────┴────────┐              ┌────────┴────────┐       │
│  │  Config Files   │              │   Asset Files   │       │
│  └─────────────────┘              └─────────────────┘       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Key Principles

| Principle | Description |
|-----------|-------------|
| **Downward Dependencies** | Higher layers depend on lower layers, never the reverse |
| **No Circular Dependencies** | Each component has clear, one-way dependencies |
| **Interface Segregation** | Libraries expose minimal, focused interfaces |
| **Data Separation** | Configuration and assets are external to code |

## Build Targets

The architecture maps to CMake build targets:

| Target | Type | Description |
|--------|------|-------------|
| `r-type_server` | Executable | Server application |
| `r-type_client` | Executable | Client application |
| `rtype_ecs` | Static Library | ECS engine core (`lib/ecs/`) |
| `rtype_network` | Static Library | Network socket layer (`lib/network/`) |
| `rtype_common` | Static Library | Shared utilities & protocol (`lib/common/`) |
| `rtype_display` | Static Library | Graphics rendering (`lib/display/`) |
| `rtype_audio` | Static Library | Sound system (`lib/audio/`) |
| `rtype_background` | Static Library | Parallax backgrounds (`lib/background/`) |
| `rtype_engine` | Static Library | Game engine abstraction (`lib/engine/`) |

## Communication Flow

```
┌────────────┐                                ┌────────────┐
│   Client   │                                │   Server   │
│            │         C_CONNECT              │            │
│            │ ─────────────────────────────► │            │
│            │         S_ACCEPT               │            │
│            │ ◄───────────────────────────── │            │
│            │                                │            │
│            │         C_INPUT                │            │
│            │ ─────────────────────────────► │            │
│            │                                │  Process   │
│            │      S_ENTITY_SPAWN            │  Inputs    │
│            │ ◄───────────────────────────── │            │
│            │      S_ENTITY_MOVE             │  Update    │
│            │ ◄───────────────────────────── │  State     │
│            │      S_ENTITY_DESTROY          │            │
│            │ ◄───────────────────────────── │            │
│  Render    │                                │            │
│  World     │                                │            │
└────────────┘                                └────────────┘
```

## Benefits of This Architecture

### Maintainability
- Clear boundaries between components
- Changes in one layer don't ripple to others
- Easy to locate code for specific functionality

### Testability
- Libraries can be tested in isolation
- Mock implementations can replace real dependencies
- Unit tests don't require full application context

### Reusability
- ECS and Network libraries are game-agnostic
- Engine can be used for other game projects
- Common definitions shared between client/server

### Scalability
- New features added without modifying core libraries
- Multiple games can share the same engine
- Easy to add new platforms or clients

## Related Documentation

- [File Architecture](./files-architecture.md) - Detailed file and directory structure
- [Network Architecture](./network-architecture.md) - Network protocol and communication
- [ECS Guide](./ecs-guide.md) - Entity Component System details
