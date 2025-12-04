/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer
*/

#include "Serializer.hpp"

#include <string>
#include <vector>

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

std::vector<uint8_t> Serializer::serialize(const std::string& str) {
    std::vector<uint8_t> result;

    if (str.size() > UINT32_MAX) {
        throw std::runtime_error(
            "String too large for serialization: max size is " +
            std::to_string(UINT32_MAX) + " bytes");
    }

    uint32_t length = static_cast<uint32_t>(str.size());
    std::vector<uint8_t> lengthBytes(sizeof(uint32_t));
    ByteOrder::writeTo(lengthBytes.data(), length);
    result.insert(result.end(), lengthBytes.begin(), lengthBytes.end());

    result.insert(result.end(), str.begin(), str.end());
    return result;
}

std::string Serializer::deserializeString(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < sizeof(uint32_t)) {
        throw std::runtime_error(
            "Buffer too small for string deserialization: expected at least " +
            std::to_string(sizeof(uint32_t)) + " bytes for length, got " +
            std::to_string(buffer.size()) + " bytes");
    }

    uint32_t length = ByteOrder::readFrom<uint32_t>(buffer.data());

    if (buffer.size() < sizeof(uint32_t) + length) {
        throw std::runtime_error(
            "Buffer size mismatch for string deserialization: expected " +
            std::to_string(sizeof(uint32_t) + length) + " bytes, got " +
            std::to_string(buffer.size()) + " bytes");
    }

    const char* strData =
        reinterpret_cast<const char*>(buffer.data() + sizeof(uint32_t));
    return std::string(strData, length);
}

}  // namespace rtype::network
