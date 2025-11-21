/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_serialization
*/

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>

#include "rtype/network/Packet.hpp"
#include "rtype/network/Serializer.hpp"
#include "rtype/network/ByteConverter.hpp"
#include "rtype/network/CircularBuffer.hpp"

using namespace rtype::network;

TEST(SerializationTest, SerializeDeserializePacket) {
    Packet packet(PacketType::PlayerInput);
    auto serialized = Serializer::serialize(packet);

    EXPECT_FALSE(serialized.empty());
    EXPECT_EQ(serialized[0], static_cast<uint8_t>(PacketType::PlayerInput));
}

TEST(SerializationTest, DeserializePacket) {
    Packet original(PacketType::EntityUpdate);
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.type(), PacketType::EntityUpdate);
}

TEST(SerializationTest, PacketWithData) {
    Packet packet(PacketType::EntitySpawn);
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    packet.setData(data);

    auto serialized = Serializer::serialize(packet);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.type(), PacketType::EntitySpawn);
    EXPECT_EQ(deserialized.data(), data);
}

TEST(PacketTest, DefaultConstructor) {
    Packet packet;
    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(PacketTest, TypedConstructor) {
    Packet packet(PacketType::PlayerInput);
    EXPECT_EQ(packet.type(), PacketType::PlayerInput);
}

TEST(ByteConverterTest, SerializeDeserializeInt) {
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    int32_t value = 12345;
    ByteConverter::serializeInt(buffer, value);

    auto offset = std::make_shared<size_t>(0);
    int32_t result = ByteConverter::deserializeInt(*buffer, offset);

    EXPECT_EQ(result, value);
}

TEST(ByteConverterTest, SerializeDeserializeFloat) {
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    float value = 3.14159f;
    ByteConverter::serializeFloat(buffer, value);

    auto offset = std::make_shared<size_t>(0);
    float result = ByteConverter::deserializeFloat(*buffer, offset);

    EXPECT_FLOAT_EQ(result, value);
}

TEST(ByteConverterTest, SerializeDeserializeString) {
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    std::string value = "Hello, World!";
    ByteConverter::serializeString(buffer, value);

    auto offset = std::make_shared<size_t>(0);
    std::string result = ByteConverter::deserializeString(*buffer, offset);

    EXPECT_EQ(result, value);
}

TEST(ByteConverterTest, SerializeDeserializeEmptyString) {
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    std::string value = "";
    ByteConverter::serializeString(buffer, value);

    auto offset = std::make_shared<size_t>(0);
    std::string result = ByteConverter::deserializeString(*buffer, offset);

    EXPECT_EQ(result, value);
}

TEST(ByteConverterTest, SerializeMultipleValues) {
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    int32_t intVal = -42;
    float floatVal = -2.5f;
    std::string strVal = "Test";

    ByteConverter::serializeInt(buffer, intVal);
    ByteConverter::serializeFloat(buffer, floatVal);
    ByteConverter::serializeString(buffer, strVal);

    auto offset = std::make_shared<size_t>(0);
    int32_t resultInt = ByteConverter::deserializeInt(*buffer, offset);
    float resultFloat = ByteConverter::deserializeFloat(*buffer, offset);
    std::string resultStr = ByteConverter::deserializeString(*buffer, offset);

    EXPECT_EQ(resultInt, intVal);
    EXPECT_FLOAT_EQ(resultFloat, floatVal);
    EXPECT_EQ(resultStr, strVal);
}

TEST(ByteConverterTest, DeserializeNegativeStringLengthThrows) {
    std::vector<uint8_t> buffer;
    buffer.push_back(0xFF);
    buffer.push_back(0xFF);
    buffer.push_back(0xFF);
    buffer.push_back(0xFF);

    auto offset = std::make_shared<size_t>(0);
    EXPECT_THROW(ByteConverter::deserializeString(buffer, offset), std::out_of_range);
}

TEST(ByteConverterTest, SerializeIntNullBufferThrows) {
    EXPECT_THROW(ByteConverter::serializeInt(nullptr, 42), std::invalid_argument);
}

TEST(ByteConverterTest, SerializeFloatNullBufferThrows) {
    EXPECT_THROW(ByteConverter::serializeFloat(nullptr, 3.14f), std::invalid_argument);
}

TEST(ByteConverterTest, SerializeStringNullBufferThrows) {
    EXPECT_THROW(ByteConverter::serializeString(nullptr, "test"), std::invalid_argument);
}

TEST(CircularBufferTest, WriteAndRead) {
    CircularBuffer buffer(10);
    std::vector<uint8_t> data = {1, 2, 3, 4};
    buffer.write(data);

    EXPECT_EQ(buffer.size(), 4);
    EXPECT_FALSE(buffer.empty());

    auto readData = buffer.read(4);
    EXPECT_EQ(readData, data);
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTest, OverwriteWhenFull) {
    CircularBuffer buffer(5);
    std::vector<uint8_t> data1 = {1, 2, 3};
    std::vector<uint8_t> data2 = {4, 5, 6};
    buffer.write(data1);
    buffer.write(data2);

    EXPECT_EQ(buffer.size(), 5);
    EXPECT_TRUE(buffer.full());

    auto readData = buffer.read(5);
    EXPECT_EQ(readData, std::vector<uint8_t>({2, 3, 4, 5, 6}));
}

TEST(CircularBufferTest, ReadPartial) {
    CircularBuffer buffer(10);
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer.write(data);

    auto readData = buffer.read(3);
    EXPECT_EQ(readData, std::vector<uint8_t>({1, 2, 3}));
    EXPECT_EQ(buffer.size(), 2);

    readData = buffer.read(2);
    EXPECT_EQ(readData, std::vector<uint8_t>({4, 5}));
    EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTest, Clear) {
    CircularBuffer buffer(10);
    std::vector<uint8_t> data = {1, 2, 3};
    buffer.write(data);
    EXPECT_EQ(buffer.size(), 3);

    buffer.clear();
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTest, CapacityAndSize) {
    CircularBuffer buffer(5);
    EXPECT_EQ(buffer.capacity(), 5);
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());

    std::vector<uint8_t> data(5, 42);
    buffer.write(data);
    EXPECT_EQ(buffer.size(), 5);
    EXPECT_TRUE(buffer.full());
}

TEST(CircularBufferTest, ZeroCapacityThrows) {
    EXPECT_THROW(CircularBuffer(0), std::invalid_argument);
}
