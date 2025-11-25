# Qt Network PoC

## 🎯 Objective
Test **Qt Network (Qt6)** for UDP networking and determine:
- Does Qt Network force us to use Qt for everything?
- Is QCoreApplication required for server loop?
- Performance comparison with other solutions

## 🔬 Key Findings

### ⚠️ QCoreApplication IS REQUIRED
Qt Network **requires** `QCoreApplication` to run the event loop. This means:
- ❌ Cannot use Qt Network without Qt framework
- ❌ Server must instantiate QCoreApplication
- ❌ Ties the entire application to Qt's event system
- ✅ But provides cross-platform networking
- ✅ Signal/slot mechanism for async I/O

## 📊 Benchmark Results

Run the test script to generate benchmark data:
```bash
cd /path/to/Rtype
chmod +x PoC/PoC_Network_Libs/qt_network_poc/test_poc.sh
./PoC/PoC_Network_Libs/qt_network_poc/test_poc.sh
```

Results are saved to `benchmark_results.csv` with:
- Configuration time
- Build time
- Server binary size
- Client binary size
- **QCoreApplication Required: YES**

## 🔧 Manual Build

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev

# Fedora
sudo dnf install qt6-qtbase-devel

# Arch
sudo pacman -S qt6-base
```

### Build Commands
```bash
mkdir -p build-qt
cd build-qt
cmake .. -DBUILD_EXAMPLES=ON -DBUILD_TESTS=OFF
cmake --build . -- -j
```

## 🧪 Manual Testing

### Start Server
```bash
./qt_udp_server 4242
```

### Run Client (in another terminal)
```bash
./qt_udp_client 127.0.0.1 4242
```

## 📦 Dependencies
- Qt6 Core
- Qt6 Network
- CMake 3.16+
- C++20 compiler

## 🆚 Comparison with Other Solutions

| Metric | Standalone ASIO | Boost.Asio | Qt Network |
|--------|-----------------|------------|------------|
| Config Time | ⏱️ See results | ⏱️ See results | ⏱️ See results |
| Build Time | ⏱️ See results | ⏱️ See results | ⏱️ See results |
| Server Size | 📦 See results | 📦 See results | 📦 See results |
| Client Size | 📦 See results | 📦 See results | 📦 See results |
| Event Loop | Optional | Optional | **REQUIRED** |
| Framework Dependency | None | None | **Qt Framework** |
| Cross-Platform | Yes | Yes | Yes |
| Learning Curve | Medium | Medium | High |

## ✅ Exit Criteria
- [x] Successful compilation with Qt Network
- [x] Functional UDP server/client
- [x] Benchmark data collected (CSV)
- [x] Determine QCoreApplication requirement

## ⚠️ Critical Limitation

**Qt Network CANNOT be used standalone.**

The server **must** use:
```cpp
QCoreApplication app(argc, argv);
// ... setup server
return app.exec(); // Qt event loop required
```

This means:
- The entire application becomes tied to Qt
- Cannot mix Qt Network with non-Qt game logic easily
- Increases binary size due to Qt dependencies
- Forces Qt's programming model (signals/slots)

## 🎓 Recommendation

**NOT RECOMMENDED** for this project because:
1. ❌ Forces Qt framework dependency on entire codebase
2. ❌ QCoreApplication mandatory = tied to Qt event loop
3. ❌ Larger binary sizes
4. ❌ Would require Qt for game logic/rendering too
5. ❌ Overkill for simple UDP networking

**Better alternatives:**
- ✅ Standalone ASIO (lightweight, no dependencies)
- ✅ Boost.Asio (mature, well-documented)
