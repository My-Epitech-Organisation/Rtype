/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ByteConverter
*/

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <string>

namespace rtype::network {

class ByteConverter {
public:
    static void serializeInt(std::shared_ptr<std::vector<uint8_t>> buffer, int32_t value);
    static void serializeFloat(std::shared_ptr<std::vector<uint8_t>> buffer, float value);
    static void serializeString(std::shared_ptr<std::vector<uint8_t>> buffer, const std::string& value);

    static int32_t deserializeInt(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset);
    static float deserializeFloat(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset);
    static std::string deserializeString(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset);

private:
    ByteConverter() = delete;
};

}  // namespace rtype::network
