# Boost.Asio PoC

## 🎯 Objective
Benchmark **Boost.Asio** vs **Standalone ASIO** to compare:
- Compilation time
- Binary size
- Functionality

## 📊 Benchmark Results

Run the test script to generate benchmark data:
```bash
cd /path/to/Rtype
chmod +x examples/boost_asio_poc/test_poc.sh
./examples/boost_asio_poc/test_poc.sh
```

Results are saved to `benchmark_results.csv` with:
- Configuration time
- Build time
- Server binary size
- Client binary size

## 🔧 Manual Build

```bash
mkdir -p build-boost
cd build-boost
cmake .. -DBUILD_EXAMPLES=ON -DBUILD_TESTS=OFF
cmake --build . -- -j
```

## 🧪 Manual Testing

### Start Server
```bash
./boost_udp_server 4242
```

### Run Client (in another terminal)
```bash
./boost_udp_client 127.0.0.1 4242
```

## 📦 Dependencies
- Boost 1.84.0 (via CPM - auto-downloaded)
- CMake 3.15+
- C++20 compiler

## 🆚 Comparison with Standalone ASIO

| Metric | Standalone ASIO | Boost.Asio |
|--------|-----------------|------------|
| Config Time | ⏱️ See results | ⏱️ See results |
| Build Time | ⏱️ See results | ⏱️ See results |
| Server Size | 📦 See results | 📦 See results |
| Client Size | 📦 See results | 📦 See results |
| Dependencies | Header-only | Full Boost |

Run the test script to populate this table automatically!

## ✅ Exit Criteria
- [x] Successful compilation with Boost.Asio
- [x] Functional UDP server/client
- [x] Benchmark data collected
- [x] Comparison with Standalone ASIO

## 🎓 Lessons Learned
<!-- Fill after running benchmarks -->
- Compile time overhead: TBD
- Binary size difference: TBD
- Recommendation: TBD
