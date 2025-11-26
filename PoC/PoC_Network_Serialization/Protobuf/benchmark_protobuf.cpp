#include "game_state.pb.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

using Clock = std::chrono::high_resolution_clock;

rtype::serialization::GameState build_packet(int entities) {
    rtype::serialization::GameState packet;
    packet.set_timestamp(123456789u);
    for (int i = 0; i < entities; ++i) {
        auto* entity = packet.add_entities();
        entity->set_id(static_cast<uint32_t>(i));
        entity->mutable_position()->set_x(100.0f + static_cast<float>(i));
        entity->mutable_position()->set_y(200.0f + static_cast<float>(i));
        entity->mutable_velocity()->set_x(5.0f);
        entity->mutable_velocity()->set_y(-3.0f);
    }
    return packet;
}

void benchmark_serialization(int entities, int iterations) {
    auto packet = build_packet(entities);
    std::string serialized;
    serialized.reserve(1024);

    // warmup
    for (int i = 0; i < 100; ++i) {
        packet.SerializeToString(&serialized);
    }

    const auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        packet.SerializeToString(&serialized);
    }
    const auto end = Clock::now();

    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const double avg_ns = static_cast<double>(ns) / static_cast<double>(iterations);
    const double throughput = 1'000'000'000.0 / avg_ns;

    std::cout << std::setw(10) << entities
              << std::setw(14) << std::fixed << std::setprecision(2) << (avg_ns / 1000.0) << " µs"
              << std::setw(16) << std::fixed << std::setprecision(0) << throughput << " pkt/s"
              << std::setw(6) << (throughput >= 60.0 ? "✓" : "✗")
              << std::endl;
}

void benchmark_roundtrip(int entities, int iterations) {
    auto packet = build_packet(entities);
    std::string serialized;
    serialized.reserve(1024);

    // warmup
    for (int i = 0; i < 100; ++i) {
        packet.SerializeToString(&serialized);
        rtype::serialization::GameState decoded;
        decoded.ParseFromString(serialized);
    }

    const auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        packet.SerializeToString(&serialized);
        rtype::serialization::GameState decoded;
        decoded.ParseFromString(serialized);
    }
    const auto end = Clock::now();

    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const double avg_ns = static_cast<double>(ns) / static_cast<double>(iterations);
    const double throughput = 1'000'000'000.0 / avg_ns;

    std::cout << std::setw(10) << entities
              << std::setw(14) << std::fixed << std::setprecision(2) << (avg_ns / 1000.0) << " µs"
              << std::setw(16) << std::fixed << std::setprecision(0) << throughput << " pkt/s"
              << std::setw(6) << (throughput >= 60.0 ? "✓" : "✗")
              << std::endl;
}

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    const int iterations = 100000;
    std::cout << "=== Protobuf Serialization Benchmark ===" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Target: 60 packets/s" << std::endl << std::endl;

    std::cout << "--- Serialize ---" << std::endl;
    std::cout << std::setw(10) << "Entities"
              << std::setw(14) << "Avg Time"
              << std::setw(16) << "Max Thpt"
              << std::setw(6) << "60Hz" << std::endl;
    std::cout << std::string(46, '-') << std::endl;

    for (int entities : {1, 2, 5, 10, 20, 50, 100}) {
        benchmark_serialization(entities, iterations);
    }

    std::cout << "\n--- Serialize + Deserialize ---" << std::endl;
    std::cout << std::setw(10) << "Entities"
              << std::setw(14) << "Avg Time"
              << std::setw(16) << "Max Thpt"
              << std::setw(6) << "60Hz" << std::endl;
    std::cout << std::string(46, '-') << std::endl;

    for (int entities : {1, 2, 5, 10, 20, 50, 100}) {
        benchmark_roundtrip(entities, iterations);
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
