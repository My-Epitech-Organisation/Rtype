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
