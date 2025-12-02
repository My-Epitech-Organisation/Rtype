---
sidebar_position: 1
---

# Data Persistence Strategy

## Executive Summary

**Decision:** Hybrid Approach (JSON + Binary + SQLite)  
**Date:** November 2025  
**Status:** ✅ Approved

After comprehensive testing of **JSON, Binary Packed, and SQLite** storage technologies, we determined the optimal format for each type of game data.

**Key Finding:** Use the right tool for each job — **JSON for configuration** (human-readable), **Binary for save files** (5x smaller, 500x faster), and **SQLite for structured data** (queryable, relational).

---

## Technology Comparison

| Format | Best For | Performance | Readability | Complexity |
|--------|----------|-------------|-------------|------------|
| **JSON** | Configuration, Level Data | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| **Binary** | Game Saves, Network Packets | ⭐⭐⭐⭐⭐ | ⭐ | ⭐⭐⭐ |
| **SQLite** | Highscores, Player Profiles | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |

---

## 1. JSON - Configuration & Debugging

### Performance Metrics

```text
Test Data: 10,000 entities
Write Time: ~320 ms
Read Time:  ~510 ms
Total:      ~830 ms
File Size:  5.6 MB
```

### Advantages ✅

**1. Human Readable**

```json
{
  "window": {
    "width": 1920,
    "height": 1080,
    "fullscreen": true,
    "vsync": true
  },
  "graphics": {
    "quality": "high",
    "shadows": true,
    "particles": true
  },
  "audio": {
    "master_volume": 0.8,
    "music_volume": 0.6,
    "sfx_volume": 0.9
  }
}
```

**Benefits:**

- ✅ Easy to inspect with any text editor
- ✅ Debugging is straightforward
- ✅ Manual editing possible for testing
- ✅ Git diff-friendly for version control

**2. Easy Integration**

```cpp
#include <nlohmann/json.hpp>

// Load config
std::ifstream file("config.json");
nlohmann::json config = nlohmann::json::parse(file);

int width = config["window"]["width"];
bool fullscreen = config["window"]["fullscreen"];

// Save config
config["graphics"]["quality"] = "medium";
std::ofstream out("config.json");
out << config.dump(4);  // Pretty-print with 4-space indent
```

**3. Flexible Schema**

```cpp
// Optional fields handled naturally
if (config.contains("experimental")) {
    bool enableFeature = config["experimental"]["new_feature"];
}

// Forward compatibility
config["new_option"] = true;  // Won't break old code
```

### Disadvantages ❌

**1. Performance Overhead**

- 🔴 **10-20x slower** than binary formats
- 🔴 Text parsing requires CPU cycles
- 🔴 Not suitable for real-time operations

**2. Large File Size**

```text
Same data:
Binary: 1.0 MB
JSON:   5.6 MB  (5.6x larger)
```

**3. Limited Precision**

```cpp
// Floating-point precision loss
float original = 3.141592653589793;
json["pi"] = original;
float loaded = json["pi"];  // May lose precision
```

### Recommended Uses ✅

```text
✅ Game Configuration
   - Window settings
   - Graphics options
   - Key bindings
   - Audio settings

✅ Level Data
   - Enemy spawn patterns
   - Wave definitions
   - Level layout
   - Collectible positions

✅ Asset Metadata
   - Texture paths
   - Sound references
   - Animation definitions

❌ NOT for:
   - Real-time game state
   - Network packets
   - Frequently saved data
```

---

## 2. Binary Packed - Game Saves & Performance

### Performance Metrics

```text
Test Data: 10,000 entities
Write Time: ~1.5 ms
Read Time:  ~87 μs
Total:      ~1.6 ms
File Size:  1.0 MB
```

**Performance vs JSON:**

- ✅ **5-20x faster** for writes
- ✅ **100-500x faster** for reads
- ✅ **5-6x smaller** file sizes

### Advantages ✅

**1. Extreme Performance**

```cpp
struct SaveGame {
    uint32_t version;
    uint32_t playerId;
    float x, y;
    uint32_t score;
    uint8_t health;
    // ... more fields
};

// Write (microseconds!)
SaveGame save{1, 42, 100.5f, 200.3f, 1000, 75};
std::ofstream file("save.dat", std::ios::binary);
file.write(reinterpret_cast<const char*>(&save), sizeof(save));

// Read (microseconds!)
SaveGame loaded;
std::ifstream in("save.dat", std::ios::binary);
in.read(reinterpret_cast<char*>(&loaded), sizeof(loaded));
```

**2. Compact File Size**

```text
10,000 entities:
Binary: 1.0 MB   ✅
JSON:   5.6 MB   (5.6x larger)
```

**3. Direct Memory Mapping**

```cpp
// Can even memory-map large files
#include <sys/mman.h>

int fd = open("entities.bin", O_RDONLY);
SaveData* data = (SaveData*)mmap(nullptr, fileSize, 
                                  PROT_READ, MAP_PRIVATE, fd, 0);
// Direct access, zero parsing!
```

### Disadvantages ❌

**1. Not Human Readable**

```bash
$ hexdump save.dat
0000000 0001 0000 002a 0000 0000 42c8 0000 4348
0000010 03e8 0000 004b 0000 ...
```

**2. Endianness Issues**

```cpp
// Must handle big-endian vs little-endian
uint32_t value = 0x12345678;

// Little-endian: 78 56 34 12
// Big-endian:    12 34 56 78

// Solution: Force network byte order
uint32_t netValue = htonl(value);
file.write(&netValue, 4);
```

**3. Versioning Challenges**

```cpp
// Adding a field breaks old saves!
struct SaveGameV1 {
    uint32_t version;
    uint32_t playerId;
    float x, y;
};

struct SaveGameV2 {
    uint32_t version;
    uint32_t playerId;
    float x, y;
    uint32_t score;  // NEW FIELD!
};

// Need explicit version handling
if (version == 1) {
    // Load old format, migrate data
} else if (version == 2) {
    // Load new format
}
```

### Recommended Uses ✅

```text
✅ Game Save Files
   - Player progress
   - World state
   - Inventory data

✅ Network Packets (as shown earlier)
   - Entity updates
   - Player inputs
   - Real-time data

✅ Cache Files
   - Compiled assets
   - Precomputed data

❌ NOT for:
   - Configuration (use JSON)
   - Debugging (not readable)
```

---

## 3. SQLite - Structured Data

### Performance Metrics

```text
Test Data: 10,000 records
Insert:  ~450 ms (with transaction)
Query:   ~15 ms (indexed)
File:    ~2.5 MB
```

### Advantages ✅

**1. SQL Queries**

```cpp
#include <sqlite3.h>

sqlite3* db;
sqlite3_open("highscores.db", &db);

// Create table
const char* createTable = R"(
    CREATE TABLE IF NOT EXISTS highscores (
        player_name TEXT,
        score INTEGER,
        level_reached INTEGER,
        timestamp INTEGER
    );
    CREATE INDEX idx_score ON highscores(score DESC);
)";
sqlite3_exec(db, createTable, nullptr, nullptr, nullptr);

// Insert
const char* insert = "INSERT INTO highscores VALUES (?, ?, ?, ?)";
sqlite3_stmt* stmt;
sqlite3_prepare_v2(db, insert, -1, &stmt, nullptr);
sqlite3_bind_text(stmt, 1, "Player1", -1, SQLITE_TRANSIENT);
sqlite3_bind_int(stmt, 2, 10000);
sqlite3_bind_int(stmt, 3, 5);
sqlite3_bind_int64(stmt, 4, time(nullptr));
sqlite3_step(stmt);
sqlite3_finalize(stmt);

// Query top 10
const char* query = "SELECT * FROM highscores ORDER BY score DESC LIMIT 10";
sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* name = (const char*)sqlite3_column_text(stmt, 0);
    int score = sqlite3_column_int(stmt, 1);
    std::cout << name << ": " << score << "\n";
}
```

**2. ACID Transactions**

```cpp
// Atomic updates
sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
// ... multiple inserts/updates
sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
// All or nothing!
```

**3. Complex Queries**

```sql
-- Player statistics
SELECT 
    player_name,
    COUNT(*) as games_played,
    AVG(score) as avg_score,
    MAX(score) as best_score,
    MAX(level_reached) as max_level
FROM highscores
WHERE timestamp > strftime('%s', 'now', '-7 days')
GROUP BY player_name
ORDER BY avg_score DESC;
```

### Disadvantages ❌

**1. Slower than Binary**

```text
10,000 inserts:
Binary:  ~1.5 ms   ✅
SQLite:  ~450 ms   (300x slower)
```

**2. Larger Files**

```text
10,000 records:
Binary:  1.0 MB
SQLite:  2.5 MB (with indexes)
```

**3. Overkill for Simple Data**

```cpp
// Simple key-value? Use JSON/Binary instead
{
    "player_name": "John",
    "high_score": 1000
}

// vs full database table
```

### Recommended Uses ✅

```text
✅ Highscore Tables
   - Leaderboards
   - Player rankings
   - Statistics

✅ Player Profiles
   - Account data
   - Achievements
   - Preferences

✅ Persistent Game State
   - Multiplayer lobbies
   - Match history
   - Session data

❌ NOT for:
   - Real-time game state
   - High-frequency updates
   - Simple key-value data
```

---

## Hybrid Strategy (Recommended)

### Use Case Matrix

| Data Type | Format | Why |
|-----------|--------|-----|
| **Game Configuration** | JSON | Human-readable, easy to edit |
| **Level Definitions** | JSON | Designers can modify, version control |
| **Player Save Files** | Binary | Fast load times, compact size |
| **Network Packets** | Binary | Minimal bandwidth, maximum speed |
| **Highscores** | SQLite | Complex queries, leaderboards |
| **Player Accounts** | SQLite | Relational data, authentication |
| **Debug Logs** | JSON | Human-readable troubleshooting |

### Implementation Example

```cpp
class GameDataManager {
    // JSON for config
    nlohmann::json config_;
    
    // Binary for saves
    struct SaveData { /* ... */ };
    
    // SQLite for scores
    sqlite3* scoreDB_;
    
public:
    // Load config (JSON)
    void loadConfig(const std::string& path) {
        std::ifstream file(path);
        config_ = nlohmann::json::parse(file);
    }
    
    // Save game (Binary)
    void saveGame(const SaveData& data) {
        std::ofstream file("save.dat", std::ios::binary);
        file.write(reinterpret_cast<const char*>(&data), sizeof(data));
    }
    
    // Load game (Binary)
    SaveData loadGame() {
        SaveData data;
        std::ifstream file("save.dat", std::ios::binary);
        file.read(reinterpret_cast<char*>(&data), sizeof(data));
        return data;
    }
    
    // Submit score (SQLite)
    void submitScore(const std::string& name, int score) {
        const char* sql = "INSERT INTO highscores (player_name, score) VALUES (?, ?)";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(scoreDB_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, score);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
};
```

---

## Performance Visualization

### Read Performance

```text
10,000 entities:

Binary:  ████                       87 μs  ✅
SQLite:  ████████████████████       15 ms
JSON:    █████████████████████████████████ 510 ms  ❌
```

### Write Performance

```text
10,000 entities:

Binary:  ████                      1.5 ms  ✅
JSON:    █████████████████████    320 ms
SQLite:  █████████████████████████ 450 ms  ❌
```

### File Size

```text
10,000 entities:

Binary:  ██                        1.0 MB  ✅
SQLite:  █████                     2.5 MB
JSON:    ███████████               5.6 MB  ❌
```

---

## Final Recommendation

✅ **Use Hybrid Strategy** for R-Type data persistence.

**Mapping:**

| Game System | Storage Format | Rationale |
|-------------|----------------|-----------|
| **Settings** | JSON | Easy editing, version control |
| **Levels** | JSON | Designer-friendly, readable |
| **Saves** | Binary | Fast loading (87μs), compact (1 MB) |
| **Network** | Binary | Minimal bandwidth, maximum speed |
| **Highscores** | SQLite | Queryable leaderboards |
| **Profiles** | SQLite | Relational player data |
| **Debug Logs** | JSON | Human-readable troubleshooting |

**Benefits:**

- ✅ **Right tool for each job**
- ✅ **Maximum performance where needed**
- ✅ **Human-friendly where useful**
- ✅ **Structured queries where required**

---

## References

- PoC implementations: `/PoC/PoC_DataStorage/`
- Strategy document: `/PoC/PoC_DataStorage/data_persistence_strategy.md`
- Binary PoC: `/PoC/PoC_DataStorage/BINARY/`
- JSON PoC: `/PoC/PoC_DataStorage/JSON/`
- SQLite PoC: `/PoC/PoC_DataStorage/SQLite/`
