# R-Type

A modern C++20 recreation of the classic R-Type game using an Entity Component System (ECS) architecture.

## Features

- 🎮 Classic R-Type gameplay mechanics
- 🌐 Client-server multiplayer architecture (UDP with Asio)
- ⚡ High-performance ECS engine
- 📚 Comprehensive documentation (Doxygen + Docusaurus)
- 🧪 Unit tests with Google Test
- 🔧 Cross-platform support (Linux, Windows)

## Prerequisites

- **CMake** 3.19+
- **Ninja** (build system)
- **C++20 compiler** (GCC 11+, Clang 14+, or MSVC 2022)
- **Git**

**Optional (for documentation):**
- **Doxygen** (for API docs)
- **Node.js** v20+ (for web docs)

## Quick Start

### 1. Clone the repository

```bash
git clone --recursive https://github.com/My-Epitech-Organisation/Rtype.git
cd Rtype
```

> **Note:** The `--recursive` flag is required to fetch the vcpkg submodule.

### 2. Setup vcpkg (first time only)

```bash
./scripts/setup-vcpkg.sh
```

This script initializes vcpkg and bootstraps it automatically.

**Alternative:** If you have your own vcpkg installation, set the `VCPKG_ROOT` environment variable:
```bash
export VCPKG_ROOT=/path/to/your/vcpkg
```

### 3. Configure and Build

```bash
# Configure (vcpkg installs dependencies automatically)
cmake --preset linux-debug    # Linux
cmake --preset windows-debug  # Windows

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

### Run

```bash
# Start server
./scripts/run_server.sh

# Start client (in another terminal)
./scripts/run_client.sh
```

## CMake Presets

| Preset | Description |
|--------|-------------|
| `linux-debug` | Linux Debug build |
| `linux-release` | Linux Release build |
| `windows-debug` | Windows Debug build (MSVC) |
| `windows-release` | Windows Release build (MSVC) |

## Documentation

### Generate Documentation

```bash
# Configure with documentation enabled
cmake --preset linux-debug -DBUILD_DOCS=ON

# Generate all documentation (Doxygen + Docusaurus)
cmake --build build --target docs

# Start documentation dev server
cmake --build build --target docs-serve
```

### Available Documentation Targets

- `docs` - Generate complete documentation
- `docs-doxygen` - Generate API reference only
- `docs-serve` - Start Docusaurus dev server
- `docs-build` - Build production documentation

### View Documentation

After building with `BUILD_DOCS=ON`:
- **Web docs**: http://localhost:3000 (when running `docs-serve`)
- **API reference**: `docs/doxygen/html/index.html`

## Project Structure

```
Rtype/
├── src/              # Source code
│   ├── client/       # Client application
│   ├── server/       # Server application
│   ├── engine/       # ECS game engine
│   ├── network/      # Network library (Asio UDP)
│   └── games/        # Game-specific code
├── include/          # Public headers (interfaces)
├── external/         # External dependencies
│   └── vcpkg/        # vcpkg submodule
├── tests/            # Unit tests
├── docs/             # Documentation
│   ├── website/      # Docusaurus site
│   └── Doxyfile      # Doxygen configuration
├── config/           # Configuration files
├── scripts/          # Build and run scripts
├── cmake/            # CMake modules
└── vcpkg.json        # vcpkg manifest (dependencies)
```

## Dependencies

Managed automatically by vcpkg:

| Package | Version | Description |
|---------|---------|-------------|
| asio | 1.32.0 | Async I/O (networking) |
| sfml | 3.0.2 | Graphics, Audio, Window |
| + others | - | See `vcpkg.json` |

## Development

### Build Options

```bash
# Debug build with tests (recommended)
cmake --preset linux-debug
cmake --build build

# Release build
cmake --preset linux-release
cmake --build build

# With documentation
cmake --preset linux-debug -DBUILD_DOCS=ON
cmake --build build --target docs
```

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines.

## License

See [LICENSE](LICENSE) for details.

## Team

Epitech Project - 2024/2025
