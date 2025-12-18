---
sidebar_label: Add an Enemy
sidebar_position: 3
---

# How to Add a New Enemy Type

This tutorial shows how to create and configure a new enemy type in R-Type.

## 📋 Overview

Adding a new enemy involves:
1. Defining enemy data in `config/game/enemies.toml`
2. Creating sprite assets
3. (Optional) Creating custom behavior system
4. Testing in-game

---

## 🎯 Step 1: Define Enemy Configuration

Edit `config/game/enemies.toml`:

```toml
[[enemy]]
id = "drone"                  # Unique identifier
name = "Attack Drone"         # Display name
health = 30                   # Hit points
speed = 180.0                 # Movement speed (pixels/second)
damage = 15                   # Contact/collision damage
score = 150                   # Points awarded on kill
sprite = "enemy_drone.png"    # Sprite filename (in assets/img/)
behavior = "zigzag"           # Movement pattern (see below)
fire_rate = 1.5               # Seconds between shots (0 = never fires)
projectile = "enemy_laser"    # Projectile type to fire
drop_chance = 0.2             # Power-up drop probability (0.0-1.0)
width = 32                    # Sprite width
height = 32                   # Sprite height
```

### Available Behavior Types

#### `straight`
Flies straight left across the screen.
```toml
behavior = "straight"
```

#### `zigzag`
Vertical zig-zag pattern while moving left.
```toml
behavior = "zigzag"
behavior_params = { amplitude = 100.0, frequency = 2.0 }
```

#### `sine_wave`
Smooth sine wave movement.
```toml
behavior = "sine_wave"
behavior_params = { amplitude = 150.0, frequency = 1.5 }
```

#### `circle`
Circular motion around spawn point.
```toml
behavior = "circle"
behavior_params = { radius = 80.0, angular_speed = 3.14 }
```

#### `kamikaze`
Rushes directly at nearest player.
```toml
behavior = "kamikaze"
behavior_params = { acceleration = 200.0 }
```

#### `stationary`
Stays in place (turret).
```toml
behavior = "stationary"
```

#### `follow_player`
Tracks and chases the nearest player.
```toml
behavior = "follow_player"
behavior_params = { max_speed = 150.0, turn_rate = 2.0 }
```

---

## 🎯 Step 2: Create Sprite Asset

### Sprite Requirements

- **Format**: PNG with transparency
- **Size**: Power of 2 recommended (32x32, 64x64, etc.)
- **Location**: `assets/img/`
- **Naming**: Match the `sprite` field in config

### Example Sprite Specifications

```
Small enemies:  32x32 pixels
Medium enemies: 48x48 pixels
Large enemies:  64x64 pixels
Bosses:         128x128 or larger
```

### Create Your Sprite

1. **Use image editor** (GIMP, Photoshop, Aseprite)
2. **Design enemy ship** with transparent background
3. **Export as PNG**: `enemy_drone.png`
4. **Place in** `assets/img/`

### Example Using Aseprite

```bash
# Create new sprite
aseprite -b --sheet enemy_drone.png --data enemy_drone.json

# Export with transparency
aseprite -b enemy_drone.aseprite --save-as enemy_drone.png
```

---

## 🎯 Step 3: Define Projectile (If Enemy Shoots)

If `fire_rate > 0`, define the projectile in `config/game/projectiles.toml`:

```toml
[[projectile]]
id = "enemy_laser"
sprite = "enemy_laser.png"
speed = 400.0
damage = 15
pierce = false                # Pass through targets
homing = false                # Track player
lifetime = 3.0                # Seconds before despawn
width = 16
height = 4
color = [255, 0, 0, 255]      # Red laser (RGBA)
```

### Projectile Types

**Basic Bullet:**
```toml
id = "enemy_bullet"
speed = 300.0
pierce = false
homing = false
```

**Homing Missile:**
```toml
id = "enemy_missile"
speed = 250.0
pierce = false
homing = true
homing_strength = 2.0         # Turn rate
```

**Piercing Laser:**
```toml
id = "enemy_beam"
speed = 500.0
pierce = true
max_pierce_count = 3          # Max enemies to hit
```

---

## 🎯 Step 4: Add to Level Waves

Edit level files in `config/game/levels/`:

```toml
# config/game/levels/level1.toml

[[wave]]
time = 45                     # Spawn at 45 seconds
enemy = "drone"               # Our new enemy
count = 8                     # Number to spawn
formation = "v"               # V-formation
spawn_interval = 0.8          # Seconds between spawns
spawn_x = 1920                # Right edge of screen
spawn_y = "random"            # Random Y position (or specific value)

[[wave]]
time = 60
enemy = "drone"
count = 5
formation = "line"
spawn_interval = 1.0
spawn_x = 1920
spawn_y = 360                 # Center of screen
```

### Formation Options

**V-Formation:**
```toml
formation = "v"
formation_params = { angle = 45.0, spacing = 50.0 }
```

**Line Formation:**
```toml
formation = "line"
formation_params = { spacing = 60.0, orientation = "horizontal" }
```

**Box/Grid:**
```toml
formation = "box"
formation_params = { rows = 3, cols = 4, spacing_x = 50.0, spacing_y = 50.0 }
```

**Random:**
```toml
formation = "random"
formation_params = { min_x = 1500, max_x = 1920, min_y = 100, max_y = 980 }
```

---

## 🎯 Step 5: (Optional) Create Custom Behavior

For complex behaviors not covered by presets, create a custom system.

### Create Behavior System

`src/games/rtype/server/Systems/DroneBehaviorSystem.hpp`:

```cpp
#ifndef RTYPE_SERVER_SYSTEMS_DRONEBEHAVIORSYSTEM_HPP_
#define RTYPE_SERVER_SYSTEMS_DRONEBEHAVIORSYSTEM_HPP_

#include "rtype/engine/ASystem.hpp"

namespace rtype::server {

class DroneBehaviorSystem : public engine::ASystem {
public:
    DroneBehaviorSystem() : ASystem("DroneBehaviorSystem") {}
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::server

#endif
```

`src/games/rtype/server/Systems/DroneBehaviorSystem.cpp`:

```cpp
#include "DroneBehaviorSystem.hpp"
#include "rtype/common/components/Position.hpp"
#include "rtype/common/components/Velocity.hpp"
#include "rtype/common/components/EnemyTag.hpp"

namespace rtype::server {

void DroneBehaviorSystem::update(ECS::Registry& registry, float dt) {
    auto view = registry.view<component::EnemyTag, 
                                component::Position, 
                                component::Velocity>();
    
    view.each([dt](auto entity, 
                   component::EnemyTag& tag,
                   component::Position& pos,
                   component::Velocity& vel) {
        // Only affect "drone" type enemies
        if (tag.type != "drone") return;
        
        // Custom behavior: spiral pattern
        float time = tag.aliveTime;
        vel.dx = -150.0f;  // Move left
        vel.dy = std::sin(time * 2.0f) * 100.0f;  // Sine wave
        
        // Add rotation (if you have rotation component)
        // rotation.angle += dt * 3.14159f;
    });
}

}  // namespace rtype::server
```

### Register System

In `GameEngine.cpp`:

```cpp
#include "Systems/DroneBehaviorSystem.hpp"

void GameEngine::initialize() {
    // ... other systems
    systems_.push_back(std::make_unique<DroneBehaviorSystem>());
}
```

---

## 🎯 Step 6: Balance Testing

### Test Checklist

- [ ] Enemy spawns correctly
- [ ] Movement pattern works as expected
- [ ] Projectiles fire at correct rate
- [ ] Collision detection works
- [ ] Health/damage values are balanced
- [ ] Score value is appropriate
- [ ] Power-up drop rate feels right
- [ ] Visual appearance is clear

### Balancing Guidelines

**Early Game (Level 1-2):**
```toml
health = 20-50
speed = 100-200
damage = 10-15
score = 100-250
```

**Mid Game (Level 3-4):**
```toml
health = 50-100
speed = 150-250
damage = 15-25
score = 250-500
```

**Late Game (Level 5+):**
```toml
health = 100-200
speed = 200-300
damage = 25-40
score = 500-1000
```

### Quick Test Mode

Add test wave at start of level for rapid iteration:

```toml
# Temporary test wave
[[wave]]
time = 5                      # Spawn early
enemy = "drone"
count = 3
formation = "line"
spawn_interval = 1.0
spawn_x = 1000                # Closer to player
spawn_y = 360
```

---

## 🎯 Step 7: Visual Enhancements

### Add Explosion Effect

Define in `config/game/effects.toml`:

```toml
[[effect]]
id = "drone_explosion"
sprite_sheet = "explosions.png"
frame_count = 8
frame_duration = 0.05         # Seconds per frame
width = 64
height = 64
sound = "explosion_small.ogg"
```

Link to enemy:

```toml
[[enemy]]
id = "drone"
# ... other properties
death_effect = "drone_explosion"
death_sound = "explosion_small.ogg"
```

### Add Trail Effect

```toml
[[enemy]]
id = "drone"
# ... other properties
trail_effect = "engine_trail"
trail_color = [100, 200, 255, 200]  # Blue trail
```

### Add Audio

Place audio files in `assets/audio/`:
- `enemy_drone_spawn.ogg`
- `enemy_drone_fire.ogg`
- `enemy_drone_death.ogg`

Reference in config:

```toml
[[enemy]]
id = "drone"
# ... other properties
spawn_sound = "enemy_drone_spawn.ogg"
fire_sound = "enemy_drone_fire.ogg"
death_sound = "enemy_drone_death.ogg"
```

---

## 🎯 Step 8: Create Boss Enemy

Bosses are special enemies with complex behavior.

```toml
[[enemy]]
id = "boss_guardian"
name = "Guardian Mech"
health = 5000                 # Much higher HP
speed = 50.0                  # Slower movement
damage = 50                   # High contact damage
score = 10000                 # Huge score
sprite = "boss_guardian.png"
behavior = "boss"             # Special boss behavior
fire_rate = 0.5               # Rapid fire
projectile = "boss_laser"
drop_chance = 1.0             # Always drop power-up
width = 256                   # Large sprite
height = 256
is_boss = true                # Mark as boss
phases = 3                    # Multi-phase boss

# Boss-specific properties
[[enemy.boss_phases]]
phase = 1
health_threshold = 3500       # HP when phase starts
behavior = "stationary"
fire_pattern = "spread"       # Spread shot
fire_rate = 0.8

[[enemy.boss_phases]]
phase = 2
health_threshold = 2000
behavior = "circle"
fire_pattern = "spiral"       # Spiral bullet pattern
fire_rate = 0.5

[[enemy.boss_phases]]
phase = 3
health_threshold = 0
behavior = "kamikaze"
fire_pattern = "random"       # Chaotic firing
fire_rate = 0.3

# Weak points (optional)
[[enemy.weak_points]]
name = "core"
offset_x = 0                  # Relative to enemy position
offset_y = 0
width = 64
height = 64
damage_multiplier = 2.0       # 2x damage to weak point
```

### Boss in Level

```toml
[[boss]]
enemy = "boss_guardian"
spawn_time = 180              # After 3 minutes
music = "boss_theme.ogg"      # Special music
warning_time = 10             # Warning 10s before spawn
warning_message = "WARNING: BOSS APPROACHING"
```

---

## 🎯 Step 9: Advanced Features

### State Machine Behavior

For enemies that change behavior based on conditions:

```cpp
struct DroneAIComponent {
    enum class State {
        Patrol,
        Attack,
        Retreat
    };
    
    State currentState = State::Patrol;
    float stateTimer = 0.0f;
    float aggroRange = 300.0f;
    float retreatHealthPercent = 0.3f;
};
```

### Flocking Behavior

For groups of enemies that move together:

```cpp
void FlockingSystem::update(ECS::Registry& registry, float dt) {
    auto view = registry.view<component::FlockingTag, 
                                component::Position, 
                                component::Velocity>();
    
    view.each([&](auto entity, auto& flock, auto& pos, auto& vel) {
        // Separation: avoid crowding neighbors
        // Alignment: steer towards average heading
        // Cohesion: steer towards average position
        calculateFlockingForces(registry, entity, pos, vel);
    });
}
```

### Dynamic Difficulty

Adjust enemy stats based on player performance:

```toml
[[enemy]]
id = "adaptive_drone"
# ... base properties
difficulty_scaling = true
health_per_level = 5          # +5 HP per level
speed_per_level = 10.0        # +10 speed per level
```

---

## 📊 Enemy Design Patterns

### Cannon Fodder
- Low HP, high count
- Simple movement
- Rarely fire
- Low score

```toml
health = 10
speed = 150
fire_rate = 5.0  # Rarely
score = 50
```

### Elite Units
- Medium HP
- Complex movement
- Frequent fire
- Medium score

```toml
health = 60
speed = 200
fire_rate = 1.5
score = 300
```

### Mini-Bosses
- High HP
- Slow, strategic movement
- Dangerous attacks
- High score

```toml
health = 500
speed = 80
fire_rate = 0.5
score = 2000
```

---

## 🧪 Testing

Create test file `tests/games/test_enemy_drone.cpp`:

```cpp
#include <gtest/gtest.h>
#include "rtype/ecs/Registry.hpp"
#include "config/EnemyLoader.hpp"

TEST(EnemyTest, DroneLoadsCorrectly) {
    ECS::Registry registry;
    auto enemyDef = loadEnemyDefinition("drone");
    
    EXPECT_EQ(enemyDef.id, "drone");
    EXPECT_EQ(enemyDef.health, 30);
    EXPECT_FLOAT_EQ(enemyDef.speed, 180.0f);
}

TEST(EnemyTest, DroneSpawnsCorrectly) {
    ECS::Registry registry;
    auto enemy = spawnEnemy(registry, "drone", 100.0f, 200.0f);
    
    EXPECT_TRUE(registry.hasComponent<component::Position>(enemy));
    EXPECT_TRUE(registry.hasComponent<component::Health>(enemy));
    EXPECT_TRUE(registry.hasComponent<component::EnemyTag>(enemy));
}
```

---

## 📚 Related Resources

- [Add a Component](./add-component.md)
- [Add a System](./add-system.md)
- [Create a Level](./create-level.md)
- [Configuration Reference](../configuration.md)
- [Asset Guide](../asset-guide.md)

**Happy enemy designing! 👾**
