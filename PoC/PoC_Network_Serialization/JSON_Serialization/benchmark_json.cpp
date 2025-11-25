// Benchmark JSON serialization performance
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

using json = nlohmann::json;
using namespace std::chrono;

struct Position {
    float x, y, rotation;

    json to_json() const {
        return json{{"x", x}, {"y", y}, {"r", rotation}};
    }
};

struct EntityState {
    uint32_t id;
    Position position;
    float velocity_x, velocity_y;
    uint8_t health, team;

    json to_json() const {
        return json{
            {"id", id},
            {"pos", position.to_json()},
            {"vel", {velocity_x, velocity_y}},
            {"hp", health},
            {"team", team}
        };
    }
};

void benchmark_serialization(int iterations, int num_entities) {
    std::vector<EntityState> entities;
    for (int i = 0; i < num_entities; ++i) {
        entities.push_back({
            .id = static_cast<uint32_t>(i),
            .position = {100.0f * i, 200.0f * i, 45.0f},
            .velocity_x = 5.0f,
            .velocity_y = -3.0f,
            .health = 100,
            .team = static_cast<uint8_t>(i % 2)
        });
    }

    // Warmup
    for (int i = 0; i < 100; ++i) {
        json j = json::array();
        for (const auto& e : entities) {
            j.push_back(e.to_json());
        }
        std::string s = j.dump(-1);
        (void)s;
    }

    // Benchmark serialization
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        json j = json::array();
        for (const auto& e : entities) {
            j.push_back(e.to_json());
        }
        std::string s = j.dump(-1);
        (void)s;
    }
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end - start).count();
    double avg_us = static_cast<double>(duration) / iterations;
    double packets_per_sec = 1000000.0 / avg_us;

    std::cout << std::setw(12) << num_entities
              << std::setw(15) << std::fixed << std::setprecision(2) << avg_us << " µs"
              << std::setw(18) << std::fixed << std::setprecision(0) << packets_per_sec << " pkt/s";

    if (packets_per_sec >= 60) {
        std::cout << "  ✓";
    } else {
        std::cout << "  ✗";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== JSON Serialization Performance Benchmark ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Testing " << 10000 << " iterations per configuration" << std::endl;
    std::cout << "Target: 60 packets/second (16.67ms per packet)" << std::endl;
    std::cout << std::endl;

    std::cout << std::setw(12) << "Entities"
              << std::setw(15) << "Avg Time"
              << std::setw(18) << "Max Throughput"
              << std::setw(8) << "60Hz?" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    benchmark_serialization(10000, 1);
    benchmark_serialization(10000, 2);
    benchmark_serialization(10000, 5);
    benchmark_serialization(10000, 10);
    benchmark_serialization(10000, 20);
    benchmark_serialization(10000, 50);
    benchmark_serialization(10000, 100);

    std::cout << std::endl;
    std::cout << "Conclusion:" << std::endl;
    std::cout << "  ✓ = Can maintain 60 Hz update rate" << std::endl;
    std::cout << "  ✗ = Cannot maintain 60 Hz (serialization bottleneck)" << std::endl;

    return 0;
}
