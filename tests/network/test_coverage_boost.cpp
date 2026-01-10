/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional coverage tests for network library
*/

#include <gtest/gtest.h>

#include "protocol/OpCode.hpp"
#include "protocol/Header.hpp"
#include "connection/ConnectionState.hpp"
#include "connection/ConnectionEvents.hpp"
#include "core/Error.hpp"
#include "Serializer.hpp"

using namespace rtype::network;

// =============================================================================
// OpCode Comprehensive Tests
// =============================================================================

TEST(OpCodeCoverage, IsReliableAllOpCodes) {
    EXPECT_TRUE(isReliable(OpCode::C_CONNECT));
    EXPECT_TRUE(isReliable(OpCode::S_ACCEPT));
    EXPECT_TRUE(isReliable(OpCode::DISCONNECT));
    EXPECT_TRUE(isReliable(OpCode::C_GET_USERS));
    EXPECT_TRUE(isReliable(OpCode::R_GET_USERS));
    EXPECT_TRUE(isReliable(OpCode::S_UPDATE_STATE));
    EXPECT_TRUE(isReliable(OpCode::S_GAME_OVER));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_SPAWN));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_DESTROY));
    EXPECT_TRUE(isReliable(OpCode::S_ENTITY_HEALTH));
    EXPECT_TRUE(isReliable(OpCode::S_POWERUP_EVENT));

    EXPECT_FALSE(isReliable(OpCode::S_ENTITY_MOVE));
    EXPECT_FALSE(isReliable(OpCode::S_ENTITY_MOVE_BATCH));
    EXPECT_FALSE(isReliable(OpCode::C_INPUT));
    EXPECT_FALSE(isReliable(OpCode::S_UPDATE_POS));
    EXPECT_FALSE(isReliable(OpCode::PING));
    EXPECT_FALSE(isReliable(OpCode::PONG));
}

TEST(OpCodeCoverage, IsClientOpCodeAllOpCodes) {
    EXPECT_TRUE(isClientOpCode(OpCode::C_CONNECT));
    EXPECT_TRUE(isClientOpCode(OpCode::C_GET_USERS));
    EXPECT_TRUE(isClientOpCode(OpCode::C_INPUT));
    EXPECT_TRUE(isClientOpCode(OpCode::PING));
    EXPECT_TRUE(isClientOpCode(OpCode::DISCONNECT));

    EXPECT_FALSE(isClientOpCode(OpCode::S_ACCEPT));
    EXPECT_FALSE(isClientOpCode(OpCode::R_GET_USERS));
    EXPECT_FALSE(isClientOpCode(OpCode::S_UPDATE_STATE));
    EXPECT_FALSE(isClientOpCode(OpCode::S_GAME_OVER));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_SPAWN));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_MOVE));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_MOVE_BATCH));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_DESTROY));
    EXPECT_FALSE(isClientOpCode(OpCode::S_ENTITY_HEALTH));
    EXPECT_FALSE(isClientOpCode(OpCode::S_POWERUP_EVENT));
    EXPECT_FALSE(isClientOpCode(OpCode::S_UPDATE_POS));
    EXPECT_FALSE(isClientOpCode(OpCode::PONG));
}

TEST(OpCodeCoverage, IsServerOpCodeAllOpCodes) {
    EXPECT_TRUE(isServerOpCode(OpCode::S_ACCEPT));
    EXPECT_TRUE(isServerOpCode(OpCode::R_GET_USERS));
    EXPECT_TRUE(isServerOpCode(OpCode::S_UPDATE_STATE));
    EXPECT_TRUE(isServerOpCode(OpCode::S_GAME_OVER));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_SPAWN));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_MOVE));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_MOVE_BATCH));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_DESTROY));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_HEALTH));
    EXPECT_TRUE(isServerOpCode(OpCode::S_POWERUP_EVENT));
    EXPECT_TRUE(isServerOpCode(OpCode::S_UPDATE_POS));
    EXPECT_TRUE(isServerOpCode(OpCode::PONG));
    EXPECT_TRUE(isServerOpCode(OpCode::DISCONNECT));

    EXPECT_FALSE(isServerOpCode(OpCode::C_CONNECT));
    EXPECT_FALSE(isServerOpCode(OpCode::C_GET_USERS));
    EXPECT_FALSE(isServerOpCode(OpCode::C_INPUT));
    EXPECT_FALSE(isServerOpCode(OpCode::PING));
}

TEST(OpCodeCoverage, IsValidOpCodeAllValues) {
    EXPECT_TRUE(isValidOpCode(0x01));  // C_CONNECT
    EXPECT_TRUE(isValidOpCode(0x02));  // S_ACCEPT
    EXPECT_TRUE(isValidOpCode(0x03));  // DISCONNECT
    EXPECT_TRUE(isValidOpCode(0x04));  // C_GET_USERS
    EXPECT_TRUE(isValidOpCode(0x05));  // R_GET_USERS
    EXPECT_TRUE(isValidOpCode(0x06));  // S_UPDATE_STATE
    EXPECT_TRUE(isValidOpCode(0x07));  // S_GAME_OVER
    EXPECT_TRUE(isValidOpCode(0x08));  // C_READY
    EXPECT_TRUE(isValidOpCode(0x09));  // S_GAME_START
    EXPECT_TRUE(isValidOpCode(0x0A));  // S_PLAYER_READY_STATE
    EXPECT_TRUE(isValidOpCode(0x10));  // S_ENTITY_SPAWN
    EXPECT_TRUE(isValidOpCode(0x11));  // S_ENTITY_MOVE
    EXPECT_TRUE(isValidOpCode(0x12));  // S_ENTITY_DESTROY
    EXPECT_TRUE(isValidOpCode(0x13));  // S_ENTITY_HEALTH
    EXPECT_TRUE(isValidOpCode(0x14));  // S_POWERUP_EVENT
    EXPECT_TRUE(isValidOpCode(0x15));  // S_ENTITY_MOVE_BATCH
    EXPECT_TRUE(isValidOpCode(0x20));  // C_INPUT
    EXPECT_TRUE(isValidOpCode(0x21));  // S_UPDATE_POS
    EXPECT_TRUE(isValidOpCode(0xF0));  // PING
    EXPECT_TRUE(isValidOpCode(0xF1));  // PONG
    EXPECT_TRUE(isValidOpCode(0xF2));  // ACK

    EXPECT_FALSE(isValidOpCode(0x00));
    EXPECT_FALSE(isValidOpCode(0x0B));
    EXPECT_FALSE(isValidOpCode(0x0F));
    EXPECT_FALSE(isValidOpCode(0x16));
    EXPECT_FALSE(isValidOpCode(0x30));
    EXPECT_FALSE(isValidOpCode(0xFF));
}

TEST(OpCodeCoverage, GetCategoryAllCategories) {
    EXPECT_EQ(getCategory(OpCode::C_CONNECT), "Session");
    EXPECT_EQ(getCategory(OpCode::S_ACCEPT), "Session");
    EXPECT_EQ(getCategory(OpCode::DISCONNECT), "Session");
    EXPECT_EQ(getCategory(OpCode::C_GET_USERS), "Session");
    EXPECT_EQ(getCategory(OpCode::R_GET_USERS), "Session");
    EXPECT_EQ(getCategory(OpCode::S_UPDATE_STATE), "Session");
    EXPECT_EQ(getCategory(OpCode::S_GAME_OVER), "Session");

    EXPECT_EQ(getCategory(OpCode::S_ENTITY_SPAWN), "Entity");
    EXPECT_EQ(getCategory(OpCode::S_ENTITY_MOVE), "Entity");
    EXPECT_EQ(getCategory(OpCode::S_ENTITY_MOVE_BATCH), "Entity");
    EXPECT_EQ(getCategory(OpCode::S_ENTITY_DESTROY), "Entity");
    EXPECT_EQ(getCategory(OpCode::S_ENTITY_HEALTH), "Entity");
    EXPECT_EQ(getCategory(OpCode::S_POWERUP_EVENT), "Entity");

    EXPECT_EQ(getCategory(OpCode::C_INPUT), "Input");
    EXPECT_EQ(getCategory(OpCode::S_UPDATE_POS), "Input");

    EXPECT_EQ(getCategory(OpCode::PING), "System");
    EXPECT_EQ(getCategory(OpCode::PONG), "System");

    // Invalid opcode returns "Unknown"
    EXPECT_EQ(getCategory(static_cast<OpCode>(0x00)), "Unknown");
}

TEST(OpCodeCoverage, ToStringAllOpCodes) {
    EXPECT_EQ(toString(OpCode::C_CONNECT), "C_CONNECT");
    EXPECT_EQ(toString(OpCode::S_ACCEPT), "S_ACCEPT");
    EXPECT_EQ(toString(OpCode::DISCONNECT), "DISCONNECT");
    EXPECT_EQ(toString(OpCode::C_GET_USERS), "C_GET_USERS");
    EXPECT_EQ(toString(OpCode::R_GET_USERS), "R_GET_USERS");
    EXPECT_EQ(toString(OpCode::S_UPDATE_STATE), "S_UPDATE_STATE");
    EXPECT_EQ(toString(OpCode::S_GAME_OVER), "S_GAME_OVER");
    EXPECT_EQ(toString(OpCode::S_ENTITY_SPAWN), "S_ENTITY_SPAWN");
    EXPECT_EQ(toString(OpCode::S_ENTITY_MOVE), "S_ENTITY_MOVE");
    EXPECT_EQ(toString(OpCode::S_ENTITY_MOVE_BATCH), "S_ENTITY_MOVE_BATCH");
    EXPECT_EQ(toString(OpCode::S_ENTITY_DESTROY), "S_ENTITY_DESTROY");
    EXPECT_EQ(toString(OpCode::S_ENTITY_HEALTH), "S_ENTITY_HEALTH");
    EXPECT_EQ(toString(OpCode::S_POWERUP_EVENT), "S_POWERUP_EVENT");
    EXPECT_EQ(toString(OpCode::C_INPUT), "C_INPUT");
    EXPECT_EQ(toString(OpCode::S_UPDATE_POS), "S_UPDATE_POS");
    EXPECT_EQ(toString(OpCode::PING), "PING");
    EXPECT_EQ(toString(OpCode::PONG), "PONG");
    EXPECT_EQ(toString(static_cast<OpCode>(0x00)), "UNKNOWN");
}

// =============================================================================
// Header Comprehensive Tests
// =============================================================================

TEST(HeaderCoverage, CreateWithAllOpCodes) {
    auto h1 = Header::create(OpCode::C_CONNECT, 1, 0, 0);
    EXPECT_TRUE(h1.isReliable());

    auto h2 = Header::create(OpCode::S_ENTITY_MOVE, 1, 0, 100);
    EXPECT_FALSE(h2.isReliable());
    EXPECT_EQ(h2.payloadSize, 100);
}

TEST(HeaderCoverage, FlagOperations) {
    Header h{};
    h.magic = kMagicByte;
    h.opcode = static_cast<std::uint8_t>(OpCode::C_INPUT);
    h.flags = Flags::kNone;
    h.reserved = {0, 0, 0};

    EXPECT_FALSE(h.isReliable());
    EXPECT_FALSE(h.isAck());
    EXPECT_FALSE(h.isCompressed());

    h.setReliable(true);
    EXPECT_TRUE(h.isReliable());

    h.setReliable(false);
    EXPECT_FALSE(h.isReliable());

    h.setAck(42);
    EXPECT_TRUE(h.isAck());
    EXPECT_EQ(h.ackId, 42);

    h.setCompressed(true);
    EXPECT_TRUE(h.isCompressed());

    h.setCompressed(false);
    EXPECT_FALSE(h.isCompressed());
}

TEST(HeaderCoverage, ValidationFunctions) {
    Header h{};
    h.magic = kMagicByte;
    h.opcode = static_cast<std::uint8_t>(OpCode::C_CONNECT);
    h.reserved = {0, 0, 0};

    EXPECT_TRUE(h.hasValidMagic());
    EXPECT_TRUE(h.hasValidOpCode());
    EXPECT_TRUE(h.hasValidReserved());
    EXPECT_TRUE(h.isValid());

    // Invalid magic
    h.magic = 0x00;
    EXPECT_FALSE(h.hasValidMagic());
    EXPECT_FALSE(h.isValid());
    h.magic = kMagicByte;

    // Invalid opcode
    h.opcode = 0xFF;
    EXPECT_FALSE(h.hasValidOpCode());
    EXPECT_FALSE(h.isValid());
    h.opcode = static_cast<std::uint8_t>(OpCode::C_CONNECT);

    // Invalid reserved
    h.reserved = {1, 0, 0};
    EXPECT_FALSE(h.hasValidReserved());
    EXPECT_FALSE(h.isValid());
}

TEST(HeaderCoverage, UserIdValidation) {
    Header h{};

    h.userId = kServerUserId;
    EXPECT_TRUE(h.isFromServer());
    EXPECT_FALSE(h.isFromUnassigned());
    EXPECT_FALSE(h.hasValidClientId());

    h.userId = kUnassignedUserId;
    EXPECT_FALSE(h.isFromServer());
    EXPECT_TRUE(h.isFromUnassigned());
    EXPECT_FALSE(h.hasValidClientId());

    h.userId = kMinClientUserId;
    EXPECT_FALSE(h.isFromServer());
    EXPECT_FALSE(h.isFromUnassigned());
    EXPECT_TRUE(h.hasValidClientId());

    h.userId = kMaxClientUserId;
    EXPECT_TRUE(h.hasValidClientId());

    h.userId = 12345;
    EXPECT_TRUE(h.hasValidClientId());
}

TEST(HeaderCoverage, CreateHelpers) {
    auto server = Header::createServer(OpCode::S_ACCEPT, 1, 4);
    EXPECT_EQ(server.userId, kServerUserId);
    EXPECT_EQ(server.payloadSize, 4);
    EXPECT_TRUE(server.isFromServer());

    auto connect = Header::createConnect(42);
    EXPECT_EQ(connect.userId, kUnassignedUserId);
    EXPECT_EQ(connect.getOpCode(), OpCode::C_CONNECT);
    EXPECT_TRUE(connect.isFromUnassigned());
}

// =============================================================================
// ConnectionState and DisconnectReason Tests
// =============================================================================

TEST(ConnectionStateCoverage, ToStringAllStates) {
    EXPECT_EQ(toString(ConnectionState::Disconnected), "Disconnected");
    EXPECT_EQ(toString(ConnectionState::Connecting), "Connecting");
    EXPECT_EQ(toString(ConnectionState::Connected), "Connected");
    EXPECT_EQ(toString(ConnectionState::Disconnecting), "Disconnecting");
}

TEST(DisconnectReasonCoverage, ToStringAllReasons) {
    EXPECT_EQ(toString(DisconnectReason::LocalRequest), "LocalRequest");
    EXPECT_EQ(toString(DisconnectReason::RemoteRequest), "RemoteRequest");
    EXPECT_EQ(toString(DisconnectReason::Timeout), "Timeout");
    EXPECT_EQ(toString(DisconnectReason::MaxRetriesExceeded), "MaxRetriesExceeded");
    EXPECT_EQ(toString(DisconnectReason::ProtocolError), "ProtocolError");
    EXPECT_EQ(toString(DisconnectReason::Banned), "Banned");
}

// =============================================================================
// Error Type Tests
// =============================================================================

TEST(ErrorCoverage, NetworkErrorValues) {
    // Just verify we can access and compare different error values
    EXPECT_NE(NetworkError::None, NetworkError::PacketTooSmall);
    EXPECT_NE(NetworkError::InvalidMagic, NetworkError::UnknownOpcode);
    EXPECT_NE(NetworkError::DuplicatePacket, NetworkError::NotConnected);
    EXPECT_NE(NetworkError::RetryLimitExceeded, NetworkError::DecompressionFailed);
}

// =============================================================================
// Serializer Template Tests
// =============================================================================

TEST(SerializerCoverage, SerializeDeserializeU32) {
    std::uint32_t original = 0x12345678;
    auto bytes = Serializer::serialize(original);
    EXPECT_EQ(bytes.size(), sizeof(std::uint32_t));

    auto restored = Serializer::deserialize<std::uint32_t>(bytes);
    EXPECT_EQ(restored, original);
}

TEST(SerializerCoverage, SerializeDeserializeU16) {
    std::uint16_t original = 0xABCD;
    auto bytes = Serializer::serialize(original);
    EXPECT_EQ(bytes.size(), sizeof(std::uint16_t));

    auto restored = Serializer::deserialize<std::uint16_t>(bytes);
    EXPECT_EQ(restored, original);
}

TEST(SerializerCoverage, DeserializeSizeMismatchThrows) {
    std::vector<std::uint8_t> smallBuffer = {0x01, 0x02};

    EXPECT_THROW(Serializer::deserialize<std::uint32_t>(smallBuffer), std::runtime_error);
}

TEST(SerializerCoverage, ByteOrderConversionMismatchThrows) {
    std::vector<std::uint8_t> smallBuffer = {0x01};

    EXPECT_THROW((Serializer::toNetworkByteOrder<std::uint32_t>(smallBuffer)), std::runtime_error);
    EXPECT_THROW((Serializer::fromNetworkByteOrder<std::uint32_t>(smallBuffer)), std::runtime_error);
}

TEST(SerializerCoverage, NetworkByteOrderRoundtrip) {
    std::uint32_t original = 0xDEADBEEF;
    auto bytes = Serializer::serialize(original);

    auto networkBytes = Serializer::toNetworkByteOrder<std::uint32_t>(bytes);
    auto hostBytes = Serializer::fromNetworkByteOrder<std::uint32_t>(networkBytes);

    auto restored = Serializer::deserialize<std::uint32_t>(hostBytes);
    EXPECT_EQ(restored, original);
}

// =============================================================================
// Header Serialization Tests
// =============================================================================

TEST(SerializerCoverage, SerializeForNetworkHeader) {
    Header h = Header::create(OpCode::C_CONNECT, 123, 1, 0);
    auto bytes = Serializer::serializeForNetwork(h);
    EXPECT_EQ(bytes.size(), kHeaderSize);
}

TEST(SerializerCoverage, DeserializeFromNetworkHeader) {
    Header original = Header::create(OpCode::S_ACCEPT, 456, 2, 10);
    auto bytes = Serializer::serializeForNetwork(original);

    auto restored = Serializer::deserializeFromNetwork<Header>(bytes);
    EXPECT_EQ(restored.getOpCode(), original.getOpCode());
    EXPECT_EQ(restored.userId, original.userId);
    EXPECT_EQ(restored.seqId, original.seqId);
}

// =============================================================================
// Safe Deserialization Tests
// =============================================================================

TEST(SerializerCoverage, SafeDeserializeHeaderTooSmall) {
    std::vector<std::uint8_t> tooSmall(10);
    std::span<const std::uint8_t> span(tooSmall);

    auto result = Serializer::safeDeserializeHeader(span);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::PacketTooSmall);
}

TEST(SerializerCoverage, SafeDeserializeHeaderSuccess) {
    Header original = Header::create(OpCode::C_INPUT, 789, 5, 0);
    auto bytes = Serializer::serializeForNetwork(original);
    std::span<const std::uint8_t> span(bytes);

    auto result = Serializer::safeDeserializeHeader(span);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value().getOpCode(), OpCode::C_INPUT);
}

