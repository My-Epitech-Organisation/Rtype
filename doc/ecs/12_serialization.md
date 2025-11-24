# Serialization

## Overview

The **Serialization** system provides save/load functionality for ECS state, enabling persistence, save games, and network synchronization.

## Core Concepts

### Component Serializer

Each component type needs a serializer that converts between component data and string representation:

```cpp
class IComponentSerializer {
public:
    virtual std::string serialize(Entity entity, Registry& registry) const = 0;
    virtual void deserialize(Entity entity, const std::string& data, Registry& registry) const = 0;
};
```

### Serializer

The main serializer coordinates saving/loading of entities and components:

```cpp
class Serializer {
public:
    explicit Serializer(Registry& reg);
    
    template<typename T>
    void registerSerializer(std::shared_ptr<IComponentSerializer> serializer);
    
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename, bool clear_existing = true);
    
    std::string serialize();
    bool deserialize(const std::string& data, bool clear_existing = true);
};
```

## Basic Usage

### Define Component _serializers

```cpp
struct Position { float x, y; };

class PositionSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& pos = registry.getComponent<Position>(entity);
        return std::to_string(pos.x) + "," + std::to_string(pos.y);
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        size_t comma = data.find(',');
        float x = std::stof(data.substr(0, comma));
        float y = std::stof(data.substr(comma + 1));
        registry.emplaceComponent<Position>(entity, x, y);
    }
};
```

### Register _serializers

```cpp
Serializer serializer(registry);

serializer.registerSerializer<Position>(std::make_shared<PositionSerializer>());
serializer.registerSerializer<Velocity>(std::make_shared<VelocitySerializer>());
serializer.registerSerializer<Health>(std::make_shared<HealthSerializer>());
```

### Save and Load

```cpp
// Save ECS state
if (serializer.saveToFile("savegame.txt")) {
    std::cout << "Game saved successfully\n";
}

// Load ECS state
if (serializer.loadFromFile("savegame.txt")) {
    std::cout << "Game loaded successfully\n";
}
```

## File Format

### Structure

```
[Entities]
entity_count
index,generation
index,generation
...

[Components:TypeName]
entity_index,serialized_data
entity_index,serialized_data
...

[Components:AnotherType]
...
```

### Example

```
[Entities]
3
0,0
1,0
2,0

[Components:Position]
0,10.5,20.3
1,5.0,15.0
2,-10.0,30.0

[Components:Health]
0,100
1,50
2,75
```

## Advanced _serializers

### JSON Serializer

```cpp
#include <nlohmann/json.hpp>

class JsonPositionSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& pos = registry.getComponent<Position>(entity);
        nlohmann::json j;
        j["x"] = pos.x;
        j["y"] = pos.y;
        return j.dump();
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        auto j = nlohmann::json::parse(data);
        registry.emplaceComponent<Position>(entity, j["x"], j["y"]);
    }
};
```

### Binary Serializer

```cpp
class BinaryPositionSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& pos = registry.getComponent<Position>(entity);
        std::string data;
        data.resize(sizeof(Position));
        std::memcpy(data.data(), &pos, sizeof(Position));
        return data;
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        Position pos;
        std::memcpy(&pos, data.data(), sizeof(Position));
        registry.emplaceComponent<Position>(entity, pos.x, pos.y);
    }
};
```

### Compressed Serializer

```cpp
class CompressedSerializer : public IComponentSerializer {
    std::shared_ptr<IComponentSerializer> inner;
    
public:
    CompressedSerializer(std::shared_ptr<IComponentSerializer> serializer)
        : inner(serializer) {}
    
    std::string serialize(Entity entity, Registry& registry) const override {
        std::string data = inner->serialize(entity, registry);
        return compress(data); // Use zlib, etc.
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        std::string decompressed = decompress(data);
        inner->deserialize(entity, decompressed, registry);
    }
};
```

## Selective Serialization

### Save Specific Components

```cpp
class SaveGame {
    Serializer serializer;
    
public:
    SaveGame(Registry& reg) : serializer(reg) {
        // Only register components that should be saved
        serializer.registerSerializer<Position>(std::make_shared<PositionSerializer>());
        serializer.registerSerializer<Health>(std::make_shared<HealthSerializer>());
        // Don't register transient components (Velocity, etc.)
    }
    
    void save(const std::string& filename) {
        serializer.saveToFile(filename);
    }
};
```

### Save Specific Entities

```cpp
void save_important_entities(Registry& registry, const std::string& filename) {
    Serializer serializer(registry);
    
    // Create temporary registry with only important entities
    Registry temp_registry;
    
    registry.view<Important>().each([&](Entity e, auto& important) {
        // Clone entity to temp registry
        Entity new_e = temp_registry.spawnEntity();
        
        // Copy components
        if (registry.hasComponent<Position>(e)) {
            auto& pos = registry.getComponent<Position>(e);
            temp_registry.emplaceComponent<Position>(new_e, pos);
        }
        // ... copy other components
    });
    
    // Save temp registry
    Serializer temp_serializer(temp_registry);
    temp_serializer.registerSerializer<Position>(/* ... */);
    temp_serializer.saveToFile(filename);
}
```

## Entity Mapping

### Handle Entity References

```cpp
struct Parent {
    Entity parent_entity;
};

class ParentSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& parent = registry.getComponent<Parent>(entity);
        // Save parent entity ID
        return std::to_string(parent.parent_entity.id);
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        uint32_t parent_id = std::stoul(data);
        Entity parent(parent_id);
        
        // Note: Assumes parent entity exists with same ID
        // May need entity ID remapping in complex scenarios
        registry.emplaceComponent<Parent>(entity, parent);
    }
};
```

### Entity ID Remapping

```cpp
class SerializerWithRemapping {
    std::unordered_map<uint32_t, Entity> id_map;
    
public:
    Entity get_mapped_entity(uint32_t old_id) {
        if (id_map.contains(old_id)) {
            return id_map[old_id];
        }
        // Create new entity and store mapping
        Entity new_entity = registry.spawnEntity();
        id_map[old_id] = new_entity;
        return new_entity;
    }
};
```

## Versioning

### Component Version Support

```cpp
class VersionedHealthSerializer : public IComponentSerializer {
    int version = 2;
    
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& health = registry.getComponent<Health>(entity);
        return std::to_string(version) + ";" + 
               std::to_string(health.hp) + ";" + 
               std::to_string(health.max_hp);
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        size_t first_semi = data.find(';');
        int saved_version = std::stoi(data.substr(0, first_semi));
        
        if (saved_version == 1) {
            // Old format: just hp
            int hp = std::stoi(data.substr(first_semi + 1));
            registry.emplaceComponent<Health>(entity, hp, 100); // Default max_hp
        } else if (saved_version == 2) {
            // New format: hp and max_hp
            size_t second_semi = data.find(';', first_semi + 1);
            int hp = std::stoi(data.substr(first_semi + 1, second_semi - first_semi - 1));
            int max_hp = std::stoi(data.substr(second_semi + 1));
            registry.emplaceComponent<Health>(entity, hp, max_hp);
        }
    }
};
```

## Performance

### Optimization Tips

```cpp
// 1. Reserve memory
void save_optimized(Registry& registry, Serializer& serializer) {
    std::ostringstream oss;
    oss.str().reserve(1024 * 1024); // 1 MB
    // ... serialize
}

// 2. Batch writes
void save_in_batches(Registry& registry) {
    std::ofstream file("save.dat", std::ios::binary);
    char buffer[4096];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    // ... write data
}

// 3. Parallel serialization (careful with thread safety)
void parallel_serialize(Registry& registry) {
    std::vector<std::string> component_data;
    
    // Serialize each component type in parallel
    std::thread t1([&]() { /* serialize Position */ });
    std::thread t2([&]() { /* serialize Velocity */ });
    
    t1.join();
    t2.join();
    
    // Combine results
}
```

## Complete Example

```cpp
#include "ECS/Serialization/Serialization.hpp"

// Define components
struct Position { float x, y; };
struct Health { int hp, max_hp; };

// Define _serializers
class PositionSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& pos = registry.getComponent<Position>(entity);
        return std::to_string(pos.x) + "," + std::to_string(pos.y);
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        size_t comma = data.find(',');
        float x = std::stof(data.substr(0, comma));
        float y = std::stof(data.substr(comma + 1));
        registry.emplaceComponent<Position>(entity, x, y);
    }
};

class HealthSerializer : public IComponentSerializer {
public:
    std::string serialize(Entity entity, Registry& registry) const override {
        auto& health = registry.getComponent<Health>(entity);
        return std::to_string(health.hp) + "," + std::to_string(health.max_hp);
    }
    
    void deserialize(Entity entity, const std::string& data, Registry& registry) const override {
        size_t comma = data.find(',');
        int hp = std::stoi(data.substr(0, comma));
        int max_hp = std::stoi(data.substr(comma + 1));
        registry.emplaceComponent<Health>(entity, hp, max_hp);
    }
};

int main() {
    ECS::Registry registry;
    
    // Create some entities
    auto e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 10.0f, 20.0f);
    registry.emplaceComponent<Health>(e1, 100, 100);
    
    auto e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 5.0f, 15.0f);
    registry.emplaceComponent<Health>(e2, 50, 75);
    
    // Setup serializer
    ECS::Serializer serializer(registry);
    serializer.registerSerializer<Position>(std::make_shared<PositionSerializer>());
    serializer.registerSerializer<Health>(std::make_shared<HealthSerializer>());
    
    // Save
    if (serializer.saveToFile("game.save")) {
        std::cout << "Saved successfully\n";
    }
    
    // Clear registry
    registry.clear();
    
    // Load
    if (serializer.loadFromFile("game.save")) {
        std::cout << "Loaded successfully\n";
        
        // Verify
        registry.view<Position, Health>().each([](Entity e, auto& pos, auto& hp) {
            std::cout << "Entity " << e.index() 
                      << " at (" << pos.x << "," << pos.y << ")"
                      << " with HP " << hp.hp << "/" << hp.max_hp << "\n";
        });
    }
    
    return 0;
}
```

## Best Practices

### ✅ Do

- Implement _serializers for all persistent components
- Version your save format
- Test save/load thoroughly
- Handle entity references carefully
- Validate loaded data
- Use binary format for performance

### ❌ Don't

- Don't serialize transient data (frame counters, etc.)
- Don't assume entity IDs persist
- Don't forget error handling
- Don't save pointers or references
- Don't use text format for large saves

## See Also

- [Registry](03_registry.md) - Entity management
- [Prefabs](11_prefabs.md) - Entity templates
- [Relationships](10_relationships.md) - Hierarchy serialization
