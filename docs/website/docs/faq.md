---
sidebar_position: 3
sidebar_label: FAQ
---

# ❓ Frequently Asked Questions

## General Questions

### What is R-Type?

R-Type is a multiplayer space shooter game inspired by the classic R-Type arcade game. Built with modern C++20 and featuring networked multiplayer, it brings the classic shoot-'em-up experience to modern platforms.

### What platforms are supported?

- **Windows**: Windows 10/11 (x64)
- **Linux**: Ubuntu 20.04+, Debian 11+, Arch Linux
- **macOS**: Not officially supported (may work with modifications)

### Is the game free?

Yes! R-Type is an open-source project available under the MIT license.

---

## Installation & Setup

### How do I install the game?

1. Download the latest release from GitHub
2. Extract the archive
3. Run the executable:
   - Windows: `r-type_client.exe`
   - Linux: `./r-type_client`

See the [Getting Started Guide](./getting-started.md) for detailed instructions.

### What are the system requirements?

**Minimum:**
- CPU: Dual-core 2.0 GHz
- RAM: 2 GB
- GPU: OpenGL 3.3 compatible
- Storage: 200 MB
- Network: Broadband for multiplayer

**Recommended:**
- CPU: Quad-core 2.5 GHz
- RAM: 4 GB
- GPU: Dedicated graphics card
- Storage: 500 MB (SSD preferred)
- Network: Low-latency connection (&lt;50ms ping)

### Do I need to install dependencies?

**Windows:** All dependencies are included in the release package.

**Linux:** You may need to install SFML:
```bash
# Ubuntu/Debian
sudo apt install libsfml-dev

# Arch Linux
sudo pacman -S sfml
```

---

## Gameplay

### How do I play multiplayer?

1. One player starts the **server**: `./r-type_server`
2. Note the server's IP address
3. All players launch the **client**: `./r-type_client`
4. Enter the server IP in the connection menu
5. Press "Connect" and wait for all players

### How many players can play together?

Up to **4 players** can play simultaneously in cooperative mode.

### Can I play solo?

Yes! Launch the game without connecting to a server for single-player practice mode.

### How do I save my progress?

Progress is automatically saved locally. Scores and achievements sync with the server during multiplayer sessions.

### Are there difficulty settings?

Yes! Choose from:
- **Easy**: For beginners
- **Normal**: Standard experience
- **Hard**: Challenging gameplay
- **Nightmare**: Expert mode

Access difficulty settings in the main menu.

---

## Controls

### How do I change the controls?

1. Edit `config/client/controls.json`
2. Restart the game for changes to take effect

Example:
```json
{
  "up": "W",
  "down": "S",
  "left": "A",
  "right": "D",
  "fire": "Space",
  "special": "LeftShift"
}
```

### Does the game support controllers?

Yes! Most standard controllers (Xbox, PlayStation, generic USB) are supported. Controller mappings are detected automatically.

### My controller isn't working. What should I do?

1. Ensure the controller is connected before launching the game
2. Check that it's recognized by your OS
3. Try a different USB port
4. See [Troubleshooting Guide](./troubleshooting.md#controller-issues)

---

## Performance

### The game is laggy. How do I improve performance?

1. **Lower graphics settings** in `config/client/video.toml`:
   ```toml
   vsync = false
   fps_limit = 60
   fullscreen = false
   ```

2. **Close background applications**
3. **Update graphics drivers**
4. **Disable shader effects** (edit config to disable colorblind/vignette)

### What FPS should I expect?

- **Modern systems**: 144+ FPS
- **Mid-range systems**: 60+ FPS
- **Minimum spec**: 30+ FPS

The game caps at 144 FPS by default but this can be changed.

### Network lag in multiplayer?

- **Check ping**: Should be <100ms for smooth gameplay
- **Use wired connection**: WiFi can introduce latency
- **Close bandwidth-heavy apps**: Streaming, downloads, etc.
- **Play on nearby servers**: Physical distance affects latency

---

## Networking

### How do I host a server?

```bash
./r-type_server
```

Configure the server in `config/server/server.toml`:
```toml
port = 8080
max_players = 4
```

### What port does the server use?

Default: **8080** (UDP)

Ensure this port is:
- Open in your firewall
- Forwarded in your router (for public servers)

### Can I play over the internet?

Yes! The host needs to:
1. Forward port 8080 (UDP) in their router
2. Share their public IP address with players
3. Ensure firewall allows the connection

### How do I find my public IP?

Visit: https://whatismyipaddress.com/

Or use command line:
```bash
# Windows
curl ifconfig.me

# Linux
curl ifconfig.me
```

### Can I host a dedicated server?

Yes! Run the server on a separate machine or VPS:
```bash
./r-type_server --dedicated
```

See [Server Administration Guide](./server-admin.md) for details.

---

## Technical Issues

### The game won't start

1. **Check system requirements**
2. **Verify all files were extracted**
3. **Try running as administrator** (Windows)
4. **Check error logs** in `logs/` directory
5. See [Troubleshooting Guide](./troubleshooting.md)

### "SFML library not found" error (Linux)

Install SFML:
```bash
sudo apt install libsfml-dev libsfml-graphics2.5 libsfml-window2.5 libsfml-system2.5 libsfml-audio2.5 libsfml-network2.5
```

### "Failed to connect to server"

1. **Verify server is running**
2. **Check IP address is correct**
3. **Ensure port 8080 is open**
4. **Check firewall settings**
5. **Verify network connectivity**

### Game crashes on startup

1. **Update graphics drivers**
2. **Verify game files integrity**
3. **Check `logs/client.log` for errors**
4. **Disable shaders** in config
5. Report the issue on GitHub with log files

### Black screen after launch

1. **Update graphics drivers**
2. **Try windowed mode**: Set `fullscreen = false` in `config/client/video.toml`
3. **Check OpenGL support**: Game requires OpenGL 3.3+
4. **Try different resolution**

---

## Configuration

### Where are the config files?

- **Client config**: `config/client/`
- **Server config**: `config/server/`
- **Game config**: `config/game/`

### How do I reset to default settings?

Delete the config files and restart. The game will regenerate defaults.

### Can I customize game difficulty?

Yes! Edit `config/game/enemies.toml` to adjust:
- Enemy health
- Bullet speed
- Spawn rates
- Damage values

### How do I change the resolution?

Edit `config/client/video.toml`:
```toml
width = 1920
height = 1080
fullscreen = true
```

---

## Development & Modding

### Can I modify the game?

Yes! R-Type is open source. Fork the repository and make changes.

### How do I build from source?

See [Contributing Guide](./contributing.md) for build instructions.

### Can I create custom levels?

Yes! Edit level files in `config/game/levels/` in TOML format.

### Can I add new enemies?

Yes! Define new enemies in `config/game/enemies.toml`. See [Add Enemy Tutorial](./tutorials/add-enemy.md).

### Can I create custom power-ups?

Yes! Edit `config/game/powerups.toml` and follow the [Add Power-Up Tutorial](./tutorials/add-powerup.md).

### Is there a modding API?

Not yet, but the ECS architecture makes it relatively easy to extend. Documentation coming soon!

---

## Community & Support

### How do I report a bug?

1. Check if it's already reported in [GitHub Issues](https://github.com/YourOrg/Rtype/issues)
2. Collect information:
   - OS and version
   - Error logs (`logs/` directory)
   - Steps to reproduce
3. Create a new issue with details

### How do I request a feature?

Open a feature request on GitHub Issues with:
- Clear description
- Use case
- Examples if applicable

### Where can I get help?

- **Documentation**: [docs/website/docs/](https://yourorg.github.io/Rtype/)
- **GitHub Issues**: Bug reports and questions
- **Discord**: (Link TBD)
- **Email**: (Contact TBD)

### How can I contribute?

See [Contributing Guide](./contributing.md) for:
- Code contributions
- Documentation improvements
- Bug fixes
- Feature development

---

## Advanced Topics

### What is the ECS architecture?

Entity Component System - a design pattern that separates data (Components) from logic (Systems). See [ECS Guide](./Architecture/ecs-guide.md).

### What network protocol does R-Type use?

RTGP (R-Type Game Protocol) v1.2.0 over UDP. See [RFC Document](./protocol/RFC_RTGP_v1.2.0.md).

### Can I use R-Type code in my project?

Yes! R-Type is MIT licensed. See [LICENSE](../../LICENSE) file.

### How do I profile performance?

See [Profiling Guide](./profiling-guide.md) for detailed instructions.

---

## Still Have Questions?

If your question isn't answered here:

1. Check the [full documentation](./intro.md)
2. Search [GitHub Issues](https://github.com/YourOrg/Rtype/issues)
3. Ask on Discord (link TBD)
4. Open a GitHub Discussion

**We're here to help! 🚀**
