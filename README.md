# R-Type

A modern C++20 recreation of the classic R-Type game using an Entity Component System (ECS) architecture.

## Features

- 🎮 Classic R-Type gameplay mechanics
- 🌐 Client-server multiplayer architecture
- ⚡ High-performance ECS engine
- 📚 Comprehensive documentation (Doxygen + Docusaurus)
- 🧪 Unit tests with Google Test
- 🔧 Cross-platform support (Linux, macOS, Windows)

## Prerequisites

- **CMake** 3.15+
- **C++20 compiler** (GCC 11+, Clang 14+, or MSVC 2022)
- **Git**

**Optional (for documentation):**
- **Doxygen** (for API docs)
- **Node.js** v20+ (for web docs)

## Quick Start

### Build

```bash
# Clone the repository
git clone https://github.com/My-Epitech-Organisation/Rtype.git
cd Rtype

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run tests
cd build
ctest --output-on-failure
```

### Run

```bash
# Start server
./scripts/run_server.sh

# Start client (in another terminal)
./scripts/run_client.sh
```

## Documentation

### Generate Documentation

```bash
# Configure with documentation enabled
cmake -S . -B build -DBUILD_DOCS=ON

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
│   └── games/        # Game-specific code
├── include/          # Public headers
├── tests/            # Unit tests
├── docs/             # Documentation
│   ├── website/      # Docusaurus site
│   └── Doxyfile      # Doxygen configuration
├── config/           # Configuration files
└── scripts/          # Build and run scripts
```

## Development

### Build Options

```bash
# Debug build with tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build

# Release build without tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build

# With documentation
cmake -S . -B build -DBUILD_DOCS=ON
cmake --build build --target docs
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines.

## License

See [LICENSE](LICENSE) for details.

## Team

Epitech Project - 2024/2025
