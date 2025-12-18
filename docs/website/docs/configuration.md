---
sidebar_position: 5
sidebar_label: Configuration Reference
---

# ⚙️ Configuration Reference

Complete reference for all R-Type configuration files.

## 📁 Configuration Structure

```
config/
├── client/          # Client-side settings
│   ├── client.toml  # Main client configuration
│   ├── video.toml   # Video/graphics settings
│   └── controls.json # Input mappings
├── server/          # Server-side settings
│   ├── config.toml  # Server configuration
│   ├── server.toml  # Network settings
│   └── gameplay.toml # Game rules
└── game/            # Game content definitions
    ├── enemies.toml
    ├── players.toml
    ├── powerups.toml
    ├── projectiles.toml
    └── levels/
        ├── level1.toml
        ├── level2.toml
        └── level3.toml
```

---

## 🖥️ Client Configuration

### `config/client/client.toml`

Main client configuration file.

#### [video]

```toml
[video]
width = 1920              # Window width (pixels)
height = 1080             # Window height (pixels)
fullscreen = false        # Fullscreen mode (true/false)
vsync = true              # Vertical sync (true/false)
maxFps = 60               # Maximum FPS (0 = unlimited)
uiScale = 1.0             # UI scaling factor (0.5-2.0)
```

**Notes:**
- `width` x `height`: Use your monitor's native resolution for best quality
- `fullscreen`: Borderless windowed not supported yet
- `vsync`: Disable for maximum FPS, enable to prevent tearing
- `maxFps`: Set to your monitor's refresh rate (60, 144, 240) or 0 for unlimited
- `uiScale`: Useful for high-DPI displays (1.5 or 2.0 for 4K)

---

#### [audio]

```toml
[audio]
masterVolume = 1.0        # Master volume (0.0-1.0)
musicVolume = 0.8         # Background music volume (0.0-1.0)
sfxVolume = 1.0           # Sound effects volume (0.0-1.0)
muted = false             # Mute all audio (true/false)
```

**Notes:**
- Volume ranges: 0.0 (silent) to 1.0 (maximum)
- `muted`: Quick mute without losing volume settings

---

#### [network]

```toml
[network]
serverAddress = "127.0.0.1"   # Server IP address
serverPort = 4000             # Server port (1024-65535)
clientPort = 0                # Client port (0 = auto)
connectionTimeout = 5000      # Timeout in milliseconds
maxRetries = 3                # Connection retry attempts
tickrate = 60                 # Network update rate (Hz)
```

**Notes:**
- `serverAddress`: 
  - `127.0.0.1` for local server
  - LAN IP (e.g., `192.168.1.100`) for local network
  - Public IP for internet play
- `serverPort`: Must match server configuration
- `clientPort`: Leave at 0 for automatic assignment
- `connectionTimeout`: Increase for slow connections (5000-10000 ms)
- `maxRetries`: Number of attempts before giving up
- `tickrate`: Should match server tickrate

---

#### [gameplay]

```toml
[gameplay]
difficulty = "normal"         # Difficulty: easy, normal, hard, nightmare
startingLives = 3             # Initial lives (1-99)
waves = 10                    # Number of enemy waves (1-50)
playerSpeed = 260.0           # Movement speed (100.0-500.0)
enemySpeedMultiplier = 1.0    # Enemy speed modifier (0.5-2.0)
friendlyFire = false          # Enable team damage (true/false)
```

**Difficulty Presets:**
- `easy`: Slower enemies, more lives, extra power-ups
- `normal`: Balanced gameplay
- `hard`: Faster enemies, fewer power-ups
- `nightmare`: One-hit kills, maximum challenge

**Notes:**
- `playerSpeed`: Higher = faster movement
- `enemySpeedMultiplier`: Adjust enemy difficulty without changing preset
- `friendlyFire`: Enables damage to teammates (multiplayer only)

---

#### [input]

```toml
[input]
moveUp = "Up"                 # Move up key
moveDown = "Down"             # Move down key
moveLeft = "Left"             # Move left key
moveRight = "Right"           # Move right key
fire = "Space"                # Fire weapon
pause = "Escape"              # Pause game
mouseSensitivity = 1.0        # Mouse sensitivity (0.1-5.0)
```

**Valid Key Names:**
- Letters: `A`-`Z`
- Numbers: `Num0`-`Num9`
- Function: `F1`-`F12`
- Arrows: `Up`, `Down`, `Left`, `Right`
- Special: `Space`, `Enter`, `Escape`, `Tab`, `Backspace`
- Modifiers: `LShift`, `RShift`, `LControl`, `RControl`, `LAlt`, `RAlt`
- Mouse: `Mouse1` (left), `Mouse2` (right), `Mouse3` (middle)

---

#### [paths]

```toml
[paths]
assetsPath = "assets"         # Assets directory
savesPath = "saves"           # Save files directory
logsPath = "logs"             # Log files directory
configPath = "config"         # Config directory
```

**Notes:**
- Paths are relative to executable location
- Use absolute paths if needed: `C:\Games\RType\assets`
- Create directories if they don't exist

---

### `config/client/video.toml`

Simplified video configuration (legacy).

```toml
resolution = "1280x720"       # WIDTHxHEIGHT format
fullscreen = false            # Fullscreen mode
vsync = true                  # Vertical sync
```

**Note:** This file is deprecated. Use `client.toml` [video] section instead.

---

### `config/client/controls.json`

Input mappings in JSON format (legacy).

```json
{
  "move_up": "Up",
  "move_down": "Down",
  "move_left": "Left",
  "move_right": "Right",
  "fire": "Space"
}
```

**Note:** This file is deprecated. Use `client.toml` [input] section instead.

---

## 🖧 Server Configuration

### `config/server/server.toml`

Main server configuration.

```toml
port = 4000                   # Server listening port (1024-65535)
max_players = 8               # Maximum players (1-16)
tickrate = 60                 # Server update rate (Hz)
timeout = 30                  # Client timeout (seconds)
password = ""                 # Server password (empty = public)
name = "R-Type Server"        # Server name
motd = "Welcome!"             # Message of the day
public = true                 # List in server browser
```

**Notes:**
- `port`: Must be open in firewall and forwarded in router
- `max_players`: Higher values increase CPU/bandwidth usage
- `tickrate`: Common values: 30, 60, 128
  - Higher = more responsive but more CPU/bandwidth
  - Lower = less resources but less smooth
- `timeout`: Kick inactive clients after X seconds
- `password`: Leave empty for public servers
- `public`: Register with master server for discovery

---

### `config/server/gameplay.toml`

Server-side gameplay rules.

```toml
difficulty = "normal"         # Server difficulty
waves = 10                    # Enemy waves per level
friendly_fire = false         # Enable team damage
respawn_time = 5              # Respawn delay (seconds)
power_up_spawn_rate = 0.3     # Power-up drop chance (0.0-1.0)
enemy_scaling = true          # Scale difficulty with players
max_enemies_on_screen = 50    # Enemy limit
level_time_limit = 600        # Time limit per level (seconds, 0=none)
```

**Enemy Scaling:**
When `enemy_scaling = true`:
- 1 player: 100% enemy count
- 2 players: 150% enemy count
- 3 players: 200% enemy count
- 4 players: 250% enemy count

**Notes:**
- `power_up_spawn_rate`: 0.0 (never) to 1.0 (always)
- `max_enemies_on_screen`: Performance limiter
- `level_time_limit`: Fail level if time expires (0 = no limit)

---

### `config/server/config.toml`

Advanced server settings.

```toml
[logging]
level = "INFO"                # Log level: DEBUG, INFO, WARNING, ERROR
file = "logs/server.log"      # Log file path
console = true                # Print to console
max_size_mb = 100             # Max log file size (MB)

[performance]
thread_count = 4              # Worker threads (0 = auto)
max_bandwidth = 0             # Max bandwidth (KB/s, 0 = unlimited)
compression = true            # Enable packet compression

[security]
rate_limit = 100              # Max packets per second per client
ban_duration = 3600           # Ban duration (seconds)
whitelist_only = false        # Only allow whitelisted IPs
admin_ips = []                # Admin IP addresses

[database]
enabled = false               # Enable stats database
path = "data/server.db"       # Database file path
backup_interval = 3600        # Backup interval (seconds)
```

---

## 🎮 Game Configuration

### `config/game/players.toml`

Player ship definitions.

```toml
[[player]]
id = "standard"
name = "Standard Fighter"
health = 100
speed = 260.0
fire_rate = 0.2               # Seconds between shots
damage = 10
sprite = "player_ship.png"
hitbox_width = 32
hitbox_height = 32

[[player]]
id = "heavy"
name = "Heavy Cruiser"
health = 150
speed = 200.0
fire_rate = 0.3
damage = 15
sprite = "player_heavy.png"
hitbox_width = 48
hitbox_height = 48
```

**Notes:**
- `fire_rate`: Lower = faster firing
- `hitbox_*`: Collision detection size
- Multiple player types can be defined

---

### `config/game/enemies.toml`

Enemy definitions.

```toml
[[enemy]]
id = "scout"
name = "Scout Ship"
health = 20
speed = 150.0
damage = 10
score = 100
sprite = "enemy_scout.png"
behavior = "straight"         # straight, zigzag, circle, kamikaze
fire_rate = 2.0               # Seconds between shots (0 = never)
projectile = "enemy_bullet"
drop_chance = 0.1             # Power-up drop rate

[[enemy]]
id = "heavy"
name = "Heavy Fighter"
health = 50
speed = 100.0
damage = 20
score = 250
sprite = "enemy_heavy.png"
behavior = "zigzag"
fire_rate = 1.0
projectile = "enemy_missile"
drop_chance = 0.3
```

**Behavior Types:**
- `straight`: Flies left in straight line
- `zigzag`: Vertical zig-zag pattern
- `circle`: Circular motion
- `kamikaze`: Rushes directly at player
- `stationary`: Stays in place (turret)
- `boss`: Complex scripted behavior

---

### `config/game/projectiles.toml`

Projectile definitions.

```toml
[[projectile]]
id = "player_bullet"
sprite = "bullet_player.png"
speed = 500.0
damage = 10
pierce = false                # Pass through enemies
homing = false                # Track nearest enemy
lifetime = 3.0                # Seconds before despawn
width = 8
height = 8

[[projectile]]
id = "laser_beam"
sprite = "laser.png"
speed = 0                     # 0 = instant
damage = 5                    # Per frame
pierce = true
homing = false
lifetime = 0.5
width = 1920                  # Full screen width
height = 16
```

**Notes:**
- `pierce`: Bullet continues through enemies
- `homing`: Auto-targets nearest enemy
- `lifetime`: Bullets despawn after X seconds
- `speed = 0`: Instant hit (laser)

---

### `config/game/powerups.toml`

Power-up definitions.

```toml
[[powerup]]
id = "force_pod"
name = "Force Pod"
sprite = "powerup_red.png"
effect = "attach_weapon"
duration = 0                  # 0 = permanent
value = "force_pod"           # Weapon ID
rarity = 0.2                  # Spawn weight (higher = rarer)

[[powerup]]
id = "shield"
name = "Shield"
sprite = "powerup_shield.png"
effect = "temporary_invincibility"
duration = 5.0                # Seconds
value = 0
rarity = 0.5

[[powerup]]
id = "extra_life"
name = "Extra Life"
sprite = "powerup_1up.png"
effect = "add_life"
duration = 0
value = 1                     # Number of lives
rarity = 0.9                  # Very rare
```

**Effect Types:**
- `attach_weapon`: Add weapon to ship
- `temporary_invincibility`: Shield
- `speed_boost`: Increase speed
- `fire_rate_boost`: Faster firing
- `add_life`: Extra life
- `mega_bomb`: Clear screen
- `score_multiplier`: Bonus points

---

### `config/game/levels/level1.toml`

Level definitions.

```toml
[level]
id = 1
name = "Asteroid Field"
background = "bg_space.png"
music = "level1_music.ogg"
duration = 180                # Seconds (0 = until boss defeated)

[[wave]]
time = 0                      # Spawn at X seconds
enemy = "scout"
count = 10
formation = "v"               # v, line, box, random
spawn_interval = 0.5          # Seconds between spawns
spawn_x = 1920                # Spawn X position
spawn_y = 360                 # Spawn Y position (or "random")

[[wave]]
time = 30
enemy = "heavy"
count = 5
formation = "line"
spawn_interval = 1.0
spawn_x = 1920
spawn_y = "random"

[[boss]]
enemy = "serpent_boss"
spawn_time = 150              # Spawn at X seconds
music = "boss_music.ogg"      # Optional boss music
```

**Formation Types:**
- `v`: V-shaped formation
- `line`: Horizontal line
- `box`: Grid formation
- `random`: Random positions
- `circle`: Circular pattern

---

## 🔄 Configuration Reload

### Hot Reload (Server)

Some server configurations can be reloaded without restart:

```bash
# Send SIGHUP signal (Linux)
kill -HUP <server_pid>

# Or use admin command (if connected as admin)
/reload config
```

**Hot-reloadable settings:**
- Gameplay settings (gameplay.toml)
- Logging settings
- Rate limits

**Requires restart:**
- Port changes
- Max players
- Tickrate

---

## 🛡️ Security Best Practices

### Public Servers

```toml
# Recommended settings for public servers
[security]
rate_limit = 100              # Prevent spam
ban_duration = 3600           # 1-hour bans
whitelist_only = false
admin_ips = ["203.0.113.42"]  # Your IP only

[server]
password = "your_password"    # Or leave empty for public
max_players = 8               # Don't exceed your bandwidth
```

### Private Servers

```toml
[security]
whitelist_only = true
admin_ips = ["192.168.1.100"]

[server]
public = false                # Don't list in browser
password = "complex_password"
```

---

## 📊 Performance Tuning

### Low-End Hardware (Client)

```toml
[video]
width = 1280
height = 720
fullscreen = false
vsync = false
maxFps = 60
uiScale = 0.8

[network]
tickrate = 30                 # Lower tickrate
```

### High-End Hardware (Client)

```toml
[video]
width = 3840
height = 2160
fullscreen = true
vsync = false
maxFps = 0                    # Unlimited
uiScale = 1.5

[network]
tickrate = 128                # High tickrate
```

### High-Capacity Server

```toml
[server]
max_players = 16
tickrate = 128

[performance]
thread_count = 8
max_bandwidth = 0             # Unlimited
compression = true
```

### Low-Bandwidth Server

```toml
[server]
max_players = 4
tickrate = 30

[performance]
compression = true
max_bandwidth = 512           # 512 KB/s per client
```

---

## 🔧 Troubleshooting

### Configuration Not Loading

1. **Check syntax**: Use a TOML validator (https://www.toml-lint.com/)
2. **Check file path**: Must be relative to executable
3. **Check permissions**: File must be readable
4. **Check logs**: Look for parsing errors

### Changes Not Applied

1. **Restart game/server**
2. **Verify file was saved**
3. **Check for syntax errors**
4. **Ensure correct file is being edited**

### Performance Issues

See [Troubleshooting Guide](./troubleshooting.md) for detailed performance tuning.

---

## 📚 Next Steps

- [Gameplay Guide](./gameplay-guide.md) - Learn how to play
- [Server Administration](./server-admin.md) - Run a server
- [Troubleshooting](./troubleshooting.md) - Fix common issues
- [Asset Guide](./asset-guide.md) - Customize game content

**Happy configuring! ⚙️**
