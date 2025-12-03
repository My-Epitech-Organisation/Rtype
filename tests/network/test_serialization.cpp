/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_serialization
*/

#include <gtest/gtest.h>
#include <vector>

#include "../../src/network/Packet.hpp"
#include "../../src/network/Serializer.hpp"

using namespace rtype::network;

struct TestData {
    int x;
    float y;
};

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

TEST(SerializationTest, SerializeDeserializeStruct) {
    TestData original{42, 3.14f};
    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserialize<TestData>(serialized);

    EXPECT_EQ(deserialized.x, 42);
    EXPECT_FLOAT_EQ(deserialized.y, 3.14f);
}

TEST(SerializationTest, DeserializeInvalidSize) {
    std::vector<uint8_t> invalidBuffer = {1, 2, 3};  // Wrong size
    EXPECT_THROW(Serializer::deserialize<TestData>(invalidBuffer), std::runtime_error);
}

TEST(SerializationTest, FuzzingInvalidBuffers) {
    // Test with various invalid buffer sizes
    std::vector<std::vector<uint8_t>> invalidBuffers = {
        {},  // Empty
        {1},  // Too small
        {1, 2, 3, 4, 5},  // Too small
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}  // Too large
    };

    for (const auto& buffer : invalidBuffers) {
        EXPECT_THROW(Serializer::deserialize<TestData>(buffer), std::runtime_error);
    }
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
