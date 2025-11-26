---
sidebar_position: 2
---

# Getting Started

This guide will help you set up your development environment and build the R-Type project.

## Prerequisites

Before you begin, ensure you have the following installed:

- **CMake** (version 3.20 or higher)
- **C++ Compiler** (GCC 11+, Clang 14+, or MSVC 2022)
- **Git**
- **Doxygen** (for API documentation)
- **Node.js** (for web documentation)

## Clone the Repository

```bash
git clone https://github.com/My-Epitech-Organisation/Rtype.git
cd R-Type
```

## Build the Project

### Debug Build

```bash
./scripts/build_debug.sh
```

### Release Build

```bash
./scripts/build_release.sh
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
  - `server.toml` - Network and server settings
  - `gameplay.toml` - Game rules and mechanics
  
- `config/client/` - Client configuration
  - `video.toml` - Graphics settings
  - `controls.json` - Input mappings

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Generating Documentation

To generate both Doxygen and Docusaurus documentation:

```bash
# From project root
make docs
```

To view the documentation with live reload:

```bash
make docs-serve
```

Or manually:

```bash
./scripts/generate_docs.sh
cd docs/website
npm start
```

## Next Steps

- Read the [Architecture Guide](./category/architecture) to understand the system design
- Browse the [API Reference](/api/index.html) for detailed class documentation
- Check out example implementations in `src/games/rtype/`
