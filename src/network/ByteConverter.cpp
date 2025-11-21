/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ByteConverter
*/

#include "ByteConverter.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>

namespace rtype::network {

void ByteConverter::serializeInt(std::shared_ptr<std::vector<uint8_t>> buffer, int32_t value) {
    if (!buffer) {
        throw std::invalid_argument("Buffer shared_ptr is null");
    }

    uint32_t uvalue = static_cast<uint32_t>(value);

    buffer->push_back((uvalue >> 24) & 0xFF);
    buffer->push_back((uvalue >> 16) & 0xFF);
    buffer->push_back((uvalue >> 8) & 0xFF);
    buffer->push_back(uvalue & 0xFF);
}

void ByteConverter::serializeFloat(std::shared_ptr<std::vector<uint8_t>> buffer, float value) {
    if (!buffer) {
        throw std::invalid_argument("Buffer shared_ptr is null");
    }

    uint32_t uvalue;

    std::memcpy(&uvalue, &value, sizeof(float));
    buffer->push_back((uvalue >> 24) & 0xFF);
    buffer->push_back((uvalue >> 16) & 0xFF);
    buffer->push_back((uvalue >> 8) & 0xFF);
    buffer->push_back(uvalue & 0xFF);
}

void ByteConverter::serializeString(std::shared_ptr<std::vector<uint8_t>> buffer, const std::string& value) {
    if (!buffer) {
        throw std::invalid_argument("Buffer shared_ptr is null");
    }
    if (value.size() > static_cast<size_t>(INT32_MAX)) {
        throw std::length_error("String size exceeds maximum allowed for serialization");
    }

    uint32_t length = static_cast<uint32_t>(value.size());

    serializeInt(buffer, static_cast<int32_t>(length));
    buffer->insert(buffer->end(), value.begin(), value.end());
}

int32_t ByteConverter::deserializeInt(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset) {
    if (!offset || *offset + 4 > buffer.size()) {
        throw std::out_of_range("Not enough bytes to deserialize int32_t");
    }
    uint32_t uvalue = (static_cast<uint32_t>(buffer[*offset]) << 24) |
                      (static_cast<uint32_t>(buffer[*offset + 1]) << 16) |
                      (static_cast<uint32_t>(buffer[*offset + 2]) << 8) |
                      static_cast<uint32_t>(buffer[*offset + 3]);
    *offset += 4;
    return static_cast<int32_t>(uvalue);
}

float ByteConverter::deserializeFloat(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset) {
    if (!offset || *offset + 4 > buffer.size()) {
        throw std::out_of_range("Not enough bytes to deserialize float");
    }
    uint32_t uvalue = (static_cast<uint32_t>(buffer[*offset]) << 24) |
                      (static_cast<uint32_t>(buffer[*offset + 1]) << 16) |
                      (static_cast<uint32_t>(buffer[*offset + 2]) << 8) |
                      static_cast<uint32_t>(buffer[*offset + 3]);
    *offset += 4;
    float value;
    std::memcpy(&value, &uvalue, sizeof(float));
    return value;
}

std::string ByteConverter::deserializeString(const std::vector<uint8_t>& buffer, std::shared_ptr<size_t> offset) {
    if (!offset) {
        throw std::invalid_argument("Offset shared_ptr is null");
    }

    int32_t rawLength = deserializeInt(buffer, offset);
    if (rawLength < 0) {
        throw std::out_of_range("Deserialized string length is negative");
    }

    uint32_t length = static_cast<uint32_t>(rawLength);
    if (*offset + length > buffer.size()) {
        throw std::out_of_range("Not enough bytes to deserialize string");
    }

    std::string value(buffer.begin() + *offset, buffer.begin() + *offset + length);

    *offset += length;
    return value;
}

}  // namespace rtype::network
