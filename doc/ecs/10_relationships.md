# Relationships

## Overview

The **RelationshipManager** provides hierarchical parent-child relationships between entities, enabling tree structures for scene graphs, UI hierarchies, and entity ownership.

## Core Concepts

### Parent-Child Relationships

- Each child has **at most one parent**
- Each parent can have **multiple children**
- Forms a **tree structure** (no cycles allowed)
- Automatic cleanup on entity destruction

### Use Cases

- **Scene Graphs**: Transform hierarchies
- **UI Trees**: Widget parent-child relationships
- **Entity Ownership**: Weapon attached to player
- **Prefab Instances**: Entity templates with children

## Basic Usage

```cpp
#include "ECS/Core/Relationship.hpp"

ECS::RelationshipManager mgr;

// Create entities
auto parent = registry.spawn_entity();
auto child1 = registry.spawn_entity();
auto child2 = registry.spawn_entity();

// Set relationships
mgr.set_parent(child1, parent);
mgr.set_parent(child2, parent);

// Query relationships
auto parent_entity = mgr.get_parent(child1); // Returns std::optional<Entity>
auto children = mgr.get_children(parent);    // Returns std::vector<Entity>
```

## API Reference

### Setting Relationships

```cpp
// Set parent-child relationship
bool set_parent(Entity child, Entity parent);

// Remove parent (orphan the child)
void remove_parent(Entity child);
```

### Querying Relationships

```cpp
// Get parent of entity
std::optional<Entity> get_parent(Entity child) const;

// Check if entity has parent
bool has_parent(Entity child) const;

// Get direct children
std::vector<Entity> get_children(Entity parent) const;

// Get all descendants (recursive)
std::vector<Entity> get_descendants(Entity parent) const;

// Get all ancestors
std::vector<Entity> get_ancestors(Entity child) const;

// Get root of hierarchy
Entity get_root(Entity entity) const;

// Check if entity is ancestor of another
bool is_ancestor(Entity potential_ancestor, Entity entity) const;
```

### Hierarchy Operations

```cpp
// Check if entity has children
bool has_children(Entity parent) const;

// Get child count
size_t child_count(Entity parent) const;

// Remove entity and all descendants
void destroy_hierarchy(Entity root);
```

## Cycle Prevention

The relationship manager prevents cycles:

```cpp
auto e1 = registry.spawn_entity();
auto e2 = registry.spawn_entity();
auto e3 = registry.spawn_entity();

mgr.set_parent(e2, e1); // e1 -> e2
mgr.set_parent(e3, e2); // e1 -> e2 -> e3

bool result = mgr.set_parent(e1, e3); // Would create cycle: e1 -> e2 -> e3 -> e1
// result = false, cycle prevented
```

## Advanced Usage

### Transform Hierarchies

```cpp
struct Transform {
    float x, y;
    float rotation;
    float local_x, local_y; // Relative to parent
};

void update_transforms(Registry& reg, RelationshipManager& mgr) {
    // Update root transforms
    reg.view<Transform>().each([&](Entity e, Transform& t) {
        if (!mgr.has_parent(e)) {
            t.x = t.local_x;
            t.y = t.local_y;
        }
    });
    
    // Update child transforms (breadth-first)
    std::queue<Entity> queue;
    
    // Add all roots to queue
    reg.view<Transform>().each([&](Entity e, Transform& t) {
        if (!mgr.has_parent(e)) {
            queue.push(e);
        }
    });
    
    while (!queue.empty()) {
        Entity parent = queue.front();
        queue.pop();
        
        auto& parent_transform = reg.get_component<Transform>(parent);
        
        for (Entity child : mgr.get_children(parent)) {
            if (reg.has_component<Transform>(child)) {
                auto& child_transform = reg.get_component<Transform>(child);
                
                // Apply parent transform
                child_transform.x = parent_transform.x + child_transform.local_x;
                child_transform.y = parent_transform.y + child_transform.local_y;
                child_transform.rotation = parent_transform.rotation + child_transform.rotation;
                
                queue.push(child);
            }
        }
    }
}
```

### UI Hierarchies

```cpp
struct Widget {
    int width, height;
    int abs_x, abs_y;  // Absolute position
    int rel_x, rel_y;  // Relative to parent
};

void update_ui_layout(Registry& reg, RelationshipManager& mgr) {
    // Root widgets
    reg.view<Widget>().each([&](Entity e, Widget& w) {
        if (!mgr.has_parent(e)) {
            w.abs_x = w.rel_x;
            w.abs_y = w.rel_y;
        }
    });
    
    // Propagate positions down hierarchy
    reg.view<Widget>().each([&](Entity e, Widget& w) {
        auto parent_opt = mgr.get_parent(e);
        if (parent_opt && reg.has_component<Widget>(*parent_opt)) {
            auto& parent_widget = reg.get_component<Widget>(*parent_opt);
            w.abs_x = parent_widget.abs_x + w.rel_x;
            w.abs_y = parent_widget.abs_y + w.rel_y;
        }
    });
}
```

### Entity Ownership

```cpp
void create_player_with_weapon(Registry& reg, RelationshipManager& mgr) {
    // Create player
    Entity player = reg.spawn_entity();
    reg.emplace_component<Player>(player);
    reg.emplace_component<Transform>(player, 0.0f, 0.0f);
    
    // Create weapon as child
    Entity weapon = reg.spawn_entity();
    reg.emplace_component<Weapon>(weapon);
    reg.emplace_component<Transform>(weapon, 1.0f, 0.0f); // Offset from player
    
    // Attach weapon to player
    mgr.set_parent(weapon, player);
}

void destroy_player(Registry& reg, RelationshipManager& mgr, Entity player) {
    // Get all children (weapons, equipment, etc.)
    auto children = mgr.get_children(player);
    
    // Destroy children first
    for (Entity child : children) {
        reg.kill_entity(child);
    }
    
    // Destroy player
    reg.kill_entity(player);
}
```

### Prefab Hierarchies

```cpp
Entity instantiate_prefab_tree(Registry& reg, RelationshipManager& mgr, 
                                const PrefabData& prefab) {
    Entity root = reg.spawn_entity();
    // Add components to root...
    
    for (const auto& child_data : prefab.children) {
        Entity child = reg.spawn_entity();
        // Add components to child...
        mgr.set_parent(child, root);
    }
    
    return root;
}
```

## Hierarchical Operations

### Traverse Hierarchy

```cpp
// Depth-first traversal
void traverse_depth_first(Entity root, RelationshipManager& mgr, 
                         std::function<void(Entity)> callback) {
    callback(root);
    
    for (Entity child : mgr.get_children(root)) {
        traverse_depth_first(child, mgr, callback);
    }
}

// Breadth-first traversal
void traverse_breadth_first(Entity root, RelationshipManager& mgr,
                           std::function<void(Entity)> callback) {
    std::queue<Entity> queue;
    queue.push(root);
    
    while (!queue.empty()) {
        Entity current = queue.front();
        queue.pop();
        
        callback(current);
        
        for (Entity child : mgr.get_children(current)) {
            queue.push(child);
        }
    }
}
```

### Find in Hierarchy

```cpp
// Find entity by predicate
std::optional<Entity> find_in_hierarchy(Entity root, RelationshipManager& mgr,
                                        std::function<bool(Entity)> predicate) {
    if (predicate(root)) return root;
    
    for (Entity child : mgr.get_children(root)) {
        auto result = find_in_hierarchy(child, mgr, predicate);
        if (result) return result;
    }
    
    return std::nullopt;
}

// Usage
auto enemy = find_in_hierarchy(scene_root, mgr, [&](Entity e) {
    return registry.has_component<Enemy>(e);
});
```

### Hierarchy Statistics

```cpp
// Count descendants
size_t count_descendants(Entity root, RelationshipManager& mgr) {
    size_t count = 0;
    traverse_depth_first(root, mgr, [&](Entity e) { count++; });
    return count - 1; // Exclude root
}

// Get hierarchy depth
int get_hierarchy_depth(Entity entity, RelationshipManager& mgr) {
    int depth = 0;
    auto current = entity;
    while (mgr.has_parent(current)) {
        depth++;
        current = *mgr.get_parent(current);
    }
    return depth;
}
```

## Thread Safety

The RelationshipManager is thread-safe:

```cpp
// ✅ Safe: Concurrent reads
std::thread t1([&] { auto children = mgr.get_children(entity1); });
std::thread t2([&] { auto parent = mgr.get_parent(entity2); });

// ✅ Safe: Concurrent modifications to different entities
std::thread t1([&] { mgr.set_parent(child1, parent1); });
std::thread t2([&] { mgr.set_parent(child2, parent2); });
```

## Performance

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| set_parent | O(1) | Plus cycle detection O(depth) |
| remove_parent | O(1) | Hash map removal |
| get_parent | O(1) | Hash map lookup |
| has_parent | O(1) | Hash map lookup |
| get_children | O(k) | k = child count |
| get_descendants | O(n) | n = descendant count (DFS) |
| get_ancestors | O(d) | d = depth in hierarchy |
| is_ancestor | O(d) | d = depth in hierarchy |

## Best Practices

### ✅ Do

- Use for logical parent-child relationships
- Clean up relationships when destroying entities
- Check for valid parents before setting
- Use for transform propagation
- Document hierarchy structure

### ❌ Don't

- Don't create deep hierarchies (performance impact)
- Don't assume parent/child always exists (use optional)
- Don't modify hierarchy during traversal
- Don't create cycles (prevented, but wastes cycles)
- Don't use for simple associations (use components instead)

## Integration Example

```cpp
class SceneGraph {
    Registry& registry;
    RelationshipManager relationships;
    Entity root;
    
public:
    SceneGraph(Registry& reg) : registry(reg) {
        root = registry.spawn_entity();
        registry.emplace_component<Transform>(root);
    }
    
    Entity create_entity(Entity parent = Entity()) {
        Entity e = registry.spawn_entity();
        registry.emplace_component<Transform>(e);
        
        if (parent.is_null()) {
            relationships.set_parent(e, root);
        } else {
            relationships.set_parent(e, parent);
        }
        
        return e;
    }
    
    void destroy_entity(Entity e) {
        // Destroy all children recursively
        for (Entity child : relationships.get_children(e)) {
            destroy_entity(child);
        }
        
        // Remove from parent
        relationships.remove_parent(e);
        
        // Destroy entity
        registry.kill_entity(e);
    }
    
    void update_transforms() {
        update_transforms_recursive(root);
    }
    
private:
    void update_transforms_recursive(Entity entity) {
        if (relationships.has_parent(entity)) {
            Entity parent = *relationships.get_parent(entity);
            auto& pt = registry.get_component<Transform>(parent);
            auto& ct = registry.get_component<Transform>(entity);
            
            // Apply parent transform
            ct.world_x = pt.world_x + ct.local_x;
            ct.world_y = pt.world_y + ct.local_y;
        }
        
        // Update children
        for (Entity child : relationships.get_children(entity)) {
            update_transforms_recursive(child);
        }
    }
};
```

## See Also

- [Prefabs](11_prefabs.md) - Entity templates
- [Serialization](12_serialization.md) - Save/load hierarchies
- [Registry](03_registry.md) - Entity management
