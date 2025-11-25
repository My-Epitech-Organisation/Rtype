// Test JSON serialization size for network packets
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using json = nlohmann::json;

// Game entity position structure
struct Position {
    float x;
    float y;
    float rotation;
    
    json to_json() const {
        return json{
            {"x", x},
            {"y", y},
            {"r", rotation}
        };
    }
};

// Game entity with full state
struct EntityState {
    uint32_t id;
    Position position;
    float velocity_x;
    float velocity_y;
    uint8_t health;
    uint8_t team;
    
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

// Network packet with multiple entities
struct GameStatePacket {
    uint32_t timestamp;
    std::vector<EntityState> entities;
    
    json to_json() const {
        json j;
        j["ts"] = timestamp;
        j["entities"] = json::array();
        for (const auto& entity : entities) {
            j["entities"].push_back(entity.to_json());
        }
        return j;
    }
};

void print_size_analysis(const std::string& name, const std::string& data) {
    std::cout << std::left << std::setw(30) << name 
              << std::right << std::setw(8) << data.size() << " bytes" << std::endl;
}

void calculate_bandwidth(const std::string& name, size_t packet_size, int packets_per_sec) {
    double bytes_per_sec = packet_size * packets_per_sec;
    double kbps = (bytes_per_sec * 8) / 1024.0;
    double mbps = kbps / 1024.0;
    
    std::cout << "\n" << name << " @ " << packets_per_sec << " packets/sec:" << std::endl;
    std::cout << "  Size per packet: " << packet_size << " bytes" << std::endl;
    std::cout << "  Bandwidth: " << std::fixed << std::setprecision(2) 
              << bytes_per_sec << " B/s = " << kbps << " Kbps = " << mbps << " Mbps" << std::endl;
}

int main() {
    std::cout << "=== JSON Serialization Size Test ===" << std::endl;
    std::cout << std::endl;
    
    // Test 1: Single position
    Position pos{100.5f, 200.75f, 45.0f};
    std::string json_pos = pos.to_json().dump();
    std::string json_pos_compact = pos.to_json().dump(-1); // No spaces
    
    std::cout << "--- Single Position ---" << std::endl;
    print_size_analysis("JSON (formatted)", json_pos);
    print_size_analysis("JSON (compact)", json_pos_compact);
    std::cout << "Content: " << json_pos_compact << std::endl;
    
    // Test 2: Single entity
    std::cout << "\n--- Single Entity ---" << std::endl;
    EntityState entity{
        .id = 1,
        .position = {100.5f, 200.75f, 45.0f},
        .velocity_x = 5.5f,
        .velocity_y = -3.2f,
        .health = 100,
        .team = 1
    };
    
    std::string json_entity = entity.to_json().dump(-1);
    print_size_analysis("JSON (compact)", json_entity);
    std::cout << "Content: " << json_entity << std::endl;
    
    // Test 3: Multiple entities (typical game state)
    std::cout << "\n--- Game State Packet ---" << std::endl;
    GameStatePacket packet;
    packet.timestamp = 1234567890;
    
    // Add 5 entities (typical for R-Type: 1 player, 4 enemies)
    for (uint32_t i = 1; i <= 5; ++i) {
        packet.entities.push_back({
            .id = i,
            .position = {100.0f * i, 200.0f * i, 45.0f * i},
            .velocity_x = 5.0f,
            .velocity_y = -3.0f,
            .health = 100,
            .team = static_cast<uint8_t>(i % 2)
        });
    }
    
    std::string json_packet = packet.to_json().dump(-1);
    print_size_analysis("5 entities (compact)", json_packet);
    
    // Test 4: Larger packet (10 entities)
    for (uint32_t i = 6; i <= 10; ++i) {
        packet.entities.push_back({
            .id = i,
            .position = {100.0f * i, 200.0f * i, 45.0f * i},
            .velocity_x = 5.0f,
            .velocity_y = -3.0f,
            .health = 100,
            .team = static_cast<uint8_t>(i % 2)
        });
    }
    
    std::string json_packet_10 = packet.to_json().dump(-1);
    print_size_analysis("10 entities (compact)", json_packet_10);
    
    // Bandwidth calculations
    std::cout << "\n=== Bandwidth Analysis (60 packets/sec) ===" << std::endl;
    calculate_bandwidth("Single position", json_pos_compact.size(), 60);
    calculate_bandwidth("Single entity", json_entity.size(), 60);
    calculate_bandwidth("5 entities packet", json_packet.size(), 60);
    calculate_bandwidth("10 entities packet", json_packet_10.size(), 60);
    
    // Maximum packet size analysis
    std::cout << "\n=== Maximum Entities per Packet ===" << std::endl;
    std::cout << "(Target: < 1500 bytes MTU, < 10 Kbps @ 60 pkt/s)" << std::endl;
    
    size_t max_packet_size = 1500; // Ethernet MTU
    size_t overhead = packet.to_json().dump(-1).size() - (packet.entities.size() * json_entity.size());
    size_t max_entities = (max_packet_size - overhead) / json_entity.size();
    
    std::cout << "  Overhead: " << overhead << " bytes" << std::endl;
    std::cout << "  Max entities (MTU 1500): " << max_entities << std::endl;
    
    // 10 Kbps limit
    size_t max_bytes_for_10kbps = (10 * 1024 / 8) / 60; // 10 Kbps / 60 pkt/s
    size_t max_entities_10kbps = (max_bytes_for_10kbps - overhead) / json_entity.size();
    std::cout << "  Max entities (10 Kbps): " << max_entities_10kbps << std::endl;
    
    // Verdict
    std::cout << "\n=== Verdict ===" << std::endl;
    if (json_packet.size() * 60 * 8 / 1024.0 < 10.0) {
        std::cout << "✓ 5 entities @ 60 pkt/s = " 
                  << std::fixed << std::setprecision(2)
                  << (json_packet.size() * 60 * 8 / 1024.0) 
                  << " Kbps < 10 Kbps ✓ ACCEPTABLE" << std::endl;
    } else {
        std::cout << "✗ JSON too large for 60 packets/sec at 10 Kbps" << std::endl;
    }
    
    std::cout << "\nConclusion:" << std::endl;
    std::cout << "  JSON size per entity: ~" << json_entity.size() << " bytes" << std::endl;
    std::cout << "  Suitable for low entity counts (< " << max_entities_10kbps << " entities)" << std::endl;
    std::cout << "  Consider binary serialization for better efficiency" << std::endl;
    
    return 0;
}
