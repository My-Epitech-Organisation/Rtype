# Engine Public API

This directory contains the **public interface** for the R-Type game engine.

## Available Interfaces

### `IRegistry.hpp`
Public interface for the Entity Component System (ECS) registry.

**Usage:**
```cpp
#include <rtype/engine/IRegistry.hpp>

rtype::engine::IRegistry& registry = getRegistry();
rtype::engine::EntityHandle player = registry.createEntity();
// Use player entity...
registry.destroyEntity(player);
```

**Key Features:**
- `createEntity()` - Create new entities
- `destroyEntity()` - Remove entities from the system
- `entityCount()` - Get number of active entities
- `clear()` - Remove all entities

## Implementation Details

This include directory exposes the public, abstract interfaces for the engine. All headers here declare abstract types and APIs for external use; concrete implementation classes and their corresponding .cpp files live under src/engine/. Implementation classes in src/engine/ may not directly inherit from these public interfaces to keep internal details separate from the public API.
