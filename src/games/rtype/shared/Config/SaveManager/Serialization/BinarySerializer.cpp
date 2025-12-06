/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BinarySerializer - Implementation
*/

#include "BinarySerializer.hpp"

namespace rtype::game::config {

void BinarySerializer::writeUint8(std::vector<uint8_t>& buffer, uint8_t value) {
    buffer.push_back(value);
}

void BinarySerializer::writeUint16(std::vector<uint8_t>& buffer,
                                   uint16_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void BinarySerializer::writeUint32(std::vector<uint8_t>& buffer,
                                   uint32_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void BinarySerializer::writeUint64(std::vector<uint8_t>& buffer,
                                   uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

void BinarySerializer::writeInt32(std::vector<uint8_t>& buffer, int32_t value) {
    writeUint32(buffer, static_cast<uint32_t>(value));
}

void BinarySerializer::writeFloat(std::vector<uint8_t>& buffer, float value) {
    uint32_t intVal;
    std::memcpy(&intVal, &value, sizeof(float));
    writeUint32(buffer, intVal);
}

void BinarySerializer::writeString(std::vector<uint8_t>& buffer,
                                   const std::string& value) {
    writeUint32(buffer, static_cast<uint32_t>(value.size()));
    buffer.insert(buffer.end(), value.begin(), value.end());
}

uint8_t BinarySerializer::readUint8(const std::vector<uint8_t>& buffer,
                                    size_t& offset) {
    if (offset >= buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint8");
    }
    return buffer[offset++];
}

uint16_t BinarySerializer::readUint16(const std::vector<uint8_t>& buffer,
                                      size_t& offset) {
    if (offset + 2 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint16");
    }
    uint16_t value = static_cast<uint16_t>(buffer[offset]) |
                     (static_cast<uint16_t>(buffer[offset + 1]) << 8);
    offset += 2;
    return value;
}

uint32_t BinarySerializer::readUint32(const std::vector<uint8_t>& buffer,
                                      size_t& offset) {
    if (offset + 4 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint32");
    }
    uint32_t value = static_cast<uint32_t>(buffer[offset]) |
                     (static_cast<uint32_t>(buffer[offset + 1]) << 8) |
                     (static_cast<uint32_t>(buffer[offset + 2]) << 16) |
                     (static_cast<uint32_t>(buffer[offset + 3]) << 24);
    offset += 4;
    return value;
}

uint64_t BinarySerializer::readUint64(const std::vector<uint8_t>& buffer,
                                      size_t& offset) {
    if (offset + 8 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint64");
    }
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<uint64_t>(buffer[offset + i]) << (i * 8);
    }
    offset += 8;
    return value;
}

int32_t BinarySerializer::readInt32(const std::vector<uint8_t>& buffer,
                                    size_t& offset) {
    return static_cast<int32_t>(readUint32(buffer, offset));
}

float BinarySerializer::readFloat(const std::vector<uint8_t>& buffer,
                                  size_t& offset) {
    uint32_t intVal = readUint32(buffer, offset);
    float value;
    std::memcpy(&value, &intVal, sizeof(float));
    return value;
}

std::string BinarySerializer::readString(const std::vector<uint8_t>& buffer,
                                         size_t& offset) {
    uint32_t length = readUint32(buffer, offset);
    if (offset + length > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading string");
    }
    std::string value(buffer.begin() + static_cast<std::ptrdiff_t>(offset),
                      buffer.begin() + static_cast<std::ptrdiff_t>(offset) +
                          static_cast<std::ptrdiff_t>(length));
    offset += length;
    return value;
}

}  // namespace rtype::game::config
