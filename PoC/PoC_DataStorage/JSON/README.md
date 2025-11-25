# JSON Storage Proof of Concept

## Overview

This PoC demonstrates JSON-based data storage and serialization integrated with the R-Type ECS (Entity Component System).

## Features

- ✅ Game configuration loading from JSON files
- ✅ ECS component serialization to/from JSON
- ✅ Integration with nlohmann/json library
- ✅ Type-safe data structures
- ✅ Entity state persistence

## Building the PoC

### Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler
- Internet connection (for downloading nlohmann/json)

### Build Instructions

```bash
# From the PoC/json_storage directory
mkdir build
cd build
cmake ..
make

# Run the PoC
./json_storage_poc
```

## Structure

```
json_storage/
├── CMakeLists.txt              # Build configuration
├── GameConfig.hpp              # Game configuration structures
├── JsonComponentSerializer.hpp # ECS-JSON integration
├── main.cpp                    # Demonstration program
└── README.md                   # This file
```

## What This PoC Demonstrates

### 1. Game Configuration Loading

The PoC loads a `game_config.json` file containing:
- Window settings (resolution, fullscreen)
- Player configuration (health, speed, etc.)
- Enemy types and their properties
- Weapon definitions
- Level configurations

**Example JSON structure:**
```json
{
  "version": "1.0.0",
  "windowWidth": 1920,
  "windowHeight": 1080,
  "fullscreen": false,
  "player": {
    "name": "Player1",
    "maxHealth": 100,
    "speed": 5.0,
    "score": 0
  },
  "enemies": [
    {
      "type": "Scout",
      "health": 50,
      "damage": 10,
      "speed": 3.0,
      "scoreValue": 100
    }
  ]
}
```

### 2. ECS Component Serialization

Components like `Position`, `Velocity`, and `Health` can be:
- Serialized to JSON format
- Saved to disk
- Loaded back into the ECS registry

### 3. Entity State Persistence

Complete game state can be saved and restored:
```cpp
// Save
saveEntitiesToJson(registry, "entities.json");

// Load
loadEntitiesFromJson(registry, "entities.json");
```

## Components Defined

- **Position**: `{x, y}` coordinates
- **Velocity**: `{dx, dy}` movement vector
- **Health**: `{current, max}` health points
- **PlayerTag**: Marks player entities
- **EnemyTag**: Marks enemy entities

## Usage Examples

### Loading Game Configuration

```cpp
#include "GameConfig.hpp"

GameConfig config = loadGameConfig("game_config.json");
std::cout << "Player health: " << config.player.maxHealth << std::endl;
```

### Using ECS with JSON

```cpp
#include "JsonComponentSerializer.hpp"
#include <ECS/ECS.hpp>

ECS::Registry registry;

// Create entity
ECS::Entity player = registry.createEntity();
registry.emplaceComponent<Position>(player, 100.0f, 200.0f);

// Save to JSON
saveEntitiesToJson(registry, "save.json");

// Load from JSON
loadEntitiesFromJson(registry, "save.json");
```

### Using ECS Serializer

```cpp
ECS::Serializer serializer(&registry);
serializer.registerSerializer<Position>(
    std::make_shared<JsonComponentSerializer<Position>>()
);

serializer.saveToFile("save.txt");
serializer.loadFromFile("save.txt");
```

## Output Files Generated

When running the PoC, the following files are created:

1. **game_config.json** - Sample game configuration
2. **entities.json** - Saved ECS entities with components
3. **ecs_save.txt** - ECS serializer format (text-based with JSON data)

## Key Findings

### ✅ JSON Parsing is Easy in C++

Using nlohmann/json makes JSON parsing straightforward:
- Type-safe conversions
- Intuitive syntax
- Automatic serialization/deserialization
- No manual parsing required

### ✅ Integration with ECS Works Well

The ECS architecture integrates cleanly with JSON:
- Components map naturally to JSON objects
- Entities can be saved/loaded efficiently
- Custom serializers provide flexibility

## Performance Notes

- JSON parsing is relatively fast for configuration files
- Text format is human-readable for debugging
- File size is larger than binary formats
- Suitable for save files and configuration, not real-time data

## Next Steps

For production use, consider:
1. Error handling and validation
2. Schema validation for JSON files
3. Encryption for save files
4. Binary serialization for performance-critical data
5. Compression for large save files

## Related Documentation

See `doc/json_storage_analysis.md` for a detailed analysis of pros and cons.
