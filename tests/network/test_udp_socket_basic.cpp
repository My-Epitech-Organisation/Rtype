/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** UdpSocket basic tests for coverage
*/

#include <gtest/gtest.h>

#include "UdpSocket.hpp"
#include "Packet.hpp"

using namespace rtype::network;

// =============================================================================
// UdpSocket Tests (placeholder implementation coverage)
// =============================================================================

TEST(UdpSocketTest, DefaultConstructor) {
    UdpSocket socket;
    // Just verify no crash
    SUCCEED();
}

TEST(UdpSocketTest, BindReturnsFalse) {
    UdpSocket socket;
    bool result = socket.bind(4242);
    EXPECT_FALSE(result);
}

TEST(UdpSocketTest, BindDifferentPorts) {
    UdpSocket socket;
    EXPECT_FALSE(socket.bind(0));
    EXPECT_FALSE(socket.bind(1024));
    EXPECT_FALSE(socket.bind(65535));
}

TEST(UdpSocketTest, ConnectReturnsFalse) {
    UdpSocket socket;
    bool result = socket.connect("127.0.0.1", 4242);
    EXPECT_FALSE(result);
}

TEST(UdpSocketTest, ConnectDifferentHosts) {
    UdpSocket socket;
    EXPECT_FALSE(socket.connect("localhost", 4242));
    EXPECT_FALSE(socket.connect("192.168.1.1", 8080));
    EXPECT_FALSE(socket.connect("0.0.0.0", 0));
}

TEST(UdpSocketTest, SendReturnsNegative) {
    UdpSocket socket;
    std::uint8_t data[] = {0x01, 0x02, 0x03};
    int result = socket.send(data, sizeof(data));
    EXPECT_EQ(result, -1);
}

TEST(UdpSocketTest, SendEmpty) {
    UdpSocket socket;
    int result = socket.send(nullptr, 0);
    EXPECT_EQ(result, -1);
}

TEST(UdpSocketTest, ReceiveReturnsNegative) {
    UdpSocket socket;
    std::uint8_t buffer[256];
    int result = socket.receive(buffer, sizeof(buffer));
    EXPECT_EQ(result, -1);
}

TEST(UdpSocketTest, ReceiveZeroBuffer) {
    UdpSocket socket;
    int result = socket.receive(nullptr, 0);
    EXPECT_EQ(result, -1);
}

TEST(UdpSocketTest, CloseNoException) {
    UdpSocket socket;
    EXPECT_NO_THROW(socket.close());
}

TEST(UdpSocketTest, DestructorNoException) {
    auto socket = std::make_unique<UdpSocket>();
    EXPECT_NO_THROW(socket.reset());
}

TEST(UdpSocketTest, MultipleCloses) {
    UdpSocket socket;
    socket.close();
    socket.close();
    socket.close();
    SUCCEED();
}

// =============================================================================
// Packet Tests
// =============================================================================

TEST(PacketTest, DefaultConstructor) {
    Packet packet;
    EXPECT_EQ(packet.type(), PacketType::Unknown);
}

TEST(PacketTest, ConstructWithType) {
    Packet packet(PacketType::PlayerInput);
    EXPECT_EQ(packet.type(), PacketType::PlayerInput);
}

TEST(PacketTest, AllPacketTypes) {
    Packet unknown(PacketType::Unknown);
    EXPECT_EQ(unknown.type(), PacketType::Unknown);

    Packet playerInput(PacketType::PlayerInput);
    EXPECT_EQ(playerInput.type(), PacketType::PlayerInput);

    Packet entityUpdate(PacketType::EntityUpdate);
    EXPECT_EQ(entityUpdate.type(), PacketType::EntityUpdate);

    Packet entitySpawn(PacketType::EntitySpawn);
    EXPECT_EQ(entitySpawn.type(), PacketType::EntitySpawn);

    Packet entityDestroy(PacketType::EntityDestroy);
    EXPECT_EQ(entityDestroy.type(), PacketType::EntityDestroy);
}

TEST(PacketTest, SetData) {
    Packet packet(PacketType::PlayerInput);
    std::vector<std::uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    packet.setData(data);

    EXPECT_EQ(packet.data(), data);
}

TEST(PacketTest, SetDataEmpty) {
    Packet packet(PacketType::EntityUpdate);
    std::vector<std::uint8_t> empty;
    packet.setData(empty);

    EXPECT_TRUE(packet.data().empty());
}

TEST(PacketTest, SetDataLarge) {
    Packet packet(PacketType::EntitySpawn);
    std::vector<std::uint8_t> largeData(1000, 0xAB);
    packet.setData(largeData);

    EXPECT_EQ(packet.data().size(), 1000u);
    EXPECT_EQ(packet.data()[0], 0xAB);
    EXPECT_EQ(packet.data()[999], 0xAB);
}

TEST(PacketTest, DataPersistsAcrossOperations) {
    Packet packet(PacketType::EntityDestroy);
    std::vector<std::uint8_t> data1 = {0x01, 0x02};
    packet.setData(data1);
    EXPECT_EQ(packet.data(), data1);

    std::vector<std::uint8_t> data2 = {0x03, 0x04, 0x05};
    packet.setData(data2);
    EXPECT_EQ(packet.data(), data2);
}

TEST(PacketTest, SetType) {
    Packet packet;
    EXPECT_EQ(packet.type(), PacketType::Unknown);

    packet.setType(PacketType::PlayerInput);
    EXPECT_EQ(packet.type(), PacketType::PlayerInput);

    packet.setType(PacketType::EntitySpawn);
    EXPECT_EQ(packet.type(), PacketType::EntitySpawn);
}

TEST(PacketTest, CopyConstructor) {
    Packet original(PacketType::EntitySpawn);
    std::vector<std::uint8_t> data = {0x10, 0x20, 0x30};
    original.setData(data);

    Packet copy = original;
    EXPECT_EQ(copy.type(), PacketType::EntitySpawn);
    EXPECT_EQ(copy.data(), data);
}

TEST(PacketTest, MoveConstructor) {
    Packet original(PacketType::EntityDestroy);
    std::vector<std::uint8_t> data = {0x40, 0x50};
    original.setData(data);

    Packet moved = std::move(original);
    EXPECT_EQ(moved.type(), PacketType::EntityDestroy);
    EXPECT_EQ(moved.data(), data);
}

TEST(PacketTest, PacketTypeEnumValues) {
    EXPECT_EQ(static_cast<std::uint8_t>(PacketType::Unknown), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(PacketType::PlayerInput), 1);
    EXPECT_EQ(static_cast<std::uint8_t>(PacketType::EntityUpdate), 2);
    EXPECT_EQ(static_cast<std::uint8_t>(PacketType::EntitySpawn), 3);
    EXPECT_EQ(static_cast<std::uint8_t>(PacketType::EntityDestroy), 4);
}
