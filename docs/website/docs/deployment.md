---
sidebar_position: 10
sidebar_label: Deployment Guide
---

# 🚀 Deployment & Packaging Guide

Complete guide for building, packaging, and deploying R-Type for distribution.

## 📋 Overview

This guide covers:
- Building release binaries
- Packaging for distribution
- Creating installers
- Deploying servers
- CI/CD automation

---

## 🔨 Building for Release

### Prerequisites

**All Platforms:**
- CMake 3.19+
- vcpkg (for dependencies)
- Git

**Windows:**
- Visual Studio 2022 or MSVC toolchain
- Windows SDK 10+

**Linux:**
- GCC 11+ or Clang 14+
- Standard build tools

---

### Build Release Binaries

#### Windows (Release Build)

```powershell
# Configure with release preset
cmake --preset windows-release

# Build
cmake --build build --config Release

# Binaries are in build/bin/Release/
```

#### Linux (Release Build)

```bash
# Configure
cmake --preset linux-release

# Build (use all cores)
cmake --build build -- -j$(nproc)

# Binaries are in build/bin/
```

### Optimization Flags

For maximum performance, edit `CMakePresets.json`:

```json
{
  "name": "windows-release-optimized",
  "inherits": "windows-release",
  "cacheVariables": {
    "CMAKE_CXX_FLAGS_RELEASE": "/O2 /Ob2 /DNDEBUG /GL",
    "CMAKE_EXE_LINKER_FLAGS_RELEASE": "/LTCG /OPT:REF /OPT:ICF"
  }
}
```

---

## 📦 Packaging Structure

### Directory Layout

```
r-type-client/
├── r-type_client.exe (or r-type_client)
├── assets/
│   ├── config.toml
│   ├── audio/
│   ├── fonts/
│   ├── img/
│   └── shaders/
├── config/
│   ├── client/
│   │   ├── client.toml
│   │   ├── video.toml
│   │   └── controls.json
│   └── game/
│       ├── enemies.toml
│       ├── players.toml
│       └── powerups.toml
├── logs/ (empty directory)
├── saves/ (empty directory)
├── README.txt
├── LICENSE.txt
└── CHANGELOG.txt

r-type-server/
├── r-type_server.exe (or r-type_server)
├── config/
│   ├── server/
│   │   ├── server.toml
│   │   ├── gameplay.toml
│   │   └── config.toml
│   └── game/
│       ├── enemies.toml
│       ├── players.toml
│       ├── powerups.toml
│       ├── projectiles.toml
│       └── levels/
├── logs/ (empty directory)
├── README_SERVER.txt
└── LICENSE.txt
```

---

## 🪟 Windows Packaging

### Collect Dependencies

Windows builds require runtime DLLs.

#### Manual Collection

```powershell
# Navigate to build directory
cd build/bin/Release

# Find required DLLs
dumpbin /dependents r-type_client.exe

# Copy SFML DLLs (from vcpkg)
$VCPKG_ROOT = "c:\path\to\vcpkg"
copy "$VCPKG_ROOT\installed\x64-windows\bin\sfml-*.dll" .
copy "$VCPKG_ROOT\installed\x64-windows\bin\openal32.dll" .
```

#### Automated Script

Create `package-windows.ps1`:

```powershell
# package-windows.ps1
param(
    [string]$BuildDir = "build/bin/Release",
    [string]$OutputDir = "dist/r-type-windows"
)

# Create output directories
New-Item -ItemType Directory -Force -Path "$OutputDir/r-type-client"
New-Item -ItemType Directory -Force -Path "$OutputDir/r-type-server"

# Copy client files
Copy-Item "$BuildDir/r-type_client.exe" "$OutputDir/r-type-client/"
Copy-Item -Recurse "assets" "$OutputDir/r-type-client/"
Copy-Item -Recurse "config/client" "$OutputDir/r-type-client/config/"
Copy-Item -Recurse "config/game" "$OutputDir/r-type-client/config/"

# Copy server files
Copy-Item "$BuildDir/r-type_server.exe" "$OutputDir/r-type-server/"
Copy-Item -Recurse "config/server" "$OutputDir/r-type-server/config/"
Copy-Item -Recurse "config/game" "$OutputDir/r-type-server/config/"

# Copy dependencies
$vcpkgDir = "$env:VCPKG_ROOT\installed\x64-windows\bin"
Copy-Item "$vcpkgDir\sfml-*.dll" "$OutputDir/r-type-client/"
Copy-Item "$vcpkgDir\openal32.dll" "$OutputDir/r-type-client/"

# Create empty directories
New-Item -ItemType Directory -Force -Path "$OutputDir/r-type-client/logs"
New-Item -ItemType Directory -Force -Path "$OutputDir/r-type-client/saves"
New-Item -ItemType Directory -Force -Path "$OutputDir/r-type-server/logs"

# Copy documentation
Copy-Item "README.md" "$OutputDir/r-type-client/README.txt"
Copy-Item "LICENSE" "$OutputDir/r-type-client/LICENSE.txt"
Copy-Item "CHANGELOG.md" "$OutputDir/r-type-client/CHANGELOG.txt"

Write-Host "✅ Windows package created in $OutputDir"
```

Run:

```powershell
.\package-windows.ps1
```

### Create ZIP Archive

```powershell
# Compress client
Compress-Archive -Path "dist/r-type-windows/r-type-client/*" `
                 -DestinationPath "dist/r-type-client-windows-v1.0.0.zip"

# Compress server
Compress-Archive -Path "dist/r-type-windows/r-type-server/*" `
                 -DestinationPath "dist/r-type-server-windows-v1.0.0.zip"
```

---

### Create Windows Installer

Using [Inno Setup](https://jrsoftware.org/isinfo.php):

#### Install Inno Setup

Download from: https://jrsoftware.org/isdl.php

#### Create Installer Script

`installer-client.iss`:

```iss
#define MyAppName "R-Type Client"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Your Organization"
#define MyAppURL "https://github.com/YourOrg/Rtype"
#define MyAppExeName "r-type_client.exe"

[Setup]
AppId={{12345678-1234-1234-1234-123456789012}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\RType
DefaultGroupName=R-Type
OutputDir=dist
OutputBaseFilename=r-type-client-setup-v{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Files]
Source: "dist\r-type-windows\r-type-client\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch R-Type"; Flags: nowait postinstall skipifsilent
```

#### Build Installer

```powershell
# Compile installer
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer-client.iss

# Output: dist/r-type-client-setup-v1.0.0.exe
```

---

## 🐧 Linux Packaging

### Create Tarball

Create `package-linux.sh`:

```bash
#!/bin/bash
set -e

BUILD_DIR="build/bin"
OUTPUT_DIR="dist/r-type-linux"
VERSION="1.0.0"

# Create directories
mkdir -p "$OUTPUT_DIR/r-type-client"
mkdir -p "$OUTPUT_DIR/r-type-server"

# Copy client files
cp "$BUILD_DIR/r-type_client" "$OUTPUT_DIR/r-type-client/"
cp -r assets "$OUTPUT_DIR/r-type-client/"
cp -r config/client "$OUTPUT_DIR/r-type-client/config/"
cp -r config/game "$OUTPUT_DIR/r-type-client/config/"
mkdir -p "$OUTPUT_DIR/r-type-client/logs"
mkdir -p "$OUTPUT_DIR/r-type-client/saves"

# Copy server files
cp "$BUILD_DIR/r-type_server" "$OUTPUT_DIR/r-type-server/"
cp -r config/server "$OUTPUT_DIR/r-type-server/config/"
cp -r config/game "$OUTPUT_DIR/r-type-server/config/"
mkdir -p "$OUTPUT_DIR/r-type-server/logs"

# Make executable
chmod +x "$OUTPUT_DIR/r-type-client/r-type_client"
chmod +x "$OUTPUT_DIR/r-type-server/r-type_server"

# Copy documentation
cp README.md "$OUTPUT_DIR/r-type-client/README.txt"
cp LICENSE "$OUTPUT_DIR/r-type-client/LICENSE.txt"

# Create tarballs
cd "$OUTPUT_DIR"
tar -czf "../r-type-client-linux-v${VERSION}.tar.gz" r-type-client/
tar -czf "../r-type-server-linux-v${VERSION}.tar.gz" r-type-server/

echo "✅ Linux packages created in dist/"
```

Run:

```bash
chmod +x package-linux.sh
./package-linux.sh
```

---

### Create .deb Package (Debian/Ubuntu)

#### Install Packaging Tools

```bash
sudo apt install devscripts debhelper
```

#### Create Debian Package Structure

```bash
mkdir -p r-type-client-1.0.0/DEBIAN
mkdir -p r-type-client-1.0.0/usr/bin
mkdir -p r-type-client-1.0.0/usr/share/r-type
mkdir -p r-type-client-1.0.0/usr/share/applications
mkdir -p r-type-client-1.0.0/usr/share/pixmaps
```

#### Create Control File

`r-type-client-1.0.0/DEBIAN/control`:

```
Package: r-type-client
Version: 1.0.0
Section: games
Priority: optional
Architecture: amd64
Depends: libsfml-graphics2.5, libsfml-audio2.5, libsfml-window2.5, libsfml-system2.5
Maintainer: Your Name <your.email@example.com>
Description: R-Type multiplayer space shooter client
 Classic arcade-style space shooter with modern multiplayer features.
 Fight through waves of aliens in this cooperative shoot-em-up.
```

#### Create Desktop Entry

`r-type-client-1.0.0/usr/share/applications/r-type.desktop`:

```desktop
[Desktop Entry]
Name=R-Type
Comment=Multiplayer Space Shooter
Exec=/usr/bin/r-type_client
Icon=r-type
Terminal=false
Type=Application
Categories=Game;ActionGame;
```

#### Build .deb

```bash
# Copy files
cp build/bin/r-type_client r-type-client-1.0.0/usr/bin/
cp -r assets r-type-client-1.0.0/usr/share/r-type/
cp -r config r-type-client-1.0.0/usr/share/r-type/

# Build package
dpkg-deb --build r-type-client-1.0.0

# Output: r-type-client-1.0.0.deb
```

#### Install Package

```bash
sudo dpkg -i r-type-client-1.0.0.deb
```

---

### Create AppImage

Using [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy):

```bash
# Download linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

# Create AppDir structure
mkdir -p RType.AppDir/usr/bin
mkdir -p RType.AppDir/usr/share/r-type

# Copy files
cp build/bin/r-type_client RType.AppDir/usr/bin/
cp -r assets RType.AppDir/usr/share/r-type/
cp -r config RType.AppDir/usr/share/r-type/

# Create AppImage
./linuxdeploy-x86_64.AppImage \
    --appdir RType.AppDir \
    --output appimage \
    --executable RType.AppDir/usr/bin/r-type_client \
    --desktop-file r-type.desktop \
    --icon-file assets/img/icon.png

# Output: R_Type-x86_64.AppImage
```

---

## 🖥️ Server Deployment

### Dedicated Server Setup

#### Linux Server (SystemD)

Create service file `/etc/systemd/system/r-type-server.service`:

```ini
[Unit]
Description=R-Type Game Server
After=network.target

[Service]
Type=simple
User=rtype
Group=rtype
WorkingDirectory=/opt/r-type-server
ExecStart=/opt/r-type-server/r-type_server
Restart=on-failure
RestartSec=10
StandardOutput=journal
StandardError=journal

# Resource limits
LimitNOFILE=4096
LimitNPROC=2048

[Install]
WantedBy=multi-user.target
```

#### Setup Server

```bash
# Create user
sudo useradd -r -s /bin/false rtype

# Install server
sudo mkdir -p /opt/r-type-server
sudo tar -xzf r-type-server-linux-v1.0.0.tar.gz -C /opt/
sudo chown -R rtype:rtype /opt/r-type-server

# Configure firewall
sudo ufw allow 4000/udp
sudo ufw reload

# Enable service
sudo systemctl daemon-reload
sudo systemctl enable r-type-server
sudo systemctl start r-type-server

# Check status
sudo systemctl status r-type-server

# View logs
sudo journalctl -u r-type-server -f
```

---

### Docker Deployment

#### Create Dockerfile

`Dockerfile.server`:

```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    libsfml-network2.5 \
    libsfml-system2.5 \
    && rm -rf /var/lib/apt/lists/*

# Create user
RUN useradd -r -s /bin/false rtype

# Copy server files
COPY --chown=rtype:rtype build/bin/r-type_server /usr/local/bin/
COPY --chown=rtype:rtype config /opt/r-type/config/

# Create directories
RUN mkdir -p /opt/r-type/logs && chown rtype:rtype /opt/r-type/logs

# Switch to non-root user
USER rtype
WORKDIR /opt/r-type

# Expose port
EXPOSE 4000/udp

# Run server
CMD ["/usr/local/bin/r-type_server"]
```

#### Build & Run

```bash
# Build image
docker build -f Dockerfile.server -t r-type-server:1.0.0 .

# Run container
docker run -d \
    --name r-type-server \
    -p 4000:4000/udp \
    -v $(pwd)/config:/opt/r-type/config:ro \
    -v $(pwd)/logs:/opt/r-type/logs \
    --restart unless-stopped \
    r-type-server:1.0.0

# View logs
docker logs -f r-type-server
```

#### Docker Compose

`docker-compose.yml`:

```yaml
version: '3.8'

services:
  r-type-server:
    image: r-type-server:1.0.0
    build:
      context: .
      dockerfile: Dockerfile.server
    ports:
      - "4000:4000/udp"
    volumes:
      - ./config:/opt/r-type/config:ro
      - ./logs:/opt/r-type/logs
      - ./data:/opt/r-type/data
    restart: unless-stopped
    environment:
      - LOG_LEVEL=INFO
    healthcheck:
      test: ["CMD", "nc", "-zu", "localhost", "4000"]
      interval: 30s
      timeout: 10s
      retries: 3
```

Run:

```bash
docker-compose up -d
```

---

## ☁️ Cloud Deployment

### AWS EC2

#### Launch Instance

```bash
# Using AWS CLI
aws ec2 run-instances \
    --image-id ami-0c55b159cbfafe1f0 \
    --instance-type t3.small \
    --key-name your-key \
    --security-groups r-type-server-sg \
    --user-data file://server-setup.sh
```

#### Setup Script

`server-setup.sh`:

```bash
#!/bin/bash
set -e

# Update system
apt-get update
apt-get upgrade -y

# Install dependencies
apt-get install -y wget tar

# Download server
cd /opt
wget https://github.com/YourOrg/Rtype/releases/download/v1.0.0/r-type-server-linux-v1.0.0.tar.gz
tar -xzf r-type-server-linux-v1.0.0.tar.gz
rm r-type-server-linux-v1.0.0.tar.gz

# Install systemd service
cat > /etc/systemd/system/r-type-server.service <<EOF
[Unit]
Description=R-Type Server
After=network.target

[Service]
Type=simple
WorkingDirectory=/opt/r-type-server
ExecStart=/opt/r-type-server/r-type_server
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# Start service
systemctl daemon-reload
systemctl enable r-type-server
systemctl start r-type-server
```

---

### DigitalOcean Droplet

Similar to AWS, create droplet and use setup script.

### Google Cloud Platform

```bash
gcloud compute instances create r-type-server \
    --image-family=ubuntu-2204-lts \
    --image-project=ubuntu-os-cloud \
    --machine-type=e2-small \
    --zone=us-central1-a \
    --metadata-from-file startup-script=server-setup.sh
```

---

## 🤖 CI/CD Automation

### GitHub Actions Workflow

`.github/workflows/release.yml`:

```yaml
name: Release

on:
  push:
    tags:
      - 'v*'

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          .\vcpkg\bootstrap-vcpkg.bat
          
      - name: Build
        run: |
          cmake --preset windows-release
          cmake --build build --config Release
          
      - name: Package
        run: .\package-windows.ps1
        
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: windows-packages
          path: dist/*.zip

  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake g++ libsfml-dev
          
      - name: Build
        run: |
          cmake --preset linux-release
          cmake --build build
          
      - name: Package
        run: ./package-linux.sh
        
      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: linux-packages
          path: dist/*.tar.gz

  release:
    needs: [build-windows, build-linux]
    runs-on: ubuntu-latest
    steps:
      - name: Download artifacts
        uses: actions/download-artifact@v3
        
      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            windows-packages/*.zip
            linux-packages/*.tar.gz
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

---

## 📊 Deployment Checklist

### Pre-Release

- [ ] All tests passing
- [ ] Performance benchmarks met
- [ ] No known critical bugs
- [ ] Documentation updated
- [ ] Changelog written
- [ ] Version numbers bumped

### Packaging

- [ ] Release builds created
- [ ] Dependencies included
- [ ] Config files included
- [ ] Assets verified
- [ ] README/LICENSE included
- [ ] File permissions correct

### Distribution

- [ ] Archives created
- [ ] Checksums generated
- [ ] Installers tested
- [ ] Uploaded to hosting
- [ ] Release notes published

### Post-Release

- [ ] Download links verified
- [ ] Installation tested
- [ ] Community announced
- [ ] Monitor for issues

---

## 📚 Related Documentation

- [Server Administration](./server-admin.md)
- [Configuration Reference](./configuration.md)
- [Troubleshooting](./troubleshooting.md)

**Happy deploying! 🚀**
