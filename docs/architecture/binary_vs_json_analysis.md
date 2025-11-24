# Binary Packed Storage vs JSON - Proof of Concept

## 📋 Overview

This Proof of Concept (PoC) benchmarks the performance difference between **binary packed storage** and **JSON serialization** for ECS (Entity Component System) data persistence in the R-Type project.

## 🎯 Objective

To answer the question: **Is raw binary faster than JSON for game data storage?**

## 🔬 Methodology

### Test Structure

The benchmark compares two serialization approaches:

1. **Binary Packed Storage**: Using `fwrite`/`fread` with C++ structs written directly to disk
2. **JSON Storage**: Using the `nlohmann/json` library for human-readable serialization

### Test Data

The PoC simulates typical ECS components:

- **Position**: 2D coordinates (8 bytes)
- **Velocity**: Movement vector (8 bytes)
- **Health**: Current and maximum health (8 bytes)
- **Sprite**: Texture path, layer, and scale (72 bytes)

Each entity contains all components, totaling **~100 bytes per entity**.

### Benchmark Parameters

- **Entity counts**: 100, 1,000, 10,000, 50,000
- **Iterations**: 20-100 per test (depending on dataset size)
- **Metrics measured**:
  - Write performance (μs)
  - Read performance (μs)
  - File size (bytes)
  - Total round-trip time

## 📊 Expected Results

### Performance

| Metric | Binary | JSON | Winner |
|--------|--------|------|--------|
| **Write Speed** | ⚡ Very Fast | 🐌 Slower | Binary |
| **Read Speed** | ⚡ Very Fast | 🐌 Slower | Binary |
| **File Size** | 📦 Compact | 📚 Verbose | Binary |

**Typical speedup**: Binary is expected to be **5-20x faster** than JSON for both read and write operations.

### File Size Comparison

For 10,000 entities:
- **Binary**: ~1 MB (packed struct data)
- **JSON**: ~3-5 MB (text-based with formatting)

**Compression ratio**: JSON files are typically **3-5x larger** than binary.

## ✅ Pros and Cons

### Binary Packed Storage

#### ✅ Advantages

1. **Performance**
   - Extremely fast read/write operations (direct memory copy)
   - Zero parsing overhead
   - Minimal CPU usage
   - Excellent for real-time game state saves

2. **Size Efficiency**
   - Compact representation
   - No metadata overhead
   - Reduced disk I/O
   - Lower bandwidth for network transmission

3. **Memory Efficiency**
   - Direct struct mapping to disk
   - No intermediate string conversions
   - Cache-friendly sequential reads

4. **Predictability**
   - Fixed-size records enable random access
   - Constant-time seeks to specific entities
   - Deterministic performance

#### ❌ Disadvantages

1. **Portability Issues**
   - Platform-dependent (endianness, padding)
   - Struct layout differences between compilers
   - Version compatibility challenges

2. **No Human Readability**
   - Cannot inspect files with text editors
   - Debugging requires specialized tools
   - Harder to manually edit test data

3. **Fragility**
   - Changing struct layout breaks existing files
   - No built-in versioning
   - Requires explicit migration strategies

4. **Type Safety**
   - No validation during deserialization
   - Corrupt data may crash the application
   - Harder to detect data corruption

5. **Flexibility**
   - Cannot easily add optional fields
   - Removing fields requires rewriting entire file
   - Schema evolution is complex

### JSON Storage

#### ✅ Advantages

1. **Human Readability**
   - Easy to inspect and debug
   - Can be edited manually for testing
   - Great for configuration files
   - Version control friendly (git diffs work well)

2. **Flexibility**
   - Schema can evolve naturally
   - Optional fields are trivial
   - Backward/forward compatibility easier
   - Can add metadata effortlessly

3. **Portability**
   - Platform-independent format
   - Language-agnostic
   - Standard libraries available everywhere
   - No endianness issues

4. **Validation**
   - Can use JSON schemas for validation
   - Libraries provide error messages
   - Easier to catch malformed data

5. **Ecosystem**
   - Many tools available (viewers, validators)
   - Wide industry adoption
   - Excellent library support

#### ❌ Disadvantages

1. **Performance**
   - Parsing overhead (tokenization, validation)
   - String to numeric conversions
   - Memory allocations during parsing
   - 5-20x slower than binary

2. **File Size**
   - Text representation is verbose
   - Repeated key names
   - Formatting characters (whitespace, quotes)
   - 3-5x larger than binary

3. **Memory Overhead**
   - Requires intermediate data structures
   - String allocations during serialization
   - Higher peak memory usage

## 🎮 Recommendations for R-Type

### Use Binary Packed Storage For:

- **Game State Snapshots**: Quick saves during gameplay
- **Network Packets**: Minimal latency communication
- **Large Datasets**: Bulk entity serialization (level data with 10,000+ entities)
- **Replay Systems**: High-frequency state recording
- **Performance-Critical Paths**: Loading screens, checkpoints

### Use JSON For:

- **Configuration Files**: Game settings, key bindings, graphics options
- **Level Editors**: Human-editable level definitions
- **Save Game Metadata**: Player profiles, achievements, statistics
- **Debugging Tools**: Inspector data, telemetry logs
- **Modding Support**: User-generated content definitions

### Hybrid Approach (Recommended)

For the best of both worlds:

```cpp
// Configuration (JSON)
config.json          // Human-editable settings

// Game State (Binary)
autosave.bin         // Fast checkpoint saves
level_data.bin       // Bulk entity data

// Debugging (JSON)
debug_snapshot.json  // Inspectable state for developers
```

## 🏗️ Implementation Considerations

### Binary Storage Best Practices

1. **Add Version Headers**: Include format version for migration
2. **Use Fixed-Size Buffers**: Avoid variable-length data
3. **Consider Endianness**: Use standard byte order (network order)
4. **Checksum/CRC**: Detect corrupted files
5. **Compression**: Add zlib/zstd for smaller files

### JSON Storage Best Practices

1. **Schema Validation**: Use JSON schema to validate structure
2. **Minimize Nesting**: Keep structure flat for performance
3. **Use Compact Format**: Disable pretty-printing for production
4. **Stream Parsing**: Use SAX-style parsers for large files
5. **Caching**: Parse once, keep in memory

## 🔧 Building and Running the PoC

```bash
# Install dependencies
sudo apt-get install nlohmann-json3-dev  # Ubuntu/Debian
# OR
brew install nlohmann-json               # macOS

# Build
cd R-Type
mkdir -p build && cd build
cmake ..
make binary_vs_json_storage

# Run benchmark
./PoC/binary_vs_json_storage
```

## 📈 Interpreting Results

The benchmark outputs:
- **Write/Read times** in microseconds (μs)
- **Speedup factors** (how many times faster binary is)
- **File sizes** in bytes
- **Total round-trip time** (write + read)

Look for:
- Consistent speedup across different dataset sizes
- File size ratios
- Performance scaling with entity count

## 🔗 Related Documentation

- [Spike] [Main] Data Storage PoC #54
- ECS Serialization Documentation: `doc/ecs/12_serialization.md`

## ⏱️ Timeline

- **Start**: 28/11/2025
- **End**: 29/11/2025
- **Status**: ✅ Complete

## 📝 Conclusion

Binary packed storage offers **significant performance advantages** (5-20x faster) and **smaller file sizes** (3-5x reduction) compared to JSON. However, JSON provides **better flexibility, portability, and debugging capabilities**.

**For R-Type**, we recommend:
- **Binary** for performance-critical game state persistence
- **JSON** for configuration and human-editable content
- **Hybrid approach** to leverage the strengths of both formats

The choice depends on the specific use case: prioritize performance for runtime data and convenience for development/configuration data.

## Results Summary

╔════════════════════════════════════════════════╗
║  Binary Packed vs JSON Storage Benchmark       ║
║  R-Type ECS Data Serialization PoC             ║
╚════════════════════════════════════════════════╝

========================================
Benchmark: 100 entities
Iterations: 100
========================================

--- WRITE PERFORMANCE ---
Binary:
  Avg: 192.371 μs
  Min: 132.819 μs
  Max: 936.307 μs

JSON:
  Avg: 3147.62 μs
  Min: 3019.41 μs
  Max: 5163.36 μs

Speedup: 16.3623x

--- READ PERFORMANCE ---
Binary:
  Avg: 10.1805 μs
  Min: 6.863 μs
  Max: 51.226 μs

JSON:
  Avg: 4873.76 μs
  Min: 4779.88 μs
  Max: 5195.72 μs

Speedup: 478.735x

--- FILE SIZE ---
Binary: 10004 bytes
JSON:   55815 bytes
Ratio:  5.57927x larger

--- TOTAL TIME (Write + Read) ---
Binary: 202.551 μs
JSON:   8021.37 μs
Speedup: 39.6017x

========================================
Benchmark: 1000 entities
Iterations: 100
========================================

--- WRITE PERFORMANCE ---
Binary:
  Avg: 274.747 μs
  Min: 116.369 μs
  Max: 1038.69 μs

JSON:
  Avg: 31373.3 μs
  Min: 30392.9 μs
  Max: 34943.8 μs

Speedup: 114.19x

--- READ PERFORMANCE ---
Binary:
  Avg: 15.3396 μs
  Min: 13.565 μs
  Max: 52.538 μs

JSON:
  Avg: 49760 μs
  Min: 47858.5 μs
  Max: 58352.9 μs

Speedup: 3243.9x

--- FILE SIZE ---
Binary: 100004 bytes
JSON:   558911 bytes
Ratio:  5.58889x larger

--- TOTAL TIME (Write + Read) ---
Binary: 290.086 μs
JSON:   81133.2 μs
Speedup: 279.687x

========================================
Benchmark: 10000 entities
Iterations: 50
========================================

--- WRITE PERFORMANCE ---
Binary:
  Avg: 1477.98 μs
  Min: 661.432 μs
  Max: 8039.38 μs

JSON:
  Avg: 322740 μs
  Min: 317628 μs
  Max: 335069 μs

Speedup: 218.366x

--- READ PERFORMANCE ---
Binary:
  Avg: 87.4094 μs
  Min: 50.725 μs
  Max: 411.142 μs

JSON:
  Avg: 510453 μs
  Min: 496991 μs
  Max: 556806 μs

Speedup: 5839.8x

--- FILE SIZE ---
Binary: 1000004 bytes
JSON:   5600013 bytes
Ratio:  5.59999x larger

--- TOTAL TIME (Write + Read) ---
Binary: 1565.39 μs
JSON:   833193 μs
Speedup: 532.259x

========================================
Benchmark: 50000 entities
Iterations: 20
========================================

--- WRITE PERFORMANCE ---
Binary:
  Avg: 5784.38 μs
  Min: 3534.42 μs
  Max: 10036.2 μs

JSON:
  Avg: 1.86998e+06 μs
  Min: 1.66637e+06 μs
  Max: 2.03027e+06 μs

Speedup: 323.28x

--- READ PERFORMANCE ---
Binary:
  Avg: 464.444 μs
  Min: 309.421 μs
  Max: 1576.16 μs

JSON:
  Avg: 2.63831e+06 μs
  Min: 2.53307e+06 μs
  Max: 3.00861e+06 μs

Speedup: 5680.58x

--- FILE SIZE ---
Binary: 5000004 bytes
JSON:   28044277 bytes
Ratio:  5.60885x larger

--- TOTAL TIME (Write + Read) ---
Binary: 6248.83 μs
JSON:   4.50829e+06 μs
Speedup: 721.461x

========================================
Benchmark Complete!
========================================