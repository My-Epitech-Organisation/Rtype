#include "game_state.pb.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

namespace {
constexpr int kPacketsPerSecond = 60;
constexpr double kBitsInByte = 8.0;
constexpr double kBytesPerKilobyte = 1024.0;

void print_size(const std::string& name, size_t bytes) {
    std::cout << std::left << std::setw(28) << name << std::right << std::setw(6)
              << bytes << " bytes" << std::endl;
}

double bandwidth_kbps(size_t bytes_per_packet) {
    const double bytes_per_second = static_cast<double>(bytes_per_packet) *
        static_cast<double>(kPacketsPerSecond);
    return bytes_per_second * kBitsInByte / kBytesPerKilobyte;
}

rtype::serialization::GameState build_packet(std::size_t entities) {
    rtype::serialization::GameState state;
    state.set_timestamp(123456789u);
    for (std::size_t i = 0; i < entities; ++i) {
        auto* entity = state.add_entities();
        entity->set_id(static_cast<uint32_t>(i + 1));
        entity->mutable_position()->set_x(100.0f + static_cast<float>(i));
        entity->mutable_position()->set_y(200.0f + static_cast<float>(i));
        entity->mutable_velocity()->set_x(5.0f);
        entity->mutable_velocity()->set_y(-3.0f);
    }
    return state;
}

void compare_sizes(const std::string& name, size_t protobuf_size, size_t json_size,
                   size_t binary_size) {
    const double vs_json = (1.0 -
        static_cast<double>(protobuf_size) / static_cast<double>(json_size)) * 100.0;
    const double vs_binary = (static_cast<double>(protobuf_size) /
        static_cast<double>(binary_size) - 1.0) * 100.0;

    std::cout << std::left << std::setw(14) << name
              << std::right << std::setw(10) << protobuf_size
              << std::setw(10) << binary_size
              << std::setw(10) << json_size
              << std::setw(12) << std::fixed << std::setprecision(1) << vs_binary
              << std::setw(12) << std::fixed << std::setprecision(1) << vs_json
              << std::endl;
}
}

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::cout << "=== Protobuf Size Test ===" << std::endl << std::endl;

    rtype::serialization::EntityState entity;
    entity.set_id(1);
    entity.mutable_position()->set_x(100.5f);
    entity.mutable_position()->set_y(200.75f);
    entity.mutable_velocity()->set_x(5.5f);
    entity.mutable_velocity()->set_y(-3.2f);

    rtype::serialization::Vec2 position;
    position.set_x(100.5f);
    position.set_y(200.75f);

    std::string buffer;

    // Position stand-in (Vec2 message)
    position.SerializeToString(&buffer);
    print_size("Vec2 (position)", buffer.size());

    // Entity state
    entity.SerializeToString(&buffer);
    print_size("EntityState", buffer.size());

    auto packet5 = build_packet(5);
    packet5.SerializeToString(&buffer);
    print_size("GameState x5", buffer.size());

    auto packet10 = build_packet(10);
    packet10.SerializeToString(&buffer);
    print_size("GameState x10", buffer.size());

    std::cout << "\n=== Bandwidth @ 60 Hz ===" << std::endl;
    packet5.SerializeToString(&buffer);
    double bw5 = bandwidth_kbps(buffer.size());
    std::cout << "5 entities:  " << std::fixed << std::setprecision(2) << bw5
              << " Kbps" << std::endl;

    packet10.SerializeToString(&buffer);
    double bw10 = bandwidth_kbps(buffer.size());
    std::cout << "10 entities: " << std::fixed << std::setprecision(2) << bw10
              << " Kbps" << std::endl;

    std::cout << "\n=== Size Comparison (bytes) ===" << std::endl;
    std::cout << std::left << std::setw(14) << "Packet" << std::right
              << std::setw(10) << "Proto"
              << std::setw(10) << "Binary"
              << std::setw(10) << "JSON"
              << std::setw(12) << "+% Bin"
              << std::setw(12) << "-% JSON" << std::endl;
    std::cout << std::string(58, '-') << std::endl;

    compare_sizes("Vec2", rtype::serialization::Vec2().ByteSizeLong(), 31, 8);
    compare_sizes("Entity", entity.ByteSizeLong(), 95, 20);
    compare_sizes("5 ent.", packet5.ByteSizeLong(), 439, 105);
    compare_sizes("10 ent.", packet10.ByteSizeLong(), 856, 205);

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
