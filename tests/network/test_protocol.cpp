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
    EXPECT_TRUE(isServerOpCode(OpCode::S_UPDATE_POS));
    EXPECT_TRUE(isServerOpCode(OpCode::DISCONNECT));  // Can be either

    EXPECT_FALSE(isServerOpCode(OpCode::C_CONNECT));
    EXPECT_FALSE(isServerOpCode(OpCode::C_INPUT));
}

TEST_F(OpCodeTest, IsValidOpCode) {
    // Valid opcodes
    EXPECT_TRUE(isValidOpCode(0x01));
    EXPECT_TRUE(isValidOpCode(0x10));
    EXPECT_TRUE(isValidOpCode(0x20));
    EXPECT_TRUE(isValidOpCode(0xF0));

    // Invalid opcodes
    EXPECT_FALSE(isValidOpCode(0x00));
    EXPECT_FALSE(isValidOpCode(0x07));
    EXPECT_FALSE(isValidOpCode(0x13));
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

    // Variable size (R_GET_USERS)
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 1).isOk());
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 100).isOk());
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, 0).isErr());
}

TEST_F(ValidatorTest, ValidateClientUserId) {
    // During handshake (C_CONNECT)
    EXPECT_TRUE(
        Validator::validateClientUserId(kUnassignedUserId, OpCode::C_CONNECT)
            .isOk());

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
