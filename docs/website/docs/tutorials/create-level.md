---
sidebar_label: Create a Level
sidebar_position: 4
---

# How to Create a Custom Level

This tutorial shows how to design and implement a new level in R-Type.

## 📋 Level Structure

A level consists of:
- **Metadata**: Name, duration, background, music
- **Waves**: Timed enemy spawn events
- **Boss**: Final boss encounter
- **Scripted Events**: Story moments, cutscenes

---

## 🎯 Step 1: Create Level File

Create `config/game/levels/level4.toml`:

```toml
[level]
id = 4                        # Unique level number
name = "Nebula Gauntlet"      # Display name
description = "Navigate through a dangerous nebula field"
background = "bg_nebula.png"  # Background image
parallax_layers = [           # Optional parallax scrolling
    { sprite = "nebula_far.png", speed = 0.2 },
    { sprite = "nebula_mid.png", speed = 0.5 },
    { sprite = "nebula_near.png", speed = 0.8 }
]
music = "level4_music.ogg"    # Background music
duration = 240                # Level duration (seconds, 0 = until boss defeated)
difficulty_multiplier = 1.5   # Difficulty scaling
```

---

## 🎯 Step 2: Design Wave Sequence

### Basic Wave Structure

```toml
[[wave]]
time = 10                     # Spawn at 10 seconds into level
enemy = "scout"               # Enemy type (from enemies.toml)
count = 12                    # Number of enemies
formation = "v"               # Formation pattern
spawn_interval = 0.5          # Seconds between individual spawns
spawn_x = 1920                # X position (right edge of screen)
spawn_y = 360                 # Y position (or "random")
```

### Full Wave Sequence Example

```toml
# Early wave - introduce level
[[wave]]
time = 5
enemy = "scout"
count = 8
formation = "line"
spawn_interval = 0.6
spawn_x = 1920
spawn_y = "random"

# Mid wave - increase difficulty
[[wave]]
time = 30
enemy = "interceptor"
count = 10
formation = "v"
spawn_interval = 0.8
spawn_x = 1920
spawn_y = 360

# Mixed wave - variety
[[wave]]
time = 45
enemy = "scout"
count = 6
formation = "box"
spawn_interval = 0.5
spawn_x = 1920
spawn_y = 200

[[wave]]
time = 48  # Overlaps with previous wave
enemy = "heavy"
count = 3
formation = "line"
spawn_interval = 1.5
spawn_x = 1920
spawn_y = 600

# Challenge wave - many enemies
[[wave]]
time = 60
enemy = "interceptor"
count = 15
formation = "random"
spawn_interval = 0.4
spawn_x = "random"  # Can also randomize X
spawn_y = "random"

# Elite wave - tough enemies
[[wave]]
time = 90
enemy = "elite_cruiser"
count = 5
formation = "wedge"
spawn_interval = 2.0
spawn_x = 1920
spawn_y = 360
```

---

## 🎯 Step 3: Formation Patterns

### Available Formations

#### V-Formation
```toml
formation = "v"
formation_params = { 
    angle = 45.0,             # Angle of V (degrees)
    spacing = 50.0,           # Distance between units
    direction = "right"       # or "left", "up", "down"
}
```

#### Line Formation
```toml
formation = "line"
formation_params = { 
    spacing = 60.0,
    orientation = "horizontal"  # or "vertical"
}
```

#### Box/Grid Formation
```toml
formation = "box"
formation_params = { 
    rows = 3,
    cols = 4,
    spacing_x = 50.0,
    spacing_y = 50.0
}
```

#### Circle Formation
```toml
formation = "circle"
formation_params = { 
    radius = 150.0
}
```

#### Wedge Formation
```toml
formation = "wedge"
formation_params = { 
    angle = 60.0,
    spacing = 40.0
}
```

#### Random Formation
```toml
formation = "random"
formation_params = { 
    min_x = 1500,
    max_x = 1920,
    min_y = 100,
    max_y = 980
}
```

---

## 🎯 Step 4: Add Boss Fight

```toml
[[boss]]
enemy = "nebula_guardian"     # Boss enemy ID
spawn_time = 180              # Spawn at 3 minutes
music = "boss_theme_2.ogg"    # Boss battle music
warning_time = 10             # Warning shown 10s before
warning_message = "DANGER: BOSS INCOMING!"
camera_shake_on_spawn = true
screen_flash_on_spawn = true
```

### Multi-Phase Boss

```toml
[[boss]]
enemy = "mothership"
spawn_time = 200
music = "final_boss.ogg"
warning_time = 15
warning_message = "FINAL BOSS: MOTHERSHIP"

# Boss phases trigger at HP thresholds
[[boss.phases]]
phase = 1
health_threshold = 10000      # Full health
behavior = "stationary"
fire_pattern = "spread"
minion_spawn_rate = 5.0       # Spawn minions every 5s
minion_type = "drone"
message = "Phase 1: Destroy the shield generators!"

[[boss.phases]]
phase = 2
health_threshold = 6000       # 60% HP
behavior = "circle"
fire_pattern = "spiral"
minion_spawn_rate = 3.0
minion_type = "interceptor"
message = "Phase 2: The shields are down!"
camera_shake_intensity = 2.0

[[boss.phases]]
phase = 3
health_threshold = 2000       # 20% HP
behavior = "kamikaze"
fire_pattern = "chaos"
minion_spawn_rate = 1.0
minion_type = "elite"
message = "Phase 3: Final assault!"
screen_tint = [255, 0, 0, 50] # Red tint (RGBA)
```

---

## 🎯 Step 5: Scripted Events

### Cutscene/Dialogue

```toml
[[event]]
type = "dialogue"
time = 0                      # Start of level
character = "commander"
text = "Entering nebula sector. Stay alert!"
duration = 3.0                # Seconds
voice_clip = "commander_alert.ogg"

[[event]]
type = "dialogue"
time = 120
character = "pilot"
text = "Enemy mothership detected ahead!"
duration = 3.0
```

### Environmental Hazards

```toml
[[event]]
type = "hazard"
time = 45
hazard = "asteroid_field"
duration = 20.0               # 20 seconds of asteroids
spawn_rate = 2.0              # Asteroids per second
damage = 25
```

### Power-Up Drops

```toml
[[event]]
type = "powerup_spawn"
time = 60
powerup = "force_pod"
position = { x = 1500, y = 360 }
```

### Checkpoint

```toml
[[event]]
type = "checkpoint"
time = 120
message = "Checkpoint reached"
save_progress = true
```

---

## 🎯 Step 6: Background & Atmosphere

### Static Background

```toml
[level]
background = "bg_nebula.png"
background_scroll_speed = 100.0  # Pixels per second
```

### Parallax Scrolling

```toml
[level]
parallax_layers = [
    { 
        sprite = "stars_far.png", 
        speed = 0.1,              # 10% of camera speed
        repeat = true 
    },
    { 
        sprite = "nebula_mid.png", 
        speed = 0.5,              # 50% of camera speed
        repeat = true 
    },
    { 
        sprite = "debris_near.png", 
        speed = 1.2,              # 120% - moves faster than camera
        repeat = true 
    }
]
```

### Lighting & Effects

```toml
[level.atmosphere]
ambient_color = [50, 100, 200, 255]  # Blue tint (RGBA)
fog_enabled = true
fog_color = [100, 150, 200, 100]
fog_density = 0.5
particle_effects = [
    { type = "stars", density = 100 },
    { type = "nebula_gas", density = 50, color = [255, 0, 255, 128] }
]
```

---

## 🎯 Step 7: Difficulty Curve

### Pacing Guidelines

**Introduction (0-30s):**
- Easy enemies
- Low spawn rate
- Teach mechanics

**Build-Up (30-90s):**
- Increase variety
- Mix enemy types
- Introduce patterns

**Climax (90-150s):**
- High difficulty
- Dense waves
- Pressure player

**Boss Intro (150-180s):**
- Calm before storm
- Final power-up
- Boss warning

**Boss Fight (180s+):**
- Epic encounter
- Multiple phases
- Victory!

### Example Pacing

```toml
# Introduction - Easy
[[wave]]
time = 5
enemy = "scout"
count = 6
spawn_interval = 1.0

# Build-up - Increasing
[[wave]]
time = 30
enemy = "scout"
count = 10
spawn_interval = 0.8

[[wave]]
time = 45
enemy = "interceptor"
count = 8
spawn_interval = 0.7

# Climax - Intense
[[wave]]
time = 90
enemy = "scout"
count = 20
spawn_interval = 0.3

[[wave]]
time = 95
enemy = "heavy"
count = 5
spawn_interval = 1.0

# Calm - Prepare for boss
[[wave]]
time = 150
enemy = "scout"
count = 3
spawn_interval = 2.0

# Boss
[[boss]]
spawn_time = 180
enemy = "boss"
```

---

## 🎯 Step 8: Testing & Balancing

### Playtesting Checklist

- [ ] Level starts correctly
- [ ] Waves spawn at right times
- [ ] Formation patterns work
- [ ] Boss appears and functions
- [ ] Difficulty feels appropriate
- [ ] Music transitions smoothly
- [ ] Background looks good
- [ ] Events trigger correctly
- [ ] Level is completable
- [ ] Performance is acceptable

### Balancing Tips

1. **Count enemy HP pools:**
   - Total damage needed to complete level
   - Ensure player has enough time/ammo

2. **Check spawn density:**
   - Don't overwhelm player
   - Allow breathing room between waves

3. **Test with different player counts:**
   - Solo should be possible
   - 4-player shouldn't be trivial

4. **Verify power-up availability:**
   - Enough power-ups to stay equipped
   - Not so many it's too easy

### Debug Mode

Add test configuration:

```toml
[level.debug]
enabled = true
player_invincible = false
skip_to_time = 0              # Jump to specific time
spawn_all_powerups = false
show_hitboxes = false
```

---

## 🎯 Step 9: Add to Level Progression

Edit `config/game/levels.toml`:

```toml
[campaign]
levels = [
    "level1.toml",
    "level2.toml",
    "level3.toml",
    "level4.toml"             # Your new level
]

[[level_unlock]]
level = 4
requires_completion = 3       # Must beat level 3
min_score = 50000             # Optional score requirement
```

---

## 🎯 Step 10: Create Assets

### Background Image

- **Resolution**: 1920x1080 or larger
- **Format**: PNG
- **Location**: `assets/img/`
- **Seamless**: Tileable for infinite scrolling

### Music

- **Format**: OGG Vorbis
- **Length**: 2-3 minutes (loopable)
- **Location**: `assets/audio/`
- **Loop point**: Set loop markers in audio editor

### Example: Creating Looping Music

Using Audacity:

```
1. Create/import 2-minute track
2. Ensure smooth loop:
   - Fade in at start
   - Fade out at end
   - Match waveforms at loop point
3. Export as OGG:
   - File → Export → Export as OGG
   - Quality: 5-7
   - Add loop metadata
```

---

## 📊 Level Design Patterns

### Wave-Based Level

Focus on timed enemy waves with boss at end.

```toml
duration = 180
# Multiple waves leading to boss
[[boss]]
spawn_time = 180
```

### Gauntlet Level

Continuous enemy stream, no distinct waves.

```toml
duration = 300
# Many small waves with short intervals
[[wave]]
time = 0
count = 3
[[wave]]
time = 5
count = 3
# ... continues
```

### Boss Rush

Multiple bosses, minimal trash mobs.

```toml
[[boss]]
spawn_time = 30
enemy = "mini_boss_1"

[[boss]]
spawn_time = 90
enemy = "mini_boss_2"

[[boss]]
spawn_time = 150
enemy = "final_boss"
```

### Survival Level

Time-based, increasing difficulty.

```toml
duration = 0  # Infinite
difficulty_increase_rate = 0.1  # 10% per minute
```

---

## 🎓 Advanced Techniques

### Dynamic Difficulty

Adjust level based on player performance:

```toml
[level.adaptive]
enabled = true
track_player_deaths = true
adjust_enemy_health = true
health_adjustment_range = [0.7, 1.3]  # 70%-130%
adjust_spawn_rate = true
spawn_rate_range = [0.8, 1.2]
```

### Branching Paths

Let players choose route through level:

```toml
[[event]]
type = "choice"
time = 60
message = "Choose your path:"
options = [
    { text = "Left path (harder, better rewards)", trigger_waves = ["hard_left_1", "hard_left_2"] },
    { text = "Right path (easier)", trigger_waves = ["easy_right_1", "easy_right_2"] }
]
```

### Destructible Environment

Objects players can destroy:

```toml
[[obstacle]]
type = "destructible"
sprite = "space_station.png"
position = { x = 1200, y = 400 }
health = 200
score = 500
drop_powerup = "mega_bomb"
```

---

## 📚 Example: Complete Level

See `config/game/levels/level2.toml` for a complete, balanced level example.

---

## 📖 Related Documentation

- [Add an Enemy](./add-enemy.md)
- [Add a Power-Up](./add-powerup.md)
- [Configuration Reference](../configuration.md)
- [Asset Guide](../asset-guide.md)

**Happy level designing! 🎮**
