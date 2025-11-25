/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer
*/

#pragma once

#include <cstdint>
#include <vector>

#include "Packet.hpp"

namespace rtype::network {

class Serializer {
   public:
    static std::vector<uint8_t> serialize(const Packet &packet);
    static Packet deserialize(const std::vector<uint8_t> &data);
};

}  // namespace rtype::network
