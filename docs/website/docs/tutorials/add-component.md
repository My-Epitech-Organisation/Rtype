---
sidebar_label: Add a Component
sidebar_position: 1
---

# How to Add a New Component

This tutorial shows how to create and use a new component in the ECS architecture.

## 📋 What is a Component?

Components are pure data structures that hold state. They don't contain logic - that belongs in Systems. A component represents a single aspect of an entity's behavior or properties.

**Examples:**
- `Position`: X, Y coordinates
- `Health`: Current HP, max HP
- `Velocity`: Speed and direction
- `Sprite`: Visual representation

---

## 🎯 Step 1: Define the Component

Components live in `include/rtype/common/components/` or game-specific directories.

### Example: Create a Shield Component

Create `include/rtype/common/components/ShieldComponent.hpp`:

```cpp
#ifndef RTYPE_COMMON_COMPONENTS_SHIELDCOMPONENT_HPP_
#define RTYPE_COMMON_COMPONENTS_SHIELDCOMPONENT_HPP_

namespace rtype::component {

/**
 * @brief Shield component providing temporary invincibility
 * 
 * When active, the entity is invulnerable to damage.
 * The shield depletes over time or when absorbing hits.
 */
struct Shield {
    float maxCapacity;      ///< Maximum shield strength
    float currentCapacity;  ///< Current shield strength
    float rechargeRate;     ///< Shield recharge per second
    float rechargeDelay;    ///< Delay before recharge starts (seconds)
    float timeSinceHit;     ///< Time since last hit
    bool isActive;          ///< Whether shield is currently active
    
    /**
     * @brief Construct a new Shield with default values
     */
    Shield(float capacity = 100.0f, float recharge = 20.0f)
        : maxCapacity(capacity)
        , currentCapacity(capacity)
        , rechargeRate(recharge)
        , rechargeDelay(2.0f)
        , timeSinceHit(0.0f)
        , isActive(true) {}
};

}  // namespace rtype::component

#endif  // RTYPE_COMMON_COMPONENTS_SHIELDCOMPONENT_HPP_
```

### Component Design Guidelines

#### ✅ Good Components (Data Only)

```cpp
struct Health {
    int current;
    int maximum;
};

struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};
```

#### ❌ Bad Components (Logic Included)

```cpp
// DON'T DO THIS - Logic belongs in Systems
struct BadHealth {
    int health;
    
    void takeDamage(int amount) {  // ❌ No methods
        health -= amount;
    }
    
    bool isDead() const {          // ❌ No logic
        return health <= 0;
    }
};
```

---

## 🎯 Step 2: Register the Component

Components must be registered with the ECS registry before use.

### In Your Game Initialization

Edit `src/games/rtype/server/GameEngine.cpp` (or client equivalent):

```cpp
#include "rtype/common/components/ShieldComponent.hpp"

void GameEngine::initialize() {
    // Register all components
    registry_.registerComponent<rtype::component::Position>();
    registry_.registerComponent<rtype::component::Health>();
    registry_.registerComponent<rtype::component::Shield>();  // New component
    // ... other components
}
```

---

## 🎯 Step 3: Create a System to Use the Component

Create a system that operates on entities with the component.

### Example: ShieldSystem

Create `src/games/rtype/server/Systems/ShieldSystem.hpp`:

```cpp
#ifndef RTYPE_SERVER_SYSTEMS_SHIELDSYSTEM_HPP_
#define RTYPE_SERVER_SYSTEMS_SHIELDSYSTEM_HPP_

#include "rtype/engine/ASystem.hpp"
#include "rtype/ecs/Registry.hpp"

namespace rtype::server {

/**
 * @brief System that manages shield recharge and depletion
 */
class ShieldSystem : public engine::ASystem {
public:
    ShieldSystem() : ASystem("ShieldSystem") {}
    
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::server

#endif  // RTYPE_SERVER_SYSTEMS_SHIELDSYSTEM_HPP_
```

Create `src/games/rtype/server/Systems/ShieldSystem.cpp`:

```cpp
#include "ShieldSystem.hpp"
#include "rtype/common/components/ShieldComponent.hpp"
#include <algorithm>

namespace rtype::server {

void ShieldSystem::update(ECS::Registry& registry, float dt) {
    // View all entities with Shield component
    auto view = registry.view<component::Shield>();
    
    view.each([dt](auto entity, component::Shield& shield) {
        // Increment time since last hit
        shield.timeSinceHit += dt;
        
        // Recharge shield if delay has passed
        if (shield.timeSinceHit >= shield.rechargeDelay) {
            if (shield.currentCapacity < shield.maxCapacity) {
                shield.currentCapacity += shield.rechargeRate * dt;
                shield.currentCapacity = std::min(
                    shield.currentCapacity, 
                    shield.maxCapacity
                );
            }
        }
        
        // Deactivate shield if depleted
        if (shield.currentCapacity <= 0.0f) {
            shield.isActive = false;
        }
        
        // Reactivate shield if recharged
        if (shield.currentCapacity > 0.0f && !shield.isActive) {
            shield.isActive = true;
        }
    });
}

}  // namespace rtype::server
```

---

## 🎯 Step 4: Use the Component in Game Logic

### Adding Components to Entities

```cpp
#include "rtype/common/components/ShieldComponent.hpp"

// Create entity with shield
auto player = registry.spawnEntity();
registry.emplaceComponent<component::Position>(player, 100.0f, 200.0f);
registry.emplaceComponent<component::Health>(player, 100, 100);
registry.emplaceComponent<component::Shield>(player, 100.0f, 20.0f);
```

### Checking for Components

```cpp
// Check if entity has shield
if (registry.hasComponent<component::Shield>(entity)) {
    auto& shield = registry.getComponent<component::Shield>(entity);
    if (shield.isActive) {
        // Shield is protecting entity
    }
}
```

### Removing Components

```cpp
// Remove shield when power-up expires
registry.removeComponent<component::Shield>(entity);
```

---

## 🎯 Step 5: Integrate with Other Systems

### Example: Damage System Integration

Modify your damage system to respect shields:

```cpp
void DamageSystem::update(ECS::Registry& registry, float dt) {
    auto view = registry.view<component::Health>();
    
    view.each([&registry](auto entity, component::Health& health) {
        // Check for incoming damage (from collision system, etc.)
        if (hasPendingDamage(entity)) {
            int damage = getPendingDamage(entity);
            
            // Check if entity has active shield
            if (registry.hasComponent<component::Shield>(entity)) {
                auto& shield = registry.getComponent<component::Shield>(entity);
                
                if (shield.isActive && shield.currentCapacity > 0.0f) {
                    // Shield absorbs damage
                    shield.currentCapacity -= static_cast<float>(damage);
                    shield.timeSinceHit = 0.0f;  // Reset recharge delay
                    
                    // Only damage health if shield was depleted
                    if (shield.currentCapacity < 0.0f) {
                        health.current += static_cast<int>(shield.currentCapacity);
                        shield.currentCapacity = 0.0f;
                    }
                } else {
                    // No shield, apply damage directly
                    health.current -= damage;
                }
            } else {
                // No shield component, apply damage directly
                health.current -= damage;
            }
            
            clearPendingDamage(entity);
        }
    });
}
```

---

## 🎯 Step 6: Add to CMakeLists

### For Header-Only Components

No CMakeLists changes needed if your component is header-only.

### For Components with Implementation

If your component has a .cpp file:

Edit `lib/common/CMakeLists.txt`:

```cmake
target_sources(rtype_common PRIVATE
    # ... existing files
    src/components/ShieldComponent.cpp
)
```

---

## 🎯 Step 7: Write Tests

Create `tests/common/test_shield_component.cpp`:

```cpp
#include <gtest/gtest.h>
#include "rtype/ecs/Registry.hpp"
#include "rtype/common/components/ShieldComponent.hpp"

TEST(ShieldComponentTest, DefaultConstruction) {
    rtype::component::Shield shield;
    
    EXPECT_FLOAT_EQ(shield.maxCapacity, 100.0f);
    EXPECT_FLOAT_EQ(shield.currentCapacity, 100.0f);
    EXPECT_TRUE(shield.isActive);
}

TEST(ShieldComponentTest, CustomCapacity) {
    rtype::component::Shield shield(200.0f, 50.0f);
    
    EXPECT_FLOAT_EQ(shield.maxCapacity, 200.0f);
    EXPECT_FLOAT_EQ(shield.rechargeRate, 50.0f);
}

TEST(ShieldComponentTest, AddToEntity) {
    ECS::Registry registry;
    registry.registerComponent<rtype::component::Shield>();
    
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<rtype::component::Shield>(entity, 100.0f, 20.0f);
    
    EXPECT_TRUE(registry.hasComponent<rtype::component::Shield>(entity));
    
    auto& shield = registry.getComponent<rtype::component::Shield>(entity);
    EXPECT_FLOAT_EQ(shield.maxCapacity, 100.0f);
}
```

Add to `tests/CMakeLists.txt`:

```cmake
add_executable(test_shield_component
    common/test_shield_component.cpp
)
target_link_libraries(test_shield_component
    rtype_common
    rtype_ecs
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME ShieldComponent COMMAND test_shield_component)
```

---

## 🎯 Step 8: Document Your Component

Add Doxygen comments:

```cpp
/**
 * @brief Shield component providing temporary invincibility
 * 
 * The Shield component grants entities temporary protection from damage.
 * It features:
 * - Automatic recharge after taking damage
 * - Configurable capacity and recharge rate
 * - Deactivation when depleted
 * 
 * ## Usage Example
 * 
 * @code
 * // Create entity with shield
 * auto entity = registry.spawnEntity();
 * registry.emplaceComponent<Shield>(entity, 100.0f, 20.0f);
 * 
 * // Check shield status
 * if (shield.isActive && shield.currentCapacity > 0.0f) {
 *     // Entity is protected
 * }
 * @endcode
 * 
 * ## Related Systems
 * - ShieldSystem: Handles recharge logic
 * - DamageSystem: Consumes shield capacity
 * - RenderSystem: May render shield visual effect
 * 
 * @see ShieldSystem
 * @see component::Health
 */
struct Shield {
    // ...
};
```

---

## 📊 Component Architecture Patterns

### Tag Components (Marker)

Empty components used for filtering:

```cpp
// Mark entities as enemies
struct EnemyTag {};

// Mark entities as players
struct PlayerTag {};

// Usage: View only enemies
auto enemyView = registry.view<EnemyTag, Position>();
```

### State Components

Components that track state machines:

```cpp
enum class MovementState {
    Idle,
    Walking,
    Running,
    Jumping
};

struct MovementStateComponent {
    MovementState currentState = MovementState::Idle;
    MovementState previousState = MovementState::Idle;
    float stateTimer = 0.0f;
};
```

### Relationship Components

Components that reference other entities:

```cpp
struct Parent {
    ECS::Entity entity;
};

struct Children {
    std::vector<ECS::Entity> entities;
};
```

---

## 🚀 Advanced Tips

### Component Pooling

The ECS automatically pools components for performance. No manual management needed.

### Component Dependencies

If a component requires other components, document it:

```cpp
/**
 * @brief Weapon component (requires Position)
 * 
 * @warning This component requires Position component to function correctly
 */
struct Weapon {
    // Weapon fires from entity's position
};
```

Enforce dependencies in system:

```cpp
void WeaponSystem::update(ECS::Registry& registry, float dt) {
    // View requires both Weapon AND Position
    auto view = registry.view<Weapon, Position>();
    
    view.each([](auto entity, Weapon& weapon, Position& pos) {
        // System only processes entities with both components
    });
}
```

### Component Serialization

For save/load functionality:

```cpp
#include <nlohmann/json.hpp>

struct Shield {
    // ... members
    
    nlohmann::json toJson() const {
        return {
            {"maxCapacity", maxCapacity},
            {"currentCapacity", currentCapacity},
            {"rechargeRate", rechargeRate}
        };
    }
    
    static Shield fromJson(const nlohmann::json& j) {
        Shield shield;
        shield.maxCapacity = j["maxCapacity"];
        shield.currentCapacity = j["currentCapacity"];
        shield.rechargeRate = j["rechargeRate"];
        return shield;
    }
};
```

---

## 🎓 Best Practices

1. **Keep components simple**: One responsibility per component
2. **No logic in components**: Logic belongs in Systems
3. **Use plain data**: Avoid pointers, use indices/IDs instead
4. **Document dependencies**: Clearly state required components
5. **Write tests**: Test component behavior in isolation
6. **Consider performance**: Small components are cached better
7. **Namespace correctly**: Use `rtype::component::`
8. **Follow naming**: Clear, descriptive names ending in "Component" or short names for common types

---

## 📚 Related Tutorials

- [Add a New System](./add-system.md)
- [Add an Enemy](./add-enemy.md)
- [ECS Architecture Guide](../Architecture/ecs-guide.md)

**Happy coding! 🚀**
