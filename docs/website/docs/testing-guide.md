---
sidebar_position: 12
sidebar_label: Testing Guide
---

# 🧪 Testing Guide

Comprehensive guide to testing in the R-Type project.

## 📋 Overview

R-Type uses a multi-layered testing approach:
- **Unit Tests**: Test individual components and systems
- **Integration Tests**: Test interaction between modules
- **System Tests**: Test full client-server scenarios
- **Performance Tests**: Measure and validate performance

### Testing Framework

- **Google Test** (gtest): Unit and integration testing
- **Google Mock** (gmock): Mocking dependencies
- **Custom Test Harnesses**: Network and ECS testing

---

## 🏗️ Test Structure

```
tests/
├── common/           # Common utilities tests
│   ├── test_logger.cpp
│   └── test_argparser.cpp
├── ecs/              # ECS framework tests
│   ├── test_registry.cpp
│   ├── test_sparse_set.cpp
│   └── test_view.cpp
├── network/          # Network layer tests
│   ├── test_packet.cpp
│   ├── test_serializer.cpp
│   └── test_udp_socket.cpp
├── engine/           # Engine tests
│   ├── test_system.cpp
│   └── test_scene.cpp
├── games/            # Game logic tests
│   └── rtype/
│       ├── test_player.cpp
│       ├── test_enemy.cpp
│       └── test_collision.cpp
├── integration/      # Integration tests
│   ├── test_client_server.cpp
│   ├── test_multiplayer.cpp
│   └── test_network_sync.cpp
└── performance/      # Performance benchmarks
    ├── bench_ecs.cpp
    └── bench_network.cpp
```

---

## ✅ Running Tests

### Run All Tests

```bash
# Build tests
cmake --preset linux-debug
cmake --build build --target tests

# Run all tests
ctest --test-dir build

# With verbose output
ctest --test-dir build --output-on-failure
```

### Run Specific Test Suite

```bash
# Run only ECS tests
ctest --test-dir build -R "ECS"

# Run only network tests
ctest --test-dir build -R "Network"

# Run specific test binary
./build/tests/test_registry
```

### Run with Filters

```bash
# Run only tests matching pattern
./build/tests/test_registry --gtest_filter="RegistryTest.*"

# Run all except slow tests
./build/tests/test_all --gtest_filter=-*Slow*

# List all tests without running
./build/tests/test_registry --gtest_list_tests
```

---

## 📝 Writing Unit Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>
#include "rtype/common/components/Position.hpp"

// Test fixture (optional, for setup/teardown)
class PositionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Runs before each test
    }
    
    void TearDown() override {
        // Runs after each test
    }
};

// Simple test
TEST(PositionTest, DefaultConstruction) {
    rtype::component::Position pos;
    
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
}

// Test with fixture
TEST_F(PositionTest, CustomPosition) {
    rtype::component::Position pos(100.0f, 200.0f);
    
    EXPECT_FLOAT_EQ(pos.x, 100.0f);
    EXPECT_FLOAT_EQ(pos.y, 200.0f);
}
```

### Testing ECS Components

```cpp
#include <gtest/gtest.h>
#include "rtype/ecs/Registry.hpp"
#include "rtype/common/components/Health.hpp"

class HealthComponentTest : public ::testing::Test {
protected:
    ECS::Registry registry;
    
    void SetUp() override {
        registry.registerComponent<rtype::component::Health>();
    }
};

TEST_F(HealthComponentTest, AddToEntity) {
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<rtype::component::Health>(entity, 100, 100);
    
    EXPECT_TRUE(registry.hasComponent<rtype::component::Health>(entity));
    
    auto& health = registry.getComponent<rtype::component::Health>(entity);
    EXPECT_EQ(health.current, 100);
    EXPECT_EQ(health.maximum, 100);
}

TEST_F(HealthComponentTest, TakeDamage) {
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<rtype::component::Health>(entity, 100, 100);
    
    auto& health = registry.getComponent<rtype::component::Health>(entity);
    health.current -= 25;
    
    EXPECT_EQ(health.current, 75);
}

TEST_F(HealthComponentTest, RemoveWhenDead) {
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<rtype::component::Health>(entity, 0, 100);
    
    auto& health = registry.getComponent<rtype::component::Health>(entity);
    
    if (health.current <= 0) {
        registry.killEntity(entity);
    }
    
    // Entity should be marked for deletion
    // (Actual removal happens in registry.update())
}
```

### Testing Systems

```cpp
#include <gtest/gtest.h>
#include "src/games/rtype/server/Systems/MovementSystem.hpp"
#include "rtype/common/components/Position.hpp"
#include "rtype/common/components/Velocity.hpp"

class MovementSystemTest : public ::testing::Test {
protected:
    ECS::Registry registry;
    std::unique_ptr<rtype::server::MovementSystem> system;
    
    void SetUp() override {
        registry.registerComponent<rtype::component::Position>();
        registry.registerComponent<rtype::component::Velocity>();
        system = std::make_unique<rtype::server::MovementSystem>();
    }
};

TEST_F(MovementSystemTest, UpdatesPosition) {
    // Create entity with position and velocity
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<rtype::component::Position>(entity, 0.0f, 0.0f);
    registry.emplaceComponent<rtype::component::Velocity>(entity, 100.0f, 50.0f);
    
    // Update system with dt = 0.1 seconds
    system->update(registry, 0.1f);
    
    // Check position updated
    auto& pos = registry.getComponent<rtype::component::Position>(entity);
    EXPECT_FLOAT_EQ(pos.x, 10.0f);  // 100 * 0.1
    EXPECT_FLOAT_EQ(pos.y, 5.0f);   // 50 * 0.1
}

TEST_F(MovementSystemTest, HandlesMultipleEntities) {
    // Create multiple entities
    std::vector<ECS::Entity> entities;
    for (int i = 0; i < 100; ++i) {
        auto entity = registry.spawnEntity();
        registry.emplaceComponent<rtype::component::Position>(entity, 0.0f, 0.0f);
        registry.emplaceComponent<rtype::component::Velocity>(entity, 
            static_cast<float>(i), static_cast<float>(i * 2));
        entities.push_back(entity);
    }
    
    // Update
    system->update(registry, 1.0f);
    
    // Verify all updated
    for (size_t i = 0; i < entities.size(); ++i) {
        auto& pos = registry.getComponent<rtype::component::Position>(entities[i]);
        EXPECT_FLOAT_EQ(pos.x, static_cast<float>(i));
        EXPECT_FLOAT_EQ(pos.y, static_cast<float>(i * 2));
    }
}
```

---

## 🔗 Integration Tests

### Client-Server Integration

```cpp
#include <gtest/gtest.h>
#include "src/server/ServerApp.hpp"
#include "src/client/ClientApp.hpp"
#include <thread>
#include <chrono>

class ClientServerTest : public ::testing::Test {
protected:
    std::unique_ptr<rtype::server::ServerApp> server;
    std::unique_ptr<rtype::client::ClientApp> client;
    std::thread serverThread;
    
    void SetUp() override {
        // Start server on separate thread
        server = std::make_unique<rtype::server::ServerApp>(4000);
        serverThread = std::thread([this]() {
            server->run();
        });
        
        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Create client
        client = std::make_unique<rtype::client::ClientApp>();
    }
    
    void TearDown() override {
        client.reset();
        server->stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        server.reset();
    }
};

TEST_F(ClientServerTest, ClientConnects) {
    bool connected = client->connect("127.0.0.1", 4000);
    EXPECT_TRUE(connected);
    
    // Wait for connection to establish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(client->isConnected());
}

TEST_F(ClientServerTest, ClientSendsPacket) {
    client->connect("127.0.0.1", 4000);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Send join request
    client->sendJoinRequest("TestPlayer");
    
    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check client received player ID
    EXPECT_NE(client->getPlayerId(), 0);
}

TEST_F(ClientServerTest, MultipleClientsJoin) {
    const int clientCount = 4;
    std::vector<std::unique_ptr<rtype::client::ClientApp>> clients;
    
    for (int i = 0; i < clientCount; ++i) {
        auto c = std::make_unique<rtype::client::ClientApp>();
        c->connect("127.0.0.1", 4000);
        c->sendJoinRequest("Player" + std::to_string(i));
        clients.push_back(std::move(c));
    }
    
    // Wait for all to join
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Verify all connected
    for (const auto& c : clients) {
        EXPECT_TRUE(c->isConnected());
        EXPECT_NE(c->getPlayerId(), 0);
    }
    
    // Check server has correct player count
    EXPECT_EQ(server->getPlayerCount(), clientCount);
}
```

### Network Synchronization Tests

```cpp
TEST(NetworkSyncTest, EntityPositionSynced) {
    // Setup server and client
    // ... (similar to above)
    
    // Server spawns entity
    auto serverEntity = server->spawnPlayer();
    server->setPosition(serverEntity, 100.0f, 200.0f);
    
    // Wait for sync
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Client should have matching entity
    auto clientEntity = client->getPlayerEntity();
    auto pos = client->getPosition(clientEntity);
    
    EXPECT_NEAR(pos.x, 100.0f, 1.0f);  // Allow small sync error
    EXPECT_NEAR(pos.y, 200.0f, 1.0f);
}
```

---

## 🎭 Mocking

### Mock Logger for Testing

```cpp
#include <gmock/gmock.h>
#include "rtype/common/Logger.hpp"

class MockLogger : public rtype::Logger {
public:
    MOCK_METHOD(void, info, (const std::string&), (override));
    MOCK_METHOD(void, warning, (const std::string&), (override));
    MOCK_METHOD(void, error, (const std::string&), (override));
    MOCK_METHOD(void, debug, (const std::string&), (override));
};

TEST(SystemTest, LogsErrors) {
    MockLogger mockLogger;
    rtype::Logger::setInstance(mockLogger);
    
    // Expect error to be logged
    EXPECT_CALL(mockLogger, error(::testing::HasSubstr("Failed")))
        .Times(1);
    
    // Run code that should log error
    // ...
    
    rtype::Logger::resetInstance();
}
```

### Mock Network Socket

```cpp
class MockUdpSocket : public rtype::network::IUdpSocket {
public:
    MOCK_METHOD(bool, bind, (uint16_t port), (override));
    MOCK_METHOD(ssize_t, sendTo, 
                (const void* data, size_t size, 
                 const std::string& ip, uint16_t port), 
                (override));
    MOCK_METHOD(ssize_t, receiveFrom, 
                (void* buffer, size_t size, 
                 std::string& senderIp, uint16_t& senderPort), 
                (override));
};

TEST(NetworkTest, SendsPacket) {
    MockUdpSocket socket;
    
    EXPECT_CALL(socket, sendTo(::testing::_, ::testing::_, "127.0.0.1", 4000))
        .Times(1)
        .WillOnce(::testing::Return(32));  // Return bytes sent
    
    // Code that uses socket
    // ...
}
```

---

## ⚡ Performance Testing

### Benchmark ECS Performance

```cpp
#include <benchmark/benchmark.h>
#include "rtype/ecs/Registry.hpp"

static void BM_EntityCreation(benchmark::State& state) {
    ECS::Registry registry;
    registry.registerComponent<rtype::component::Position>();
    
    for (auto _ : state) {
        auto entity = registry.spawnEntity();
        registry.emplaceComponent<rtype::component::Position>(entity, 0.0f, 0.0f);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EntityCreation);

static void BM_ViewIteration(benchmark::State& state) {
    ECS::Registry registry;
    registry.registerComponent<rtype::component::Position>();
    registry.registerComponent<rtype::component::Velocity>();
    
    // Create entities
    for (int i = 0; i < state.range(0); ++i) {
        auto entity = registry.spawnEntity();
        registry.emplaceComponent<rtype::component::Position>(entity, 0.0f, 0.0f);
        registry.emplaceComponent<rtype::component::Velocity>(entity, 1.0f, 1.0f);
    }
    
    for (auto _ : state) {
        auto view = registry.view<rtype::component::Position, 
                                    rtype::component::Velocity>();
        view.each([](auto entity, auto& pos, auto& vel) {
            pos.x += vel.dx;
            pos.y += vel.dy;
        });
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ViewIteration)->Range(100, 10000);

BENCHMARK_MAIN();
```

### Run Benchmarks

```bash
# Build benchmarks
cmake --build build --target benchmarks

# Run
./build/tests/benchmarks

# With filters
./build/tests/benchmarks --benchmark_filter=BM_ViewIteration

# Output to file
./build/tests/benchmarks --benchmark_out=results.json
```

---

## 📊 Code Coverage

### Generate Coverage Report

```bash
# Build with coverage enabled
cmake --preset linux-debug -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build

# Run tests
ctest --test-dir build

# Generate coverage report
lcov --capture --directory build --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info

# Generate HTML report
genhtml coverage.info --output-directory coverage_report

# View report
firefox coverage_report/index.html
```

### Coverage Goals

- **Overall**: 80%+
- **Critical systems**: 90%+
- **ECS core**: 95%+
- **Network layer**: 85%+

---

## 🎯 Test-Driven Development (TDD)

### TDD Workflow

1. **Write Failing Test**

```cpp
TEST(PlayerTest, FiresProjectile) {
    // This test will fail initially
    Player player;
    player.fire();
    
    EXPECT_EQ(player.getProjectileCount(), 1);
}
```

2. **Implement Minimal Code**

```cpp
class Player {
    int projectileCount = 0;
public:
    void fire() {
        projectileCount++;
    }
    int getProjectileCount() const {
        return projectileCount;
    }
};
```

3. **Test Passes**

```bash
./build/tests/test_player
# [PASSED] PlayerTest.FiresProjectile
```

4. **Refactor**

Improve implementation while keeping tests passing.

5. **Repeat**

Add more tests for edge cases, error handling, etc.

---

## 🐛 Debugging Tests

### Run with Debugger

```bash
# GDB
gdb ./build/tests/test_registry
(gdb) run
(gdb) break RegistryTest
(gdb) continue

# LLDB
lldb ./build/tests/test_registry
(lldb) run
(lldb) breakpoint set --name RegistryTest
(lldb) continue
```

### Add Debug Output

```cpp
TEST(MyTest, DebuggingIssue) {
    int value = computeValue();
    
    // Temporary debug output
    std::cout << "Debug: value = " << value << std::endl;
    
    EXPECT_EQ(value, 42);
}
```

### Use Test Fixtures for Debugging

```cpp
class DebugTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "=== Test starting ===" << std::endl;
    }
    
    void TearDown() override {
        std::cout << "=== Test finished ===" << std::endl;
    }
};
```

---

## 📋 Testing Checklist

### Before Committing

- [ ] All tests pass locally
- [ ] No new warnings
- [ ] Code coverage maintained or improved
- [ ] Added tests for new features
- [ ] Updated existing tests if API changed
- [ ] Ran static analysis (cpplint)

### Before Releasing

- [ ] Full test suite passes
- [ ] Integration tests pass
- [ ] Performance benchmarks acceptable
- [ ] No memory leaks (valgrind)
- [ ] Documentation updated

---

## 🔧 Continuous Integration

Tests run automatically on:
- Every commit (via pre-commit hooks)
- Every pull request (GitHub Actions)
- Nightly builds (full test suite)

See `.github/workflows/tests.yml` for CI configuration.

---

## 📚 Best Practices

1. **Test One Thing**: Each test should verify one specific behavior
2. **Use Descriptive Names**: Test names should describe what they test
3. **AAA Pattern**: Arrange, Act, Assert
4. **Fast Tests**: Unit tests should be < 100ms each
5. **Independent Tests**: Tests shouldn't depend on execution order
6. **Mock External Dependencies**: Don't rely on network, filesystem, etc.
7. **Test Edge Cases**: Null pointers, empty containers, boundary values
8. **Fail Fast**: Tests should fail quickly and clearly
9. **Maintainable**: Keep tests simple and readable
10. **Coverage != Quality**: High coverage doesn't mean good tests

---

## 📖 Related Documentation

- [Contributing Guide](./contributing.md)
- [ECS Architecture](./Architecture/ecs-guide.md)
- [Network Protocol](./protocol/RFC_RTGP_v1.2.0.md)

**Happy testing! ✅**
