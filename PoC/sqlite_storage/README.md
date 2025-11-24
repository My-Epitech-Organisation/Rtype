# SQLite Storage PoC for R-Type

## 📖 Overview

This Proof of Concept (PoC) demonstrates the integration of SQLite3 with the R-Type ECS (Entity Component System) framework. It tests the feasibility of using SQL databases for game data persistence.

## 🎯 Objectives

- ✅ Integrate SQLite3 library with the ECS framework
- ✅ Perform `SELECT * FROM highscores` query
- ✅ Test ECS-to-SQLite data persistence
- ✅ Benchmark performance
- ✅ Assess complexity and practicality

## 🔧 Prerequisites

### Required Dependencies

- **CMake** >= 3.20
- **C++ Compiler** with C++20 support (GCC 10+, Clang 12+)
- **SQLite3** development library
- **R-Type ECS Library** (built from `src/ECS/`)

### Installing SQLite3

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libsqlite3-dev sqlite3
```

#### Fedora/RHEL
```bash
sudo dnf install sqlite-devel sqlite
```

#### Arch Linux
```bash
sudo pacman -S sqlite
```

#### macOS
```bash
brew install sqlite3
```

## 🏗️ Building the PoC

### Step 1: Build the ECS Library

First, ensure the ECS library is built:

```bash
cd /home/samtess/Epitech/Tek3/R-Type/R-Type
mkdir -p build
cd build
cmake ..
make rtype_ecs_static
```

### Step 2: Build the SQLite PoC

```bash
cd ../PoC/sqlite_storage
mkdir -p build
cd build
cmake ..
make
```

## 🚀 Running the PoC

```bash
./sqlite_storage_poc
```

## 📊 Expected Output

The PoC will:

1. **Initialize Database**
   - Create `rtype_highscores.db` file
   - Create `highscores` table schema

2. **Insert Sample Data**
   - Add 5 sample highscore records

3. **Perform SELECT * Query**
   - Retrieve all highscores from database
   - Display in formatted table

4. **Test ECS Integration**
   - Create player entities in ECS
   - Persist to SQLite database

5. **Run Performance Benchmark**
   - Insert 1000 records
   - Measure query performance

### Sample Output
```
╔═══════════════════════════════════════════════════════════════╗
║     SQLite Storage PoC for R-Type - Using ECS Framework      ║
╚═══════════════════════════════════════════════════════════════╝
✅ Database opened successfully: rtype_highscores.db
✅ Table 'highscores' created/verified successfully

🧹 Clearing previous highscores...

📝 Inserting sample highscores...

🔍 Performing SELECT * FROM highscores...
✅ Retrieved 5 highscore records

╔══════════════════════════════════════════════════════════════════════════════╗
║                             🏆 HIGH SCORES 🏆                                 ║
╠═════╦════════════════════╦═══════════╦═══════╦═════════════════════════════╣
║ ID  ║ Player Name        ║ Score     ║ Level ║ Date                        ║
╠═════╬════════════════════╬═══════════╬═══════╬═════════════════════════════╣
║ 1   ║ Sarah Williams     ║ 32000     ║ 10    ║ Sun Nov 24 12:34:56 2025    ║
║ 2   ║ Jane Smith         ║ 25000     ║ 8     ║ Sun Nov 24 12:34:56 2025    ║
║ 3   ║ Mike Johnson       ║ 18000     ║ 5     ║ Sun Nov 24 12:34:56 2025    ║
╚═════╩════════════════════╩═══════════╩═══════╩═════════════════════════════╝

📦 Testing ECS Integration with SQLite...
✅ Created 3 player entities in ECS
✅ Saved 3 player scores to SQLite database

⚡ Running Performance Benchmark...
📝 Inserted 1000 records in 2453 ms
   Average: 2.453 ms per insert
🔍 Retrieved 1008 records in 423 μs (0.423 ms)

✅ PoC completed successfully!
```

## 📁 Generated Files

- `rtype_highscores.db` - SQLite database file with highscore data
- Stored in the build directory where the PoC is executed

## 🔍 Inspecting the Database

You can inspect the generated database using the SQLite command-line tool:

```bash
# Open the database
sqlite3 rtype_highscores.db

# View schema
.schema

# Query highscores
SELECT * FROM highscores ORDER BY score DESC LIMIT 10;

# Exit
.quit
```

## 📝 Code Structure

```
PoC/sqlite_storage/
├── CMakeLists.txt           # Build configuration
├── main.cpp                 # PoC implementation
├── README.md                # This file
└── SQLITE_ANALYSIS.md       # Pros/Cons analysis document
```

## 🧪 What This PoC Tests

### ✅ Successful Tests

1. **SQLite Integration** - Library compiles and links correctly
2. **Database Operations** - CREATE, INSERT, SELECT work as expected
3. **ECS Compatibility** - Can extract data from ECS and store in SQLite
4. **Query Performance** - Measured baseline performance metrics
5. **Data Persistence** - Database survives application restarts

### 📊 Key Findings

- **Integration Complexity**: ⚠️ HIGH - Requires schema management, error handling
- **Performance**: ⚠️ SLOW for real-time (1-3ms per insert vs ECS's nanoseconds)
- **Use Case**: ✅ GOOD for highscores, profiles (not real-time gameplay)

## 📚 Related Documentation

- **Analysis Document**: [SQLITE_ANALYSIS.md](./SQLITE_ANALYSIS.md)
- **ECS Documentation**: [../../doc/ecs/README.md](../../doc/ecs/README.md)
- **Related Issue**: #54 - Data Storage PoC

## 🎯 Exit Criteria Status

- ✅ SQLite3 integrated successfully
- ✅ `SELECT * FROM highscores` executed and working
- ✅ Code snippet provided (main.cpp)
- ✅ Complexity assessment: **HIGH** (see SQLITE_ANALYSIS.md)
- ✅ Timebox: Completed within 28-29/11/2025

## 🏁 Conclusion

SQLite integration **works correctly** but is **not recommended for core gameplay data** due to:
- High performance overhead (milliseconds vs nanoseconds)
- Architectural mismatch with ECS
- Added complexity and maintenance burden

**Recommended use**: Highscores, player profiles, match history (non-real-time data only).

See [SQLITE_ANALYSIS.md](./SQLITE_ANALYSIS.md) for detailed pros/cons analysis.

## 👥 Authors

R-Type Development Team

## 📅 Date

November 28-29, 2025
