# R-Type Game Design Document

This document describes the game mechanics, entity roles, and player experience design for R-Type.

## Overview

R-Type is a **horizontal scrolling shoot 'em up** (shmup) featuring cooperative multiplayer gameplay. Players control space fighters defending humanity against an alien invasion known as the Bydo Empire.

---

## Core Mechanics

### Movement System

Players can move freely within the visible screen area:

| Direction | Description |
|-----------|-------------|
| Up/Down | Vertical movement within screen bounds |
| Left/Right | Horizontal movement (limited to left portion of screen) |

**Movement Properties:**

- **Speed:** Configurable, default 200 units/second
- **Bounds:** Player cannot leave the visible screen
- **Collision:** Instant death on contact with enemies or obstacles

### Shooting System

#### Primary Weapon

- **Type:** Energy projectiles
- **Fire Rate:** Configurable (default: 5 shots/second)
- **Damage:** Base damage = 10
- **Behavior:** Travels horizontally to the right

#### Charge Shot (Power Beam)

- **Mechanic:** Hold fire button to charge
- **Charge Time:** 0.5s (level 1) → 1.5s (level 3, max)
- **Damage Scaling:**
  - Level 1: 30 damage
  - Level 2: 60 damage
  - Level 3: 100 damage (piercing)
- **Visual Feedback:** Ship glows progressively brighter

### Force Pod System

The iconic **Force** is a detachable module:

| State | Behavior |
|-------|----------|
| Attached (Front) | Blocks enemy projectiles, adds forward firepower |
| Attached (Rear) | Protects from behind, adds rear firepower |
| Detached | Floats independently, can be called back |

**Force Properties:**

- Invulnerable to all attacks
- Damages enemies on contact
- Enhances weapon systems when attached

### Power-Up System

Power-ups appear from destroyed enemies (configurable drop rate):

| Power-Up | Effect | Visual |
|----------|--------|--------|
| Speed Up | +50 movement speed | Blue orb |
| Laser | Straight piercing beam | Red orb |
| Reflect | Bouncing projectiles | Yellow orb |
| Homing | Seeking missiles | Green orb |
| Shield | Absorbs 3 hits | White orb |

---

## Entity Roles

### Player Entity

The player ship is the main protagonist:

```text
Components:
├── TransformComponent (position, rotation)
├── VelocityComponent (movement speed)
├── SpriteComponent (visual representation)
├── HealthComponent (lives remaining)
├── WeaponComponent (current weapon type, charge level)
├── InputComponent (player controls binding)
└── CollisionComponent (hitbox for damage detection)
```

**Player Properties:**

- **Lives:** 3 (configurable)
- **Respawn:** 2 second invulnerability after death
- **Hitbox:** Smaller than visual sprite for fair gameplay

### Enemy Types

#### Grunt (Basic Enemy)

- **Role:** Cannon fodder, introduces attack patterns
- **Health:** 10
- **Behavior:** Linear movement, single shot
- **Points:** 100

#### Chaser

- **Role:** Pressure player movement
- **Health:** 20
- **Behavior:** Follows player position with delay
- **Points:** 200

#### Bomber

- **Role:** Area denial
- **Health:** 30
- **Behavior:** Drops explosive projectiles in arc
- **Points:** 300

#### Turret (Stationary)

- **Role:** Zone control
- **Health:** 50
- **Behavior:** Rotates to track player, fires bursts
- **Points:** 250

#### Elite

- **Role:** Mini-boss, tests player skill
- **Health:** 100
- **Behavior:** Complex patterns, multiple attack types
- **Points:** 1000

### Boss Entities

Each stage ends with a boss encounter:

**Boss Structure:**

- Multiple destructible parts (weak points)
- Phase-based attack patterns
- Environmental hazards integration
- Dramatic visual and audio feedback

**Example - Stage 1 Boss (Dobkeratops):**

- **Phase 1:** Tail sweep, targeted shots
- **Phase 2:** Spawns minions, faster attacks
- **Phase 3:** Desperate mode, continuous fire

### Projectiles

#### Player Projectiles

| Type | Speed | Damage | Behavior |
|------|-------|--------|----------|
| Basic | 400 | 10 | Straight line |
| Laser | 600 | 25 | Piercing |
| Missile | 300 | 40 | Homing |
| Wave | 350 | 15 | Sine wave |

#### Enemy Projectiles

| Type | Speed | Behavior |
|------|-------|----------|
| Bullet | 200 | Straight line |
| Aimed | 250 | Targets player position |
| Spread | 180 | 3-5 projectiles in fan |
| Bomb | 150 | Arcing, explodes on impact |

### Obstacles

- **Walls:** Instant death on collision
- **Destructible:** Can be shot (blocks path)
- **Moving:** Platforms, crushers (timing-based)

---

## Multiplayer Design

### Cooperative Gameplay (2-4 Players)

**Shared Elements:**

- Screen scrolls for all players
- Power-ups visible to all, first-come-first-served
- Score tracked individually

**Coordination Mechanics:**

- Players can overlap without collision
- Revive mechanic: touch fallen ally to restore (costs 1 life)
- Combo multiplier increases with coordinated kills

### Network Considerations

**Authority Model:**

- Server authoritative for enemy spawns, hit detection
- Client predictive for player movement
- Interpolation for smooth remote player display

**Latency Compensation:**

- Local player movement is immediate
- Enemy positions interpolated over 100ms
- Hit registration uses server timestamp

---

## Player Experience

### Difficulty Curve

```text
Intensity
    │
    │         ████ Boss
    │        ██
    │   ████████
    │  ██
    │ ██  ████
    │██  ██
    │█  ██
    └─────────────────── Time
    Start    Mid    Boss
```

**Pacing Principles:**

1. **Escalation:** Gradually introduce new enemy types
2. **Rest Periods:** Brief calm after intense sections
3. **Climax:** Boss fights as peak intensity
4. **Rhythm:** Alternate between dense and sparse waves

### Feedback Systems

#### Visual Feedback

| Event | Feedback |
|-------|----------|
| Hit Enemy | Flash white, particle burst |
| Destroy Enemy | Explosion animation, score popup |
| Player Damage | Screen shake, red flash |
| Charge Shot Ready | Ship glow, audio cue |
| Power-Up Collect | Particle trail, UI update |

#### Audio Feedback

- **Shooting:** Satisfying "pew" with pitch variation
- **Hit Confirm:** Distinct impact sound
- **Explosions:** Layered, proportional to enemy size
- **Music:** Dynamic, intensifies during boss fights
- **Low Health:** Heartbeat/alarm audio cue

### Accessibility Considerations

As documented in our [Accessibility Features](../Comparative-Studies/Accessibility/accessibility-features.md):

1. **Slow Mode:** Reduces game speed to 50% for players who need more reaction time
2. **Custom Controls:** Full key rebinding support
3. **Colorblind Support:** Distinct shapes + colors, configurable palette

### Readability

**Visual Hierarchy:**

1. **Player:** Brightest, most saturated
2. **Enemies:** Distinct silhouettes per type
3. **Projectiles:**
   - Player: Cool colors (blue/green)
   - Enemy: Warm colors (red/orange/pink)
4. **Background:** Muted, parallax for depth

**Hitbox Clarity:**

- Visual sprites larger than actual hitbox
- Optional hitbox display mode for practice
- Clear collision feedback

---

## Progression System

### Stage Structure

Each stage follows this pattern:

```text
[Intro] → [Wave 1] → [Mid-Boss] → [Wave 2] → [Boss] → [Results]
   5s       60s         30s         60s        90s       10s
```

### Scoring

| Action | Points |
|--------|--------|
| Destroy enemy | Enemy value × combo |
| Perfect boss | 10,000 bonus |
| No deaths stage | 5,000 bonus |
| Speed bonus | Time remaining × 100 |

### Combo System

- Each kill within 2 seconds increases combo
- Combo multiplier: 1× → 2× → 4× → 8× (max)
- Taking damage resets combo

---

## Configuration

All gameplay values are configurable in `config/server/gameplay.toml`:

```toml
[player]
lives = 3
base_speed = 200
fire_rate = 5.0
respawn_invuln = 2.0

[enemies]
spawn_rate_multiplier = 1.0
health_multiplier = 1.0
speed_multiplier = 1.0

[difficulty]
base = "normal"  # easy, normal, hard, insane
dynamic_scaling = true
```

---

## Future Considerations

- **Roguelike Mode:** Procedural stages, permanent upgrades
- **Custom Ships:** Different stats and special abilities
- **Level Editor:** Community-created stages
- **Leaderboards:** Global and friend rankings
