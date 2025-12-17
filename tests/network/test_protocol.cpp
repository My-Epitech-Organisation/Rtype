/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Protocol tests - OpCode, Header, Payloads validation
*/

#include <gtest/gtest.h>

#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "protocol/Validator.hpp"

using namespace rtype::network;

// ============================================================================
// OpCode Tests
// ============================================================================

class OpCodeTest : public ::testing::Test {};

TEST_F(OpCodeTest, OpCodeValuesMatchRFC) {
    // RFC Section 5.1 - Session Management
    EXPECT_EQ(static_cast<uint8_t>(OpCode::C_CONNECT), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_ACCEPT), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::DISCONNECT), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::C_GET_USERS), 0x04);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::R_GET_USERS), 0x05);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_UPDATE_STATE), 0x06);

    // RFC Section 5.2 - Entity Management
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_ENTITY_SPAWN), 0x10);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_ENTITY_MOVE), 0x11);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_ENTITY_DESTROY), 0x12);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_ENTITY_HEALTH), 0x13);

    // RFC Section 5.3 - Input & Reconciliation
    EXPECT_EQ(static_cast<uint8_t>(OpCode::C_INPUT), 0x20);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::S_UPDATE_POS), 0x21);

    // RFC Section 7 - Reserved
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PING), 0xF0);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PONG), 0xF1);
}

TEST_F(OpCodeTest, IsReliableMatchesRFC) {
    // Reliable opcodes (require ACK)
    EXPECT_TRUE(isReliable(OpCode::C_CONNECT));
    EXPECT_TRUE(isReliable(OpCode::S_ACCEPT));
    EXPECT_TRUE(isReliable(OpCode::DISCONNECT));
    EXPECT_TRUE(isReliable(OpCode::C_GET_USERS));
    EXPECT_TRUE(isReliable(OpCode::R_GET_USERS));
    EXPECT_TRUE(isReliable(OpCode::S_UPDATE_STATE));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_SPAWN));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_DESTROY));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_HEALTH));
    EXPECT_TRUE(isReliable(OpCode::S_GAME_OVER));
    EXPECT_TRUE(isReliable(OpCode::S_POWERUP_EVENT));

    // Unreliable opcodes (no ACK needed)
    EXPECT_FALSE(isReliable(OpCode::S_ENTITY_MOVE));
    EXPECT_FALSE(isReliable(OpCode::C_INPUT));
    EXPECT_FALSE(isReliable(OpCode::S_UPDATE_POS));
    EXPECT_FALSE(isReliable(OpCode::PING));
    EXPECT_FALSE(isReliable(OpCode::PONG));
}

TEST_F(OpCodeTest, IsClientOpCode) {
    EXPECT_TRUE(isClientOpCode(OpCode::C_CONNECT));
    EXPECT_TRUE(isClientOpCode(OpCode::C_GET_USERS));
    EXPECT_TRUE(isClientOpCode(OpCode::C_INPUT));
    EXPECT_TRUE(isClientOpCode(OpCode::DISCONNECT));  // Can be either

    EXPECT_FALSE(isClientOpCode(OpCode::S_ACCEPT));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_SPAWN));
}

TEST_F(OpCodeTest, IsServerOpCode) {
    EXPECT_TRUE(isServerOpCode(OpCode::S_ACCEPT));
    EXPECT_TRUE(isServerOpCode(OpCode::R_GET_USERS));
    EXPECT_TRUE(isServerOpCode(OpCode::S_UPDATE_STATE));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_SPAWN));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_MOVE));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_DESTROY));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_HEALTH));
    EXPECT_TRUE(isServerOpCode(OpCode::S_UPDATE_POS));
    EXPECT_TRUE(isServerOpCode(OpCode::DISCONNECT));  // Can be either

    EXPECT_FALSE(isServerOpCode(OpCode::C_CONNECT));
    EXPECT_FALSE(isServerOpCode(OpCode::C_INPUT));
}

TEST_F(OpCodeTest, IsValidOpCode) {
    // Valid opcodes
    EXPECT_TRUE(isValidOpCode(0x01));
    EXPECT_TRUE(isValidOpCode(0x10));
    EXPECT_TRUE(isValidOpCode(0x13));
    EXPECT_TRUE(isValidOpCode(0x20));
    EXPECT_TRUE(isValidOpCode(0xF0));

    // Invalid opcodes
    EXPECT_FALSE(isValidOpCode(0x00));
    EXPECT_TRUE(isValidOpCode(0x07));  // S_GAME_OVER is a valid OpCode
    EXPECT_FALSE(isValidOpCode(0x30));
    EXPECT_FALSE(isValidOpCode(0xFF));  // Not defined
}

TEST_F(OpCodeTest, ToString) {
    EXPECT_EQ(toString(OpCode::C_CONNECT), "C_CONNECT");
    EXPECT_EQ(toString(OpCode::S_ENTITY_SPAWN), "S_ENTITY_SPAWN");
    EXPECT_EQ(toString(OpCode::PING), "PING");
}

TEST_F(OpCodeTest, GetCategory) {
    EXPECT_EQ(getCategory(OpCode::C_CONNECT), "Session");
    EXPECT_EQ(getCategory(OpCode::S_ENTITY_SPAWN), "Entity");
    EXPECT_EQ(getCategory(OpCode::C_INPUT), "Input");
    EXPECT_EQ(getCategory(OpCode::PING), "System");
}

// ============================================================================
// Header Tests
// ============================================================================

class HeaderTest : public ::testing::Test {};

TEST_F(HeaderTest, SizeIs16Bytes) {
    // This is also a compile-time check via static_assert
    EXPECT_EQ(sizeof(Header), 16u);
    EXPECT_EQ(sizeof(Header), kHeaderSize);
}

TEST_F(HeaderTest, IsTriviallyCopiable) {
    EXPECT_TRUE(std::is_trivially_copyable_v<Header>);
}

TEST_F(HeaderTest, IsStandardLayout) {
    EXPECT_TRUE(std::is_standard_layout_v<Header>);
}

TEST_F(HeaderTest, CreateSetsCorrectValues) {
    auto header = Header::create(OpCode::C_INPUT, 0x12345678, 42, 10);

    EXPECT_EQ(header.magic, kMagicByte);
    EXPECT_EQ(header.opcode, static_cast<uint8_t>(OpCode::C_INPUT));
    EXPECT_EQ(header.payloadSize, 10);
    EXPECT_EQ(header.userId, 0x12345678u);
    EXPECT_EQ(header.seqId, 42);
    EXPECT_EQ(header.ackId, 0);
    EXPECT_EQ(header.flags, Flags::kNone);  // C_INPUT is unreliable
}

TEST_F(HeaderTest, CreateServerSetsServerUserId) {
    auto header = Header::createServer(OpCode::S_ACCEPT, 1, 4);

    EXPECT_EQ(header.userId, kServerUserId);
    EXPECT_TRUE(header.isFromServer());
}

TEST_F(HeaderTest, CreateConnectUsesUnassignedId) {
    auto header = Header::createConnect(1);

    EXPECT_EQ(header.userId, kUnassignedUserId);
    EXPECT_TRUE(header.isFromUnassigned());
    EXPECT_EQ(header.getOpCode(), OpCode::C_CONNECT);
    EXPECT_TRUE(header.isReliable());
}

TEST_F(HeaderTest, ReliableFlagSetForReliableOpcodes) {
    auto reliable = Header::create(OpCode::C_CONNECT, 0, 1, 0);
    auto unreliable = Header::create(OpCode::C_INPUT, 1, 1, 1);

    EXPECT_TRUE(reliable.isReliable());
    EXPECT_FALSE(unreliable.isReliable());
}

TEST_F(HeaderTest, SetAckSetsFlag) {
    Header header = Header::create(OpCode::S_ENTITY_MOVE, kServerUserId, 10, 0);

    EXPECT_FALSE(header.isAck());
    header.setAck(5);
    EXPECT_TRUE(header.isAck());
    EXPECT_EQ(header.ackId, 5);
}

TEST_F(HeaderTest, ValidationChecks) {
    Header valid = Header::create(OpCode::PING, 1, 1, 0);
    EXPECT_TRUE(valid.hasValidMagic());
    EXPECT_TRUE(valid.hasValidOpCode());
    EXPECT_TRUE(valid.hasValidReserved());
    EXPECT_TRUE(valid.isValid());

    Header invalidMagic = valid;
    invalidMagic.magic = 0x00;
    EXPECT_FALSE(invalidMagic.hasValidMagic());
    EXPECT_FALSE(invalidMagic.isValid());

    Header invalidOpcode = valid;
    invalidOpcode.opcode = 0xFF;
    EXPECT_FALSE(invalidOpcode.hasValidOpCode());
    EXPECT_FALSE(invalidOpcode.isValid());

    Header invalidReserved = valid;
    invalidReserved.reserved[0] = 0x01;
    EXPECT_FALSE(invalidReserved.hasValidReserved());
    EXPECT_FALSE(invalidReserved.isValid());
}

TEST_F(HeaderTest, UserIdRangeValidation) {
    EXPECT_EQ(kServerUserId, 0xFFFFFFFF);
    EXPECT_EQ(kUnassignedUserId, 0x00000000);
    EXPECT_EQ(kMinClientUserId, 0x00000001);
    EXPECT_EQ(kMaxClientUserId, 0xFFFFFFFE);

    Header client = Header::create(OpCode::C_INPUT, 0x00000001, 1, 1);
    EXPECT_TRUE(client.hasValidClientId());

    Header server = Header::createServer(OpCode::S_ACCEPT, 1, 4);
    EXPECT_FALSE(server.hasValidClientId());  // Server ID not in client range
}

// ============================================================================
// Payload Tests
// ============================================================================

class PayloadTest : public ::testing::Test {};

TEST_F(PayloadTest, PayloadSizesMatchRFC) {
    // These are compile-time assertions, but let's verify at runtime too
    EXPECT_EQ(sizeof(AcceptPayload), 4u);
    EXPECT_EQ(sizeof(GetUsersResponseHeader), 1u);
    EXPECT_EQ(sizeof(UpdateStatePayload), 1u);
    EXPECT_EQ(sizeof(EntitySpawnPayload), 13u);
    EXPECT_EQ(sizeof(EntityMovePayload), 20u);
    EXPECT_EQ(sizeof(EntityDestroyPayload), 4u);
    EXPECT_EQ(sizeof(InputPayload), 1u);
    EXPECT_EQ(sizeof(UpdatePosPayload), 8u);
}

TEST_F(PayloadTest, AllPayloadsAreTriviallyCopiable) {
    EXPECT_TRUE(std::is_trivially_copyable_v<AcceptPayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<UpdateStatePayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<EntitySpawnPayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<EntityMovePayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<EntityDestroyPayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<InputPayload>);
    EXPECT_TRUE(std::is_trivially_copyable_v<UpdatePosPayload>);
}

TEST_F(PayloadTest, GetPayloadSizeReturnsCorrectValues) {
    EXPECT_EQ(getPayloadSize(OpCode::C_CONNECT), 0u);
    EXPECT_EQ(getPayloadSize(OpCode::S_ACCEPT), sizeof(AcceptPayload));
    EXPECT_EQ(getPayloadSize(OpCode::R_GET_USERS), 0u);  // Variable
    EXPECT_EQ(getPayloadSize(OpCode::S_ENTITY_SPAWN), sizeof(EntitySpawnPayload));
    EXPECT_EQ(getPayloadSize(OpCode::S_ENTITY_MOVE), sizeof(EntityMovePayload));
    EXPECT_EQ(getPayloadSize(OpCode::C_INPUT), sizeof(InputPayload));
}

TEST_F(PayloadTest, InputMaskFlags) {
    InputPayload input{0};

    input.inputMask = InputMask::kUp | InputMask::kShoot;
    EXPECT_TRUE(input.isUp());
    EXPECT_FALSE(input.isDown());
    EXPECT_FALSE(input.isLeft());
    EXPECT_FALSE(input.isRight());
    EXPECT_TRUE(input.isShoot());

    input.inputMask = InputMask::kDown | InputMask::kLeft | InputMask::kRight;
    EXPECT_FALSE(input.isUp());
    EXPECT_TRUE(input.isDown());
    EXPECT_TRUE(input.isLeft());
    EXPECT_TRUE(input.isRight());
    EXPECT_FALSE(input.isShoot());
}

TEST_F(PayloadTest, EntitySpawnPayloadType) {
    EntitySpawnPayload spawn{1, static_cast<uint8_t>(EntityType::Player),
                             100.0f, 200.0f};
    EXPECT_EQ(spawn.getType(), EntityType::Player);

    spawn.type = static_cast<uint8_t>(EntityType::Bydos);
    EXPECT_EQ(spawn.getType(), EntityType::Bydos);
}

TEST_F(PayloadTest, UpdateStatePayloadState) {
    UpdateStatePayload state{static_cast<uint8_t>(GameState::Running)};
    EXPECT_EQ(state.getState(), GameState::Running);

    state.stateId = static_cast<uint8_t>(GameState::GameOver);
    EXPECT_EQ(state.getState(), GameState::GameOver);
}

TEST_F(PayloadTest, GameOverPayloadStructure) {
    GameOverPayload gameOver;
    EXPECT_EQ(sizeof(GameOverPayload), sizeof(std::uint32_t));
    
    gameOver.finalScore = 12345;
    EXPECT_EQ(gameOver.finalScore, 12345u);
    
    // Test with max score
    gameOver.finalScore = 0xFFFFFFFF;
    EXPECT_EQ(gameOver.finalScore, 0xFFFFFFFFu);
}

TEST_F(PayloadTest, PowerUpEventPayloadStructure) {
    PowerUpEventPayload powerUp;
    EXPECT_EQ(sizeof(PowerUpEventPayload), 
              sizeof(std::uint32_t) + sizeof(std::uint8_t) + sizeof(float));
    
    powerUp.playerId = 42;
    powerUp.powerUpType = 3;  // DoubleDamage
    powerUp.duration = 10.5F;
    
    EXPECT_EQ(powerUp.playerId, 42u);
    EXPECT_EQ(powerUp.powerUpType, 3u);
    EXPECT_FLOAT_EQ(powerUp.duration, 10.5F);
}

// ============================================================================
// Validator Tests
// ============================================================================

class ValidatorTest : public ::testing::Test {};

TEST_F(ValidatorTest, ValidateMagic) {
    EXPECT_TRUE(Validator::validateMagic(kMagicByte).isOk());
    EXPECT_TRUE(Validator::validateMagic(0xA1).isOk());

    EXPECT_TRUE(Validator::validateMagic(0x00).isErr());
    EXPECT_TRUE(Validator::validateMagic(0xFF).isErr());
    EXPECT_EQ(Validator::validateMagic(0x00).error(), NetworkError::InvalidMagic);
}

TEST_F(ValidatorTest, ValidateOpCode) {
    auto result = Validator::validateOpCode(0x01);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), OpCode::C_CONNECT);

    auto invalid = Validator::validateOpCode(0x00);
    EXPECT_TRUE(invalid.isErr());
    EXPECT_EQ(invalid.error(), NetworkError::UnknownOpcode);
}

TEST_F(ValidatorTest, ValidateHeader) {
    Header valid = Header::create(OpCode::PING, 1, 1, 0);
    EXPECT_TRUE(Validator::validateHeader(valid).isOk());

    Header invalidMagic = valid;
    invalidMagic.magic = 0x00;
    auto result = Validator::validateHeader(invalidMagic);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidMagic);
}

TEST_F(ValidatorTest, ValidatePacketSize) {
    EXPECT_TRUE(Validator::validatePacketSize(kHeaderSize).isOk());
    EXPECT_TRUE(Validator::validatePacketSize(kMaxPacketSize).isOk());

    auto tooSmall = Validator::validatePacketSize(kHeaderSize - 1);
    EXPECT_TRUE(tooSmall.isErr());
    EXPECT_EQ(tooSmall.error(), NetworkError::PacketTooSmall);

    auto tooLarge = Validator::validatePacketSize(kMaxPacketSize + 1);
    EXPECT_TRUE(tooLarge.isErr());
    EXPECT_EQ(tooLarge.error(), NetworkError::PacketTooLarge);
}

TEST_F(ValidatorTest, ValidatePayloadSize) {
    // Fixed size opcodes
    EXPECT_TRUE(
        Validator::validatePayloadSize(OpCode::S_ACCEPT, sizeof(AcceptPayload))
            .isOk());
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::S_ACCEPT, 0).isErr());

    // Empty payloads
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::C_CONNECT, 0).isOk());
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::C_CONNECT, 1).isErr());

    // Variable size (R_GET_USERS) - need payload data for validation
    std::uint8_t payload0[1] = {0};  // count = 0, expected size = 1
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 1, payload0).isOk());

    std::uint8_t payload1[5] = {1, 0, 0, 0, 0};  // count = 1, expected size = 5
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 5, payload1).isOk());

    // Invalid: payload size doesn't match count
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 100, payload0).isErr());

    // Invalid: payload too small for header
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 0, {}).isErr());
}

TEST_F(ValidatorTest, ValidateClientUserId) {
    // During handshake (C_CONNECT)
    EXPECT_TRUE(
        Validator::validateClientUserId(kUnassignedUserId, OpCode::C_CONNECT)
            .isOk());

    // Invalid: C_CONNECT must use unassigned ID
    EXPECT_TRUE(
        Validator::validateClientUserId(1, OpCode::C_CONNECT).isErr());

    // Normal operation
    EXPECT_TRUE(Validator::validateClientUserId(1, OpCode::C_INPUT).isOk());
    EXPECT_TRUE(
        Validator::validateClientUserId(kMaxClientUserId, OpCode::C_INPUT)
            .isOk());

    // Invalid: server ID from client
    EXPECT_TRUE(
        Validator::validateClientUserId(kServerUserId, OpCode::C_INPUT).isErr());

    // Invalid: unassigned after handshake
    EXPECT_TRUE(
        Validator::validateClientUserId(kUnassignedUserId, OpCode::C_INPUT)
            .isErr());
}

TEST_F(ValidatorTest, ValidateServerUserId) {
    EXPECT_TRUE(Validator::validateServerUserId(kServerUserId).isOk());
    EXPECT_TRUE(Validator::validateServerUserId(1).isErr());
    EXPECT_TRUE(Validator::validateServerUserId(0).isErr());
}

TEST_F(ValidatorTest, ValidateRGetUsersPayload) {
    // Valid: count = 0, size = 1
    std::uint8_t payload0[1] = {0};
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload0, 1)).isOk());

    // Valid: count = 1, size = 1 + 4 = 5
    std::uint8_t payload1[5] = {1, 0, 0, 0, 0};  // count=1, userId=0
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload1, 5)).isOk());

    // Valid: count = 255 (max), size = 1 + 255*4 = 1021
    std::uint8_t payload255[1021];
    payload255[0] = 255;
    // Fill rest with zeros
    std::memset(payload255 + 1, 0, 1020);
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload255, 1021)).isOk());

    // Invalid: count > 255
    std::uint8_t payload256[1] = {0};  // Can't set to 256 since uint8_t max is 255
    // But since uint8_t can't hold 256, we can't test this directly
    // The comparison will work since 255 is the max

    // Invalid: payload too small
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload0, 0)).isErr());

    // Invalid: size doesn't match count (count=1, size=2)
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload1, 2)).isErr());

    // Invalid: size doesn't match count (count=1, size=6)
    EXPECT_TRUE(Validator::validateRGetUsersPayload(std::span<const std::uint8_t>(payload1, 6)).isErr());
}

TEST_F(ValidatorTest, ValidatePacketRGetUsers) {
    // Valid R_GET_USERS packet: count=1, userId=0x12345678
    std::uint8_t packet[21];  // 16 header + 5 payload
    std::memset(packet, 0, sizeof(packet));

    // Header
    packet[0] = kMagicByte;  // magic
    packet[1] = 0x05;        // opcode R_GET_USERS
    // payloadSize: 5 in network byte order (big-endian)
    packet[2] = 0x00;        // payloadSize high byte
    packet[3] = 0x05;        // payloadSize low byte
    // userId: server (0xFFFFFFFF) in network byte order (big-endian)
    packet[4] = 0xFF;
    packet[5] = 0xFF;
    packet[6] = 0xFF;
    packet[7] = 0xFF;
    // seqId: 1 in big-endian network byte order (0x0001)
    packet[8] = 0x00;
    packet[9] = 0x01;
    // ackId: 0
    packet[10] = 0x00;
    packet[11] = 0x00;
    // flags
    packet[12] = 0;
    // reserved
    packet[13] = 0;
    packet[14] = 0;
    packet[15] = 0;

    // Payload
    packet[16] = 1;  // count=1
    packet[17] = 0x12;
    packet[18] = 0x34;
    packet[19] = 0x56;
    packet[20] = 0x78;  // userId=0x12345678

    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(packet, sizeof(packet)), true).isOk());

    // Invalid: payload size doesn't match count
    packet[3] = 0x02;   // payloadSize=2 (should be 5)
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(packet, 16 + 2), true).isErr());
}

// ============================================================================
// ByteOrderSpec Tests - RFC-Compliant Serialization
// ============================================================================

#include "protocol/ByteOrderSpec.hpp"
#include "Serializer.hpp"

class ByteOrderSpecTest : public ::testing::Test {};

TEST_F(ByteOrderSpecTest, HeaderRoundTrip) {
    Header original = Header::create(OpCode::C_CONNECT, 42, 1, 100);

    // Serialize to network, then back
    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<Header>(bytes);

    EXPECT_EQ(restored.magic, original.magic);
    EXPECT_EQ(restored.opcode, original.opcode);
    EXPECT_EQ(restored.payloadSize, original.payloadSize);
    EXPECT_EQ(restored.userId, original.userId);
    EXPECT_EQ(restored.seqId, original.seqId);
    EXPECT_EQ(restored.ackId, original.ackId);
    EXPECT_EQ(restored.flags, original.flags);
}

TEST_F(ByteOrderSpecTest, HeaderPreservesAllFields) {
    Header original{};
    original.magic = kMagicByte;
    original.opcode = static_cast<uint8_t>(OpCode::S_ACCEPT);
    original.payloadSize = 0x1234;
    original.userId = 0xDEADBEEF;
    original.seqId = 0xABCD;
    original.ackId = 0xEF01;
    original.flags = Flags::kReliable | Flags::kIsAck;
    original.reserved = {0, 0, 0};

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<Header>(bytes);

    EXPECT_EQ(restored.magic, kMagicByte);
    EXPECT_EQ(restored.payloadSize, 0x1234);
    EXPECT_EQ(restored.userId, 0xDEADBEEF);
    EXPECT_EQ(restored.seqId, 0xABCD);
    EXPECT_EQ(restored.ackId, 0xEF01);
    EXPECT_EQ(restored.flags, Flags::kReliable | Flags::kIsAck);
}

TEST_F(ByteOrderSpecTest, EntitySpawnPayloadRoundTrip) {
    EntitySpawnPayload original{};
    original.entityId = 12345;
    original.type = static_cast<uint8_t>(EntityType::Player);
    original.posX = 100.5f;
    original.posY = 200.75f;

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<EntitySpawnPayload>(bytes);

    EXPECT_EQ(restored.entityId, 12345);
    EXPECT_EQ(restored.type, static_cast<uint8_t>(EntityType::Player));
    EXPECT_FLOAT_EQ(restored.posX, 100.5f);
    EXPECT_FLOAT_EQ(restored.posY, 200.75f);
}

TEST_F(ByteOrderSpecTest, EntityMovePayloadRoundTrip) {
    EntityMovePayload original{};
    original.entityId = 999;
    original.posX = -50.0f;
    original.posY = 75.25f;
    original.velX = 1.5f;
    original.velY = -2.5f;

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<EntityMovePayload>(bytes);

    EXPECT_EQ(restored.entityId, 999);
    EXPECT_FLOAT_EQ(restored.posX, -50.0f);
    EXPECT_FLOAT_EQ(restored.posY, 75.25f);
    EXPECT_FLOAT_EQ(restored.velX, 1.5f);
    EXPECT_FLOAT_EQ(restored.velY, -2.5f);
}

TEST_F(ByteOrderSpecTest, SerializerHighLevelAPI) {
    // Test the new Serializer::serializeForNetwork / deserializeFromNetwork
    Header original = Header::createServer(OpCode::S_ENTITY_SPAWN, 42, 13);

    auto bytes = Serializer::serializeForNetwork(original);
    EXPECT_EQ(bytes.size(), sizeof(Header));

    auto restored = Serializer::deserializeFromNetwork<Header>(bytes);
    EXPECT_EQ(restored.magic, kMagicByte);
    EXPECT_EQ(restored.opcode, static_cast<uint8_t>(OpCode::S_ENTITY_SPAWN));
    EXPECT_EQ(restored.userId, kServerUserId);
    EXPECT_EQ(restored.seqId, 42);
    EXPECT_EQ(restored.payloadSize, 13);
}

TEST_F(ByteOrderSpecTest, AcceptPayloadRoundTrip) {
    AcceptPayload original{};
    original.newUserId = 0x12345678;

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<AcceptPayload>(bytes);

    EXPECT_EQ(restored.newUserId, 0x12345678);
}

TEST_F(ByteOrderSpecTest, UpdatePosPayloadRoundTrip) {
    UpdatePosPayload original{};
    original.posX = 123.456f;
    original.posY = -789.012f;

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    auto restored = ByteOrderSpec::deserializeFromNetwork<UpdatePosPayload>(bytes);

    EXPECT_FLOAT_EQ(restored.posX, 123.456f);
    EXPECT_FLOAT_EQ(restored.posY, -789.012f);
}

TEST_F(ByteOrderSpecTest, SingleBytePayloadsUnchanged) {
    // Single-byte payloads should not be affected by byte order
    InputPayload input{};
    input.inputMask = InputMask::kUp | InputMask::kShoot;

    auto bytes = ByteOrderSpec::serializeToNetwork(input);
    auto restored = ByteOrderSpec::deserializeFromNetwork<InputPayload>(bytes);

    EXPECT_EQ(restored.inputMask, InputMask::kUp | InputMask::kShoot);
    EXPECT_TRUE(restored.isUp());
    EXPECT_TRUE(restored.isShoot());
    EXPECT_FALSE(restored.isDown());
}

TEST_F(ByteOrderSpecTest, EmptyPayloadsSerializeToEmptyBuffers) {
    // Empty payloads should serialize to empty buffers (0 bytes) despite sizeof(T) == 1
    ConnectPayload connect{};
    auto connectBytes = ByteOrderSpec::serializeToNetwork(connect);
    EXPECT_EQ(connectBytes.size(), 0u);

    PingPayload ping{};
    auto pingBytes = ByteOrderSpec::serializeToNetwork(ping);
    EXPECT_EQ(pingBytes.size(), 0u);

    PongPayload pong{};
    auto pongBytes = ByteOrderSpec::serializeToNetwork(pong);
    EXPECT_EQ(pongBytes.size(), 0u);
}

TEST_F(ByteOrderSpecTest, EmptyPayloadsDeserializeFromEmptyBuffers) {
    // Empty payloads should deserialize from empty buffers
    std::vector<uint8_t> emptyBuffer;

    auto connect = ByteOrderSpec::deserializeFromNetwork<ConnectPayload>(emptyBuffer);
    auto ping = ByteOrderSpec::deserializeFromNetwork<PingPayload>(emptyBuffer);
    auto pong = ByteOrderSpec::deserializeFromNetwork<PongPayload>(emptyBuffer);

    // Should succeed without throwing
    SUCCEED();
}

TEST_F(ByteOrderSpecTest, DisconnectPayloadSerializeDeserialize) {
    // DisconnectPayload has a reason field (uint8_t)
    DisconnectPayload original{};
    original.reason = 3;  // RemoteRequest

    auto bytes = ByteOrderSpec::serializeToNetwork(original);
    EXPECT_EQ(bytes.size(), 1u);  // Should be 1 byte
    EXPECT_EQ(bytes[0], 3u);

    auto restored = ByteOrderSpec::deserializeFromNetwork<DisconnectPayload>(bytes);
    EXPECT_EQ(restored.reason, 3u);
}

TEST_F(ByteOrderSpecTest, DeserializeFromRawPointer) {
    Header original = Header::create(OpCode::C_INPUT, 42, 100);
    auto bytes = ByteOrderSpec::serializeToNetwork(original);

    auto restored = Serializer::deserializeFromNetwork<Header>(
        std::span<const std::uint8_t>(bytes));

    EXPECT_EQ(restored.magic, kMagicByte);
    EXPECT_EQ(restored.opcode, static_cast<uint8_t>(OpCode::C_INPUT));
    EXPECT_EQ(restored.userId, 42);
    EXPECT_EQ(restored.seqId, 100);
}
