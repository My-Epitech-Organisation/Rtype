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

TEST(SerializationTest, RoundTripAllPacketTypes) {
    std::vector<PacketType> types = {
        PacketType::Unknown,
        PacketType::PlayerInput,
        PacketType::EntityUpdate,
        PacketType::EntitySpawn,
        PacketType::EntityDestroy
    };

    for (auto type : types) {
        Packet original(type);
        auto serialized = Serializer::serialize(original);
        auto deserialized = Serializer::deserialize(serialized);
        EXPECT_EQ(deserialized.type(), type);
    }
}

TEST(SerializationTest, EmptyPacketData) {
    Packet packet(PacketType::PlayerInput);
    auto serialized = Serializer::serialize(packet);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_TRUE(deserialized.data().empty());
}

TEST(SerializationTest, LargePacketData) {
    Packet packet(PacketType::EntityUpdate);
    std::vector<uint8_t> largeData(1000, 0xFF);
    packet.setData(largeData);

    auto serialized = Serializer::serialize(packet);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.data().size(), 1000u);
    EXPECT_EQ(deserialized.data(), largeData);
}

TEST(SerializationTest, BinaryDataIntegrity) {
    Packet packet(PacketType::EntitySpawn);
    std::vector<uint8_t> binaryData = {0x00, 0x01, 0x7F, 0x80, 0xFE, 0xFF};
    packet.setData(binaryData);

    auto serialized = Serializer::serialize(packet);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.data(), binaryData);
}

TEST(SerializationTest, MultipleSerializationsAreConsistent) {
    Packet packet(PacketType::PlayerInput);
    std::vector<uint8_t> data = {1, 2, 3};
    packet.setData(data);

    auto serialized1 = Serializer::serialize(packet);
    auto serialized2 = Serializer::serialize(packet);

    EXPECT_EQ(serialized1, serialized2);
}

TEST(SerializationTest, DeserializeEmptyVector) {
    std::vector<uint8_t> emptyData;
    auto packet = Serializer::deserialize(emptyData);

    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(SerializationTest, DeserializeOnlyType) {
    std::vector<uint8_t> typeOnly = {static_cast<uint8_t>(PacketType::EntityDestroy)};
    auto packet = Serializer::deserialize(typeOnly);

    EXPECT_EQ(packet.type(), PacketType::EntityDestroy);
    EXPECT_TRUE(packet.data().empty());
}

TEST(SerializationTest, SerializedSizeMatchesExpected) {
    Packet packet(PacketType::PlayerInput);
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    packet.setData(data);

    auto serialized = Serializer::serialize(packet);

    // Size should be 1 (type) + 5 (data) = 6
    EXPECT_EQ(serialized.size(), 6u);
}

TEST(SerializationTest, SerializeEmptyDataPacket) {
    Packet packet(PacketType::Unknown);
    auto serialized = Serializer::serialize(packet);

    // Only the type byte
    EXPECT_EQ(serialized.size(), 1u);
    EXPECT_EQ(serialized[0], static_cast<uint8_t>(PacketType::Unknown));
}

TEST(SerializationTest, DataPreservedAfterRoundTrip) {
    Packet original(PacketType::EntityUpdate);
    std::vector<uint8_t> testData;
    for (uint8_t i = 0; i < 255; ++i) {
        testData.push_back(i);
    }
    original.setData(testData);

    auto serialized = Serializer::serialize(original);
    auto deserialized = Serializer::deserialize(serialized);

    EXPECT_EQ(deserialized.data().size(), 255u);
    for (size_t i = 0; i < 255; ++i) {
        EXPECT_EQ(deserialized.data()[i], static_cast<uint8_t>(i));
    }
}

TEST(SerializationTest, UnknownPacketTypeValue) {
    // Simulate an unknown packet type (value 99)
    std::vector<uint8_t> unknownType = {99, 1, 2, 3};
    auto packet = Serializer::deserialize(unknownType);

    // Should cast to the enum value (may be undefined behavior in strict terms)
    // but the data should still be preserved
    EXPECT_EQ(packet.data().size(), 3u);
}
