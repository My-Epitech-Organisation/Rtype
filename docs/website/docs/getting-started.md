---
sidebar_position: 2
---

# Getting Started

This guide will help you set up your development environment and build the R-Type project.

## Prerequisites

Before you begin, ensure you have the following installed:

- **CMake** (version 3.19 or higher)
- **Ninja** (build system)
- **C++ Compiler** with C++20 support (GCC 11+, Clang 14+, or MSVC 2022)
- **Git**

**Optional (for documentation):**
- **Doxygen** (for API documentation)
- **Node.js** v20+ (for web documentation)

## Clone the Repository

```bash
git clone --recursive https://github.com/My-Epitech-Organisation/Rtype.git
cd Rtype
```

> **Note:** The `--recursive` flag is required to fetch the vcpkg submodule.

## Setup vcpkg (first time only)

```bash
./scripts/setup-vcpkg.sh
```

This script initializes vcpkg and bootstraps it automatically.

**Alternative:** If you have your own vcpkg installation, set the `VCPKG_ROOT` environment variable:
```bash
export VCPKG_ROOT=/path/to/your/vcpkg
```

## Build the Project

### Configure

```bash
# Linux Debug
cmake --preset linux-debug

# Linux Release
cmake --preset linux-release

# Windows Debug (MSVC)
cmake --preset windows-debug

# Windows Release (MSVC)
cmake --preset windows-release
```

### Build

```bash
cmake --build build
```

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## Run the Application

### Start the Server

```bash
./scripts/run_server.sh
```

### Start the Client

```bash
./scripts/run_client.sh
```

## Configuration

Configuration files are located in the `config/` directory:

- `config/server/` - Server configuration
  - `server.toml` - Network settings (port, max players, etc.)
  - `gameplay.toml` - Game rules and mechanics
  - `config.toml` - General server configuration

- `config/client/` - Client configuration
  - `video.toml` - Graphics settings (resolution, fullscreen, vsync)
  - `controls.json` - Input mappings (key bindings, gamepad)
  - `client.toml` - General client configuration

- `config/game/` - Game entity configurations
  - `enemies.toml` - Enemy types and properties
  - `players.toml` - Player ship configurations
  - `powerups.toml` - Power-up definitions
  - `projectiles.toml` - Projectile configurations
  - `levels/` - Level and wave definitions

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Generating Documentation

To generate both Doxygen and Docusaurus documentation:

```bash
# Configure CMake with documentation enabled
cmake --preset linux-debug -DBUILD_DOCS=ON

# Generate complete documentation
cmake --build build --target docs
```

To view the documentation with live reload:

```bash
cmake --build build --target docs-serve
```

This will start a dev server at `http://localhost:3000`

Or manually with npm:

```bash
cd docs/website
npm install
npm start
```

### Available Documentation Targets

- `docs` - Generate complete documentation (Doxygen + Docusaurus)
- `docs-doxygen` - Generate API reference only
- `docs-serve` - Start Docusaurus dev server
- `docs-build` - Build production documentation

## Next Steps

- Read the [Architecture Guide](./category/architecture) to understand the system design
- Browse the [API Reference](/api/index.html) for detailed class documentation
- Check out example implementations in `src/games/rtype/`
