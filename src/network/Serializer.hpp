/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "Packet.hpp"

namespace rtype::network {

class Serializer {
   public:
    static std::vector<uint8_t> serialize(const Packet& packet);
    static Packet deserialize(const std::vector<uint8_t>& data);

    template <typename T>
    static std::vector<uint8_t> serialize(const T& data) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "T must be trivially copyable");
        std::vector<uint8_t> result(sizeof(T));
        std::memcpy(result.data(), &data, sizeof(T));
        return result;
    }

    template <typename T>
    static T deserialize(const std::vector<uint8_t>& buffer) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "T must be trivially copyable");
        if (buffer.size() != sizeof(T)) {
            throw std::runtime_error("Invalid buffer size for deserialization");
        }
        T result;
        std::memcpy(&result, buffer.data(), sizeof(T));
        return result;
    }
};

}  // namespace rtype::network
