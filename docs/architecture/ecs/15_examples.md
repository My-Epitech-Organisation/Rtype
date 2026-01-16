# Examples

## Complete Game Examples

### 1. Simple Shooter Game

```cpp
#include "ecs/src/ECS.hpp"
#include <iostream>
#include <cmath>

// Components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
struct Damage { int amount; };
struct Lifetime { float remaining; };

// Tags
struct Player {};
struct Enemy {};
struct Bullet {};

// Systems
void movement_system(ECS::Registry& reg, float dt) {
    reg.view<Position, Velocity>().each([dt](auto e, Position& pos, Velocity& vel) {
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    });
}

void lifetime_system(ECS::Registry& reg, float dt) {
    ECS::CommandBuffer cmd(reg);

    reg.view<Lifetime>().each([&, dt](ECS::Entity e, Lifetime& life) {
        life.remaining -= dt;
        if (life.remaining <= 0.0f) {
            cmd.destroyEntityDeferred(e);
        }
    });

    cmd.flush();
}

void collision_system(ECS::Registry& reg) {
    ECS::CommandBuffer cmd(reg);

    // Bullets vs Enemies
    reg.view<Position, Bullet>().each([&](ECS::Entity bullet, const Position& b_pos) {
        reg.view<Position, Health, Enemy>().each([&](ECS::Entity enemy, const Position& e_pos, Health& hp) {
            float dx = b_pos.x - e_pos.x;
            float dy = b_pos.y - e_pos.y;
            float dist_sq = dx * dx + dy * dy;

            if (dist_sq < 25.0f) { // Collision radius
                hp.hp -= 10;
                cmd.destroyEntityDeferred(bullet);

                if (hp.hp <= 0) {
                    cmd.destroyEntityDeferred(enemy);
                }
            }
        });
    });

    cmd.flush();
}

int main() {
    ECS::Registry registry;

    // Create player
    auto player = registry.spawnEntity();
    registry.emplaceComponent<Position>(player, 100.0f, 100.0f);
    registry.emplaceComponent<Velocity>(player, 0.0f, 0.0f);
    registry.emplaceComponent<Health>(player, 100);
    registry.emplaceComponent<Player>(player);

    // Create enemies
    for (int i = 0; i < 10; ++i) {
        auto enemy = registry.spawnEntity();
        registry.emplaceComponent<Position>(enemy, 200.0f + i * 50.0f, 50.0f);
        registry.emplaceComponent<Velocity>(enemy, -10.0f, 0.0f);
        registry.emplaceComponent<Health>(enemy, 30);
        registry.emplaceComponent<Enemy>(enemy);
    }

    // Game loop
    float dt = 0.016f;
    for (int frame = 0; frame < 600; ++frame) {
        // Spawn bullet every 30 frames
        if (frame % 30 == 0) {
            auto& player_pos = registry.getComponent<Position>(player);
            auto bullet = registry.spawnEntity();
            registry.emplaceComponent<Position>(bullet, player_pos.x, player_pos.y);
            registry.emplaceComponent<Velocity>(bullet, 50.0f, 0.0f);
            registry.emplaceComponent<Lifetime>(bullet, 2.0f);
            registry.emplaceComponent<Bullet>(bullet);
        }

        // Run systems
        movement_system(registry, dt);
        collision_system(registry);
        lifetime_system(registry, dt);

        // Print stats every 60 frames
        if (frame % 60 == 0) {
            size_t enemy_count = 0;
            registry.view<Enemy>().each([&](auto e) { enemy_count++; });
            std::cout << "Frame " << frame << ": " << enemy_count << " enemies remaining\n";
        }
    }

    return 0;
}
```

### 2. Particle System

```cpp
#include "ecs/src/ECS.hpp"
#include <random>

struct Particle {
    float size;
    float alpha;
};

struct Color { uint8_t r, g, b, a; };

void create_explosion(ECS::Registry& reg, float x, float y, int particle_count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angle_dist(0.0f, 6.28318f);
    std::uniform_real_distribution<float> speed_dist(10.0f, 50.0f);

    for (int i = 0; i < particle_count; ++i) {
        float angle = angle_dist(gen);
        float speed = speed_dist(gen);

        auto particle = reg.spawnEntity();
        reg.emplaceComponent<Position>(particle, x, y);
        reg.emplaceComponent<Velocity>(particle,
            std::cos(angle) * speed,
            std::sin(angle) * speed);
        reg.emplaceComponent<Particle>(particle, 5.0f, 1.0f);
        reg.emplaceComponent<Color>(particle, 255, 128, 0, 255);
        reg.emplaceComponent<Lifetime>(particle, 2.0f);
    }
}

void particle_update_system(ECS::Registry& reg, float dt) {
    reg.view<Particle, Color, Velocity>().each([dt](auto e, Particle& p, Color& c, Velocity& vel) {
        // Fade out
        p.alpha -= 0.5f * dt;
        c.a = static_cast<uint8_t>(p.alpha * 255);

        // Shrink
        p.size -= 2.0f * dt;

        // Apply gravity
        vel.dy += 98.0f * dt;
    });
}

int main() {
    ECS::Registry registry;

    // Create explosion
    create_explosion(registry, 100.0f, 100.0f, 100);

    // Simulate
    float dt = 0.016f;
    for (int frame = 0; frame < 120; ++frame) {
        movement_system(registry, dt);
        particle_update_system(registry, dt);
        lifetime_system(registry, dt);
    }

    return 0;
}
```

### 3. Transform Hierarchy

```cpp
#include "ecs/src/ECS.hpp"
#include "ECS/Core/Relationship.hpp"

struct Transform {
    float local_x, local_y;
    float world_x, world_y;
    float rotation;
};

void update_transform_hierarchy(ECS::Registry& reg, ECS::RelationshipManager& relationships) {
    // Update root transforms
    reg.view<Transform>().each([&](ECS::Entity e, Transform& t) {
        if (!relationships.hasParent(e)) {
            t.world_x = t.local_x;
            t.world_y = t.local_y;
        }
    });

    // Recursive transform propagation
    std::function<void(ECS::Entity)> update_recursive = [&](ECS::Entity entity) {
        for (ECS::Entity child : relationships.getChildren(entity)) {
            if (reg.hasComponent<Transform>(child)) {
                auto& parent_t = reg.getComponent<Transform>(entity);
                auto& child_t = reg.getComponent<Transform>(child);

                // Apply parent transform
                float cos_r = std::cos(parent_t.rotation);
                float sin_r = std::sin(parent_t.rotation);

                child_t.world_x = parent_t.world_x +
                    (child_t.local_x * cos_r - child_t.local_y * sin_r);
                child_t.world_y = parent_t.world_y +
                    (child_t.local_x * sin_r + child_t.local_y * cos_r);
                child_t.rotation = parent_t.rotation + child_t.rotation;

                update_recursive(child);
            }
        }
    };

    // Update all roots
    reg.view<Transform>().each([&](ECS::Entity e, Transform& t) {
        if (!relationships.hasParent(e)) {
            update_recursive(e);
        }
    });
}

int main() {
    ECS::Registry registry;
    ECS::RelationshipManager relationships;

    // Create hierarchy: body -> arm -> hand
    auto body = registry.spawnEntity();
    registry.emplaceComponent<Transform>(body, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f);

    auto arm = registry.spawnEntity();
    registry.emplaceComponent<Transform>(arm, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    relationships.setParent(arm, body);

    auto hand = registry.spawnEntity();
    registry.emplaceComponent<Transform>(hand, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    relationships.setParent(hand, arm);

    // Animate
    for (int frame = 0; frame < 360; ++frame) {
        auto& body_t = registry.getComponent<Transform>(body);
        body_t.rotation = frame * 0.0174f; // Convert to radians

        update_transform_hierarchy(registry, relationships);

        auto& hand_t = registry.getComponent<Transform>(hand);
        if (frame % 60 == 0) {
            std::cout << "Hand position: (" << hand_t.world_x << ", " << hand_t.world_y << ")\n";
        }
    }

    return 0;
}
```

### 4. Entity Pooling

```cpp
#include "ecs/src/ECS.hpp"

struct Inactive {}; // Tag for pooled entities

class EntityPool {
    ECS::Registry& registry;
    ECS::Group<Inactive> inactive_group;

public:
    EntityPool(ECS::Registry& reg)
        : registry(reg), inactive_group(reg) {
        inactive_group.rebuild();
    }

    ECS::Entity acquire() {
        if (!inactive_group.empty()) {
            ECS::Entity e = *inactive_group.begin();
            registry.removeComponent<Inactive>(e);
            inactive_group.rebuild();
            return e;
        }
        return registry.spawnEntity();
    }

    void release(ECS::Entity e) {
        // Remove all components except position
        if (registry.hasComponent<Velocity>(e)) {
            registry.removeComponent<Velocity>(e);
        }
        if (registry.hasComponent<Health>(e)) {
            registry.removeComponent<Health>(e);
        }

        // Mark as inactive
        registry.emplaceComponent<Inactive>(e);
        inactive_group.rebuild();
    }

    size_t available_count() const {
        return inactive_group.size();
    }
};

int main() {
    ECS::Registry registry;
    EntityPool pool(registry);

    // Pre-warm pool
    for (int i = 0; i < 100; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, 0.0f, 0.0f);
        pool.release(e);
    }

    std::cout << "Pool has " << pool.available_count() << " entities\n";

    // Acquire and use
    std::vector<ECS::Entity> active_entities;
    for (int i = 0; i < 50; ++i) {
        auto e = pool.acquire();
        registry.emplaceComponent<Velocity>(e, 1.0f, 0.0f);
        registry.emplaceComponent<Health>(e, 100);
        active_entities.push_back(e);
    }

    std::cout << "Pool has " << pool.available_count() << " entities after acquiring 50\n";

    // Release back to pool
    for (auto e : active_entities) {
        pool.release(e);
    }

    std::cout << "Pool has " << pool.available_count() << " entities after releasing\n";

    return 0;
}
```

### 5. Spatial Partitioning

```cpp
#include "ecs/src/ECS.hpp"
#include <unordered_map>
#include <vector>

struct SpatialHash {
    float cell_size;
    std::unordered_map<int64_t, std::vector<ECS::Entity>> cells;

    SpatialHash(float size) : cell_size(size) {}

    int64_t hash(int x, int y) const {
        return (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
    }

    void insert(ECS::Entity e, float x, float y) {
        int cell_x = static_cast<int>(x / cell_size);
        int cell_y = static_cast<int>(y / cell_size);
        cells[hash(cell_x, cell_y)].push_back(e);
    }

    std::vector<ECS::Entity> query(float x, float y, float radius) const {
        std::vector<ECS::Entity> results;

        int min_x = static_cast<int>((x - radius) / cell_size);
        int max_x = static_cast<int>((x + radius) / cell_size);
        int min_y = static_cast<int>((y - radius) / cell_size);
        int max_y = static_cast<int>((y + radius) / cell_size);

        for (int cy = min_y; cy <= max_y; ++cy) {
            for (int cx = min_x; cx <= max_x; ++cx) {
                auto it = cells.find(hash(cx, cy));
                if (it != cells.end()) {
                    results.insert(results.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return results;
    }

    void clear() {
        cells.clear();
    }
};

void spatial_collision_system(ECS::Registry& reg) {
    SpatialHash spatial(50.0f); // 50 unit cells

    // Build spatial hash
    reg.view<Position>().each([&](ECS::Entity e, const Position& pos) {
        spatial.insert(e, pos.x, pos.y);
    });

    // Check collisions only in nearby cells
    reg.view<Position>().each([&](ECS::Entity e1, const Position& pos1) {
        auto nearby = spatial.query(pos1.x, pos1.y, 25.0f);

        for (ECS::Entity e2 : nearby) {
            if (e1.id >= e2.id) continue; // Avoid duplicate checks

            if (reg.hasComponent<Position>(e2)) {
                auto& pos2 = reg.getComponent<Position>(e2);
                float dx = pos1.x - pos2.x;
                float dy = pos1.y - pos2.y;
                float dist_sq = dx * dx + dy * dy;

                if (dist_sq < 625.0f) { // 25 * 25
                    // Handle collision
                }
            }
        }
    });
}

int main() {
    ECS::Registry registry;

    // Create entities in spatial grid
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            auto e = registry.spawnEntity();
            registry.emplaceComponent<Position>(e, x * 100.0f, y * 100.0f);
        }
    }

    spatial_collision_system(registry);

    return 0;
}
```

## Code Snippets

### Debug Entity Inspector

```cpp
void inspect_entity(ECS::Registry& reg, ECS::Entity entity) {
    std::cout << "Entity " << entity.index()
              << " (gen: " << entity.generation() << ")\n";

    if (!reg.isAlive(entity)) {
        std::cout << "  [DEAD]\n";
        return;
    }

    std::cout << "  Components:\n";
    if (reg.hasComponent<Position>(entity)) {
        auto& pos = reg.getComponent<Position>(entity);
        std::cout << "    Position: (" << pos.x << ", " << pos.y << ")\n";
    }
    if (reg.hasComponent<Velocity>(entity)) {
        auto& vel = reg.getComponent<Velocity>(entity);
        std::cout << "    Velocity: (" << vel.dx << ", " << vel.dy << ")\n";
    }
    if (reg.hasComponent<Health>(entity)) {
        auto& hp = reg.getComponent<Health>(entity);
        std::cout << "    Health: " << hp.hp << " HP\n";
    }
}
```

### Frame Timer

```cpp
class FrameTimer {
    std::chrono::high_resolution_clock::time_point last_time;
    float accumulator = 0.0f;
    int frame_count = 0;

public:
    FrameTimer() : last_time(std::chrono::high_resolution_clock::now()) {}

    float tick() {
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        accumulator += dt;
        frame_count++;

        if (accumulator >= 1.0f) {
            std::cout << "FPS: " << frame_count << "\n";
            accumulator = 0.0f;
            frame_count = 0;
        }

        return dt;
    }
};
```

## See Also

- [API Quick Reference](15_api_reference.md) - Complete API documentation
- [Optimization Guide](14_optimization.md) - Performance tips
