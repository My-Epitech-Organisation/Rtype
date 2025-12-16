---
sidebar_position: 4
sidebar_label: Troubleshooting
---

# 🔧 Troubleshooting Guide

This guide helps you diagnose and fix common issues with R-Type.

## 🚨 Quick Diagnostics

Before diving into specific issues, try these quick fixes:

1. ✅ **Restart the game**
2. ✅ **Verify all files were extracted**
3. ✅ **Update graphics drivers**
4. ✅ **Check system requirements**
5. ✅ **Run as administrator** (Windows)
6. ✅ **Check firewall/antivirus**

---

## 🎮 Game Launch Issues

### Game Won't Start

#### Symptoms
- Double-clicking executable does nothing
- Immediate crash with no error message
- Application fails to launch

#### Solutions

**Windows:**
```powershell
# Check if MSVC runtime is installed
# Download and install:
# https://aka.ms/vs/17/release/vc_redist.x64.exe

# Run as administrator
Right-click r-type_client.exe → "Run as administrator"

# Check Windows Event Viewer for crash details
eventvwr.msc
```

**Linux:**
```bash
# Check for missing libraries
ldd ./r-type_client

# Install dependencies
sudo apt install libsfml-dev libsfml-graphics2.5 libsfml-window2.5 libsfml-audio2.5

# Check permissions
chmod +x r-type_client

# Run from terminal to see errors
./r-type_client
```

#### Still Not Working?
- Check `logs/client.log` for error messages
- Verify OpenGL 3.3+ support: `glxinfo | grep "OpenGL version"`
- Try running from command line to see error output

---

### Black Screen on Launch

#### Symptoms
- Game window opens but shows black screen
- Can hear music but no visuals
- Window is unresponsive

#### Solutions

1. **Try Windowed Mode**

Edit `config/client/video.toml`:
```toml
fullscreen = false
width = 1280
height = 720
```

2. **Update Graphics Drivers**

**Windows:**
- NVIDIA: https://www.nvidia.com/drivers
- AMD: https://www.amd.com/en/support
- Intel: https://downloadcenter.intel.com/

**Linux:**
```bash
# NVIDIA proprietary drivers
sudo ubuntu-drivers autoinstall

# AMD open-source drivers (usually pre-installed)
sudo apt install mesa-vulkan-drivers
```

3. **Disable Shaders**

Rename shader files temporarily:
```bash
cd assets/shaders
mv colorblind.frag colorblind.frag.bak
mv vignette.frag vignette.frag.bak
```

4. **Check OpenGL Support**

```bash
# Linux
glxinfo | grep "OpenGL version"

# Should show 3.3 or higher
```

**Windows:** Download OpenGL Extensions Viewer: https://realtech-vr.com/glview/

---

### Crash on Startup

#### Symptoms
- Game starts then immediately crashes
- Error dialog appears
- Log files show exceptions

#### Solutions

1. **Check Log Files**

```bash
# Windows
type logs\client.log

# Linux
cat logs/client.log
```

Look for:
- `SFML error`
- `Failed to load`
- `Segmentation fault`
- `Access violation`

2. **Common Error Messages**

**"Failed to load assets/img/..."**
```bash
# Verify assets exist
ls assets/img/
ls assets/audio/
ls assets/fonts/

# Re-extract game files
# Ensure assets/ directory is in same folder as executable
```

**"Failed to create window"**
- Reduce resolution in `config/client/video.toml`
- Try windowed mode
- Update graphics drivers

**"SFML Audio module not found"**
```bash
# Linux
sudo apt install libsfml-audio2.5 libopenal1

# Windows: Verify all DLLs are present
dir *.dll
```

3. **Debug Mode (Developers)**

```bash
# Run with debug symbols
./r-type_client --debug --verbose

# Or check with GDB
gdb ./r-type_client
(gdb) run
(gdb) backtrace  # After crash
```

---

## 🌐 Networking Issues

### Can't Connect to Server

#### Symptoms
- "Connection failed" error
- Timeout when connecting
- "Server not found"

#### Diagnosis Checklist

```bash
# 1. Verify server is running
# On server machine, you should see:
# "Server listening on 0.0.0.0:8080"

# 2. Test network connectivity
ping SERVER_IP

# 3. Test port connectivity
# Windows
Test-NetConnection -ComputerName SERVER_IP -Port 8080

# Linux
nc -zv SERVER_IP 8080
# or
telnet SERVER_IP 8080
```

#### Solutions

1. **Firewall Configuration**

**Windows (Client & Server):**
```powershell
# Allow R-Type through firewall
netsh advfirewall firewall add rule name="R-Type Client" dir=in action=allow program="C:\Path\To\r-type_client.exe" enable=yes

netsh advfirewall firewall add rule name="R-Type Server" dir=in action=allow protocol=UDP localport=8080
```

**Linux (Server):**
```bash
# UFW
sudo ufw allow 8080/udp
sudo ufw reload

# iptables
sudo iptables -A INPUT -p udp --dport 8080 -j ACCEPT
sudo iptables-save
```

2. **Router Port Forwarding (For Internet Play)**

Access your router admin panel (usually http://192.168.1.1):
- Forward external port 8080 → internal IP:8080 (UDP)
- Protocol: UDP
- Internal IP: Server machine's local IP

3. **Check Server Configuration**

Edit `config/server/server.toml`:
```toml
# Bind to all interfaces for remote connections
host = "0.0.0.0"  # NOT "127.0.0.1"
port = 8080
max_players = 4
timeout_seconds = 30
```

4. **Verify IP Address**

**Server needs to share:**
- **Local network**: Local IP (e.g., 192.168.1.100)
- **Internet**: Public IP (find at https://whatismyip.com/)

**Client connects to:**
```
# Local game
127.0.0.1

# LAN game
192.168.1.100

# Internet game
203.0.113.42  # Public IP
```

---

### High Lag / Latency

#### Symptoms
- Delayed input response
- Rubberbanding (stuttering movement)
- Frequent disconnections

#### Solutions

1. **Check Network Quality**

```bash
# Continuous ping test
ping -t SERVER_IP  # Windows
ping SERVER_IP     # Linux (Ctrl+C to stop)

# Acceptable values:
# < 50ms: Excellent
# 50-100ms: Good
# 100-200ms: Playable
# > 200ms: Poor
```

2. **Optimize Connection**

- **Use Wired Ethernet** instead of WiFi
- **Close bandwidth-heavy applications**:
  - Streaming (Netflix, YouTube)
  - Downloads (Steam, torrents)
  - Video calls (Zoom, Discord)
- **Close background apps** using network

3. **Server-Side Optimization**

Edit `config/server/gameplay.toml`:
```toml
# Reduce tick rate for lower-end connections
tick_rate = 30  # Default: 60
# Lower value = less bandwidth but less responsive

# Adjust prediction buffer
client_prediction_buffer = 3  # frames
```

4. **Client-Side Settings**

Edit `config/client/client.toml`:
```toml
# Enable client-side prediction
prediction_enabled = true

# Interpolation smoothing
interpolation_delay = 50  # ms
```

---

### Disconnection Issues

#### Symptoms
- "Connection lost" mid-game
- Random disconnects
- "Server timeout"

#### Solutions

1. **Check Logs**

```bash
# Server logs
cat logs/server.log | grep -i "disconnect\|error\|timeout"

# Client logs
cat logs/client.log | grep -i "disconnect\|error\|timeout"
```

2. **Increase Timeout**

Server `config/server/server.toml`:
```toml
# Increase timeout for unreliable connections
timeout_seconds = 60  # Default: 30
keepalive_interval = 10  # seconds
```

3. **NAT/Router Issues**

Some routers aggressively close UDP connections. Solutions:
- **Enable UPnP** in router settings
- **Set static port forwarding** (see Port Forwarding section above)
- **Enable "DMZ" mode** for server machine (less secure, last resort)

4. **Packet Loss Detection**

```bash
# Monitor packet loss
# Linux
ping -c 100 SERVER_IP | grep "packet loss"

# Windows
ping -n 100 SERVER_IP | findstr "Lost"
```

If packet loss > 5%, check:
- ISP issues
- WiFi interference
- Hardware problems
- Network congestion

---

## 🎨 Graphics Issues

### Low FPS / Stuttering

#### Symptoms
- Frame rate below 60 FPS
- Choppy/jerky movement
- Input lag

#### Solutions

1. **Enable Performance Mode**

Edit `config/client/video.toml`:
```toml
# Disable VSync for maximum FPS
vsync = false

# Set FPS limit
fps_limit = 60  # or 144 for high-refresh monitors

# Lower resolution
width = 1280
height = 720
fullscreen = false

# Reduce quality
antialiasing = 0
```

2. **Disable Shaders**

```bash
# Rename shaders to disable them
cd assets/shaders
mv colorblind.frag colorblind.frag.disabled
mv vignette.frag vignette.frag.disabled
```

3. **Update Drivers**

See [Black Screen on Launch](#black-screen-on-launch) section.

4. **Check CPU/GPU Usage**

**Windows:**
```powershell
# Task Manager → Performance tab
# Check if CPU or GPU is at 100%
```

**Linux:**
```bash
# Monitor resource usage
htop

# Check GPU usage (NVIDIA)
nvidia-smi -l 1

# Check GPU usage (AMD)
radeontop
```

If CPU/GPU is maxed:
- Close background applications
- Lower graphics settings
- Check for overheating (clean fans/heatsinks)

5. **Disable Background Recording**

- **NVIDIA GeForce Experience**: Disable Instant Replay
- **AMD ReLive**: Disable recording
- **Windows Game Bar**: Disable (Settings → Gaming → Game Bar)
- **OBS/Streamlabs**: Stop recording/streaming

---

### Screen Tearing

#### Symptoms
- Horizontal lines during fast movement
- Image appears "split"

#### Solution

Enable VSync in `config/client/video.toml`:
```toml
vsync = true
```

**Note:** VSync may cap FPS at 60. Alternative:
```toml
vsync = false
fps_limit = 144  # Match your monitor's refresh rate
```

---

### Incorrect Resolution

#### Symptoms
- UI elements cut off
- Game doesn't fill screen
- Stretched/distorted graphics

#### Solutions

1. **Set Correct Resolution**

Edit `config/client/video.toml`:
```toml
# Use your monitor's native resolution
width = 1920
height = 1080
fullscreen = true
```

Find native resolution:
- **Windows**: Settings → Display → Resolution
- **Linux**: `xrandr | grep '*'`

2. **Aspect Ratio Issues**

For ultra-wide monitors (21:9, 32:9):
```toml
width = 3440   # 21:9
height = 1440
fullscreen = true
```

---

## 🎵 Audio Issues

### No Sound

#### Solutions

1. **Check Audio Settings**

Edit `config/client/client.toml`:
```toml
[audio]
master_volume = 100
music_volume = 80
sfx_volume = 80
muted = false
```

2. **Verify Audio Files**

```bash
# Check if audio files exist
ls assets/audio/
# Should contain .ogg files
```

3. **Test System Audio**

**Linux:**
```bash
# Test audio
speaker-test -t wav -c 2

# Check PulseAudio
pactl list sinks
pulseaudio --check

# Restart PulseAudio if needed
pulseaudio -k && pulseaudio --start
```

**Windows:**
- Check Volume Mixer: Ensure R-Type isn't muted
- Check default audio device in Settings

4. **Install Audio Libraries (Linux)**

```bash
sudo apt install libopenal1 libvorbis0a libflac8
```

---

### Crackling / Distorted Audio

#### Solutions

1. **Increase Audio Buffer**

Edit `config/client/client.toml`:
```toml
[audio]
buffer_size = 2048  # Default: 1024
# Higher = less crackling but more latency
```

2. **Update Audio Drivers**

**Windows:** Update from Device Manager

**Linux:**
```bash
# Update PulseAudio/PipeWire
sudo apt update && sudo apt upgrade pulseaudio

# Or switch to PipeWire (Ubuntu 22.04+)
sudo apt install pipewire pipewire-audio-client-libraries
```

3. **Disable Audio Effects**

- Windows: Sound Settings → Device Properties → Disable all enhancements
- Linux: PulseAudio → Disable echo cancellation

---

## 🎮 Controller Issues

### Controller Not Detected

#### Solutions

1. **Connect Before Launch**

Controllers must be connected before starting the game.

2. **Test Controller**

**Windows:**
```
Start → "Set up USB game controllers" → Test controller
```

**Linux:**
```bash
# Install joystick test tool
sudo apt install joystick jstest-gtk

# Test controller
jstest /dev/input/js0
```

3. **Controller Mapping**

Edit `config/client/controls.json`:
```json
{
  "controller": {
    "enabled": true,
    "deadzone": 0.15,
    "button_fire": 0,
    "button_special": 1,
    "axis_horizontal": 0,
    "axis_vertical": 1
  }
}
```

---

### Controller Input Lag

#### Solutions

1. **Use Wired Connection**

Wireless controllers add latency. Use USB cable if possible.

2. **Reduce Deadzone**

```json
{
  "controller": {
    "deadzone": 0.05  # Reduce from 0.15
  }
}
```

3. **Close Background Apps**

Controller software (Steam, DS4Windows) can add input lag.

---

## 🛠️ Advanced Troubleshooting

### Enable Debug Logging

Edit `config/client/client.toml`:
```toml
[logging]
level = "DEBUG"  # Default: "INFO"
# Levels: DEBUG, INFO, WARNING, ERROR
log_to_file = true
log_path = "logs/client_debug.log"
```

### Collect Crash Dumps

**Windows:**
```powershell
# Enable crash dumps
reg add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps" /v DumpFolder /t REG_EXPAND_SZ /d "%LOCALAPPDATA%\CrashDumps" /f

# Dumps will appear in:
# %LOCALAPPDATA%\CrashDumps\r-type_client.exe.*.dmp
```

**Linux:**
```bash
# Enable core dumps
ulimit -c unlimited

# Core dumps appear in current directory or /var/crash/
```

### Reset All Settings

```bash
# Backup current config
cp -r config config.backup

# Delete config directory
rm -rf config

# Restart game - will regenerate defaults
./r-type_client
```

---

## 📝 Reporting Issues

If none of these solutions work, please report the issue on GitHub:

### What to Include

1. **System Information**
   - OS and version
   - CPU, RAM, GPU
   - Graphics driver version

2. **Log Files**
   - `logs/client.log`
   - `logs/server.log` (if applicable)

3. **Steps to Reproduce**
   - What you did
   - What you expected
   - What actually happened

4. **Config Files**
   - Your modified config files (if any)

5. **Screenshots/Videos**
   - Visual issues benefit from screenshots

### How to Report

1. Check [existing issues](https://github.com/YourOrg/Rtype/issues)
2. Create new issue with template
3. Include all information above
4. Be responsive to follow-up questions

---

## 🆘 Emergency Contacts

- **GitHub Issues**: https://github.com/YourOrg/Rtype/issues
- **Discord**: (Link TBD)
- **Email**: support@rtype-game.com (TBD)

**Don't give up! We're here to help! 🚀**
