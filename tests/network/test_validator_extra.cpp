#include <gtest/gtest.h>

#include "protocol/Validator.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(ValidatorExtra, UnknownOpCode) {
    Header h = Header::create(OpCode::PING, 1, 1, 0);
    h.opcode = 0xFF; // invalid opcode
    auto res = Validator::validateHeader(h);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::UnknownOpcode);
}

TEST(ValidatorExtra, ClientUnassignedIdInvalid) {
    auto res = Validator::validateClientUserId(kUnassignedUserId, OpCode::PONG);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::InvalidUserId);
}

TEST(ValidatorExtra, PayloadSizeValidVariableOpcode) {
    // Test R_GET_USERS with empty payload span (triggers line 107 branch)
    auto res = Validator::validatePayloadSize(OpCode::R_GET_USERS, 5, {});
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorExtra, PayloadSizeR_GET_USERS_CountTooHigh) {
    // payload[0] > kMaxUsersInResponse triggers line 111 branch
    std::vector<uint8_t> payload = {255};  // count = 255 > max
    auto res = Validator::validatePayloadSize(OpCode::R_GET_USERS, 1, payload);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorExtra, ClientUserIdInRangeValid) {
    // Test valid client ID within kMinClientUserId and kMaxClientUserId
    auto res = Validator::validateClientUserId(kMinClientUserId, OpCode::PONG);
    EXPECT_TRUE(res.isOk());
    
    res = Validator::validateClientUserId(kMaxClientUserId, OpCode::PONG);
    EXPECT_TRUE(res.isOk());
}

TEST(ValidatorExtra, ClientUserIdServerIdInvalid) {
    // Client cannot use server ID
    auto res = Validator::validateClientUserId(kServerUserId, OpCode::PONG);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::InvalidUserId);
}

TEST(ValidatorExtra, ClientUserIdOutOfRangeInvalid) {
    // Test user ID of 0 (kUnassignedUserId) for non-CONNECT opcode
    auto res = Validator::validateClientUserId(kUnassignedUserId, OpCode::PONG);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::InvalidUserId);
}
