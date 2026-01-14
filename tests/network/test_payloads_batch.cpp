/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Payloads batch tests - EntityMoveBatch coverage
*/

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"
#include "Serializer.hpp"

using namespace rtype::network;

// =============================================================================
// EntityMoveBatchHeader Tests
// =============================================================================

TEST(EntityMoveBatchHeaderTest, SizeIsOneByte) {
    EXPECT_EQ(sizeof(EntityMoveBatchHeader), 1u);
}

TEST(EntityMoveBatchHeaderTest, DefaultConstruction) {
    EntityMoveBatchHeader header{};
    EXPECT_EQ(header.count, 0);
}

TEST(EntityMoveBatchHeaderTest, SetCount) {
    EntityMoveBatchHeader header{};
    header.count = 42;
    EXPECT_EQ(header.count, 42);
}

TEST(EntityMoveBatchHeaderTest, MaxCount) {
    EntityMoveBatchHeader header{};
    header.count = static_cast<std::uint8_t>(kMaxEntitiesPerBatch);
    EXPECT_EQ(header.count, 69);
}

TEST(EntityMoveBatchHeaderTest, FullRange) {
    for (std::uint8_t i = 0; i <= 69; ++i) {
        EntityMoveBatchHeader header{};
        header.count = i;
        EXPECT_EQ(header.count, i);
    }
}

// =============================================================================
// kMaxEntitiesPerBatch Tests
// =============================================================================

TEST(MaxEntitiesPerBatchTest, ValueIs69) {
    EXPECT_EQ(kMaxEntitiesPerBatch, 69u);
}

TEST(MaxEntitiesPerBatchTest, FitsInPayload) {
    // 1 byte header + 69 * 20 bytes per entity = 1381 bytes
    // kMaxPayloadSize should be > 1381
    std::size_t batchSize = 1 + (kMaxEntitiesPerBatch * sizeof(EntityMovePayload));
    EXPECT_LE(batchSize, kMaxPayloadSize);
}

// =============================================================================
// EntityMovePayload Tests
// =============================================================================

TEST(EntityMovePayloadTest, Size) {
    EXPECT_EQ(sizeof(EntityMovePayload), 20u);
}

TEST(EntityMovePayloadTest, DefaultValues) {
    EntityMovePayload payload{};
    EXPECT_EQ(payload.entityId, 0u);
    EXPECT_EQ(payload.posX, 0.0f);
    EXPECT_EQ(payload.posY, 0.0f);
    EXPECT_EQ(payload.velX, 0.0f);
    EXPECT_EQ(payload.velY, 0.0f);
}

TEST(EntityMovePayloadTest, SetValues) {
    EntityMovePayload payload{};
    payload.entityId = 12345;
    payload.posX = 100.5f;
    payload.posY = 200.75f;
    payload.velX = 10.0f;
    payload.velY = -5.5f;

    EXPECT_EQ(payload.entityId, 12345u);
    EXPECT_FLOAT_EQ(payload.posX, 100.5f);
    EXPECT_FLOAT_EQ(payload.posY, 200.75f);
    EXPECT_FLOAT_EQ(payload.velX, 10.0f);
    EXPECT_FLOAT_EQ(payload.velY, -5.5f);
}

TEST(EntityMovePayloadTest, Serialization) {
    EntityMovePayload payload{};
    payload.entityId = 0xDEADBEEF;
    payload.posX = 123.456f;
    payload.posY = -789.012f;
    payload.velX = 1.5f;
    payload.velY = -2.5f;

    auto bytes = Serializer::serialize(payload);
    EXPECT_EQ(bytes.size(), sizeof(EntityMovePayload));

    EntityMovePayload deserialized{};
    std::memcpy(&deserialized, bytes.data(), sizeof(EntityMovePayload));

    EXPECT_EQ(deserialized.entityId, payload.entityId);
    EXPECT_FLOAT_EQ(deserialized.posX, payload.posX);
    EXPECT_FLOAT_EQ(deserialized.posY, payload.posY);
}

// =============================================================================
// EntityHealthPayload Tests
// =============================================================================

TEST(EntityHealthPayloadTest, Size) {
    EXPECT_EQ(sizeof(EntityHealthPayload), 12u);
}

TEST(EntityHealthPayloadTest, SetValues) {
    EntityHealthPayload payload{};
    payload.entityId = 999;
    payload.current = 75;
    payload.max = 100;

    EXPECT_EQ(payload.entityId, 999u);
    EXPECT_EQ(payload.current, 75);
    EXPECT_EQ(payload.max, 100);
}

TEST(EntityHealthPayloadTest, NegativeHealth) {
    EntityHealthPayload payload{};
    payload.current = -10;
    payload.max = 100;

    EXPECT_EQ(payload.current, -10);
}

TEST(EntityHealthPayloadTest, Serialization) {
    EntityHealthPayload payload{};
    payload.entityId = 42;
    payload.current = 50;
    payload.max = 100;

    auto bytes = Serializer::serialize(payload);
    EXPECT_EQ(bytes.size(), sizeof(EntityHealthPayload));
}

// =============================================================================
// PowerUpEventPayload Tests
// =============================================================================

TEST(PowerUpEventPayloadTest, Size) {
    EXPECT_EQ(sizeof(PowerUpEventPayload), 9u);
}

TEST(PowerUpEventPayloadTest, SetValues) {
    PowerUpEventPayload payload{};
    payload.playerId = 1;
    payload.powerUpType = 3;
    payload.duration = 10.5f;

    EXPECT_EQ(payload.playerId, 1u);
    EXPECT_EQ(payload.powerUpType, 3);
    EXPECT_FLOAT_EQ(payload.duration, 10.5f);
}

TEST(PowerUpEventPayloadTest, Serialization) {
    PowerUpEventPayload payload{};
    payload.playerId = 42;
    payload.powerUpType = 5;
    payload.duration = 30.0f;

    auto bytes = Serializer::serialize(payload);
    EXPECT_EQ(bytes.size(), sizeof(PowerUpEventPayload));
}

// =============================================================================
// EntityDestroyPayload Tests
// =============================================================================

TEST(EntityDestroyPayloadTest, Size) {
    EXPECT_EQ(sizeof(EntityDestroyPayload), 4u);
}

TEST(EntityDestroyPayloadTest, SetEntityId) {
    EntityDestroyPayload payload{};
    payload.entityId = 0xCAFEBABE;

    EXPECT_EQ(payload.entityId, 0xCAFEBABE);
}

TEST(EntityDestroyPayloadTest, Serialization) {
    EntityDestroyPayload payload{};
    payload.entityId = 0x12345678;

    auto bytes = Serializer::serialize(payload);
    EXPECT_EQ(bytes.size(), sizeof(EntityDestroyPayload));
}

// =============================================================================
// InputPayload Tests
// =============================================================================

TEST(InputPayloadTest, Size) {
    EXPECT_EQ(sizeof(InputPayload), 2u);
}

TEST(InputPayloadTest, InputMask) {
    InputPayload payload{};
    payload.inputMask = 0b10101010;

    EXPECT_EQ(payload.inputMask, 0b10101010);
}

TEST(InputPayloadTest, AllBitsSet) {
    InputPayload payload{};
    payload.inputMask = 0xFFFF;

    EXPECT_EQ(payload.inputMask, 0xFFFF);
}

// =============================================================================
// Batch Serialization Tests
// =============================================================================

TEST(BatchSerializationTest, HeaderSerialization) {
    EntityMoveBatchHeader header{};
    header.count = 5;

    auto bytes = Serializer::serialize(header);
    EXPECT_EQ(bytes.size(), sizeof(EntityMoveBatchHeader));
    EXPECT_EQ(bytes[0], 5);
}

TEST(BatchSerializationTest, MultipleMovePayloads) {
    std::vector<EntityMovePayload> moves;
    for (int i = 0; i < 10; ++i) {
        EntityMovePayload move{};
        move.entityId = static_cast<std::uint32_t>(i + 1);
        move.posX = static_cast<float>(i * 100);
        move.posY = static_cast<float>(i * 50);
        move.velX = 0.0f;
        move.velY = 0.0f;
        moves.push_back(move);
    }

    for (const auto& move : moves) {
        auto bytes = Serializer::serialize(move);
        EXPECT_EQ(bytes.size(), sizeof(EntityMovePayload));
    }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST(PayloadEdgeCasesTest, ZeroBatchCount) {
    EntityMoveBatchHeader header{};
    header.count = 0;

    EXPECT_EQ(header.count, 0);
}

TEST(PayloadEdgeCasesTest, MaxEntityId) {
    EntityMovePayload payload{};
    payload.entityId = 0xFFFFFFFF;

    EXPECT_EQ(payload.entityId, 0xFFFFFFFF);
}

TEST(PayloadEdgeCasesTest, NegativePosition) {
    EntityMovePayload payload{};
    payload.posX = -1000.0f;
    payload.posY = -2000.0f;

    EXPECT_FLOAT_EQ(payload.posX, -1000.0f);
    EXPECT_FLOAT_EQ(payload.posY, -2000.0f);
}

TEST(PayloadEdgeCasesTest, VerySmallFloat) {
    EntityMovePayload payload{};
    payload.velX = 0.0001f;
    payload.velY = -0.0001f;

    EXPECT_FLOAT_EQ(payload.velX, 0.0001f);
    EXPECT_FLOAT_EQ(payload.velY, -0.0001f);
}

TEST(PayloadEdgeCasesTest, LargeFloat) {
    EntityMovePayload payload{};
    payload.posX = 100000.0f;
    payload.posY = 100000.0f;

    EXPECT_FLOAT_EQ(payload.posX, 100000.0f);
    EXPECT_FLOAT_EQ(payload.posY, 100000.0f);
}

// =============================================================================
// Other Payload Types
// =============================================================================

TEST(EntitySpawnPayloadTest, Size) {
    EXPECT_EQ(sizeof(EntitySpawnPayload), 13u);
}

TEST(EntitySpawnPayloadTest, SetValues) {
    EntitySpawnPayload payload{};
    payload.entityId = 42;
    payload.type = static_cast<std::uint8_t>(EntityType::Player);
    payload.posX = 100.0f;
    payload.posY = 200.0f;

    EXPECT_EQ(payload.entityId, 42u);
    EXPECT_EQ(payload.getType(), EntityType::Player);
}

TEST(EntitySpawnPayloadTest, AllTypes) {
    EntitySpawnPayload payload{};

    payload.type = static_cast<std::uint8_t>(EntityType::Player);
    EXPECT_EQ(payload.getType(), EntityType::Player);

    payload.type = static_cast<std::uint8_t>(EntityType::Bydos);
    EXPECT_EQ(payload.getType(), EntityType::Bydos);

    payload.type = static_cast<std::uint8_t>(EntityType::Missile);
    EXPECT_EQ(payload.getType(), EntityType::Missile);

    payload.type = static_cast<std::uint8_t>(EntityType::Pickup);
    EXPECT_EQ(payload.getType(), EntityType::Pickup);

    payload.type = static_cast<std::uint8_t>(EntityType::Obstacle);
    EXPECT_EQ(payload.getType(), EntityType::Obstacle);
}

TEST(AcceptPayloadTest, Size) {
    EXPECT_EQ(sizeof(AcceptPayload), 4u);
}

TEST(AcceptPayloadTest, SetUserId) {
    AcceptPayload payload{};
    payload.newUserId = 12345;

    EXPECT_EQ(payload.newUserId, 12345u);
}

TEST(UpdateStatePayloadTest, Size) {
    EXPECT_EQ(sizeof(UpdateStatePayload), 1u);
}

TEST(UpdateStatePayloadTest, SetState) {
    UpdateStatePayload payload{};
    payload.stateId = static_cast<std::uint8_t>(GameState::Running);

    EXPECT_EQ(payload.getState(), GameState::Running);
}

TEST(UpdateStatePayloadTest, AllStates) {
    UpdateStatePayload payload{};

    payload.stateId = static_cast<std::uint8_t>(GameState::Lobby);
    EXPECT_EQ(payload.getState(), GameState::Lobby);

    payload.stateId = static_cast<std::uint8_t>(GameState::Running);
    EXPECT_EQ(payload.getState(), GameState::Running);

    payload.stateId = static_cast<std::uint8_t>(GameState::Paused);
    EXPECT_EQ(payload.getState(), GameState::Paused);

    payload.stateId = static_cast<std::uint8_t>(GameState::GameOver);
    EXPECT_EQ(payload.getState(), GameState::GameOver);
}

TEST(GameOverPayloadTest, Size) {
    EXPECT_EQ(sizeof(GameOverPayload), 8u);
}

TEST(GameOverPayloadTest, SetScore) {
    GameOverPayload payload{};
    payload.finalScore = 999999;

    EXPECT_EQ(payload.finalScore, 999999u);
}

TEST(DisconnectPayloadTest, Size) {
    EXPECT_EQ(sizeof(DisconnectPayload), 1u);
}

TEST(DisconnectPayloadTest, DefaultReason) {
    DisconnectPayload payload{};
    // Default is 4 (LocalRequest)
    EXPECT_EQ(payload.reason, 4);
}

TEST(GetUsersResponseHeaderTest, Size) {
    EXPECT_EQ(sizeof(GetUsersResponseHeader), 1u);
}

TEST(GetUsersResponseHeaderTest, SetCount) {
    GetUsersResponseHeader header{};
    header.count = 100;

    EXPECT_EQ(header.count, 100);
}

TEST(GetUsersResponseHeaderTest, MaxUsers) {
    EXPECT_EQ(kMaxUsersInResponse, 255u);

    GetUsersResponseHeader header{};
    header.count = static_cast<std::uint8_t>(kMaxUsersInResponse);
    EXPECT_EQ(header.count, 255);
}

// =============================================================================
// InputMask Tests
// =============================================================================

TEST(InputMaskTest, Values) {
    EXPECT_EQ(InputMask::kNone, 0x00);
    EXPECT_EQ(InputMask::kUp, 0x01);
    EXPECT_EQ(InputMask::kDown, 0x02);
    EXPECT_EQ(InputMask::kLeft, 0x04);
    EXPECT_EQ(InputMask::kRight, 0x08);
    EXPECT_EQ(InputMask::kShoot, 0x10);
}

TEST(InputMaskTest, Combinations) {
    std::uint8_t upRight = InputMask::kUp | InputMask::kRight;
    EXPECT_EQ(upRight, 0x09);

    std::uint8_t allDirections = InputMask::kUp | InputMask::kDown | InputMask::kLeft | InputMask::kRight;
    EXPECT_EQ(allDirections, 0x0F);

    std::uint8_t shootUp = InputMask::kShoot | InputMask::kUp;
    EXPECT_EQ(shootUp, 0x11);
}

TEST(InputMaskTest, AllCombinations) {
    // Test all direction combinations with shoot
    std::uint8_t allWithShoot = InputMask::kUp | InputMask::kDown |
                                 InputMask::kLeft | InputMask::kRight | InputMask::kShoot;
    EXPECT_EQ(allWithShoot, 0x1F);
}

// =============================================================================
// EntityType Tests
// =============================================================================

TEST(EntityTypeTest, Values) {
    EXPECT_EQ(static_cast<std::uint8_t>(EntityType::Player), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(EntityType::Bydos), 1);
    EXPECT_EQ(static_cast<std::uint8_t>(EntityType::Missile), 2);
    EXPECT_EQ(static_cast<std::uint8_t>(EntityType::Pickup), 3);
    EXPECT_EQ(static_cast<std::uint8_t>(EntityType::Obstacle), 4);
}

// =============================================================================
// GameState Tests
// =============================================================================

TEST(GameStateTest, Values) {
    EXPECT_EQ(static_cast<std::uint8_t>(GameState::Lobby), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(GameState::Running), 1);
    EXPECT_EQ(static_cast<std::uint8_t>(GameState::Paused), 2);
    EXPECT_EQ(static_cast<std::uint8_t>(GameState::GameOver), 3);
}

// =============================================================================
// ConnectPayload and GetUsersRequestPayload Tests (empty structs)
// =============================================================================

TEST(EmptyPayloadsTest, ConnectPayloadSize) {
    EXPECT_EQ(sizeof(ConnectPayload), 1u);  // Empty struct is 1 byte in C++
}

TEST(EmptyPayloadsTest, GetUsersRequestPayloadSize) {
    EXPECT_EQ(sizeof(GetUsersRequestPayload), 1u);  // Empty struct is 1 byte in C++
}
