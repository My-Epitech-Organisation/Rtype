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

// Include before NetworkClient to access private members for testing
#define private public
#define protected public

#include "client/network/NetworkClient.hpp"

#undef private
#undef protected

#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::client;
using namespace rtype::network;

// =============================================================================
// Test Fixtures
// =============================================================================

class NetworkClientTest : public ::testing::Test {
   protected:
    void SetUp() override {}
    void TearDown() override {}

    // Helper to create a valid header
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

    // Helper to create a packet buffer from header and payload
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

    EXPECT_NE(client.onEntityMoveBatchCallback_, nullptr);
}

TEST_F(NetworkClientTest, OnEntityHealth_SetCallback) {
    NetworkClient client;

    client.onEntityHealth([](EntityHealthEvent) {});

    EXPECT_NE(client.onEntityHealthCallback_, nullptr);
}

TEST_F(NetworkClientTest, OnPowerUpEvent_SetCallback) {
    NetworkClient client;

    client.onPowerUpEvent([](PowerUpEvent) {});

    EXPECT_NE(client.onPowerUpCallback_, nullptr);
}

TEST_F(NetworkClientTest, OnGameOver_SetCallback) {
    NetworkClient client;

    client.onGameOver([](GameOverEvent) {});

    EXPECT_NE(client.onGameOverCallback_, nullptr);
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

    // Should not crash with empty payload
    client.handleEntityMoveBatch(header, emptyPayload);
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_ZeroCount) {
    NetworkClient client;
    Buffer payload = {0};  // count = 0

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    client.handleEntityMoveBatch(header, payload);
    // Should return early without processing
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_CountTooHigh) {
    NetworkClient client;
    Buffer payload = {static_cast<std::uint8_t>(kMaxEntitiesPerBatch + 1)};

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    client.handleEntityMoveBatch(header, payload);
    // Should return early due to count > max
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_PayloadTooSmall) {
    NetworkClient client;
    Buffer payload = {5};  // count = 5, but no entity data

    Header header = createHeader(OpCode::S_ENTITY_MOVE_BATCH, 1);

    client.handleEntityMoveBatch(header, payload);
    // Should return early due to insufficient data
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_ValidSingleEntity) {
    NetworkClient client;
    bool callbackCalled = false;
    EntityMoveBatchEvent receivedEvent;

    client.onEntityMoveBatch([&](EntityMoveBatchEvent event) {
        callbackCalled = true;
        receivedEvent = event;
    });

    // Create payload with 1 entity
    EntityMovePayload movePayload{};
    movePayload.entityId = 42;
    movePayload.posX = 100.0f;
    movePayload.posY = 200.0f;
    movePayload.velX = 10.0f;
    movePayload.velY = -5.0f;

    auto serialized = Serializer::serializeForNetwork(movePayload);

    Buffer payload;
    payload.push_back(1);  // count = 1
    payload.insert(payload.end(), serialized.begin(), serialized.end());

    Header header =
        createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                     static_cast<std::uint16_t>(payload.size()));

    client.handleEntityMoveBatch(header, payload);
    client.dispatchCallbacks();

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

    // Create payload with 3 entities
    Buffer payload;
    payload.push_back(3);  // count = 3

    for (int i = 0; i < 3; ++i) {
        EntityMovePayload movePayload{};
        movePayload.entityId = static_cast<std::uint32_t>(i + 1);
        movePayload.posX = static_cast<float>(i * 100);
        movePayload.posY = static_cast<float>(i * 50);
        movePayload.velX = static_cast<float>(i);
        movePayload.velY = static_cast<float>(-i);

        auto serialized = Serializer::serializeForNetwork(movePayload);
        payload.insert(payload.end(), serialized.begin(), serialized.end());
    }

    Header header =
        createHeader(OpCode::S_ENTITY_MOVE_BATCH,
                     static_cast<std::uint16_t>(payload.size()));

    client.handleEntityMoveBatch(header, payload);
    client.dispatchCallbacks();

    EXPECT_TRUE(callbackCalled);
    ASSERT_EQ(receivedEvent.entities.size(), 3u);
    EXPECT_EQ(receivedEvent.entities[0].entityId, 1u);
    EXPECT_EQ(receivedEvent.entities[1].entityId, 2u);
    EXPECT_EQ(receivedEvent.entities[2].entityId, 3u);
}

TEST_F(NetworkClientTest, HandleEntityMoveBatch_FallbackToMoveCallback) {
    NetworkClient client;
    int moveCallbackCount = 0;

    // Only set onEntityMove, not onEntityMoveBatch
    client.onEntityMove([&](EntityMoveEvent) { ++moveCallbackCount; });

    // Create payload with 2 entities
    Buffer payload;
    payload.push_back(2);  // count = 2

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

    client.handleEntityMoveBatch(header, payload);
    client.dispatchCallbacks();

    // Should call onEntityMove for each entity
    EXPECT_EQ(moveCallbackCount, 2);
}

// =============================================================================
// handleEntityHealth Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityHealth_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};  // Too small for EntityHealthPayload

    Header header = createHeader(OpCode::S_ENTITY_HEALTH, 3);

    client.handleEntityHealth(header, smallPayload);
    // Should return early
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

    client.handleEntityHealth(header, serialized);
    client.dispatchCallbacks();

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

    // Should not crash even without callback set
    client.handleEntityHealth(header, serialized);
    client.dispatchCallbacks();
}

// =============================================================================
// handlePowerUpEvent Tests
// =============================================================================

TEST_F(NetworkClientTest, HandlePowerUpEvent_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1};  // Too small

    Header header = createHeader(OpCode::S_POWERUP_EVENT, 2);

    client.handlePowerUpEvent(header, smallPayload);
    // Should return early
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

    client.handlePowerUpEvent(header, serialized);
    client.dispatchCallbacks();

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

    // Should not crash without callback
    client.handlePowerUpEvent(header, serialized);
    client.dispatchCallbacks();
}

// =============================================================================
// handleGameOver Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleGameOver_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};  // Too small for GameOverPayload

    Header header = createHeader(OpCode::S_GAME_OVER, 3);

    client.handleGameOver(header, smallPayload);
    // Should return early
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

    client.handleGameOver(header, serialized);
    client.dispatchCallbacks();

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

    // Should not crash without callback
    client.handleGameOver(header, serialized);
    client.dispatchCallbacks();
}

// =============================================================================
// handleEntitySpawn Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntitySpawn_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};  // Too small

    Header header = createHeader(OpCode::S_ENTITY_SPAWN, 3);

    client.handleEntitySpawn(header, smallPayload);
    // Should return early
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

    client.handleEntitySpawn(header, serialized);
    client.dispatchCallbacks();

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.entityId, 42u);
    EXPECT_EQ(receivedEvent.type, EntityType::Player);
}

// =============================================================================
// handleEntityMove Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleEntityMove_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};  // Too small

    Header header = createHeader(OpCode::S_ENTITY_MOVE, 3);

    client.handleEntityMove(header, smallPayload);
    // Should return early
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
    movePayload.posX = 150.0f;
    movePayload.posY = 250.0f;
    movePayload.velX = 5.0f;
    movePayload.velY = -3.0f;

    auto serialized = Serializer::serializeForNetwork(movePayload);

    Header header = createHeader(OpCode::S_ENTITY_MOVE,
                                 static_cast<std::uint16_t>(serialized.size()));

    client.handleEntityMove(header, serialized);
    client.dispatchCallbacks();

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
    Buffer smallPayload = {0, 1, 2};  // Too small

    Header header = createHeader(OpCode::S_ENTITY_DESTROY, 3);

    client.handleEntityDestroy(header, smallPayload);
    // Should return early
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

    client.handleEntityDestroy(header, serialized);
    client.dispatchCallbacks();

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEntityId, 777u);
}

// =============================================================================
// handleUpdatePos Tests
// =============================================================================

TEST_F(NetworkClientTest, HandleUpdatePos_PayloadTooSmall) {
    NetworkClient client;
    Buffer smallPayload = {0, 1, 2};  // Too small

    Header header = createHeader(OpCode::S_UPDATE_POS, 3);

    client.handleUpdatePos(header, smallPayload);
    // Should return early
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

    client.handleUpdatePos(header, serialized);
    client.dispatchCallbacks();

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

    client.handleUpdateState(header, emptyPayload);
    // Should return early
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

    client.handleUpdateState(header, serialized);
    client.dispatchCallbacks();

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedEvent.state, GameState::Running);
}

// =============================================================================
// Disconnect Callback Tests
// =============================================================================

TEST_F(NetworkClientTest, OnDisconnected_MultipleCallbacks) {
    NetworkClient client;
    int callCount = 0;

    client.onDisconnected([&](DisconnectReason) { ++callCount; });
    client.onDisconnected([&](DisconnectReason) { ++callCount; });

    EXPECT_EQ(client.onDisconnectedCallbacks_.size(), 2u);
}

TEST_F(NetworkClientTest, OnConnected_MultipleCallbacks) {
    NetworkClient client;

    client.onConnected([](std::uint32_t) {});
    client.onConnected([](std::uint32_t) {});

    EXPECT_EQ(client.onConnectedCallbacks_.size(), 2u);
}

// =============================================================================
// Queue Callback Tests
// =============================================================================

TEST_F(NetworkClientTest, QueueCallback_ExecutesOnDispatch) {
    NetworkClient client;
    bool executed = false;

    client.queueCallback([&]() { executed = true; });

    EXPECT_FALSE(executed);
    client.dispatchCallbacks();
    EXPECT_TRUE(executed);
}

TEST_F(NetworkClientTest, QueueCallback_MultipleCallbacks) {
    NetworkClient client;
    int counter = 0;

    client.queueCallback([&]() { ++counter; });
    client.queueCallback([&]() { ++counter; });
    client.queueCallback([&]() { ++counter; });

    client.dispatchCallbacks();
    EXPECT_EQ(counter, 3);
}

// =============================================================================
// startReceive Tests
// =============================================================================

TEST_F(NetworkClientTest, StartReceive_DoesNotCrash) {
    NetworkClient client;

    // Should not crash even if socket is not connected
    // startReceive has early return if socket is not open
    client.startReceive();
}

// =============================================================================
// handlePong Tests
// =============================================================================

TEST_F(NetworkClientTest, HandlePong_DoesNotCrash) {
    NetworkClient client;
    Header header = createHeader(OpCode::PONG, 0);
    Buffer emptyPayload;

    // Should not crash
    client.handlePong(header, emptyPayload);
}

// =============================================================================
// Event Struct Tests
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
    // Default enum value
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
    // Should compile and not crash
    SUCCEED();
}
