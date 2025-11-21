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

Implementation headers (`.hpp` files with corresponding `.cpp`) are located in `src/engine/`.
This directory contains **only** abstract interfaces for public consumption.
