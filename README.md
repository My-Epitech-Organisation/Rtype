<div align="center">

# 🚀 R-Type

### *Classic Space Shooter Reimagined*

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue?style=for-the-badge)](https://github.com/My-Epitech-Organisation/Rtype)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)](LICENSE)
[![CMake](https://img.shields.io/badge/CMake-3.19+-064F8C?style=for-the-badge&logo=cmake)](https://cmake.org/)

A modern multiplayer remake of the legendary R-Type arcade game, built with cutting-edge C++20 technology and networked gameplay.

[Download](https://github.com/My-Epitech-Organisation/Rtype/releases) • [Documentation](docs/website/docs/intro.md) • [Report Bug](https://github.com/My-Epitech-Organisation/Rtype/issues)

---

</div>

## ✨ Features

<table>
<tr>
<td width="50%">

### 🎮 Gameplay
- **Classic R-Type Action** - Authentic arcade experience
- **4 Player Co-op** - Team up with friends online
- **Wave System** - Progressive difficulty levels
- **Power-ups** - Collect weapons and shields
- **Boss Battles** - Epic end-level encounters

</td>
<td width="50%">

### ⚙️ Technical
- **Multiplayer** - Real-time networked gameplay
- **Cross-Platform** - Windows & Linux support
- **High Performance** - Smooth 60 FPS gameplay
- **Custom ECS Engine** - Entity Component System
- **UDP Networking** - Low-latency multiplayer

</td>
</tr>
</table>

---

## 🎯 Quick Start

### For Players

#### 1️⃣ Download & Install

**Requirements:**
- Windows 10+ or Linux (Ubuntu 20.04+)
- OpenGL 3.3+ compatible graphics card
- 512 MB RAM
- 100 MB disk space

#### 2️⃣ Run the Game

**Windows:**
```powershell
# Start server (host a game)
.\scripts\run_server.bat

# Start client (join a game)
.\scripts\run_client.bat
```

**Linux:**
```bash
# Start server
./scripts/run_server.sh

# Start client
./scripts/run_client.sh
```

#### 3️⃣ Configure

Edit `config/client/client.toml` to customize:
- **Graphics** - Resolution, fullscreen, VSync
- **Controls** - Keyboard and gamepad mappings
- **Audio** - Volume settings
- **Network** - Server address and port

---

## 🎮 How to Play

### Controls

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| Move | Arrow Keys | Left Stick |
| Fire | Space | A Button |
| Charge Beam | Hold Space | Hold A |
| Pause | ESC | Start |

### Game Modes

- **🏠 Host Server** - Start a game for others to join
- **🌐 Join Server** - Connect to an existing game
- **👥 Multiplayer** - Up to 4 players cooperative

---

## 🛠️ Building from Source

<details>
<summary><b>For Developers & Contributors</b></summary>

### Prerequisites

- **CMake** 3.19+
- **Ninja** build system
- **C++20 compiler** (GCC 11+, Clang 14+, or MSVC 2022)
- **Git** with submodules support

### Build Steps

```bash
# 1. Clone with submodules
git clone --recursive https://github.com/My-Epitech-Organisation/Rtype.git
cd Rtype

# 2. Setup dependencies (first time only)
./scripts/setup-vcpkg.sh

# 3. Configure
cmake --preset linux-release    # Linux
cmake --preset windows-release  # Windows

# 4. Build
cmake --build build --config Release

# 5. Run
./build/bin/r-type_server  # Server
./build/bin/r-type_client  # Client
```

### Advanced Options

```bash
# Debug build with tests
cmake --preset linux-debug
cmake --build build
ctest --test-dir build

# Build with documentation
cmake --preset linux-release -DBUILD_DOCS=ON
cmake --build build --target docs
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed development guidelines.

</details>

---

## 📚 Documentation

| Resource | Description |
|----------|-------------|
| [📖 User Guide](docs/website/docs/getting-started.md) | Installation and gameplay instructions |
| [🏗️ Architecture](docs/website/docs/Architecture/overview.md) | Technical design and structure |
| [🌐 Network Protocol](docs/RFC/RFC_RTGP_v1.2.0.md) | Multiplayer protocol specification |
| [🔧 Configuration](config/) | Server and client settings |
| [🐛 Troubleshooting](docs/website/docs/contributing.md#troubleshooting) | Common issues and solutions |

---

## 🤝 Contributing

We welcome contributions! Whether you're fixing bugs, adding features, or improving documentation:

1. 🍴 Fork the repository
2. 🔨 Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. ✅ Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. 📤 Push to the branch (`git push origin feature/AmazingFeature`)
5. 🎉 Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

---

## 📊 Project Status

![Build Status](https://img.shields.io/badge/build-passing-brightgreen?style=flat-square)
![Tests](https://img.shields.io/badge/tests-passing-brightgreen?style=flat-square)
![Coverage](https://img.shields.io/badge/coverage-85%25-yellow?style=flat-square)

**Current Version:** 1.0.0  
**Status:** Active Development  
**Last Updated:** December 2025

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 👥 Team

**Epitech Project** - R-Type 2025/2026

Made with ❤️ by students passionate about game development

---

<div align="center">

### ⭐ Star us on GitHub!

If you enjoy R-Type, consider giving us a star. It helps the project grow! 🚀

[⬆ Back to Top](#-r-type)

</div>
