---
sidebar_position: 2
sidebar_label: Gameplay Guide
---

# 🎮 Gameplay Guide

Welcome to R-Type! This guide will help you master the game and survive the alien invasion.

## 🚀 Getting Started

### Launching the Game

1. **Start the Server** (for multiplayer):
   ```bash
   ./r-type_server
   ```

2. **Launch the Client**:
   ```bash
   ./r-type_client
   ```

3. **Connect to Server**: Enter the server IP address in the main menu

### Main Menu Options

- **Solo Play**: Practice mode against AI
- **Multiplayer**: Join or host a game with up to 4 players
- **Settings**: Configure video, audio, and controls
- **Quit**: Exit the game

---

## 🎯 Game Objectives

### Primary Goal
Survive waves of alien enemies and defeat the boss at the end of each level.

### Scoring System
- **Small Enemy**: 100 points
- **Medium Enemy**: 250 points
- **Large Enemy**: 500 points
- **Boss**: 5000 points
- **Power-up Collection**: 50 points
- **Perfect Wave**: 2x multiplier

### Lives & Continues
- Start with **3 lives**
- Extra life every **10,000 points**
- Game Over when all lives are lost
- Multiplayer: Team shares a life pool

---

## 🕹️ Controls

### Default Keyboard Controls

| Action | Key |
|--------|-----|
| Move Up | ↑ / W |
| Move Down | ↓ / S |
| Move Left | ← / A |
| Move Right | → / D |
| Fire | Space |
| Special Weapon | Left Shift |
| Pause | Escape |

### Controller Support
- **Left Stick**: Movement
- **Button A / Cross**: Fire
- **Button B / Circle**: Special Weapon
- **Start**: Pause

> 💡 **Tip**: Controls can be customized in `config/client/controls.json`

---

## 💪 Power-Ups

### Weapon Power-Ups

#### 🔴 Red Power-Up - Force Pod
- Attaches a powerful Force unit to your ship
- Can be positioned above or below your ship
- Acts as a shield and fires simultaneously with your main weapon
- **Strategy**: Position it in the direction of incoming threats

#### 🔵 Blue Power-Up - Laser Beam
- Fires a continuous laser beam
- Penetrates multiple enemies
- High damage output
- **Best for**: Boss fights and dense enemy formations

#### 🟡 Yellow Power-Up - Wave Beam
- Fires a wave-like spread shot
- Covers vertical area
- Lower damage but excellent coverage
- **Best for**: Clearing small enemies

#### 🟢 Green Power-Up - Homing Missiles
- Fires auto-targeting missiles
- Pursues nearest enemy
- Moderate damage
- **Best for**: Evasive enemies

### Utility Power-Ups

- **🛡️ Shield**: Temporary invincibility (5 seconds)
- **⚡ Speed Up**: Increases movement speed
- **💥 Mega Bomb**: Clears all enemies on screen
- **❤️ Extra Life**: Adds one life

---

## 👾 Enemy Types

### Wave 1: Scouts
- **Appearance**: Small red ships
- **Pattern**: Flies in simple formations
- **Weakness**: Low HP, predictable movement
- **Threat**: ⭐

### Wave 2: Interceptors
- **Appearance**: Medium blue ships
- **Pattern**: Zig-zag movement, occasional shots
- **Weakness**: Moderate HP
- **Threat**: ⭐⭐

### Wave 3: Heavy Cruisers
- **Appearance**: Large green ships
- **Pattern**: Slow movement, heavy firepower
- **Weakness**: Slow, vulnerable to sustained fire
- **Threat**: ⭐⭐⭐

### Wave 4: Kamikaze Units
- **Appearance**: Small purple ships
- **Pattern**: Rushes directly at player
- **Weakness**: Very low HP
- **Threat**: ⭐⭐⭐⭐ (collision damage)

### Bosses

#### 🐲 Level 1 Boss: "Serpent"
- **Weak Point**: Head (red core)
- **Pattern**: Undulating movement, fires spread shots
- **Strategy**: Stay mobile, focus fire on the head
- **HP**: 5000

#### 🦂 Level 2 Boss: "Scorpion"
- **Weak Points**: Multiple segments
- **Pattern**: Tail whip attacks, laser beam
- **Strategy**: Destroy segments from tail to head
- **HP**: 8000

#### 👁️ Level 3 Boss: "Eye of Chaos"
- **Weak Point**: Central eye
- **Pattern**: Rotating laser beams, spawns minions
- **Strategy**: Clear minions first, then attack eye during cooldown
- **HP**: 12000

---

## 🎓 Advanced Strategies

### Movement Techniques

#### Micro-dodging
- Make small, precise movements to avoid bullets
- Keep your ship in the lower-left quadrant for maximum reaction time

#### Safe Zones
- Each level has areas with fewer enemy spawns
- Learn these zones to recover between waves

#### Streaming
- Keep enemies on screen longer to control spawn rate
- Useful for farming power-ups

### Weapon Management

#### Force Pod Positioning
- **Top**: Protects from overhead attacks
- **Bottom**: Clears ground-level threats
- **Front**: Maximum offensive power

#### Weapon Switching
- Don't be afraid to sacrifice a power-up for a better one
- Laser is best for bosses, Wave for crowds

### Multiplayer Tactics

#### Formation Flying
- **Diamond**: One player point, others cover flanks
- **Line**: All players horizontal, maximum firepower
- **Box**: Defensive formation for tough sections

#### Power-Up Sharing
- Communicate who needs which power-up
- Stronger players should tank with shields

#### Boss Coordination
- Assign weak points to different players
- One player draws fire while others attack

---

## 🏆 Achievements & Challenges

### Unlockables
- **Ace Pilot**: Complete game without losing a life
- **Pacifist**: Complete Level 1 without firing (dodge only)
- **Speedrunner**: Complete game under 30 minutes
- **Collector**: Collect all power-ups in a single run
- **Team Player**: Win a 4-player game with all players surviving

### Daily Challenges
- Check the server for rotating challenges
- Earn bonus points and special badges

---

## 🆘 Quick Tips

1. **Stay in motion**: A moving target is harder to hit
2. **Learn enemy patterns**: Predictability is your advantage
3. **Don't hoard special weapons**: Use them when overwhelmed
4. **Watch the edges**: Enemies often spawn from off-screen
5. **Practice makes perfect**: Each death teaches a lesson
6. **Use headphones**: Audio cues warn of incoming attacks
7. **Take breaks**: Fatigue reduces reaction time
8. **Watch replays**: Learn from your mistakes

---

## 📊 Difficulty Modes

### Easy
- Reduced enemy health
- More power-ups
- Slower bullet speed
- **Recommended for**: New players

### Normal
- Balanced gameplay
- Standard spawn rates
- **Recommended for**: Regular players

### Hard
- Increased enemy health
- Faster bullets
- Fewer power-ups
- **Recommended for**: Veterans

### Nightmare
- Maximum difficulty
- One-hit kills
- Limited continues
- **Recommended for**: Masochists

---

## 🎬 Next Steps

- Read the [Troubleshooting Guide](./troubleshooting.md) if you encounter issues
- Check out [Configuration Reference](./configuration.md) to customize your experience
- Join our community to share strategies and compete on leaderboards

**Good luck, pilot! The fate of humanity rests in your hands! 🚀**
