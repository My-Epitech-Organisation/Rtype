# Proof of Concepts (PoC)

This directory contains various Proof of Concepts for the R-Type project.

## 📦 Available PoCs

### Binary vs JSON Storage

**File**: `binary_vs_json_storage.cpp`

Benchmarks the performance difference between binary packed storage and JSON serialization for ECS data persistence.

**Key Questions**:
- Is raw binary faster than JSON?
- What are the file size differences?
- When should we use each format?

**Documentation**: See [`../doc/poc_binary_vs_json_storage.md`](../doc/poc_binary_vs_json_storage.md)

**Build & Run**:
```bash
cd build
cmake ..
make binary_vs_json_storage
./PoC/binary_vs_json_storage
```

**Dependencies**:
- nlohmann_json (3.11.0+)
- C++20 compiler

## 🎯 Purpose

PoCs help us:
- Validate technical decisions
- Compare alternative approaches
- Measure performance characteristics
- Reduce implementation risks

## 📋 PoC Guidelines

When creating a new PoC:

1. **Define clear questions**: What are you testing?
2. **Set a timebox**: 1-3 days maximum
3. **Measure objectively**: Include benchmarks
4. **Document findings**: Create a markdown doc
5. **Share results**: Present to the team

## 🔗 Related Issues

- [Spike] [Main] Data Storage PoC #54
- Binary Packed Storage PoC #55

## 📚 Structure

```
PoC/
├── README.md                      # This file
├── CMakeLists.txt                 # Build configuration
├── binary_vs_json_storage.cpp     # Binary vs JSON benchmark
└── ...                            # Future PoCs
```

## 🚀 Adding New PoCs

1. Create your `.cpp` file in this directory
2. Add build target to `CMakeLists.txt`
3. Create documentation in `../doc/`
4. Update this README with your PoC info
5. Link related issues

Happy experimenting! 🧪
