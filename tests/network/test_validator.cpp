#include <gtest/gtest.h>

#include "protocol/Validator.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"
#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::network;

TEST(ValidatorBasicTest, Magic) {
    auto ok = Validator::validateMagic(kMagicByte);
    EXPECT_TRUE(ok.isOk());
    auto bad = Validator::validateMagic(0x00);
    EXPECT_TRUE(bad.isErr());
}

TEST(ValidatorBasicTest, OpCode) {
    auto ok = Validator::validateOpCode(static_cast<std::uint8_t>(OpCode::C_CONNECT));
    EXPECT_TRUE(ok.isOk());
    auto bad = Validator::validateOpCode(0x99);
    EXPECT_TRUE(bad.isErr());
}

TEST(ValidatorHeaderTest, HeaderInvalidCases) {
    Header h = Header::create(OpCode::C_INPUT, kMinClientUserId, 1, sizeof(InputPayload));
    // invalid magic
    h.magic = 0x00;
    EXPECT_TRUE(Validator::validateHeader(h).isErr());

    h.magic = kMagicByte;
    // invalid opcode
    h.opcode = 0x99;
    EXPECT_TRUE(Validator::validateHeader(h).isErr());

    // reserved non-zero
    h.opcode = static_cast<std::uint8_t>(OpCode::C_INPUT);
    h.reserved = {1, 0, 0};
    EXPECT_TRUE(Validator::validateHeader(h).isErr());
}

TEST(ValidatorPacketSize, Checks) {
    EXPECT_TRUE(Validator::validatePacketSize(kHeaderSize).isOk());
    EXPECT_TRUE(Validator::validatePacketSize(kHeaderSize - 1).isErr());
    EXPECT_TRUE(Validator::validatePacketSize(kMaxPacketSize + 1).isErr());
}

TEST(ValidatorPayloadSize, FixedAndVariable) {
    // fixed-size: C_INPUT expects InputPayload (1 byte)
    auto res = Validator::validatePayloadSize(OpCode::C_INPUT, sizeof(InputPayload));
    EXPECT_TRUE(res.isOk());
    res = Validator::validatePayloadSize(OpCode::C_INPUT, 0);
    EXPECT_TRUE(res.isErr());

    // variable: R_GET_USERS
    std::vector<std::uint8_t> payload;
    payload.push_back(2); // count
    uint32_t id1 = 1, id2 = 2;
    ByteOrder::writeTo(reinterpret_cast<std::uint8_t*>(&id1), id1); // won't be used directly
    // append IDs in LE/BE? We'll just append bytes
    for (int i = 0; i < 4; ++i) payload.push_back(0);
    for (int i = 0; i < 4; ++i) payload.push_back(0);

    res = Validator::validatePayloadSize(OpCode::R_GET_USERS, payload.size(), std::span<const std::uint8_t>(payload));
    EXPECT_TRUE(res.isOk());

    // malformed count too large
    std::vector<std::uint8_t> badPayload{255};
    EXPECT_TRUE(Validator::validatePayloadSize(OpCode::R_GET_USERS, badPayload.size(), std::span<const std::uint8_t>(badPayload)).isErr());
}

TEST(ValidatorUserId, ClientServer) {
    // C_CONNECT may be unassigned
    EXPECT_TRUE(Validator::validateClientUserId(kUnassignedUserId, OpCode::C_CONNECT).isOk());
    // C_CONNECT with assigned id invalid
    EXPECT_TRUE(Validator::validateClientUserId(kMinClientUserId, OpCode::C_CONNECT).isErr());

    // server user id invalid for clients
    EXPECT_TRUE(Validator::validateClientUserId(kServerUserId, OpCode::C_INPUT).isErr());

    // valid client id for input
    EXPECT_TRUE(Validator::validateClientUserId(kMinClientUserId, OpCode::C_INPUT).isOk());

    // server user id check
    EXPECT_TRUE(Validator::validateServerUserId(kServerUserId).isOk());
    EXPECT_TRUE(Validator::validateServerUserId(kMinClientUserId).isErr());
}

TEST(ValidatorBufferBoundsSafeDeserialize, Bounds) {
    std::vector<std::uint8_t> buf(kHeaderSize, 0);
    std::span<const std::uint8_t> span(buf);
    EXPECT_TRUE(Validator::validateBufferBounds(span, 0, kHeaderSize).isOk());
    EXPECT_TRUE(Validator::validateBufferBounds(span, 1, kHeaderSize).isErr());

    // safeDeserialize with insufficient bytes
    auto res = Validator::safeDeserialize<Header>(span, 1);
    EXPECT_TRUE(res.isErr());
}

TEST(ValidatorValidatePacket, Pipeline) {
    // Create a valid header+payload for C_INPUT
    Header h = Header::create(OpCode::C_INPUT, kMinClientUserId, 10, sizeof(InputPayload));
    auto headerBytes = ByteOrderSpec::serializeToNetwork(h);

    std::vector<std::uint8_t> raw(headerBytes.begin(), headerBytes.end());
    raw.push_back(InputMask::kShoot);

    auto result = Validator::validatePacket(std::span<const std::uint8_t>(raw), false);
    EXPECT_TRUE(result.isOk());

    // Packet with mismatched payload size
    Header h2 = Header::create(OpCode::C_INPUT, kMinClientUserId, 20, sizeof(InputPayload));
    auto hb2 = ByteOrderSpec::serializeToNetwork(h2);
    std::vector<std::uint8_t> raw2(hb2.begin(), hb2.end());
    // missing payload
    auto res2 = Validator::validatePacket(std::span<const std::uint8_t>(raw2), false);
    EXPECT_TRUE(res2.isErr());
}
