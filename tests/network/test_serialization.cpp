/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_serialization
*/

#include "../../src/network/Packet.hpp"
#include "../../src/network/Serializer.hpp"
#include <gtest/gtest.h>

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
