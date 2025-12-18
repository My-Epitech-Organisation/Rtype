---
sidebar_position: 13
sidebar_label: Asset Guide
---

# 🎨 Asset Creation Guide

Guide for creating and managing game assets in R-Type.

## 📋 Asset Types

R-Type uses the following asset types:
- **Sprites**: PNG images for entities, backgrounds, UI
- **Audio**: OGG Vorbis for music and sound effects
- **Fonts**: TTF/OTF fonts for text rendering
- **Shaders**: GLSL fragment shaders for effects
- **Configuration**: TOML files for game data

---

## 🖼️ Sprite Assets

### Requirements

**Format**: PNG with transparency (RGBA)
**Color Depth**: 32-bit (8-bit per channel + alpha)
**Resolution**: Power of 2 recommended for best performance

### Sprite Sizes

| Asset Type | Recommended Size | Notes |
|------------|------------------|-------|
| Player Ship | 64x64 | Centered in sprite |
| Small Enemy | 32x32 | Simple shapes |
| Medium Enemy | 48x48 or 64x64 | More detail |
| Large Enemy | 96x96 or 128x128 | Boss elements |
| Boss | 256x256+ | Can be larger |
| Projectile | 8x8 to 16x16 | Small, clean |
| Power-Up | 32x32 | Recognizable icon |
| Background | 1920x1080+ | Tileable |
| UI Elements | Varies | Match UI scale |

### Creating Sprites

#### Using Aseprite

```bash
# Create new sprite
aseprite -b --sheet enemy_scout.png --data enemy_scout.json

# Create animation
aseprite -b \
  --sheet-type rows \
  --frame-range 0,7 \
  --sheet explosion.png \
  --data explosion.json \
  explosion.aseprite
```

#### Using GIMP

1. **Create New Image**
   - File → New
   - Set size (e.g., 64x64)
   - Fill: Transparency

2. **Design Sprite**
   - Use layers for organization
   - Keep centered for rotation
   - Use consistent style

3. **Export**
   - File → Export As
   - Format: PNG
   - Enable: Save Alpha Channel
   - Compression: 9 (maximum)

#### Pixel Art Guidelines

```
✅ Good Practices:
- Consistent pixel size
- Limited color palette (16-32 colors)
- Clear silhouette
- Readable at game resolution
- Anti-aliasing for smooth edges (optional)

❌ Avoid:
- Mixed pixel sizes
- Too many colors
- Blurry/muddy details
- Asymmetric designs (for centered objects)
```

### Sprite Sheets

For animated sprites:

```
sprite_sheet.png:
┌─────┬─────┬─────┬─────┐
│ F0  │ F1  │ F2  │ F3  │  Animation frames
└─────┴─────┴─────┴─────┘

Config (enemies.toml):
sprite = "enemy_animated.png"
frame_count = 4
frame_width = 64
frame_height = 64
frame_duration = 0.1  # seconds per frame
```

### Transparency

```cpp
// Ensure proper alpha blending
// Sprites should have:
// - Fully transparent (alpha = 0) for background
// - Fully opaque (alpha = 255) for solid pixels
// - Semi-transparent (alpha = 128) for effects
```

---

## 🎵 Audio Assets

### Music

**Format**: OGG Vorbis
**Sample Rate**: 44.1 kHz
**Bitrate**: 128-192 kbps
**Channels**: Stereo
**Length**: 2-4 minutes (loopable)

#### Creating Loop-able Music

Using Audacity:

1. **Import/Create Track**
2. **Design Loop Point**
   - Fade in at start (0.5s)
   - Fade out at end (0.5s)
   - Ensure waveforms match at loop point
3. **Export as OGG**
   - File → Export → Export as OGG
   - Quality: 5-7
   - Add metadata:
     ```
     TITLE: Level 1 Theme
     ARTIST: Composer Name
     LOOP_START: 0
     LOOP_END: 120.0
     ```

#### Music Categories

```toml
# Main menu (calm, atmospheric)
menu_music.ogg

# Level themes (energetic, driving)
level1_music.ogg
level2_music.ogg
level3_music.ogg

# Boss battles (intense, epic)
boss_music.ogg

# Victory/defeat (short, conclusive)
victory.ogg
defeat.ogg
```

### Sound Effects

**Format**: OGG Vorbis
**Sample Rate**: 44.1 kHz
**Bitrate**: 96-128 kbps
**Length**: < 2 seconds (typically)

#### Sound Effect List

```
Player:
- player_shoot.ogg (0.1s) - Laser shot
- player_hit.ogg (0.2s) - Taking damage
- player_death.ogg (0.5s) - Explosion
- player_powerup.ogg (0.3s) - Collecting power-up

Enemies:
- enemy_shoot.ogg (0.1s) - Enemy fire
- enemy_hit.ogg (0.1s) - Hitting enemy
- enemy_death_small.ogg (0.3s) - Small enemy dies
- enemy_death_large.ogg (0.5s) - Large enemy dies

Environment:
- explosion_small.ogg (0.5s)
- explosion_large.ogg (1.0s)
- warning.ogg (0.5s) - Boss warning
- level_complete.ogg (2.0s)

UI:
- ui_click.ogg (0.05s)
- ui_hover.ogg (0.05s)
- ui_select.ogg (0.1s)
- ui_error.ogg (0.2s)
```

#### Creating Sound Effects

**Tools:**
- **SFXR/BFXR**: Generate retro game sounds
- **Audacity**: Edit and process sounds
- **Freesound.org**: Find free sound effects (check licenses)

**Processing:**

```
1. Record/generate sound
2. Normalize to -3 dB
3. Remove silence from start/end
4. Apply fade in/out (10ms)
5. Export as OGG (quality: 5)
```

---

## 🔤 Font Assets

### Requirements

**Format**: TTF or OTF
**License**: Ensure commercial use allowed
**Style**: Readable at small sizes

### Recommended Fonts

```
UI Text:
- Roboto (clean, modern)
- Open Sans (highly readable)
- Source Sans Pro (versatile)

Retro/Pixel Games:
- Press Start 2P
- VT323
- Pixellari

Score/Numbers:
- Orbitron (futuristic)
- Audiowide (tech-style)
```

### Font Usage

```toml
# assets/config.toml
[fonts]
ui = "fonts/Roboto-Regular.ttf"
ui_bold = "fonts/Roboto-Bold.ttf"
score = "fonts/Orbitron-Bold.ttf"
dialogue = "fonts/OpenSans-Regular.ttf"
```

---

## 🌈 Shader Assets

### Fragment Shaders

**Format**: GLSL (OpenGL Shading Language)
**Location**: `assets/shaders/`

### Example: Colorblind Mode

`assets/shaders/colorblind.frag`:

```glsl
#version 330 core

uniform sampler2D texture;
uniform int mode; // 0=normal, 1=protanopia, 2=deuteranopia, 3=tritanopia

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    vec4 color = texture2D(texture, TexCoords);
    
    if (mode == 1) { // Protanopia (red-blind)
        float r = 0.56667 * color.r + 0.43333 * color.g;
        float g = 0.55833 * color.r + 0.44167 * color.g;
        float b = color.b;
        FragColor = vec4(r, g, b, color.a);
    }
    else if (mode == 2) { // Deuteranopia (green-blind)
        float r = 0.625 * color.r + 0.375 * color.g;
        float g = 0.70 * color.r + 0.30 * color.g;
        float b = color.b;
        FragColor = vec4(r, g, b, color.a);
    }
    else if (mode == 3) { // Tritanopia (blue-blind)
        float r = color.r;
        float g = color.g;
        float b = 0.95 * color.r + 0.05 * color.g;
        FragColor = vec4(r, g, b, color.a);
    }
    else {
        FragColor = color;
    }
}
```

### Example: Vignette Effect

`assets/shaders/vignette.frag`:

```glsl
#version 330 core

uniform sampler2D texture;
uniform float radius;    // 0.0 - 1.0
uniform float softness;  // 0.0 - 1.0

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    vec4 color = texture2D(texture, TexCoords);
    
    // Calculate distance from center
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(TexCoords, center);
    
    // Calculate vignette
    float vignette = smoothstep(radius, radius - softness, dist);
    
    // Apply vignette
    FragColor = vec4(color.rgb * vignette, color.a);
}
```

---

## 📝 Configuration Assets

### Enemy Definition

`config/game/enemies.toml`:

```toml
[[enemy]]
id = "scout"
name = "Scout Fighter"
health = 20
speed = 150.0
damage = 10
score = 100
sprite = "enemy_scout.png"        # Reference to sprite
behavior = "straight"
fire_rate = 2.0
projectile = "enemy_bullet"       # Reference to projectile
drop_chance = 0.1                 # 10% power-up drop

# Visual effects
death_effect = "explosion_small"
death_sound = "enemy_death_small.ogg"
spawn_sound = "enemy_spawn.ogg"

# Animation (if sprite sheet)
animated = true
frame_count = 4
frame_duration = 0.15
```

### Level Definition

`config/game/levels/level1.toml`:

```toml
[level]
id = 1
name = "Asteroid Field"
background = "bg_space_1.png"
music = "level1_music.ogg"
duration = 180

# Parallax scrolling
[[level.parallax]]
sprite = "stars_far.png"
speed = 0.1
[[level.parallax]]
sprite = "asteroids_near.png"
speed = 0.8

# Enemy waves
[[wave]]
time = 10
enemy = "scout"
count = 8
formation = "v"
spawn_interval = 0.5
spawn_x = 1920
spawn_y = 360

# Boss
[[boss]]
enemy = "boss_serpent"
spawn_time = 180
music = "boss_music.ogg"
warning_time = 10
```

---

## 📂 Asset Organization

```
assets/
├── img/
│   ├── entities/
│   │   ├── players/
│   │   │   ├── player_ship.png
│   │   │   └── player_ship_boost.png
│   │   ├── enemies/
│   │   │   ├── enemy_scout.png
│   │   │   ├── enemy_interceptor.png
│   │   │   └── boss_serpent.png
│   │   ├── projectiles/
│   │   │   ├── bullet_player.png
│   │   │   ├── laser_beam.png
│   │   │   └── missile.png
│   │   └── powerups/
│   │       ├── powerup_red.png
│   │       ├── powerup_blue.png
│   │       └── powerup_shield.png
│   ├── backgrounds/
│   │   ├── bg_space_1.png
│   │   ├── bg_nebula.png
│   │   └── parallax/
│   ├── effects/
│   │   ├── explosion_small.png
│   │   ├── explosion_large.png
│   │   └── engine_trail.png
│   └── ui/
│       ├── button_normal.png
│       ├── button_hover.png
│       └── health_bar.png
├── audio/
│   ├── music/
│   │   ├── menu_music.ogg
│   │   ├── level1_music.ogg
│   │   └── boss_music.ogg
│   └── sfx/
│       ├── player/
│       ├── enemies/
│       └── ui/
├── fonts/
│   ├── Roboto-Regular.ttf
│   ├── Roboto-Bold.ttf
│   └── Orbitron-Bold.ttf
└── shaders/
    ├── colorblind.frag
    ├── vignette.frag
    └── scanlines.frag
```

---

## 🎨 Style Guidelines

### Color Palette

```
Primary Colors:
- Player: Blue (#00BFFF)
- Enemies: Red (#FF4444)
- Power-ups: Yellow (#FFD700), Green (#00FF00)

Secondary Colors:
- UI: Dark Blue (#1A1A2E), Light Gray (#EAEAEA)
- Effects: White (#FFFFFF), Orange (#FFA500)

Background:
- Space: Dark Blue/Black (#0A0A1A)
- Nebula: Purple/Pink (#8844AA, #FF44AA)
```

### Visual Consistency

```
✅ Consistent:
- Same art style across all assets
- Matching color palette
- Uniform outline thickness (pixel art)
- Similar level of detail

❌ Inconsistent:
- Mixing realistic and cartoonish styles
- Clashing color schemes
- Different pixel densities
- Varying quality levels
```

---

## 🔧 Asset Tools

### Recommended Software

**2D Graphics:**
- [Aseprite](https://www.aseprite.org/) - Pixel art animation
- [GIMP](https://www.gimp.org/) - Free image editor
- [Inkscape](https://inkscape.org/) - Vector graphics
- [Krita](https://krita.org/) - Digital painting

**Audio:**
- [Audacity](https://www.audacityteam.org/) - Audio editing
- [LMMS](https://lmms.io/) - Music production
- [BFXR](https://www.bfxr.net/) - Sound effect generator
- [Bosca Ceoil](https://boscaceoil.net/) - Simple music maker

**Fonts:**
- [FontForge](https://fontforge.org/) - Font editor
- [Google Fonts](https://fonts.google.com/) - Free fonts
- [DaFont](https://www.dafont.com/) - Font repository

---

## 📏 Performance Considerations

### Optimization Tips

1. **Sprite Atlases**: Combine multiple sprites into one texture
2. **Compression**: Use PNG compression (OptiPNG, PNGCrush)
3. **Resolution**: Don't exceed necessary size
4. **Audio**: Use appropriate bitrate (128kbps for music, 96kbps for SFX)
5. **Lazy Loading**: Load assets on-demand when possible

### Asset Budget

```
Target Size:
- Total assets: < 500 MB
- Single sprite: < 1 MB
- Music track: < 5 MB
- Sound effect: < 100 KB
- Font: < 1 MB
```

---

## 🧪 Testing Assets

### Asset Checklist

- [ ] Correct format and resolution
- [ ] Proper transparency (for sprites)
- [ ] No artifacts or compression errors
- [ ] Loops correctly (for music)
- [ ] Readable/clear in-game
- [ ] Consistent with art style
- [ ] Proper naming convention
- [ ] Correct file location

### In-Game Testing

```bash
# Run game and check:
1. Asset loads without errors
2. Appears correctly on screen
3. Animates smoothly (if applicable)
4. Sounds play without popping/clipping
5. Music loops seamlessly
6. No performance impact
```

---

## 📚 Asset Attribution

If using third-party assets:

```markdown
## Credits

### Graphics
- Player ship sprite by [Artist Name] (CC BY 4.0)
- Enemy sprites from [Source] (Public Domain)

### Audio
- Menu music by [Composer] (CC BY-SA 3.0)
- Sound effects from [Freesound.org] (CC0)

### Fonts
- Roboto by Google Fonts (Apache License 2.0)
- Orbitron by Matt McInerney (SIL Open Font License)
```

---

## 📖 Related Documentation

- [Configuration Reference](./configuration.md)
- [Add an Enemy Tutorial](./tutorials/add-enemy.md)
- [Shader Development](./shader-development.md)

**Happy creating! 🎨**
