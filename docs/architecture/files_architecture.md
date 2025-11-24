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
├── src/
├── assets/
├── config/
├── docs/
├── scripts/
├── tests/
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
    README.md               # human-friendly readme, project overview
    files_architecture.md   # this document
    architecture/
      overview.md           # high-level diagrams, module descriptions
      engine.md             # engine / ECS design details
      network.md            # network architecture & netcode
    protocol/
      rtype_protocol.md     # “mini-RFC” of the network protocol
    gameplay/
      design_doc.md         # gameplay & level design
    accessibility/
      accessibility.md      # features & decisions for accessibility
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

  **Note**: Tests use relative paths to include implementation headers from `src/`:

  ```text
  tests/
    CMakeLists.txt
    ecs/
      test_registry.cpp        # Includes "../../src/engine/ecs/Registry.hpp"
      CMakeLists.txt
    network/
      test_serialization.cpp   # Includes "../../src/network/Serializer.hpp"
      CMakeLists.txt
    games/
      rtype/
        test_spawning.cpp      # Includes relative paths to components
        CMakeLists.txt
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

### Implementation Headers (`src/`)

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
include/rtype/engine/IRegistry.hpp    # Public interface
src/engine/ecs/Registry.hpp           # Implementation header (co-located)
src/engine/ecs/Registry.cpp           # Implementation
```

---

## 2. Source code layout (`src/`)

```text
src/
├── engine/
├── network/
├── games/
│   └── rtype/
│       ├── shared/
│       ├── server/
│       └── client/
├── server/
└── client/
```

### 2.1. `src/engine/` – core engine & ECS (lowest level)

This directory implements the **engine core** and ECS.

**Header placement strategy**: Implementation headers (`.hpp` files with corresponding `.cpp`)
are **co-located** with their source files for better cohesion. Only abstract interfaces
are exposed in `include/rtype/engine/`.

```text
src/engine/
  ecs/
    Entity.hpp          # Implementation header (co-located)
    Entity.cpp
    Registry.hpp        # Implementation header (co-located)
    Registry.cpp
    CMakeLists.txt
  core/
    Time.hpp            # Implementation header (co-located)
    Time.cpp
    Logger.cpp
    Config.cpp
    CMakeLists.txt
```

* Compiled into library: **`rtype_engine`**.
* Responsibilities:

  * `Entity`/`Component`/`System` primitives.
  * Engine loop helpers, time management.
  * Logging, config loading, basic utilities.
* **Does not know anything about R-Type**, networking, or rendering libraries.

> Dependency rule: `rtype_engine` is the **base layer** – nothing inside `src/engine/` depends on other project modules.

---

### 2.2. `src/network/` – shared network library

**Header placement**: Implementation headers co-located with `.cpp` files.

```text
src/network/
  UdpSocket.hpp         # Implementation header (co-located)
  UdpSocket.cpp
  Packet.hpp            # Implementation header (co-located)
  Packet.cpp
  Serializer.hpp        # Implementation header (co-located)
  Serializer.cpp
  CMakeLists.txt
```

* Compiled into library: **`rtype_network`**.
* Responsibilities:

  * UDP socket wrappers (and optional TCP if justified).
  * Packet structure, (de)serialization of messages.
  * Definition of message types (e.g. `PlayerInput`, `EntitySpawn`, `EntityUpdate`, etc.).
* **Depends on**:

  * `rtype_engine` (for logging, maybe time, basic types).
* Used by:

  * `r-type_server` (server executable).
  * `r-type_client` (client executable).
  * Game-specific logic in `games/rtype/{shared,server,client}`.

> Dependency rule: network is **above engine**, but **below game logic**.

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

  * `rtype_engine`        ← `src/engine/`
  * `rtype_network`       ← `src/network/`
  * `rtype_game_shared`  ← `src/games/rtype/shared/`
  * `rtype_game_server`  ← `src/games/rtype/server/`
  * `rtype_game_client`  ← `src/games/rtype/client/`

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
rtype_engine         rtype_engine       # core ECS & engine utilities
   ▲                     ▲
   │                     │
rtype_network        rtype_network      # network helpers, packets, messages
   ▲                     ▲
   │                     │
rtype_game_shared    rtype_game_shared  # shared game components & systems
   ▲                     ▲
   │                     │
rtype_game_server    rtype_game_client
   ▲                     ▲
   │                     │
r-type_server        r-type_client
```

#### Summary:

* `rtype_engine`
  ↳ No internal dependency on other project targets.

* `rtype_network`
  ↳ Depends **only** on `rtype_engine`.

* `rtype_game_shared`
  ↳ Depends **only** on `rtype_engine`.

* `rtype_game_server`
  ↳ Depends on `rtype_engine`, `rtype_network`, `rtype_game_shared`.

* `rtype_game_client`
  ↳ Depends on `rtype_engine`, `rtype_network`, `rtype_game_shared`.

* `r-type_server`
  ↳ Depends on `rtype_engine`, `rtype_network`, `rtype_game_shared`, `rtype_game_server`.

* `r-type_client`
  ↳ Depends on `rtype_engine`, `rtype_network`, `rtype_game_shared`, `rtype_game_client`.

> Rule of thumb: **no cyclic dependencies**. Higher-level modules may depend on lower-level ones, but never the opposite.

---

## 4. How to know where to put new code

When adding new functionality, use these questions:

1. **Is this generic engine/ECS functionality (no game-specific logic)?**
   → Put it in `src/engine/` + `include/rtype/engine/`.

2. **Is this low-level networking code or serialization logic reused by client & server?**
   → Put it in `src/network/` + `include/rtype/network/`.

3. **Is this a component or system that exists on both client & server (Transform, Velocity…)?**
   → Put it in `src/games/rtype/shared/`.

4. **Is this authoritative gameplay logic (scoring, collisions, wave spawning)?**
   → Put it in `src/games/rtype/server/`.

5. **Is this rendering, input, UI, or sound logic used only on the client?**
   → Put it in `src/games/rtype/client/`.

6. **Is this about bootstrapping the process (CLI args, main loop, config loading)?**
   → Put it in `src/server/` or `src/client/`.

---

## 5. Testing and documentation links

* **Tests** for a module should live in `tests/<module>/` and link against the corresponding target:

  * `tests/ecs/` → tests for `rtype_engine`.
  * `tests/network/` → tests for `rtype_network`.
  * `tests/games/rtype/` → tests for `rtype_*`.

* **Architecture documentation** provides more details on design:

  * `docs/architecture/overview.md`
  * `docs/architecture/engine.md`
  * `docs/architecture/network.md`

* The **network protocol** is documented in:

  * `docs/protocol/rtype_protocol.md`

This document (`docs/files_architecture.md`) should be the first step to understand **where things live**; the other docs then explain **how they work**.
