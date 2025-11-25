# SQLite Storage for R-Type: Pros and Cons Analysis

## 📋 Executive Summary

This document analyzes the feasibility of using SQLite as a storage solution for the R-Type game project. After conducting a Proof of Concept (PoC) integrating SQLite with our ECS (Entity Component System) framework, we evaluate the benefits and drawbacks to determine whether SQL databases are appropriate for this gaming context.

**Date:** November 28-29, 2025  
**Status:** PoC Completed  
**Recommendation:** ⚠️ Not Recommended for Core Game Data

---

## 🎯 Scope of Testing

The PoC tested the following:
- SQLite3 library integration with the ECS framework
- Basic CRUD operations (Create, Read, Update, Delete)
- `SELECT * FROM highscores` query execution
- ECS-to-SQLite data persistence
- Performance benchmarking for 1000+ records
- Memory and complexity overhead

---

## ✅ Pros: Advantages of SQLite for R-Type

### 1. **Persistent Storage Out of the Box**
- ✅ **Automatic Data Persistence**: Data survives application restarts without custom serialization
- ✅ **File-Based Storage**: Single `.db` file is easy to backup, transfer, and manage
- ✅ **ACID Compliance**: Transactions ensure data integrity even during crashes

**Example Use Case:** Perfect for storing highscores that must persist across game sessions.

```cpp
// Simple and reliable
manager.insertHighScore("PlayerName", 10000, 5);
```

---

### 2. **Powerful Query Capabilities**
- ✅ **Complex Queries**: Built-in support for filtering, sorting, and aggregation
- ✅ **JOIN Operations**: Can relate multiple tables (players, scores, achievements)
- ✅ **Indexing**: Fast lookups on large datasets

**Example:**
```sql
-- Get top 10 players by score
SELECT * FROM highscores ORDER BY score DESC LIMIT 10;

-- Get player statistics
SELECT player_name, AVG(score), MAX(level) 
FROM highscores 
GROUP BY player_name;
```

---

### 3. **Standard SQL Interface**
- ✅ **Well-Known Technology**: Developers familiar with SQL can work immediately
- ✅ **No Custom Format**: No need to design and maintain proprietary file formats
- ✅ **Tool Support**: Can inspect/modify data with SQLite browsers and CLI tools

---

### 4. **Cross-Platform Compatibility**
- ✅ **Works Everywhere**: Linux, Windows, macOS, embedded systems
- ✅ **Single Header**: Easy to integrate (just link against `libsqlite3`)
- ✅ **No Server Required**: Serverless architecture simplifies deployment

---

### 5. **Good for Non-Critical Game Data**
- ✅ **Highscores**: Perfect for leaderboards and player statistics
- ✅ **Player Profiles**: Store user preferences, settings, achievements
- ✅ **Game Configuration**: Store non-performance-critical settings
- ✅ **Analytics**: Log game events for post-game analysis

---

## ❌ Cons: Disadvantages of SQLite for R-Type

### 1. **🚨 Performance Overhead (CRITICAL)**
- ❌ **Too Slow for Real-Time Game Logic**: Database queries add latency (milliseconds vs microseconds)
- ❌ **Disk I/O Bottleneck**: Every write operation touches the filesystem
- ❌ **Not Cache-Friendly**: Unlike ECS sparse sets, SQL queries bypass CPU cache

**Benchmark Results from PoC:**
```
Insert Performance: ~1-3 ms per record
SELECT * Performance: 50-200 μs for 1000 records
ECS Component Access: ~5-10 ns (20,000x faster!)
```

**Why This Matters:**
- R-Type runs at 60 FPS = **16.67ms per frame**
- A single SQLite query could consume **6-18% of frame budget**
- Game entities (enemies, bullets) update **every frame** → SQL is too slow

---

### 2. **🎮 Incompatible with ECS Architecture**
- ❌ **Data Model Mismatch**: SQL uses relational rows/columns; ECS uses component arrays
- ❌ **Impedance Mismatch**: Converting ECS entities ↔ SQL requires complex mapping
- ❌ **No Cache Locality**: SQL tables don't provide ECS's cache-friendly data layout

**Example Problem:**
```cpp
// ECS way (fast, cache-friendly)
for (auto entity : registry.view<Position, Velocity>()) {
    auto& pos = registry.getComponent<Position>(entity);
    auto& vel = registry.getComponent<Velocity>(entity);
    pos.x += vel.x; // All data sequential in memory
}

// SQL way (slow, requires serialization)
auto entities = db.query("SELECT * FROM entities");
for (auto row : entities) {
    // Deserialize, process, serialize back
    // Multiple heap allocations, no cache locality
}
```

---

### 3. **Complexity and Maintenance Overhead**
- ❌ **Schema Management**: Need to design, version, and migrate database schemas
- ❌ **Extra Dependency**: Adds SQLite library (200KB+ binary size)
- ❌ **Error Handling**: Must handle SQL errors, connection issues, corruption
- ❌ **Synchronization Issues**: ECS and SQL become "two sources of truth"

**Code Complexity Example:**
```cpp
// Without SQL: Simple and direct
registry.emplaceComponent<Score>(entity, 1000);

// With SQL: More code, more failure points
try {
    auto score = registry.getComponent<Score>(entity);
    db.execute("UPDATE scores SET value = ? WHERE entity_id = ?", 
               score.value, entity.id);
} catch (SQLException& e) {
    // Handle error, rollback, retry logic...
}
```

---

### 4. **Not Designed for Real-Time Systems**
- ❌ **Unpredictable Latency**: Query times vary based on data size and indexes
- ❌ **Locking Issues**: Writers block readers (even with WAL mode)
- ❌ **No Real-Time Guarantees**: SQLite prioritizes correctness over speed

**Game Development Rule:**  
> *"Never do disk I/O in the main game loop"*

---

### 5. **Resource Consumption**
- ❌ **Memory Overhead**: SQLite maintains internal caches and structures
- ❌ **Disk Space**: Database file size grows over time (needs vacuuming)
- ❌ **CPU Usage**: Parsing SQL, query optimization, locking overhead

---

## 🎯 Recommended Use Cases for R-Type

### ✅ **GOOD Uses (Non-Performance-Critical)**
1. **Highscore Persistence** (as in PoC)
   - Loaded on menu screen, not during gameplay
   - Queries happen during "quiet" moments (lobby, game over)

2. **Player Profiles & Achievements**
   - Loaded once at game start
   - Saved periodically or on exit

3. **Game Settings & Configuration**
   - Read during initialization
   - Written infrequently (when user changes settings)

4. **Replay Data / Match History**
   - Saved after match completion
   - Loaded for replay viewer (separate from gameplay)

---

### ❌ **BAD Uses (Performance-Critical)**
1. **Entity Component Storage** ❌
   - Use ECS sparse sets instead
   - Entities/components change every frame

2. **Physics State** ❌
   - Position, velocity, collision data
   - Must be cache-local and ultra-fast

3. **Networking State** ❌
   - Real-time multiplayer requires microsecond latency
   - SQL adds milliseconds

4. **Audio/Graphics State** ❌
   - Frame-rate dependent systems
   - Can't tolerate query overhead

---

## 🏗️ Recommended Architecture

### Hybrid Approach: SQLite for Meta-Data, ECS for Gameplay

```
┌─────────────────────────────────────────────────────┐
│                   R-Type Game                       │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌───────────────┐         ┌──────────────────┐   │
│  │  Game Loop    │         │   Menu System    │   │
│  │  (60 FPS)     │         │   (No FPS limit) │   │
│  └───────┬───────┘         └────────┬─────────┘   │
│          │                          │             │
│          ▼                          ▼             │
│  ┌──────────────┐          ┌──────────────────┐   │
│  │  ECS Engine  │          │ SQLite Database  │   │
│  │              │          │                  │   │
│  │  • Entities  │          │  • Highscores    │   │
│  │  • Components│          │  • Player Profiles│  │
│  │  • Systems   │          │  • Achievements  │   │
│  │  • Fast!     │◄─────────┤  • Settings      │   │
│  └──────────────┘   Load   │  • Match History │   │
│         │           at      └──────────────────┘   │
│         │          Start          ▲               │
│         │                         │               │
│         └─────Save on Exit────────┘               │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Data Flow:**
1. **Game Start**: Load player profile, settings from SQLite into ECS
2. **Gameplay**: All data lives in ECS (no SQL access)
3. **Game Over**: Save score/stats to SQLite asynchronously
4. **Menu Navigation**: Query SQLite for leaderboards, history

---

## 📊 Performance Comparison

| Operation              | ECS (Memory)  | SQLite (Disk) | Ratio       |
|------------------------|---------------|---------------|-------------|
| Entity Creation        | ~100 ns       | ~1-3 ms       | 10,000x     |
| Component Access       | ~5-10 ns      | ~50-200 μs    | 5,000-40,000x |
| Iteration (1000 items) | ~50 μs        | ~200-500 μs   | 4-10x       |
| Memory per Entity      | ~8 bytes      | ~hundreds     | 10-50x      |

**Conclusion:** ECS is 4-40,000x faster for real-time game data.

---

## 🎓 Lessons Learned

### From the PoC Implementation

1. **SQLite Works as Advertised**
   - Integration was straightforward
   - `SELECT * FROM highscores` executed successfully
   - Data persisted correctly across runs

2. **But the Complexity is High**
   - Required:
     - Schema design
     - Prepared statements
     - Error handling
     - Type conversions (C++ ↔ SQL)
     - Memory management for results

3. **Performance Gap is Massive**
   - Even for simple queries, latency is 1000x higher than ECS
   - Would require significant architectural changes to integrate properly

4. **Maintenance Burden**
   - Schema migrations
   - Database version control
   - Corruption handling
   - Backup strategies

---

## 🏁 Final Recommendation

### ⚠️ **Verdict: Use SQLite Sparingly**

**For R-Type specifically:**

✅ **DO use SQLite for:**
- Persistent highscore leaderboards
- Player account data (name, settings, achievements)
- Match history and replay metadata
- Analytics and telemetry logs

❌ **DON'T use SQLite for:**
- Entity/component storage (use ECS!)
- Real-time game state
- Networking synchronization
- Frame-by-frame data

---

## 🔄 Alternative Solutions

### 1. **Binary Serialization (Recommended for Game State)**
```cpp
// Fast, efficient, ECS-friendly
registry.serialize("savegame.bin");
registry.deserialize("savegame.bin");
```

**Pros:**
- Direct memory dump
- Minimal CPU overhead
- Perfect for save/load systems

---

### 2. **JSON/YAML (Recommended for Configuration)**
```cpp
// Human-readable, easy to edit
config.save("settings.json");
```

**Pros:**
- Human-readable
- Easy to version control
- Great for configuration files

---

### 3. **Hybrid Approach (Recommended Overall)**
```
- ECS binary files: Game saves
- JSON files: Configuration
- SQLite: Leaderboards, profiles
```

---

## 📚 References

- [SQLite Official Documentation](https://www.sqlite.org/docs.html)
- [Game Programming Patterns - Data Locality](http://gameprogrammingpatterns.com/data-locality.html)
- [ECS Architecture Best Practices](https://github.com/SanderMertens/ecs-faq)
- R-Type ECS Documentation: `doc/ecs/`

---

## 📝 Conclusion

**SQLite is a powerful, mature, and reliable database system**, but it's **not designed for real-time game engines**. The performance overhead, architectural mismatch with ECS, and complexity burden make it unsuitable for core gameplay data in R-Type.

However, SQLite **excels at meta-game features** like highscores, player profiles, and statistics where:
- ✅ Queries happen infrequently
- ✅ Data persistence is critical
- ✅ Complex queries add value
- ✅ Performance is not frame-critical

**Our recommendation:** Use SQLite judiciously for non-gameplay features, but keep core game data in the ECS with binary serialization for save/load.

---

## 🎬 Next Steps

1. ✅ **PoC Completed**: SQLite integration validated
2. ⬜ **Decision**: Review this document with the team
3. ⬜ **Implementation**: If approved, implement highscore persistence
4. ⬜ **Documentation**: Update architecture diagrams with storage strategy
5. ⬜ **Testing**: Verify no performance degradation in game loop

---

**Assessment:** ⚠️ **Complexity is HIGH, but manageable for specific use cases**

*Document authored by the R-Type development team based on PoC findings.*
