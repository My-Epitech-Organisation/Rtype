#include "game_state.pb.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::string build_json_entity(uint32_t id, float x, float y, float vx, float vy) {
    std::ostringstream oss;
    oss << "{\"id\":" << id
        << ",\"pos\":{\"x\":" << x << ",\"y\":" << y << "}"
        << ",\"vel\":[" << vx << "," << vy << "]}";
    return oss.str();
}

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    rtype::serialization::EntityState entity;
    entity.set_id(7);
    entity.mutable_position()->set_x(123.456f);
    entity.mutable_position()->set_y(789.012f);
    entity.mutable_velocity()->set_x(5.5f);
    entity.mutable_velocity()->set_y(-3.2f);

    std::string protobuf_bytes;
    entity.SerializeToString(&protobuf_bytes);

    std::string json_string = build_json_entity(7, 123.456f, 789.012f, 5.5f, -3.2f);

    std::cout << "=== Protobuf vs JSON ===" << std::endl;
    std::cout << "Protobuf bytes (hex): ";
    for (unsigned char c : protobuf_bytes) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << ' ';
    }
    std::cout << std::dec << std::endl;
    std::cout << "Size: " << protobuf_bytes.size() << " bytes" << std::endl;

    std::cout << "\nJSON string:" << std::endl;
    std::cout << json_string << std::endl;
    std::cout << "Size: " << json_string.size() << " bytes" << std::endl;

    double reduction = (1.0 -
        static_cast<double>(protobuf_bytes.size()) / static_cast<double>(json_string.size())) * 100.0;
    std::cout << "\n✓ Protobuf saves " << std::fixed << std::setprecision(1)
              << reduction << "% vs JSON" << std::endl;

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
