---
sidebar_position: 1
---

# Architecture Overview

The R-Type game engine is built on a modular, component-based architecture using the Entity Component System (ECS) pattern.

## High-Level Architecture

```
┌─────────────────────────────────────────┐
│           Application Layer              │
│  ┌─────────────┐      ┌──────────────┐  │
│  │   Client    │      │    Server    │  │
│  └─────────────┘      └──────────────┘  │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│            Game Layer                    │
│  ┌─────────────────────────────────┐    │
│  │   Game-Specific Systems         │    │
│  │  • RenderSystem                 │    │
│  │  • MovementSystem               │    │
│  │  • EnemySpawnSystem             │    │
│  └─────────────────────────────────┘    │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│          Engine Core Layer               │
│  ┌─────────────┐      ┌──────────────┐  │
│  │     ECS     │      │   Network    │  │
│  │  • Registry │      │ • UdpSocket  │  │
│  │  • Entity   │      │ • Packet     │  │
│  └─────────────┘      └──────────────┘  │
└─────────────────────────────────────────┘
```

## Core Components

### Entity Component System (ECS)

The ECS architecture separates data (Components) from behavior (Systems), allowing for flexible and efficient game object management.

#### Entity
- Unique identifier for game objects
- Lightweight integer ID
- No logic or data attached directly

#### Component
- Pure data structures
- Examples: `TransformComponent`, `VelocityComponent`, `SpriteComponent`
- Stored in the Registry

#### System
- Contains game logic
- Operates on entities with specific component combinations
- Examples: `MovementSystem`, `RenderSystem`, `CollisionSystem`

#### Registry
- Central database for all entities and components
- Provides queries to find entities with specific components
- Manages entity lifecycle

### Network Layer

The network layer handles communication between clients and the server:

- **UdpSocket**: Low-level UDP socket wrapper
- **Packet**: Message container with serialization support
- **Serializer**: Converts game data to/from network format

### Game-Specific Layer

Game logic is implemented in the `src/games/rtype/` directory:

- **Client Systems**: Rendering, input handling, UI
- **Server Systems**: Game logic, physics, AI, spawning
- **Shared Components**: Data structures used by both client and server

## Design Principles

### Separation of Concerns
- Engine code is game-agnostic
- Game-specific logic is isolated in the `games/` directory
- Network layer is independent of game logic

### Modularity
- Each system is independently testable
- Components can be mixed and matched
- Easy to add new features without modifying core engine

### Performance
- Cache-friendly data layout
- Minimal virtual function calls
- Efficient component storage

## Data Flow

1. **Input** → Client receives user input
2. **Client Processing** → Local prediction and rendering
3. **Network** → Client sends input to server
4. **Server Processing** → Authoritative game state update
5. **Network** → Server broadcasts state to clients
6. **Client Update** → Clients reconcile with server state

## Directory Structure

```
src/
├── engine/          # Core engine components
│   ├── core/        # Time, utilities
│   └── ecs/         # ECS implementation
├── network/         # Network layer
├── games/rtype/     # R-Type game implementation
│   ├── client/      # Client-side systems
│   ├── server/      # Server-side systems
│   └── shared/      # Shared components
├── client/          # Client application entry
└── server/          # Server application entry
```

## Next Steps

- Explore the [ECS Guide](./ecs-guide) for detailed ECS usage
- Read about [Network Architecture](./network-architecture)
- Check the [API Reference](/api/) for implementation details
