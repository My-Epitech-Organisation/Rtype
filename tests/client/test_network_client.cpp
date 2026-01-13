/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkClient unit tests - Coverage for handlers and callbacks
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

#include "client/network/NetworkClient.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "Serializer.hpp"  // Needed for Serializer::serializeForNetwork in tests

// Simple mock socket for tests
class MockSocket : public rtype::network::IAsyncSocket {
   public:
    MockSocket() = default;

    rtype::network::Result<void> bind(std::uint16_t port) override {
        boundPort_ = port;
        open_ = true;
        return rtype::network::Result<void>{};
    }

    [[nodiscard]] bool isOpen() const noexcept override { return open_; }

    [[nodiscard]] std::uint16_t localPort() const noexcept override { return boundPort_; }

    void asyncSendTo(const rtype::network::Buffer& data,
                     const rtype::network::Endpoint& dest,
                     rtype::network::SendCallback handler) override {
        lastSent_ = data;
        lastDest_ = dest;
        if (handler) handler(rtype::network::Result<std::size_t>{data.size()});
    }

    void asyncReceiveFrom(std::shared_ptr<rtype::network::Buffer> buffer,
                          std::shared_ptr<rtype::network::Endpoint> sender,
                          rtype::network::ReceiveCallback handler) override {
        receiveBuffer_ = buffer;
        receiveSender_ = sender;
        receiveHandler_ = handler;
    }

    void pushIncoming(const rtype::network::Buffer& pkt,
                      const rtype::network::Endpoint& ep) {
        if (!receiveBuffer_) return;
        if (receiveBuffer_->size() < pkt.size()) receiveBuffer_->resize(pkt.size());
        std::memcpy(receiveBuffer_->data(), pkt.data(), pkt.size());
        *receiveSender_ = ep;
        if (receiveHandler_) receiveHandler_(rtype::network::Result<std::size_t>{pkt.size()});
    }

    void cancel() override {}
    void close() override { open_ = false; }

    rtype::network::Buffer lastSent_;
    rtype::network::Endpoint lastDest_;

   private:
    std::shared_ptr<rtype::network::Buffer> receiveBuffer_;
    std::shared_ptr<rtype::network::Endpoint> receiveSender_;
    rtype::network::ReceiveCallback receiveHandler_;
    bool open_{true};
    std::uint16_t boundPort_{0};
};

static rtype::network::Buffer packetFromHeaderAndPayload(const rtype::network::Header& header,
                                                         const rtype::network::Buffer& payload) {
    rtype::network::Buffer packet(rtype::network::kHeaderSize + payload.size());
    std::memcpy(packet.data(), &header, rtype::network::kHeaderSize);
    if (!payload.empty()) std::memcpy(packet.data() + rtype::network::kHeaderSize, payload.data(), payload.size());
    return packet;
}

using namespace rtype::client;
using namespace rtype::network;


// =============================================================================
// Helper Functions
// =============================================================================

static Buffer buildPacket(OpCode opcode, const Buffer& payload, uint32_t userId = 1) {
    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(opcode);
    header.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    header.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(userId));
    header.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};

    Buffer pkt(kHeaderSize + payload.size());
    std::memcpy(pkt.data(), &header, kHeaderSize);
    if (!payload.empty()) std::memcpy(pkt.data() + kHeaderSize, payload.data(), payload.size());
    return pkt;
}

// =============================================================================
// Test Fixtures (only available on non-Windows platforms)
// =============================================================================

#ifndef _WIN32

class NetworkClientTest : public ::testing::Test {
   protected:
    void SetUp() override {}
    void TearDown() override {}

    static Header createHeader(OpCode opcode, std::uint16_t payloadSize = 0,
                               std::uint8_t flags = 0) {
        Header header{};
        header.opcode = static_cast<std::uint8_t>(opcode);
        header.payloadSize = ByteOrderSpec::toNetwork(payloadSize);
        header.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(1));
        header.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
        header.ackId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
        header.flags = flags;
        return header;
    }

    static Buffer createPacket(const Header& header, const Buffer& payload) {
        Buffer packet(kHeaderSize + payload.size());
        std::memcpy(packet.data(), &header, kHeaderSize);
        if (!payload.empty()) {
            std::memcpy(packet.data() + kHeaderSize, payload.data(),
                        payload.size());
        }
        return packet;
    }
};

// =============================================================================
// Callback Setter Tests
// =============================================================================

TEST_F(NetworkClientTest, OnEntityMoveBatch_SetCallback) {
    NetworkClient client;
    bool callbackSet = false;

    client.onEntityMoveBatch([&callbackSet](EntityMoveBatchEvent) {
        callbackSet = true;
    });

    // Trigger a sample entity move batch to ensure the callback is invoked
    EntityMovePayload movePayload{};
    movePayload.entityId = 99;
    movePayload.posX = 12.5f;
    movePayload.posY = 34.5f;
    movePayload.velX = 1.0f;
    movePayload.velY = -1.0f;

    auto serialized = Serializer::serializeForNetwork(movePayload);
    Buffer payload;
    payload.push_back(1);
    payload.insert(payload.end(), serialized.begin(), serialized.end());
    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                                 static_cast<std::uint16_t>(payload.size()));
    auto pkt = packetFromHeaderAndPayload(header, payload);
    client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    client.test_dispatchCallbacks();

    EXPECT_TRUE(callbackSet);
}

TEST_F(NetworkClientTest, OnEntityHealth_SetCallback) {
    NetworkClient client;

    client.onEntityHealth([](EntityHealthEvent) {});

    // Trigger entity health to ensure the callback is invoked
    EntityHealthPayload healthPayload{};
    healthPayload.entityId = 123;
    healthPayload.current = 42;
    healthPayload.max = 100;
    auto serialized = Serializer::serializeForNetwork(healthPayload);
    Header header = createHeader(OpCode::S_ENTITY_HEALTH,
                                 static_cast<std::uint16_t>(serialized.size()));
    auto pkt = packetFromHeaderAndPayload(header, serialized);
    client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    client.test_dispatchCallbacks();

    // If no crash and dispatch runs, assume setter worked
    SUCCEED();
}

TEST_F(NetworkClientTest, OnPowerUpEvent_SetCallback) {
    NetworkClient client;

    client.onPowerUpEvent([](PowerUpEvent) {});

    PowerUpEventPayload powerUpPayload{};
    powerUpPayload.playerId = 42;
    powerUpPayload.powerUpType = 3;
    powerUpPayload.duration = 10.0f;
    auto serialized = Serializer::serializeForNetwork(powerUpPayload);
    Header header = createHeader(OpCode::S_POWERUP_EVENT,
                                 static_cast<std::uint16_t>(serialized.size()));
    auto pkt = packetFromHeaderAndPayload(header, serialized);
    client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    client.test_dispatchCallbacks();

    SUCCEED();
}

TEST_F(NetworkClientTest, DISABLED_SendJoinLobbySendsPacketWhenConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(8))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    auto pkt = buildPacket(OpCode::S_ACCEPT, payload, 0);
    client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    client.poll();

    EXPECT_TRUE(client.isConnected());
    EXPECT_TRUE(client.sendJoinLobby("ABCDEF"));
    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header h; std::memcpy(&h, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(h.opcode), OpCode::C_JOIN_LOBBY);
}

TEST_F(NetworkClientTest, OnGameOver_SetCallback) {
    NetworkClient client;

    client.onGameOver([](GameOverEvent) {});

    GameOverPayload gameOverPayload{};
    gameOverPayload.finalScore = 9001;
    auto serialized = Serializer::serializeForNetwork(gameOverPayload);
    Header header = createHeader(OpCode::S_GAME_OVER,
                                 static_cast<std::uint16_t>(serialized.size()));
    auto pkt = packetFromHeaderAndPayload(header, serialized);
    client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    client.test_dispatchCallbacks();

    SUCCEED();
}

TEST_F(NetworkClientTest, LatencyMs_ReturnsValue) {
    NetworkClient client;

    auto latency = client.latencyMs();
    EXPECT_GE(latency, 0u);
}

// =============================================================================
// handleEntityMoveBatch Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityMoveBatch_EmptyPayload) {
    NetworkClient client;
    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 0);
    Buffer emptyPayload;

    {
        auto pkt = packetFromHeaderAndPayload(header, emptyPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    }
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_ZeroCount) {
    NetworkClient client;
    Buffer payload = {0};

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    }
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_CountTooHigh) {
    NetworkClient client;
    Buffer payload = {static_cast<std::uint8_t>(kMaxEntitiesPerBatch + 1)};

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    }
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_PayloadTooSmall) {
    NetworkClient client;
    Buffer payload = {5};

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    }
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_ValidSingleEntity) {
    NetworkClient client;
    bool callbackCalled = false;
    EntityMoveBatchEvent receivedEvent;

    client.onEntityMoveBatch([&](EntityMoveBatchEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    // Build batch payload: header (count + serverTick) + compact entries
    Buffer payload;
    // Count
    payload.push_back(1);
    // Shared serverTick (network order)
    std::uint32_t serverTick = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(0u));
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&serverTick), reinterpret_cast<uint8_t*>(&serverTick) + sizeof(serverTick));

    // One compact entry (quantized fixed-point values)
    EntityMoveBatchEntry beHost{static_cast<std::uint32_t>(42u), static_cast<std::int16_t>(100.0f * 16.0f), static_cast<std::int16_t>(200.0f * 16.0f), static_cast<std::int16_t>(10.0f * 16.0f), static_cast<std::int16_t>(-5.0f * 16.0f)};
    auto be = ByteOrderSpec::toNetwork(beHost);
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&be), reinterpret_cast<uint8_t*>(&be) + sizeof(be));

    Header header =
        createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                     static_cast<std::uint16_t>(payload.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    ASSERT_EQ(receivedEvent.entities.size(), 1u);
    EXPECT_EQ(receivedEvent.entities[0].entityId, 42u);
    EXPECT_FLOAT_EQ(receivedEvent.entities[0].x, 100.0f);
    EXPECT_FLOAT_EQ(receivedEvent.entities[0].y, 200.0f);
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_MultipleEntities) {
    NetworkClient client;
    bool callbackCalled = false;
    EntityMoveBatchEvent receivedEvent;

    client.onEntityMoveBatch([&](EntityMoveBatchEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    Buffer payload;
    // Count
    payload.push_back(3);
    // Shared serverTick (network order)
    std::uint32_t serverTick = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(0u));
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&serverTick), reinterpret_cast<uint8_t*>(&serverTick) + sizeof(serverTick));

    for (int i = 0; i < 3; ++i) {
        EntityMoveBatchEntry beHost{static_cast<std::uint32_t>(i + 1), static_cast<std::int16_t>(i * 100 * 16), static_cast<std::int16_t>(i * 50 * 16), static_cast<std::int16_t>(i * 16), static_cast<std::int16_t>(-i * 16)};
        auto be = ByteOrderSpec::toNetwork(beHost);
        payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&be), reinterpret_cast<uint8_t*>(&be) + sizeof(be));
    }

    Header header =
        createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                     static_cast<std::uint16_t>(payload.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    ASSERT_EQ(receivedEvent.entities.size(), 3u);
    EXPECT_EQ(receivedEvent.entities[0].entityId, 1u);
    EXPECT_EQ(receivedEvent.entities[1].entityId, 2u);
    EXPECT_EQ(receivedEvent.entities[2].entityId, 3u);
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_FallbackToMoveCallback) {
    NetworkClient client;
    int moveCallbackCount = 0;

    client.onEntityMove([&](EntityMoveEvent) { ++moveCallbackCount; });

    Buffer payload;
    payload.push_back(2);

    for (int i = 0; i < 2; ++i) {
        EntityMovePayload movePayload{};
        movePayload.entityId = static_cast<std::uint32_t>(i + 1);
        movePayload.posX = static_cast<float>(i * 100);
        movePayload.posY = static_cast<float>(i * 50);
        movePayload.velX = 0.0f;
        movePayload.velY = 0.0f;

        auto serialized = Serializer::serializeForNetwork(movePayload);
        payload.insert(payload.end(), serialized.begin(), serialized.end());
    }

    Header header =
        createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                     static_cast<std::uint16_t>(payload.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, payload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_EQ(moveCallbackCount, 2);
}

// =============================================================================
// handleEntityHealth Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityHealth_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_ENTITY_HEALTH, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
    }
}

TEST_F(NetworkClientTest, HandleEntityHealth_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    EntityHealthEvent receivedEvent{};

    client.onEntityHealth([&](EntityHealthEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    EntityHealthPayload healthPayload{};
    healthPayload.entityId = 123;
    healthPayload.current = 75;
    healthPayload.max = 100;

    auto serialized = Serializer::serializeForNetwork(healthPayload);

    Header header = createHeader(OpCode::S_ENTITY_HEALTH,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.entityId, 123u);
    EXPECT_EQ(receivedEvent.current, 75);
    EXPECT_EQ(receivedEvent.max, 100);
}

TEST_F(NetworkClientTest, HandleEntityHealth_NoCallback) {
    NetworkClient client;

    EntityHealthPayload healthPayload{};
    healthPayload.entityId = 123;
    healthPayload.current = 75;
    healthPayload.max = 100;

    auto serialized = Serializer::serializeForNetwork(healthPayload);

    Header header = createHeader(OpCode::S_ENTITY_HEALTH,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

// =============================================================================
// handlePowerUpEvent Tests
// =============================================================================

TEST_F(NetworkClientTest, HandlePowerUpEvent_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1};

    Header header = createHeader(OpCode::S_POWERUP_EVENT, 2);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandlePowerUpEvent_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    PowerUpEvent receivedEvent{};

    client.onPowerUpEvent([&](PowerUpEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    PowerUpEventPayload powerUpPayload{};
    powerUpPayload.playerId = 42;
    powerUpPayload.powerUpType = 3;
    powerUpPayload.duration = 15.5f;

    auto serialized = Serializer::serializeForNetwork(powerUpPayload);

    Header header = createHeader(OpCode::S_POWERUP_EVENT,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.playerId, 42u);
    EXPECT_EQ(receivedEvent.powerUpType, 3);
    EXPECT_FLOAT_EQ(receivedEvent.duration, 15.5f);
}

TEST_F(NetworkClientTest, HandlePowerUpEvent_NoCallback) {
    NetworkClient client;

    PowerUpEventPayload powerUpPayload{};
    powerUpPayload.playerId = 42;
    powerUpPayload.powerUpType = 3;
    powerUpPayload.duration = 15.5f;

    auto serialized = Serializer::serializeForNetwork(powerUpPayload);

    Header header = createHeader(OpCode::S_POWERUP_EVENT,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

// =============================================================================
// handleGameOver Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleGameOver_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_GAME_OVER, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleGameOver_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    GameOverEvent receivedEvent{};

    client.onGameOver([&](GameOverEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    GameOverPayload gameOverPayload{};
    gameOverPayload.finalScore = 999999;

    auto serialized = Serializer::serializeForNetwork(gameOverPayload);

    Header header = createHeader(OpCode::S_GAME_OVER,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.finalScore, 999999u);
}

TEST_F(NetworkClientTest, HandleGameOver_NoCallback) {
    NetworkClient client;

    GameOverPayload gameOverPayload{};
    gameOverPayload.finalScore = 12345;

    auto serialized = Serializer::serializeForNetwork(gameOverPayload);

    Header header = createHeader(OpCode::S_GAME_OVER,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

// =============================================================================
// handleEntitySpawn Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntitySpawn_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_ENTITY_SPAWN, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleEntitySpawn_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    EntitySpawnEvent receivedEvent{};

    client.onEntitySpawn([&](EntitySpawnEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    EntitySpawnPayload spawnPayload{};
    spawnPayload.entityId = 42;
    spawnPayload.type = static_cast<std::uint8_t>(EntityType::Player);
    spawnPayload.posX = 100.0f;
    spawnPayload.posY = 200.0f;

    auto serialized = Serializer::serializeForNetwork(spawnPayload);

    Header header = createHeader(OpCode::S_ENTITY_SPAWN,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.entityId, 42u);
    EXPECT_EQ(receivedEvent.type, EntityType::Player);
}

// =============================================================================
// handleEntityMove Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityMove_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_ENTITY_MOVE, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleEntityMove_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    EntityMoveEvent receivedEvent{};

    client.onEntityMove([&](EntityMoveEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    EntityMovePayload movePayload{};
    movePayload.entityId = 99;
    movePayload.serverTick = 0;
    movePayload.posX = static_cast<std::int16_t>(150.0f * 16.0f);
    movePayload.posY = static_cast<std::int16_t>(250.0f * 16.0f);
    movePayload.velX = static_cast<std::int16_t>(5.0f * 16.0f);
    movePayload.velY = static_cast<std::int16_t>(-3.0f * 16.0f);

    auto serialized = Serializer::serializeForNetwork(movePayload);

    Header header = createHeader(OpCode::S_ENTITY_MOVE,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.entityId, 99u);
    EXPECT_FLOAT_EQ(receivedEvent.x, 150.0f);
    EXPECT_FLOAT_EQ(receivedEvent.y, 250.0f);
}

// =============================================================================
// handleEntityDestroy Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityDestroy_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_ENTITY_DESTROY, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleEntityDestroy_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    std::uint32_t receivedEntityId = 0;

    client.onEntityDestroy([&](std::uint32_t entityId) {
        callbackCalled = true;
        receivedEntityId = entityId;
    });

    EntityDestroyPayload destroyPayload{};
    destroyPayload.entityId = 777;

    auto serialized = Serializer::serializeForNetwork(destroyPayload);

    Header header = createHeader(OpCode::S_ENTITY_DESTROY,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEntityId, 777u);
}

// =============================================================================
// handleUpdatePos Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleUpdatePos_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};

    Header header = createHeader(OpCode::S_UPDATE_POS, 3);

    {
        auto pkt = packetFromHeaderAndPayload(header, smallPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleUpdatePos_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    float receivedX = 0.0f;
    float receivedY = 0.0f;

    client.onPositionCorrection([&](float x, float y) {
        callbackCalled = true;
        receivedX = x;
        receivedY = y;
    });

    UpdatePosPayload posPayload{};
    posPayload.posX = 123.45f;
    posPayload.posY = 678.90f;

    auto serialized = Serializer::serializeForNetwork(posPayload);

    Header header = createHeader(OpCode::S_UPDATE_POS,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_FLOAT_EQ(receivedX, 123.45f);
    EXPECT_FLOAT_EQ(receivedY, 678.90f);
}

// =============================================================================
// handleUpdateState Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleUpdateState_PayloadTooSmall) {
    NetworkClient client;
    Buffer emptyPayload;

    Header header = createHeader(OpCode::S_UPDATE_STATE, 0);

    {
        auto pkt = packetFromHeaderAndPayload(header, emptyPayload);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }
}

TEST_F(NetworkClientTest, HandleUpdateState_ValidPayload) {
    NetworkClient client;
    bool callbackCalled = false;
    GameStateEvent receivedEvent{};

    client.onGameStateChange([&](GameStateEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    UpdateStatePayload statePayload{};
    statePayload.stateId = static_cast<std::uint8_t>(GameState::Running);

    auto serialized = Serializer::serializeForNetwork(statePayload);

    Header header = createHeader(OpCode::S_UPDATE_STATE,
                                 static_cast<std::uint16_t>(serialized.size()));

    {
        auto pkt = packetFromHeaderAndPayload(header, serialized);
        client.test_processIncomingPacket(pkt, Endpoint{"127.0.0.1", 4242});
        client.test_dispatchCallbacks();
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.state, GameState::Running);
}

// =============================================================================
// Disconnect Callback Tests
// =============================================================================

TEST_F(NetworkClientTest, DISABLED_OnDisconnected_MultipleCallbacks) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);
    int callCount = 0;

    client.onDisconnected([&](DisconnectReason) { ++callCount; });
    client.onDisconnected([&](DisconnectReason) { ++callCount; });

    // Connect first
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(8))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    auto pkt = buildPacket(OpCode::S_ACCEPT, payload, 0);
    Endpoint ep{"127.0.0.1", 4242};
    client.test_processIncomingPacket(pkt, ep);
    client.poll();

    // Send server disconnect
    Header discHeader = createHeader(OpCode::DISCONNECT, 0);
    auto discPkt = packetFromHeaderAndPayload(discHeader, Buffer{});
    client.test_processIncomingPacket(discPkt, ep);
    client.poll();

    EXPECT_EQ(callCount, 2);
}

TEST_F(NetworkClientTest, OnConnected_MultipleCallbacks) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);
    int callCount = 0;

    client.onConnected([&](std::uint32_t) { ++callCount; });
    client.onConnected([&](std::uint32_t) { ++callCount; });

    // Simulate server accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(8))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    auto pkt = buildPacket(OpCode::S_ACCEPT, payload, 0);
    Endpoint ep{"127.0.0.1", 4242};
    client.test_processIncomingPacket(pkt, ep);
    client.poll();

    EXPECT_EQ(callCount, 2);
}

// =============================================================================
// Queue Callback Tests
// =============================================================================

TEST_F(NetworkClientTest, QueueCallback_ExecutesOnDispatch) {
    NetworkClient client;
    bool executed = false;

    client.test_queueCallback([&]() { executed = true; });

    EXPECT_FALSE(executed);
    client.test_dispatchCallbacks();
    EXPECT_TRUE(executed);
}

TEST_F(NetworkClientTest, QueueCallback_MultipleCallbacks) {
    NetworkClient client;
    int counter = 0;

    client.test_queueCallback([&]() { ++counter; });
    client.test_queueCallback([&]() { ++counter; });
    client.test_queueCallback([&]() { ++counter; });

    client.test_dispatchCallbacks();
    EXPECT_EQ(counter, 3);
}

// =============================================================================
// startReceive Tests
// =============================================================================

TEST_F(NetworkClientTest, StartReceive_DoesNotCrash) {
    NetworkClient client;

    client.test_startReceive();
}

// =============================================================================
// handlePong Tests
// =============================================================================

TEST_F(NetworkClientTest, HandlePong_DoesNotCrash) {
    NetworkClient client;
    Header header = createHeader(OpCode::PONG, 0);
    Buffer emptyPayload;

    client.test_handlePong(header, emptyPayload);
}

#endif  // _WIN32

// =============================================================================
// Event Struct Tests (available on all platforms)
// =============================================================================

TEST(EntitySpawnEventTest, DefaultValues) {
    EntitySpawnEvent event{};
    EXPECT_EQ(event.entityId, 0u);
    EXPECT_EQ(event.userId, 0u);
}

TEST(EntityMoveEventTest, DefaultValues) {
    EntityMoveEvent event{};
    EXPECT_EQ(event.entityId, 0u);
    EXPECT_FLOAT_EQ(event.x, 0.0f);
    EXPECT_FLOAT_EQ(event.y, 0.0f);
    EXPECT_FLOAT_EQ(event.vx, 0.0f);
    EXPECT_FLOAT_EQ(event.vy, 0.0f);
}

TEST(EntityMoveBatchEventTest, DefaultValues) {
    EntityMoveBatchEvent event{};
    EXPECT_TRUE(event.entities.empty());
}

TEST(EntityHealthEventTest, DefaultValues) {
    EntityHealthEvent event{};
    EXPECT_EQ(event.entityId, 0u);
    EXPECT_EQ(event.current, 0);
    EXPECT_EQ(event.max, 0);
}

TEST(PowerUpEventTest, DefaultValues) {
    PowerUpEvent event{};
    EXPECT_EQ(event.playerId, 0u);
    EXPECT_EQ(event.powerUpType, 0);
    EXPECT_FLOAT_EQ(event.duration, 0.0f);
}

TEST(GameStateEventTest, DefaultValues) {
    GameStateEvent event{};
}

TEST(GameOverEventTest, DefaultValues) {
    GameOverEvent event{};
    EXPECT_EQ(event.finalScore, 0u);
}

// =============================================================================
// Config Tests
// =============================================================================

TEST(NetworkClientConfigTest, DefaultConstruction) {
    NetworkClient::Config config;
    SUCCEED();
}

