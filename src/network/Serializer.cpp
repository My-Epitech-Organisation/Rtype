/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer
*/

#include "Serializer.hpp"

namespace rtype::network {

std::vector<uint8_t> Serializer::serialize(const Packet& packet) {
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(packet.type()));
    const auto& data = packet.data();
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

Packet Serializer::deserialize(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return Packet();
    }

    Packet packet(static_cast<PacketType>(data[0]));
    if (data.size() > 1) {
        std::vector<uint8_t> payload(data.begin() + 1, data.end());
        packet.setData(payload);
    }
    return packet;
}

}  // namespace rtype::network
