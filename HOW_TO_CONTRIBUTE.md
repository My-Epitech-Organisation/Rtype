# Contributing to R-Type

Thank you for considering contributing to R-Type! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Commit Conventions](#commit-conventions)
- [Pull Request Process](#pull-request-process)
- [Testing](#testing)
- [Documentation](#documentation)

---

## Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment. Be considerate, respectful, and collaborative.

---

## Getting Started

### Prerequisites

- **C++20** compatible compiler (GCC 11+, Clang 13+, MSVC 19.29+)
- **CMake** 3.15 or higher
- **vcpkg** package manager
- **Git**

### Setup

1. Clone the repository:

   ```bash
   git clone --recursive https://github.com/My-Epitech-Organisation/Rtype.git
   cd Rtype
   ```

2. Build the project:

   ```bash
   # Linux/macOS
   ./build.sh --setup-vcpkg

   # Windows
   .\build.ps1 -SetupVcpkg
   ```

3. Run tests to verify your setup:

   ```bash
   ./build.sh -t   # or .\build.ps1 -Test on Windows
   ```

---

## Development Workflow

### Branch Naming Convention

Use descriptive branch names with prefixes:

| Prefix | Purpose | Example |
|--------|---------|---------|
| `feature/` | New features | `feature/add-multiplayer-lobby` |
| `fix/` | Bug fixes | `fix/collision-detection-edge-case` |
| `docs/` | Documentation | `docs/update-architecture` |
| `refactor/` | Code refactoring | `refactor/ecs-optimization` |
| `test/` | Test additions | `test/network-unit-tests` |
| `ci/` | CI/CD changes | `ci/add-windows-build` |

### Workflow Steps

1. Create a new branch from `main`:

   ```bash
   git checkout main
   git pull origin main
   git checkout -b feature/your-feature-name
   ```

2. Make your changes following the [Coding Standards](#coding-standards)

3. Write/update tests for your changes

4. Commit your changes using [Commit Conventions](#commit-conventions)

5. Push and create a Pull Request

---

## Coding Standards

### General Rules

1. **No raw pointers** - Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) or references
2. **No non-const references as parameters** - Pass by value or const reference, return by value
3. **Use `const` wherever possible** - Mark member functions and parameters as `const` when applicable
4. **Prefer value semantics** - Return by value, let the compiler optimize with RVO/NRVO

### C++ Style Guide

```cpp
// ✅ Good: Return by value, const reference parameter
TransformComponent updateMovement(TransformComponent transform,
                                   const VelocityComponent& velocity,
                                   float deltaTime);

// ❌ Bad: Non-const reference parameter
void updateMovement(TransformComponent& transform,
                    const VelocityComponent& velocity);

// ❌ Bad: Raw pointer
Entity* findEntity(EntityId id);

// ✅ Good: Return optional or smart pointer
std::optional<Entity> findEntity(EntityId id);
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes/Structs | PascalCase | `TransformComponent` |
| Functions/Methods | camelCase | `updatePosition()` |
| Variables | camelCase | `playerVelocity` |
| Constants | SCREAMING_SNAKE_CASE | `MAX_PLAYERS` |
| Namespaces | lowercase | `rtype::ecs` |
| Files | PascalCase | `TransformComponent.hpp` |
| Template parameters | PascalCase | `template<typename ComponentType>` |

### File Organization

```text
include/rtype/
├── engine/
│   ├── ecs/
│   │   ├── Entity.hpp
│   │   ├── Registry.hpp
│   │   └── Components/
│   └── core/
└── games/
    └── rtype/
        ├── Components/
        └── Systems/
```

### Header Guards

Use `#pragma once` for header guards:

```cpp
#pragma once

namespace rtype::ecs {
// ...
}
```

---

## Commit Conventions

We follow **Conventional Commits** specification.

### Format

```text
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |
| `style` | Code style changes (formatting, no logic change) |
| `refactor` | Code refactoring |
| `test` | Adding or updating tests |
| `chore` | Maintenance tasks |
| `ci` | CI/CD changes |
| `perf` | Performance improvements |

### Scopes

| Scope | Description |
|-------|-------------|
| `ecs` | Entity Component System |
| `network` | Networking code |
| `client` | Client-specific code |
| `server` | Server-specific code |
| `engine` | Game engine core |
| `build` | Build system |
| `deps` | Dependencies |

### Examples

```bash
# Feature
feat(ecs): add component pooling for better memory efficiency

# Bug fix
fix(network): resolve packet loss on high latency connections

# Documentation
docs(architecture): update ECS diagrams

# Breaking change (use ! after type)
feat(ecs)!: redesign Registry API for better type safety

BREAKING CHANGE: Registry::getComponent now returns std::optional
```

---

## Pull Request Process

### Before Submitting

- [ ] Code compiles without warnings
- [ ] All tests pass (`./build.sh -t`)
- [ ] New code has appropriate tests
- [ ] Documentation is updated if needed
- [ ] Commits follow conventions
- [ ] Branch is up-to-date with `main`

### PR Template

When creating a PR, include:

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
Describe tests added/modified

## Checklist
- [ ] Tests pass locally
- [ ] Code follows style guidelines
- [ ] Documentation updated
```

### Review Process

1. At least **1 approval** required
2. All CI checks must pass
3. No unresolved conversations
4. Squash commits if needed before merge

---

## Testing

### Running Tests

```bash
# Run all tests
./build.sh -t

# Run tests with verbose output
./build.sh -t -v

# Run specific test
cd build/ubuntu-latest  # or your preset
ctest -R "TestName" --output-on-failure
```

### Writing Tests

Use Google Test framework:

```cpp
#include <gtest/gtest.h>
#include "rtype/engine/ecs/Registry.hpp"

TEST(RegistryTest, CreateEntity) {
    rtype::ecs::Registry registry;
    auto entity = registry.createEntity();

    EXPECT_TRUE(registry.isValid(entity));
}

TEST(RegistryTest, AddComponent) {
    rtype::ecs::Registry registry;
    auto entity = registry.createEntity();

    registry.addComponent<Position>(entity, Position{10.0f, 20.0f});

    EXPECT_TRUE(registry.hasComponent<Position>(entity));
}
```

### Test Organization

```text
tests/
├── ecs/
│   ├── test_registry.cpp
│   ├── test_entity.cpp
│   └── test_view.cpp
├── network/
│   ├── test_protocol.cpp
│   └── test_serialization.cpp
└── games/
    └── rtype/
        └── test_systems.cpp
```

---

## Documentation

### Code Documentation

Use Doxygen-style comments for public APIs:

```cpp
/**
 * @brief Creates a new entity in the registry
 *
 * @return Entity The newly created entity with a unique ID
 *
 * @example
 * auto entity = registry.createEntity();
 * registry.addComponent<Position>(entity, {0.0f, 0.0f});
 */
Entity createEntity();
```

### Documentation Website

The documentation is built with Docusaurus:

```bash
cd docs/website
npm install
npm run start  # Development server
npm run build  # Production build
```

---

## Questions?

If you have questions, feel free to:

- Open an issue for discussion
- Contact the maintainers

Thank you for contributing! 🚀
