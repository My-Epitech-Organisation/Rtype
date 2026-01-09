/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer Packet tests for coverage
*/

#include <gtest/gtest.h>

#include "Serializer.hpp"
#include "Packet.hpp"

using namespace rtype::network;

// =============================================================================
// Packet Serialization Tests
// =============================================================================

TEST(SerializerPacketTest, SerializeEmptyPacket) {
    Packet packet(PacketType::Unknown);
    auto bytes = Serializer::serialize(packet);

    EXPECT_EQ(bytes.size(), 1u);
    EXPECT_EQ(bytes[0], static_cast<std::uint8_t>(PacketType::Unknown));
}

TEST(SerializerPacketTest, SerializePacketWithData) {
    Packet packet(PacketType::PlayerInput);
    std::vector<std::uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    packet.setData(data);

    auto bytes = Serializer::serialize(packet);

    EXPECT_EQ(bytes.size(), 5u);
    EXPECT_EQ(bytes[0], static_cast<std::uint8_t>(PacketType::PlayerInput));
    EXPECT_EQ(bytes[1], 0x01);
    EXPECT_EQ(bytes[2], 0x02);
    EXPECT_EQ(bytes[3], 0x03);
    EXPECT_EQ(bytes[4], 0x04);
}

TEST(SerializerPacketTest, SerializeAllPacketTypes) {
    std::vector<PacketType> types = {
        PacketType::Unknown,
        PacketType::PlayerInput,
        PacketType::EntityUpdate,
        PacketType::EntitySpawn,
        PacketType::EntityDestroy
    };

    for (auto type : types) {
        Packet packet(type);
        auto bytes = Serializer::serialize(packet);
        EXPECT_EQ(bytes[0], static_cast<std::uint8_t>(type));
    }
}

TEST(SerializerPacketTest, DeserializeEmptyBuffer) {
    std::vector<std::uint8_t> empty;
    Packet packet = Serializer::deserialize(empty);

    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(SerializerPacketTest, DeserializeTypeOnly) {
    std::vector<std::uint8_t> data = {static_cast<std::uint8_t>(PacketType::EntitySpawn)};
    Packet packet = Serializer::deserialize(data);

    EXPECT_EQ(packet.type(), PacketType::EntitySpawn);
    EXPECT_TRUE(packet.data().empty());
}

TEST(SerializerPacketTest, DeserializeWithPayload) {
    std::vector<std::uint8_t> data = {
        static_cast<std::uint8_t>(PacketType::EntityUpdate),
        0xAA, 0xBB, 0xCC
    };
    Packet packet = Serializer::deserialize(data);

    EXPECT_EQ(packet.type(), PacketType::EntityUpdate);
    ASSERT_EQ(packet.data().size(), 3u);
    EXPECT_EQ(packet.data()[0], 0xAA);
    EXPECT_EQ(packet.data()[1], 0xBB);
    EXPECT_EQ(packet.data()[2], 0xCC);
}

TEST(SerializerPacketTest, RoundtripEmpty) {
    Packet original(PacketType::EntityDestroy);
    auto bytes = Serializer::serialize(original);
    Packet deserialized = Serializer::deserialize(bytes);

    EXPECT_EQ(deserialized.type(), original.type());
    EXPECT_EQ(deserialized.data(), original.data());
}

TEST(SerializerPacketTest, RoundtripWithData) {
    Packet original(PacketType::PlayerInput);
    std::vector<std::uint8_t> payload = {0x10, 0x20, 0x30, 0x40, 0x50};
    original.setData(payload);

    auto bytes = Serializer::serialize(original);
    Packet deserialized = Serializer::deserialize(bytes);

    EXPECT_EQ(deserialized.type(), original.type());
    EXPECT_EQ(deserialized.data(), original.data());
}

TEST(SerializerPacketTest, SerializeLargePayload) {
    Packet packet(PacketType::EntityUpdate);
    std::vector<std::uint8_t> largeData(1000, 0x42);
    packet.setData(largeData);

    auto bytes = Serializer::serialize(packet);

    EXPECT_EQ(bytes.size(), 1001u);
    EXPECT_EQ(bytes[0], static_cast<std::uint8_t>(PacketType::EntityUpdate));
    for (std::size_t i = 1; i < bytes.size(); ++i) {
        EXPECT_EQ(bytes[i], 0x42);
    }
}

// =============================================================================
// String Serialization Tests
// =============================================================================

TEST(SerializerStringTest, SerializeEmptyString) {
    std::string empty;
    auto bytes = Serializer::serialize(empty);

    EXPECT_EQ(bytes.size(), sizeof(std::uint32_t));
}

TEST(SerializerStringTest, SerializeShortString) {
    std::string str = "Hello";
    auto bytes = Serializer::serialize(str);

    EXPECT_EQ(bytes.size(), sizeof(std::uint32_t) + 5);
}

TEST(SerializerStringTest, DeserializeEmptyString) {
    std::string original;
    auto bytes = Serializer::serialize(original);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializerStringTest, DeserializeShortString) {
    std::string original = "Test";
    auto bytes = Serializer::serialize(original);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializerStringTest, RoundtripString) {
    std::string original = "Hello, World! This is a test string.";
    auto bytes = Serializer::serialize(original);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized, original);
}

TEST(SerializerStringTest, DeserializeBufferTooSmall) {
    std::vector<std::uint8_t> tooSmall = {0x01, 0x02};

    EXPECT_THROW(Serializer::deserializeString(tooSmall), std::runtime_error);
}

TEST(SerializerStringTest, DeserializeBufferSizeMismatch) {
    std::vector<std::uint8_t> buffer(sizeof(std::uint32_t));
    // Set length to 100 but buffer doesn't have 100 more bytes
    std::uint32_t fakeLength = 100;
    std::memcpy(buffer.data(), &fakeLength, sizeof(std::uint32_t));

    EXPECT_THROW(Serializer::deserializeString(buffer), std::runtime_error);
}

TEST(SerializerStringTest, SerializeLongString) {
    std::string longStr(1000, 'x');
    auto bytes = Serializer::serialize(longStr);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized, longStr);
}

TEST(SerializerStringTest, SerializeSpecialCharacters) {
    std::string special = "Hello\n\t\r\0World";
    auto bytes = Serializer::serialize(special);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized.size(), special.size());
}

TEST(SerializerStringTest, SerializeUnicodeString) {
    std::string unicode = "H\xc3\xa9llo W\xc3\xb6rld \xe4\xbd\xa0\xe5\xa5\xbd";
    auto bytes = Serializer::serialize(unicode);
    std::string deserialized = Serializer::deserializeString(bytes);

    EXPECT_EQ(deserialized, unicode);
}
