---
sidebar_position: 11
sidebar_label: Server Administration
---

# 🖥️ Server Administration Guide

Complete guide for running, monitoring, and maintaining R-Type game servers.

## 📋 Overview

This guide covers:
- Server installation and setup
- Configuration management
- Monitoring and logging
- Performance optimization
- Security best practices
- Troubleshooting

---

## 🚀 Quick Start

### Installation

#### Linux

```bash
# Download server package
wget https://github.com/YourOrg/Rtype/releases/download/v1.0.0/r-type-server-linux-v1.0.0.tar.gz

# Extract
tar -xzf r-type-server-linux-v1.0.0.tar.gz
cd r-type-server

# Run server
./r-type_server
```

#### Windows

```powershell
# Extract ZIP package
Expand-Archive -Path r-type-server-windows-v1.0.0.zip -DestinationPath C:\rtype-server

# Navigate to directory
cd C:\rtype-server

# Run server
.\r-type_server.exe
```

### Basic Configuration

Edit `config/server/server.toml`:

```toml
port = 4000                   # Server port
max_players = 8               # Maximum concurrent players
tickrate = 60                 # Server update rate (Hz)
name = "My R-Type Server"     # Server name
password = ""                 # Leave empty for public server
```

---

## ⚙️ Configuration

### Server Settings

`config/server/server.toml`:

```toml
# Network Configuration
port = 4000
host = "0.0.0.0"              # Bind to all interfaces
max_players = 8
timeout = 30                  # Client timeout (seconds)

# Server Identity
name = "Official R-Type Server #1"
motd = "Welcome to R-Type! Have fun!"
password = ""                 # Optional password protection
region = "US-East"            # Server region
public = true                 # List in server browser

# Performance
tickrate = 60                 # Update rate (30, 60, or 128)
max_bandwidth = 0             # KB/s per client (0 = unlimited)
compression = true            # Enable packet compression

# Features
allow_spectators = true
max_spectators = 4
enable_chat = true
enable_voice = false
```

### Gameplay Settings

`config/server/gameplay.toml`:

```toml
# Difficulty
difficulty = "normal"         # easy, normal, hard, nightmare
enemy_scaling = true          # Scale with player count
friendly_fire = false

# Progression
starting_level = 1
max_level = 10
respawn_time = 5              # Seconds
respawn_invincibility = 3     # Seconds of invincibility

# Power-ups
power_up_spawn_rate = 0.3
power_up_duration = 30        # Seconds

# Scoring
score_multiplier = 1.0
team_shared_score = true

# Time Limits
level_time_limit = 600        # Seconds per level (0 = none)
game_time_limit = 0           # Total game time (0 = none)

# Advanced
max_enemies_on_screen = 50
projectile_limit = 200
```

---

## 📊 Monitoring

### Log Files

Logs are written to `logs/` directory:

- `server.log` - General server logs
- `error.log` - Error messages only
- `access.log` - Connection/disconnection events
- `game.log` - Gameplay events

### Log Configuration

`config/server/config.toml`:

```toml
[logging]
level = "INFO"                # DEBUG, INFO, WARNING, ERROR
file = "logs/server.log"
console = true                # Also print to console
max_size_mb = 100             # Max log file size
rotate = true                 # Rotate when full
keep_rotated = 10             # Keep last 10 rotated logs
```

### Real-time Monitoring

#### View Logs

```bash
# Follow latest logs
tail -f logs/server.log

# Search for errors
grep -i error logs/server.log

# Count connections today
grep "Client connected" logs/access.log | wc -l
```

#### Server Statistics

Run admin command:

```bash
# If server has admin console
echo "stats" | nc localhost 4000
```

Or check web dashboard (if configured):

```
http://your-server:8080/stats
```

---

## 🔐 Security

### Firewall Configuration

#### Linux (UFW)

```bash
# Allow game port
sudo ufw allow 4000/udp

# Allow admin port (if enabled)
sudo ufw allow from 203.0.113.42 to any port 4001

# Enable firewall
sudo ufw enable
sudo ufw status
```

#### Linux (iptables)

```bash
# Allow game port
sudo iptables -A INPUT -p udp --dport 4000 -j ACCEPT

# Save rules
sudo iptables-save > /etc/iptables/rules.v4
```

#### Windows Firewall

```powershell
# Allow inbound UDP on port 4000
New-NetFirewallRule -DisplayName "R-Type Server" `
    -Direction Inbound `
    -Protocol UDP `
    -LocalPort 4000 `
    -Action Allow
```

### Access Control

`config/server/config.toml`:

```toml
[security]
# Rate limiting
rate_limit = 100              # Max packets/second per client
burst_limit = 200             # Burst allowance

# Banning
auto_ban_threshold = 5        # Ban after X warnings
ban_duration = 3600           # Ban duration (seconds)
permanent_bans = [            # Permanent ban list
    "203.0.113.100",
    "203.0.113.101"
]

# Whitelist (optional)
whitelist_only = false        # Only allow whitelisted IPs
whitelist = [
    "203.0.113.42",
    "192.168.1.0/24"          # Allow entire subnet
]

# Admin IPs
admin_ips = [
    "203.0.113.42"            # Your IP
]
admin_password = "your_secure_password"
```

### DDoS Protection

```toml
[security.ddos]
enabled = true
max_connections_per_ip = 4
connection_rate_limit = 10    # Connections per minute
packet_rate_limit = 1000      # Packets per second
```

---

## 👥 User Management

### Admin Commands

Connect as admin:

```bash
# Using netcat
nc -u localhost 4000
AUTH admin your_secure_password
```

Available commands:

```
STATS                         # Show server statistics
PLAYERS                       # List connected players
KICK <player_id> <reason>     # Kick player
BAN <ip> <duration> <reason>  # Ban IP address
UNBAN <ip>                    # Unban IP address
BROADCAST <message>           # Send message to all
CONFIG RELOAD                 # Reload configuration
SHUTDOWN                      # Graceful shutdown
```

### Ban Management

#### Manual Ban

```bash
# Ban IP for 1 hour
echo "BAN 203.0.113.100 3600 Cheating" | nc -u localhost 4000
```

#### View Ban List

```bash
cat logs/bans.log
```

#### Edit Ban List

Edit `config/server/bans.json`:

```json
{
  "bans": [
    {
      "ip": "203.0.113.100",
      "reason": "Cheating",
      "timestamp": "2025-12-15T10:30:00Z",
      "expires": "2025-12-15T11:30:00Z",
      "permanent": false
    }
  ]
}
```

---

## 🚀 Performance Optimization

### Server Resources

**Minimum Requirements:**
- CPU: Dual-core 2.0 GHz
- RAM: 1 GB
- Network: 10 Mbps upload
- Disk: 100 MB

**Recommended (8-16 players):**
- CPU: Quad-core 2.5 GHz
- RAM: 2 GB
- Network: 50 Mbps upload
- Disk: 500 MB (SSD preferred)

### CPU Optimization

```toml
[performance]
thread_count = 4              # Worker threads (0 = auto)
use_affinity = true           # Pin threads to CPU cores
priority = "high"             # Process priority
```

### Network Optimization

```toml
[network]
send_buffer_size = 262144     # 256 KB
receive_buffer_size = 262144
tcp_nodelay = true            # Disable Nagle's algorithm
compression_level = 6         # 0-9, higher = more CPU
```

### Memory Management

```toml
[performance]
preallocate_entities = 1000   # Pre-allocate entity pool
preallocate_packets = 500     # Pre-allocate packet buffer
gc_interval = 60              # Garbage collection (seconds)
```

---

## 📈 Monitoring Tools

### Built-in Web Dashboard

Enable web dashboard:

```toml
[web]
enabled = true
port = 8080
host = "127.0.0.1"            # Only local access
username = "admin"
password = "secure_password"
```

Access: `http://localhost:8080`

### Prometheus Metrics

Export metrics for Prometheus:

```toml
[metrics]
enabled = true
port = 9090
path = "/metrics"
```

Metrics exposed:
- `rtype_players_total` - Current player count
- `rtype_games_active` - Active games
- `rtype_packets_sent` - Packets sent per second
- `rtype_cpu_usage` - CPU usage percentage
- `rtype_memory_usage` - Memory usage (bytes)

### System Monitoring

#### Linux (htop)

```bash
sudo apt install htop
htop -p $(pgrep r-type_server)
```

#### Resource Usage Script

`monitor.sh`:

```bash
#!/bin/bash
while true; do
    echo "=== $(date) ==="
    
    # CPU usage
    CPU=$(ps -p $(pgrep r-type_server) -o %cpu= | tr -d ' ')
    echo "CPU: ${CPU}%"
    
    # Memory usage
    MEM=$(ps -p $(pgrep r-type_server) -o %mem= | tr -d ' ')
    echo "Memory: ${MEM}%"
    
    # Network usage
    echo "Network:"
    ss -ntu | grep :4000 | wc -l
    
    # Active connections
    echo "Connections: $(ss -ntu | grep :4000 | wc -l)"
    
    echo ""
    sleep 30
done
```

---

## 🔄 Maintenance

### Backup

#### Automatic Backup

`backup.sh`:

```bash
#!/bin/bash
BACKUP_DIR="/var/backups/rtype"
DATE=$(date +%Y%m%d_%H%M%S)

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Backup config
tar -czf "$BACKUP_DIR/config_$DATE.tar.gz" config/

# Backup logs
tar -czf "$BACKUP_DIR/logs_$DATE.tar.gz" logs/

# Backup database (if any)
if [ -f "data/server.db" ]; then
    cp "data/server.db" "$BACKUP_DIR/server_$DATE.db"
fi

# Keep only last 7 backups
find "$BACKUP_DIR" -type f -mtime +7 -delete

echo "Backup completed: $DATE"
```

Add to crontab:

```bash
# Run daily at 3 AM
0 3 * * * /opt/rtype-server/backup.sh
```

### Updates

#### Manual Update

```bash
# Stop server
sudo systemctl stop r-type-server

# Backup current version
sudo cp -r /opt/rtype-server /opt/rtype-server.backup

# Download new version
wget https://github.com/YourOrg/Rtype/releases/download/v1.1.0/r-type-server-linux-v1.1.0.tar.gz

# Extract
sudo tar -xzf r-type-server-linux-v1.1.0.tar.gz -C /opt/

# Restore config
sudo cp -r /opt/rtype-server.backup/config/* /opt/rtype-server/config/

# Restart server
sudo systemctl start r-type-server

# Verify
sudo systemctl status r-type-server
```

#### Automated Updates

`auto-update.sh`:

```bash
#!/bin/bash
CURRENT_VERSION=$(cat /opt/rtype-server/VERSION)
LATEST_VERSION=$(curl -s https://api.github.com/repos/YourOrg/Rtype/releases/latest | jq -r .tag_name)

if [ "$CURRENT_VERSION" != "$LATEST_VERSION" ]; then
    echo "Update available: $CURRENT_VERSION -> $LATEST_VERSION"
    # Perform update...
else
    echo "Server is up to date"
fi
```

---

## 🔧 Troubleshooting

### Server Won't Start

**Check logs:**

```bash
tail -n 50 logs/error.log
```

**Common issues:**

1. **Port already in use:**
   ```bash
   sudo lsof -i :4000
   # Kill process or change port
   ```

2. **Permission denied:**
   ```bash
   chmod +x r-type_server
   ```

3. **Missing dependencies:**
   ```bash
   ldd r-type_server
   # Install missing libraries
   ```

### High CPU Usage

**Check game load:**

```bash
echo "STATS" | nc -u localhost 4000
```

**Optimize settings:**

```toml
[performance]
tickrate = 30                 # Reduce from 60
max_enemies_on_screen = 30    # Reduce from 50
```

### High Memory Usage

**Enable memory profiling:**

```toml
[logging]
memory_profiling = true
profile_interval = 60         # Seconds
```

**Check for leaks:**

```bash
valgrind --leak-check=full ./r-type_server
```

### Network Issues

**Test connectivity:**

```bash
# From client machine
nc -zvu SERVER_IP 4000
```

**Check firewall:**

```bash
sudo iptables -L -n -v | grep 4000
```

**Monitor packet loss:**

```bash
# Server side
watch -n 1 'ss -u -a | grep :4000'
```

---

## 📋 Systemd Service

### Create Service File

`/etc/systemd/system/r-type-server.service`:

```ini
[Unit]
Description=R-Type Game Server
After=network.target
Documentation=https://yourorg.github.io/Rtype/

[Service]
Type=simple
User=rtype
Group=rtype
WorkingDirectory=/opt/rtype-server
ExecStart=/opt/rtype-server/r-type_server
ExecReload=/bin/kill -HUP $MAINPID
Restart=on-failure
RestartSec=10
StandardOutput=journal
StandardError=journal
SyslogIdentifier=rtype-server

# Resource limits
LimitNOFILE=4096
LimitNPROC=2048
MemoryLimit=2G
CPUQuota=200%

# Security
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/opt/rtype-server/logs /opt/rtype-server/data

[Install]
WantedBy=multi-user.target
```

### Manage Service

```bash
# Enable on boot
sudo systemctl enable r-type-server

# Start/stop/restart
sudo systemctl start r-type-server
sudo systemctl stop r-type-server
sudo systemctl restart r-type-server

# Reload config
sudo systemctl reload r-type-server

# View status
sudo systemctl status r-type-server

# View logs
sudo journalctl -u r-type-server -f
```

---

## 🌐 Reverse Proxy

### Nginx (for web dashboard)

`/etc/nginx/sites-available/rtype-dashboard`:

```nginx
server {
    listen 80;
    server_name dashboard.rtype-server.com;

    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

Enable:

```bash
sudo ln -s /etc/nginx/sites-available/rtype-dashboard /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

---

## 📚 Related Documentation

- [Deployment Guide](./deployment.md)
- [Configuration Reference](./configuration.md)
- [Troubleshooting](./troubleshooting.md)
- [Network Protocol](./protocol/RFC_RTGP_v1.4.3.md)

**Happy server administration! 🎮**
