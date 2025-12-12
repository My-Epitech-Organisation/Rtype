#include <gtest/gtest.h>

#include <vector>

#include "protocol/Validator.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(ValidatorBranches, PacketTooSmall) {
    std::vector<uint8_t> smallBuf(1);
    auto res = Validator::validatePacket(std::span<const uint8_t>(smallBuf), false);
    EXPECT_TRUE(res.isErr());
}

TEST(ValidatorBranches, InvalidMagic) {
    Header h = Header::create(OpCode::PING, 1, 0, 0);
    h.magic = 0xFF;
    auto buf = ByteOrderSpec::serializeToNetwork(h);
    buf.resize(kHeaderSize); // ensure header-sized
    auto res = Validator::validatePacket(std::span<const uint8_t>(buf), false);
    EXPECT_EQ(res.error(), NetworkError::InvalidMagic);
}

TEST(ValidatorBranches, PayloadMaxSizeTooLarge) {
    Header h = Header::create(OpCode::PONG, 1, 0, 0);
    h.payloadSize = static_cast<uint16_t>(kMaxPayloadSize + 1);
    auto buf = ByteOrderSpec::serializeToNetwork(h);
    buf.resize(kHeaderSize);
    auto res = Validator::validatePacket(std::span<const uint8_t>(buf), false);
    EXPECT_EQ(res.error(), NetworkError::PacketTooLarge);
}

TEST(ValidatorBranches, HeaderReservedInvalid) {
    Header h = Header::create(OpCode::PING, 1, 0, 0);
    h.reserved = {1, 2, 3};
    auto buf = ByteOrderSpec::serializeToNetwork(h);
    buf.resize(kHeaderSize);
    auto res = Validator::validatePacket(std::span<const uint8_t>(buf), false);
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorBranches, PayloadSizeMismatch) {
    Header h = Header::create(OpCode::PONG, 1, 0, 10);
    // Actual size will be header size only -> mismatch
    auto buf = ByteOrderSpec::serializeToNetwork(h);
    buf.resize(kHeaderSize);
    auto res = Validator::validatePacket(std::span<const uint8_t>(buf), false);
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorBranches, RGetUsersInvalidCount) {
    Header h = Header::create(OpCode::R_GET_USERS, 1, 0, 1 + 5 * sizeof(uint32_t));
    // payload[0] = 6 which is more than kMaxUsersInResponse
    std::vector<uint8_t> payload(1 + 6 * 4);
    payload[0] = 6;
    auto headerBuf = ByteOrderSpec::serializeToNetwork(h);
    std::vector<uint8_t> buf = headerBuf;
    buf.insert(buf.end(), payload.begin(), payload.end());
    auto res = Validator::validatePacket(std::span<const uint8_t>(buf), true);
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorBranches, ValidateClientUserIdVarious) {
    // C_CONNECT allowed with unassigned id
    auto ok = Validator::validateClientUserId(kUnassignedUserId, OpCode::C_CONNECT);
    EXPECT_TRUE(ok.isOk());
    // C_CONNECT with assigned id -> invalid
    auto err = Validator::validateClientUserId(2, OpCode::C_CONNECT);
    EXPECT_TRUE(err.isErr());

    // Server user id in client packet invalid
    err = Validator::validateClientUserId(kServerUserId, OpCode::PING);
    EXPECT_TRUE(err.isErr());

    // Valid client id
    ok = Validator::validateClientUserId(kMinClientUserId, OpCode::PING);
    EXPECT_TRUE(ok.isOk());
}
