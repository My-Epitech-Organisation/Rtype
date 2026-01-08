#include <gtest/gtest.h>

#include "server/lobby/LobbyDiscoveryServer.hpp"
#include "server/lobby/LobbyManager.hpp"

#include "protocol/Header.hpp"

using namespace rtype::server;
using namespace rtype::network;

TEST(LobbyDiscoveryServerTest, BuildLobbyListPacketHasHeaderAndCount) {
    LobbyManager::Config cfg;
    cfg.basePort = 43300;
    cfg.instanceCount = 1;
    cfg.maxInstances = 4;

    LobbyManager manager(cfg);

    auto code = manager.createLobby(true);
    ASSERT_FALSE(code.empty());

    // Allow lobby to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    LobbyDiscoveryServer server(cfg.basePort, manager);

    // Start and stop the discovery server to exercise start/stop/poll paths
    ASSERT_TRUE(server.start());
    EXPECT_TRUE(server.isRunning());

    // Poll should be safe to call
    server.poll();

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(LobbyDiscoveryServerTest, BuildLobbyListPacketZeroLobbies) {
    LobbyManager::Config cfg;
    cfg.basePort = 43310;
    cfg.instanceCount = 1; // manager allowed, but don't start any lobbies

    LobbyManager manager(cfg);
    LobbyDiscoveryServer server(cfg.basePort, manager);

    auto packet = server.buildLobbyListPacket();
    // Packet should contain header + payload (1 byte count == 0)
    ASSERT_GE(packet.size(), static_cast<size_t>(rtype::network::kHeaderSize + 1));

    rtype::network::Header header;
    std::memcpy(&header, packet.data(), rtype::network::kHeaderSize);
    EXPECT_EQ(header.magic, rtype::network::kMagicByte);
    EXPECT_EQ(header.opcode, static_cast<std::uint8_t>(rtype::network::OpCode::S_LOBBY_LIST));

    size_t payloadOffset = rtype::network::kHeaderSize;
    EXPECT_EQ(packet[payloadOffset], 0); // count == 0
}

TEST(LobbyDiscoveryServerTest, BuildLobbyListPacketRespectsMax) {
    LobbyManager::Config cfg;
    cfg.basePort = 43320;
    cfg.instanceCount = 10; // create more than the max response size
    cfg.maxInstances = 10;

    LobbyManager manager(cfg);

    // Create several lobbies
    std::vector<std::string> codes;
    for (int i = 0; i < 8; ++i) {
        codes.push_back(manager.createLobby(true));
    }
    // Allow lobbies to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    LobbyDiscoveryServer server(cfg.basePort, manager);
    auto packet = server.buildLobbyListPacket();

    // First payload byte is count and must be <= kMaxLobbiesInResponse
    size_t payloadOffset = rtype::network::kHeaderSize;
    std::uint8_t count = packet[payloadOffset];
    EXPECT_LE(count, rtype::network::kMaxLobbiesInResponse);

    // Payload size in header should match (1 + count * sizeof(LobbyInfo))
    rtype::network::Header header;
    std::memcpy(&header, packet.data(), rtype::network::kHeaderSize);
    auto payloadSize = rtype::network::ByteOrderSpec::fromNetwork(header.payloadSize);
    EXPECT_EQ(payloadSize, static_cast<std::uint16_t>(1 + count * sizeof(rtype::network::LobbyInfo)));
}
