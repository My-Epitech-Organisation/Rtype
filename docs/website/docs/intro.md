---
sidebar_position: 1
---

# R-Type Documentation

Welcome to the R-Type game engine documentation!

## Overview

R-Type is a multiplayer space shooter game built with a custom Entity Component System (ECS) architecture. This documentation provides comprehensive guides for understanding the architecture, developing features, and contributing to the project.

## Quick Start

Choose your path:

- **[Architecture](./category/architecture)** - Learn about the system design and components
- **[API Reference](/api/index.html)** - Browse the complete C++ API documentation (Doxygen)
- **[Getting Started](./getting-started)** - Set up your development environment

## Features

- 🎮 Custom ECS (Entity Component System) engine
- 🌐 Network multiplayer support with UDP
- 🎨 Modular component-based architecture
- 🔧 Cross-platform build system with CMake
- 📊 Comprehensive testing suite

## Project Structure

```
R-Type/
├── lib/           # Core libraries
│   ├── ecs/       # Entity Component System
│   ├── engine/    # Game engine utilities
│   ├── engine_core/ # Core engine functionality
│   ├── network/   # Network layer (UDP, Protocol, Serialization)
│   └── common/    # Common utilities
├── src/           # Application code
│   ├── client/    # Client application
│   ├── server/    # Server application
│   └── games/     # Game implementations (R-Type)
├── include/       # Public API headers
│   └── rtype/     # Public interfaces
├── tests/         # Unit and integration tests
├── docs/          # Documentation (you are here!)
├── config/        # Configuration files (TOML/JSON)
└── assets/        # Game assets (textures, audio, fonts)
```

## Getting Help

- 📖 Read the [documentation](./category/architecture)
- 🐛 Report bugs on [GitHub Issues](https://github.com/My-Epitech-Organisation/Rtype/issues)
- 💬 Join discussions on Discord

## Contributing

We welcome contributions! Please read our [contributing guidelines](./contributing) before submitting pull requests.
