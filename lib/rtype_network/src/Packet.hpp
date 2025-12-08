/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Packet
*/

#pragma once

#include <cstdint>
#include <vector>

namespace rtype::network {

enum class PacketType : uint8_t {
    Unknown = 0,
    PlayerInput,
    EntityUpdate,
    EntitySpawn,
    EntityDestroy
};

class Packet {
   public:
    Packet();
    explicit Packet(PacketType type);

    PacketType type() const { return type_; }
    const std::vector<uint8_t>& data() const { return data_; }

    void setType(PacketType type) { type_ = type; }
    void setData(const std::vector<uint8_t>& data) { data_ = data; }

   private:
    PacketType type_;
    std::vector<uint8_t> data_;
};

}  // namespace rtype::network
