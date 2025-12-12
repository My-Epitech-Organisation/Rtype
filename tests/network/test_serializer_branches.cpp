#include <gtest/gtest.h>

#include "Serializer.hpp"

using namespace rtype::network;

TEST(SerializerBranches, SerializeDeserializePacketEmpty) {
    Packet p;
    auto data = Serializer::serialize(p);
    Packet d = Serializer::deserialize(data);
    ASSERT_EQ(d.type(), PacketType::Unknown);
}

TEST(SerializerBranches, SerializeDeserializePacketWithPayload) {
    Packet p(PacketType::PlayerInput);
    std::vector<uint8_t> payload = {1, 2, 3};
    p.setData(payload);
    auto data = Serializer::serialize(p);
    ASSERT_GT(data.size(), 1u);
    Packet d = Serializer::deserialize(data);
    ASSERT_EQ(d.type(), PacketType::PlayerInput);
    ASSERT_EQ(d.data(), payload);
}

TEST(SerializerBranches, DeserializeStringSmallBuffer) {
    std::vector<uint8_t> buf = {0x00, 0x00}; // less than uint32_t
    EXPECT_THROW(Serializer::deserializeString(buf), std::runtime_error);
}

TEST(SerializerBranches, DeserializeStringLengthMismatch) {
    // Length says 4 but buffer has only 2 bytes
    uint32_t length = 4;
    std::vector<uint8_t> buf(sizeof(uint32_t) + 2);
    rtype::network::ByteOrder::writeTo(buf.data(), length);
    buf[4] = 'A';
    buf[5] = 'B';
    EXPECT_THROW(Serializer::deserializeString(buf), std::runtime_error);
}
