# File & Directory Architecture

This document describes the repository layout and the dependency rules between the different modules of the project.
Goal: any new contributor should be able to **find the right place to work** and **understand who depends on whom** in a few minutes.

---

## 1. High-level repository layout

```text
r-type/
├── CMakeLists.txt
├── cmake/
├── external/
├── include/
├── lib/                # Core libraries (ECS, network, common, display, audio, background)
├── src/
├── assets/
├── config/
├── docs/
├── scripts/
├── tests/
├── executable/        # Pre-built executables and runtime assets
└── .github/
```

### Root files and directories

* **`CMakeLists.txt`**
  Top-level CMake entry point.

  * Adds subdirectories (`src/`, `tests/`, etc.).
  * Defines global options, warnings, and toolchain settings.

* **`cmake/`**
  CMake helper modules:

  * `FindXXX.cmake` files for third-party libraries.
  * Toolchain files (if needed).
  * Conan / vcpkg integration helpers.

* **`external/`** *(optional)*
  Scripts or pinned external configs (e.g. Conan profiles, patched CMake modules).
  We do **not** vendor full third-party libraries’ source code here – only thin integration helpers.

* **`lib/`**
  **Core reusable libraries** organized by functionality:

  ```text
  lib/
    audio/              # Audio system (ALevelMusic, battleMusic, chillMusic, etc.)
    background/         # Background effects (AsteroidsSpace, BlackHole, Labs, etc.)
    common/             # Shared utilities (ArgParser, Config, DLLoader, Logger, SafeQueue)
    display/            # Display abstraction (SFML/SDL2 backends)
    ecs/                # Entity Component System implementation
    engine/             # Game engine core
    network/            # Network stack (UDP, packets, serialization, protocol)
  ```

  Each library has its own `CMakeLists.txt` and `src/` directory.

* **`include/`**
  **Public interface headers only** (abstract base classes, pure interfaces).

  Following modern C++ best practices, this directory contains **only** public APIs:
  - Abstract interfaces (`IRegistry`, `INetworkSocket`, `IPacket`)
  - Public enums and type definitions
  - **No implementation details** (implementation headers are co-located with `.cpp` files in `src/`)

  ```text
  include/
    README.md           # API documentation overview
    rtype/
      engine/
        IRegistry.hpp   # ECS registry interface
        README.md
      network/
        INetworkSocket.hpp  # UDP socket interface
        IPacket.hpp         # Network packet interface
        README.md
      games/
        rtype/
          shared/       # Reserved for shared game interfaces
          client/       # Reserved for client interfaces
          server/       # Reserved for server interfaces
  ```

  This provides a **clean public API** while keeping implementation headers private in `src/`.

* **`src/`**
  All implementation code (C++ sources **and implementation headers**) for our libraries and executables.
  **This is the core of the project**, detailed in section 2.

  **Important**: Implementation headers (`.hpp` with corresponding `.cpp`) are **co-located**
  with their source files, not in `include/`. This improves cohesion and makes it clear
  which headers are part of the public API versus internal implementation.

* **`assets/`**
  Game resources used at runtime:

  * `textures/` (sprites, backgrounds…)
  * `audio/` (music, SFX)
  * `fonts/` (UI fonts, debug overlay fonts)

* **`config/`**
  Configuration files for client and server:

  ```text
  config/
    client/
      controls.json   # key bindings, gamepad mappings, sensitivity
      video.toml      # resolution, fullscreen, vsync, etc.
    server/
      server.toml     # port, tick rate, max players, etc.
      gameplay.toml   # difficulty, wave settings, score multipliers
  ```

* **`docs/`**
  Project documentation (all in English):

  ```text
  docs/
    README.md                   # documentation overview
    DOCUMENTATION_GUIDE.md      # how to write and generate docs
    DEPENDENCY_MANAGEMENT.md    # vcpkg + CPM dependency strategy
    stress_test_summary.md      # server stress test results
    Doxyfile                    # Doxygen configuration
    architecture/
      files_architecture.md     # this document
      ecs/                      # ECS technical documentation
    RFC/
      RFC_RTGP_v1.4.3.md        # network protocol specification
    technical/
      ArgParser.md              # command line parser documentation
      Logger.md                 # logging system documentation
      EntityConfig_Usage.md     # entity configuration usage
    website/                    # Docusaurus documentation website
  ```

* **`scripts/`**
  Helper scripts for developers:

  ```text
  scripts/
    build_debug.sh
    build_release.sh
    run_client.sh
    run_server.sh
    format_all.sh           # optional, clang-format wrapper
  ```

* **`tests/`**
  Unit and integration tests, organized by module.

  **Note**: Tests link against library targets and use include directories:

  ```text
  tests/
    CMakeLists.txt
    ecs/
      test_registry.cpp        # Tests for lib/ecs/
      CMakeLists.txt
    common/
      test_logger.cpp          # Tests for lib/common/
      CMakeLists.txt
    network/
      test_serialization.cpp   # Tests for lib/network/
      CMakeLists.txt
    games/
      rtype/
        test_spawning.cpp      # Game-specific tests
        CMakeLists.txt
    integration/
      test_client_server.cpp   # Integration tests
  ```

* **`.github/workflows/`**
  CI configuration (GitHub Actions):

  * Build the project.
  * Run tests.
  * Optional: static analysis / formatting checks.

---

## 1.1. Header Placement Strategy

This project follows modern C++ best practices for header organization:

### Public API Headers (`include/rtype/`)

* **Only abstract interfaces** and public type definitions
* Files like `IRegistry.hpp`, `INetworkSocket.hpp`, `IPacket.hpp`
* **No implementation details** - these are pure interfaces
* Stable API that external code can depend on

### Implementation Headers (`lib/` and `src/`)

* **Co-located with `.cpp` files** for better cohesion
* Files like `Entity.hpp`, `Registry.hpp`, `UdpSocket.hpp`, `Packet.hpp`
* Contain concrete class definitions and implementation details
* Not exposed to external code - internal use only

### Benefits

* **Clear separation** between public API and implementation
* **Better encapsulation** - clients only see what they need
* **Easier maintenance** - implementation changes don't affect public API
* **Module cohesion** - headers live next to their implementations

### Example

```text
include/rtype/engine/IRegistry.hpp    # Public interface (abstract)
lib/ecs/src/core/Registry/Registry.hpp  # Implementation header (co-located)
lib/ecs/src/core/Registry/RegistryEntity.cpp  # Implementation
```

---

## 2. Source code layout (`src/` and `lib/`)

The project separates reusable libraries (`lib/`) from game-specific code (`src/`):

```text
lib/
├── audio/              # Audio/music system
├── background/         # Background rendering effects
├── common/             # Shared utilities
│   └── src/
│       ├── ArgParser/  # Command line parsing
│       ├── Config/     # Configuration loading (TOML)
│       ├── DLLoader/   # Dynamic library loading
│       ├── Logger/     # Logging system
│       └── SafeQueue/  # Thread-safe queue
├── display/            # Display abstraction layer
│   ├── Clock/          # Timing utilities
│   ├── SDL2/           # SDL2 backend
│   └── SFML/           # SFML backend
├── ecs/                # Entity Component System
│   └── src/
│       ├── core/       # Entity, Registry, CommandBuffer, Prefab, Relationship
│       ├── serialization/
│       ├── signal/     # SignalDispatcher
│       ├── storage/    # SparseSet, ISparseSet, TagSparseSet
│       ├── system/     # SystemScheduler
│       ├── traits/     # ComponentTraits
│       └── view/       # View, ParallelView, Group, ExcludeView
├── engine/             # Game engine core
└── network/            # Network stack
    └── src/
        ├── compression/    # LZ4 compression
        ├── connection/     # Connection management
        ├── core/           # Core network types
        ├── protocol/       # OpCodes, Payloads, Validator
        ├── reliability/    # Reliable UDP layer
        └── transport/      # Transport abstraction

src/
├── games/
│   ├── rtype/          # R-Type game module
│   │   ├── shared/     # Shared components & systems
│   │   ├── server/     # Server-side game logic
│   │   └── client/     # Client-side rendering & input
│   └── snake/          # Snake game module (additional game)
│       ├── shared/
│       ├── server/
│       └── client/
├── server/             # Server executable
└── client/             # Client executable
```

### 2.1. `lib/ecs/` – Entity Component System (lowest level)

The ECS library provides high-performance entity management with sparse set storage.

```text
lib/ecs/src/
  ECS.hpp               # Main include file
  core/
    Entity.hpp          # Entity identifier with generational indices
    Registry/           # Registry implementation (split across files)
      Registry.hpp      # Main interface and declarations
      RegistryEntity.cpp    # Entity lifecycle implementation
      RegistryComponent.inl # Component management (template)
      RegistrySingleton.inl # Singleton resources (template)
      RegistryView.inl      # View creation (template)
    CommandBuffer.hpp   # Deferred ECS operations
    Prefab.hpp          # Entity templates
    Relationship.hpp    # Parent-child hierarchies
  storage/
    ISparseSet.hpp      # Sparse set interface
    SparseSet.hpp       # Generic component storage
    TagSparseSet.hpp    # Zero-size tag components
  view/
    View.hpp            # Standard component queries
    ParallelView.hpp    # Multi-threaded iteration
    Group.hpp           # Cached entity collections
    ExcludeView.hpp     # Exclusion filtering
  system/
    SystemScheduler.hpp # Dependency-based system execution
  signal/
    SignalDispatcher.hpp # Component lifecycle events
  serialization/        # Save/load ECS state
  traits/
    ComponentTraits.hpp # Component type analysis
```

* Compiled into library: **`rtype_ecs`**.
* Responsibilities:
  * `Entity` identifiers with generational indices
  * `Registry` for entity/component management
  * `View`/`ParallelView` for efficient iteration
  * Signal/observer pattern support
  * Prefab templates and relationships
* **Does not know anything about R-Type**, networking, or rendering.

> Dependency rule: `rtype_ecs` is the **base layer** – nothing inside `lib/ecs/` depends on other project modules.

### 2.1.1. `lib/common/` – Shared Utilities

```text
lib/common/src/
  ArgParser/            # Command line argument parser
    ArgParser.hpp
    Option.hpp
    ParseResult.hpp
    NumberParser.hpp
  Config/               # TOML configuration loading
  DLLoader/             # Dynamic library loading (plugins)
  Logger/               # Thread-safe logging system
    Logger.hpp
    LogLevel.hpp
    Macros.hpp
    FileWriter.hpp
    Timestamp.hpp
    ColorFormatter.hpp
  SafeQueue/            # Thread-safe queue
  Types.hpp             # Common type definitions
```

* Compiled into library: **`rtype_common`** (header-only interface).
* Responsibilities:
  * Argument parsing, configuration loading
  * Logging infrastructure
  * Thread-safe utilities

---

### 2.2. `lib/network/` – Network Library

The network library implements the RTGP protocol over UDP with reliability.

```text
lib/network/src/
  UdpSocket.hpp         # UDP socket wrapper
  UdpSocket.cpp
  Packet.hpp            # Packet structure
  Packet.cpp
  Serializer.hpp        # Binary serialization
  Serializer.cpp
  Protocol.hpp          # Protocol definitions
  compression/          # LZ4 compression support
  connection/           # Connection state management
  core/                 # Core types and constants
  protocol/
    OpCode.hpp          # Operation codes
    Payloads.hpp        # Payload structures
    Validator.hpp       # Packet validation
  reliability/          # Reliable UDP layer (RUDP)
  transport/            # Transport abstraction
```

* Compiled into library: **`rtype_network`**.
* Responsibilities:
  * UDP socket wrappers with asio
  * Binary packet serialization/deserialization
  * RTGP protocol implementation (see [RFC_RTGP_v1.4.3](../RFC/RFC_RTGP_v1.4.3.md))
  * LZ4 compression for large payloads
  * Reliable delivery layer with ACK/retransmission
* **Depends on**:
  * `rtype_common` (for logging, types)
  * External: asio, lz4
* Used by:
  * `r-type_server` (server executable)
  * `r-type_client` (client executable)
  * Game-specific logic in `games/rtype/{shared,server,client}`

> Dependency rule: network is **above common/ecs**, but **below game logic**.

---

### 2.3. `src/games/rtype/` – game module (R-Type)

The game module R-Type is split into three parts:

```text
src/games/rtype/
  shared/     # logic & components shared by client and server
  server/     # authoritative gameplay logic for the server
  client/     # rendering & input logic for the client
```

#### 2.3.1. `src/games/rtype/shared/`

**Header placement**: Shared components header co-located in `shared/`.

```text
src/games/rtype/shared/
  Components.hpp            # Shared components definitions (co-located)
  CMakeLists.txt
  Components/
    TransformComponent.cpp
    VelocityComponent.cpp
    NetworkIdComponent.cpp
  Systems/
    MovementSystem.cpp
    LifetimeSystem.cpp
    CMakeLists.txt
```

* Compiled into library: **`rtype_game_shared`**.
* Responsibilities:

  * Components and systems that are **meaningful on both client and server**:

    * Transforms, velocities, networking IDs, life timers, etc.
* **Depends on**:

  * `rtype_engine` (ECS).
* May be used by:

  * `rtype_game_server` (server game module).
  * `rtype_game_client` (client game module).

#### 2.3.2. `src/games/rtype/server/`

```text
src/games/rtype/server/
  Systems/
    EnemySpawnSystem.cpp
    CollisionSystem.cpp
    DamageSystem.cpp
    ScoreSystem.cpp
    GameRulesSystem.cpp
  RTypeServerModule.cpp
```

* Compiled into library: **`rtype_game_server`**.
* Responsibilities:

  * **Authoritative game logic**:

    * Enemy spawning patterns.
    * Collision detection and resolution.
    * Health, damage, scoring.
    * Win/lose conditions, wave progression.
  * Integration hooks:

    * `register_rtype_game_server_systems(Engine&)`
    * `setup_rtype_game_server_world(Engine&)`
* **Depends on**:

  * `rtype_engine` (ECS).
  * `rtype_network` (for event/message types, if needed).
  * `rtype_game_shared` (shared components and systems).

#### 2.3.3. `src/games/rtype/client/`

```text
src/games/rtype/client/
  Systems/
    RenderSystem.cpp
    InputSystem.cpp
    SoundSystem.cpp
    UiSystem.cpp
  RTypeClientModule.cpp
```

* Compiled into library: **`rtype_game_client`**.
* Responsibilities:

  * **Client-side logic**:

    * Rendering (reading `Transform` + `Sprite`/`Animation` components).
    * Input collection (keyboard/gamepad) and translation to network inputs.
    * Audio playback.
    * UI and HUD rendering.
  * Integration hooks:

    * `register_rtype_game_client_systems(Engine&)`
    * `setup_rtype_game_client_world(Engine&)`
* **Depends on**:

  * `rtype_engine`.
  * `rtype_network` (to send inputs, receive updates).
  * `rtype_game_shared`.

> Dependency rule: the **game module** sits on top of engine + network, and is itself used by the executables.

---

### 2.4. `src/server/` – server executable

```text
src/server/
  main.cpp
  ServerApp.cpp
```

* Compiled into executable: **`r-type_server`**.

* Responsibilities:

  * Parse command line arguments.
  * Load server configuration from `config/server/`.
  * Initialize logging, engine instance, network stack.
  * Call R-Type module hooks:

    * `register_rtype_game_server_systems(engine);`
    * `setup_rtype_game_server_world(engine);`
  * Run the main loop:

    * Poll network packets.
    * Inject inputs/events into ECS.
    * Tick ECS systems.
    * Send updates to clients.

* **Depends on**:

  * `rtype_engine`
  * `rtype_network`
  * `rtype_game_shared`
  * `rtype_game_server`

> Think of `src/server/` as the **host** for the “R-Type server game”.

---

### 2.5. `src/client/` – client executable

```text
src/client/
  main.cpp
  ClientApp.cpp
```

* Compiled into executable: **`r-type_client`**.

* Responsibilities:

  * Parse command line arguments (server IP, player name, options).
  * Load client configuration from `config/client/`.
  * Initialize window, renderer, audio system.
  * Initialize engine instance + network client.
  * Call R-Type client hooks:

    * `register_rtype_game_client_systems(engine);`
    * `setup_rtype_game_client_world(engine);`
  * Run the main loop:

    * Collect local input, build network messages.
    * Poll network updates and apply them to ECS.
    * Tick ECS systems (render, sound, UI, prediction).

* **Depends on**:

  * `rtype_engine`
  * `rtype_network`
  * `rtype_game_shared`
  * `rtype_game_client`

> `src/client/` is the **host** for the “R-Type client game”.

---

## 3. CMake targets & dependency graph

### 3.1. Library & executable targets

Typical targets:

* **Libraries**

  * `rtype_ecs`           ← `lib/ecs/`
  * `rtype_common`        ← `lib/common/`
  * `rtype_network`       ← `lib/network/`
  * `rtype_display`       ← `lib/display/`
  * `rtype_audio`         ← `lib/audio/`
  * `rtype_background`    ← `lib/background/`
  * `rtype_engine`        ← `lib/engine/`
  * `rtype_game_shared`   ← `src/games/rtype/shared/`
  * `rtype_game_server`   ← `src/games/rtype/server/`
  * `rtype_game_client`   ← `src/games/rtype/client/`

* **Executables**

  * `r-type_server`       ← `src/server/`
  * `r-type_client`       ← `src/client/`

**CMake Include Strategy**:

* Each library uses `target_include_directories` with `${CMAKE_CURRENT_SOURCE_DIR}`
  to expose its local headers
* Libraries propagate their include directories via `PUBLIC` linkage
* Executables and tests access headers through library dependencies
* `include/` is added globally for public API interfaces

### 3.2. Dependency rules (who can depend on whom?)

From **lowest** to **highest** layer:

```text
rtype_ecs + rtype_common              # core ECS & utilities (base)
           ▲
           │
rtype_network + rtype_display         # network & display libraries
           ▲
           │
  rtype_game_shared                   # shared game components
       ▲         ▲
       │         │
rtype_game_server  rtype_game_client  # game-specific logic
       ▲         ▲
       │         │
 r-type_server   r-type_client        # executables
```

#### Summary:

* `rtype_ecs`
  ↳ No internal dependency on other project targets.

* `rtype_common`
  ↳ No internal dependency on other project targets.

* `rtype_network`
  ↳ Depends on `rtype_common` (for logging).

* `rtype_display`
  ↳ Depends on `rtype_common`.

* `rtype_game_shared`
  ↳ Depends on `rtype_ecs`, `rtype_common`.

* `rtype_game_server`
  ↳ Depends on `rtype_ecs`, `rtype_network`, `rtype_game_shared`.

* `rtype_game_client`
  ↳ Depends on `rtype_ecs`, `rtype_network`, `rtype_display`, `rtype_game_shared`.

* `r-type_server`
  ↳ Depends on `rtype_ecs`, `rtype_network`, `rtype_game_shared`, `rtype_game_server`.

* `r-type_client`
  ↳ Depends on `rtype_ecs`, `rtype_network`, `rtype_display`, `rtype_game_shared`, `rtype_game_client`.

> Rule of thumb: **no cyclic dependencies**. Higher-level modules may depend on lower-level ones, but never the opposite.

---

## 4. How to know where to put new code

When adding new functionality, use these questions:

1. **Is this generic ECS functionality (entities, components, views)?**
   → Put it in `lib/ecs/src/`.

2. **Is this a reusable utility (logging, config, arg parsing)?**
   → Put it in `lib/common/src/`.

3. **Is this low-level networking code or protocol logic?**
   → Put it in `lib/network/src/`.

4. **Is this display/rendering abstraction (not game-specific)?**
   → Put it in `lib/display/`.

5. **Is this a component or system that exists on both client & server (Transform, Velocity…)?**
   → Put it in `src/games/rtype/shared/`.

6. **Is this authoritative gameplay logic (scoring, collisions, wave spawning)?**
   → Put it in `src/games/rtype/server/`.

7. **Is this rendering, input, UI, or sound logic used only on the client?**
   → Put it in `src/games/rtype/client/`.

8. **Is this about bootstrapping the process (CLI args, main loop, config loading)?**
   → Put it in `src/server/` or `src/client/`.

---

## 5. Testing and documentation links

* **Tests** for a module should live in `tests/<module>/` and link against the corresponding target:

  * `tests/ecs/` → tests for `rtype_ecs`.
  * `tests/common/` → tests for `rtype_common` (Logger, ArgParser).
  * `tests/network/` → tests for `rtype_network`.
  * `tests/games/rtype/` → tests for game-specific code.
  * `tests/integration/` → integration tests for client-server flows.

* **Architecture documentation** provides more details on design:

  * `docs/architecture/ecs/` – ECS technical documentation
  * `docs/technical/` – Technical component docs (Logger, ArgParser)

* The **network protocol** is documented in:

  * `docs/RFC/RFC_RTGP_v1.4.3.md`

This document (`docs/architecture/files_architecture.md`) should be the first step to understand **where things live**; the other docs then explain **how they work**.

  * `docs/protocol/rtype_protocol.md`

This document (`docs/files_architecture.md`) should be the first step to understand **where things live**; the other docs then explain **how they work**.
