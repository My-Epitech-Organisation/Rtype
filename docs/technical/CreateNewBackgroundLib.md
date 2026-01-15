# Guide to Creating a Background Library

This document explains how to create a dynamic or static background library for the R-Type game.

## Prerequisites

Your library must be compiled as a dynamic library (`.so` on Linux, `.dll` on Windows).

## Interfaces to Implement

The main interface is `IBackground`.
You can find its definition in `lib/background/IBackground.hpp`.

It is **strongly recommended** to inherit from the abstract class `ABackground` (`lib/background/ABackground.hpp`). This class already implements ECS registry and AssetManager handling for you, as well as automatic cleanup of created entities.

### Base Class: `ABackground`

If you inherit from `ABackground`, you mainly need to implement:

1.  **Constructor**: Call the parent constructor with the registry, the asset manager, and the background name.
2.  **`createEntitiesBackground()`**: This is where you will create your entities (background sprites, planets, particles, etc.) using `_registry`.

The `unloadEntitiesBackground()` method is already implemented in `ABackground` and will delete all entities added to `_listEntities`. Make sure to add your created entities to this vector if you override the creation.

## Entry Point

Your dynamic library must expose a `createBackground` function with C linkage (`extern "C"`).

### Signature

```cpp
extern "C" {
    IBackground* createBackground(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager);
}
```

### Complete Example (Entrypoint)

```cpp
#include "MySpaceBackground.hpp"

#ifdef _WIN32
    #define RTYPE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_EXPORT
#endif

extern "C" {
    RTYPE_EXPORT IBackground *createBackground(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager) {
        return new MySpaceBackground(std::move(registry), std::move(assetManager));
    }
}
```

### Class Implementation Example

```cpp
#include "ABackground.hpp"

class MySpaceBackground : public ABackground {
public:
    MySpaceBackground(std::shared_ptr<ECS::Registry> registry, std::shared_ptr<AssetManager> assetManager)
        : ABackground(std::move(registry), std::move(assetManager), "MySpace") {}

    void createEntitiesBackground() override {
        // Creating a background entity
        ECS::Entity bgEntity = _registry->spawn_entity();
        _registry->add_component<ECS::Position>(bgEntity, ECS::Position{0, 0});
        // ... adding other components ...
        
        // Important for automatic cleanup
        _listEntities.push_back(bgEntity);
    }
};

## Compilation and Installation

To use this library in R-Type, it must be compiled as a shared library and placed in the `plugins/background/` directory.

### CMake Example

```cmake
add_library(MySpaceBackground SHARED
    entrypoint.cpp
    MySpaceBackground.cpp
    # ... other source files
)

# Link necessary libraries
target_link_libraries(MySpaceBackground PRIVATE ...)
```

### Installation Path

The compiled shared library must be placed in:
`plugins/background/<YourLibName>.so` (or `.dll` on Windows)

Example: `plugins/background/MySpace.so`

The game will automatically detect and list available backgrounds from this directory.
```
