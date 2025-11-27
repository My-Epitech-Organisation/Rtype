/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_packet
*/

#include "../../src/network/Packet.hpp"
#include <gtest/gtest.h>

using namespace rtype::network;

TEST(PacketTest, DefaultConstructorSetsUnknownType) {
    Packet packet;

    EXPECT_EQ(packet.type(), PacketType::Unknown);
    EXPECT_TRUE(packet.data().empty());
}

TEST(PacketTest, ConstructorWithPlayerInputType) {
    Packet packet(PacketType::PlayerInput);

    EXPECT_EQ(packet.type(), PacketType::PlayerInput);
}

TEST(PacketTest, ConstructorWithEntityUpdateType) {
    Packet packet(PacketType::EntityUpdate);

    EXPECT_EQ(packet.type(), PacketType::EntityUpdate);
}

TEST(PacketTest, ConstructorWithEntitySpawnType) {
    Packet packet(PacketType::EntitySpawn);

    EXPECT_EQ(packet.type(), PacketType::EntitySpawn);
}

TEST(PacketTest, ConstructorWithEntityDestroyType) {
    Packet packet(PacketType::EntityDestroy);

    EXPECT_EQ(packet.type(), PacketType::EntityDestroy);
}

TEST(PacketTest, SetTypChangesType) {
    Packet packet;
    EXPECT_EQ(packet.type(), PacketType::Unknown);

    packet.setType(PacketType::PlayerInput);
    EXPECT_EQ(packet.type(), PacketType::PlayerInput);

    packet.setType(PacketType::EntityDestroy);
    EXPECT_EQ(packet.type(), PacketType::EntityDestroy);
}

TEST(PacketTest, SetDataStoresData) {
    Packet packet;
    std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04};

    packet.setData(testData);

    EXPECT_EQ(packet.data(), testData);
    EXPECT_EQ(packet.data().size(), 4u);
}

TEST(PacketTest, SetEmptyData) {
    Packet packet;
    std::vector<uint8_t> emptyData;

    packet.setData(emptyData);

    EXPECT_TRUE(packet.data().empty());
}

TEST(PacketTest, SetLargeData) {
    Packet packet;
    std::vector<uint8_t> largeData(1024, 0xAB);

    packet.setData(largeData);

    EXPECT_EQ(packet.data().size(), 1024u);
    EXPECT_EQ(packet.data()[0], 0xAB);
    EXPECT_EQ(packet.data()[1023], 0xAB);
}

TEST(PacketTest, OverwriteExistingData) {
    Packet packet;
    std::vector<uint8_t> data1 = {1, 2, 3};
    std::vector<uint8_t> data2 = {4, 5, 6, 7, 8};

    packet.setData(data1);
    EXPECT_EQ(packet.data().size(), 3u);

    packet.setData(data2);
    EXPECT_EQ(packet.data().size(), 5u);
    EXPECT_EQ(packet.data(), data2);
}

TEST(PacketTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(PacketType::Unknown), 0);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::PlayerInput), 1);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::EntityUpdate), 2);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::EntitySpawn), 3);
    EXPECT_EQ(static_cast<uint8_t>(PacketType::EntityDestroy), 4);
}
