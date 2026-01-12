#include <gtest/gtest.h>

#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(OpCodeTest, Reliability) {
    EXPECT_TRUE(isReliable(OpCode::S_ACCEPT));
    EXPECT_FALSE(isReliable(OpCode::S_ENTITY_MOVE));
    EXPECT_FALSE(isReliable(OpCode::PONG));
}

TEST(OpCodeTest, ClientServerClassification) {
    EXPECT_TRUE(isClientOpCode(OpCode::C_CONNECT));
    EXPECT_TRUE(isClientOpCode(OpCode::C_INPUT));
    EXPECT_TRUE(isClientOpCode(OpCode::DISCONNECT));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ACCEPT));
    EXPECT_TRUE(isServerOpCode(OpCode::S_ENTITY_SPAWN));
    EXPECT_TRUE(isServerOpCode(OpCode::PONG));
}

TEST(OpCodeTest, ValidityAndUnknown) {
    EXPECT_TRUE(isValidOpCode(static_cast<std::uint8_t>(OpCode::C_CONNECT)));
    EXPECT_FALSE(isValidOpCode(0x99));
    EXPECT_EQ(getCategory(OpCode::C_CONNECT), "Session");
    EXPECT_EQ(getCategory(static_cast<OpCode>(0)), "Unknown");
    EXPECT_EQ(toString(OpCode::C_CONNECT), "C_CONNECT");
    EXPECT_EQ(toString(static_cast<OpCode>(0)), "UNKNOWN");
}

TEST(OpCodeTest, ChatOpcodes) {
    // Validity
    EXPECT_TRUE(isValidOpCode(static_cast<std::uint8_t>(OpCode::C_CHAT)));
    EXPECT_TRUE(isValidOpCode(static_cast<std::uint8_t>(OpCode::S_CHAT)));

    // Reliability
    EXPECT_TRUE(isReliable(OpCode::C_CHAT));
    EXPECT_TRUE(isReliable(OpCode::S_CHAT));

    // Classification
    EXPECT_TRUE(isClientOpCode(OpCode::C_CHAT));
    EXPECT_FALSE(isServerOpCode(OpCode::C_CHAT));

    EXPECT_TRUE(isServerOpCode(OpCode::S_CHAT));
    EXPECT_FALSE(isClientOpCode(OpCode::S_CHAT));

    // String conversion
    EXPECT_EQ(toString(OpCode::C_CHAT), "C_CHAT");
    EXPECT_EQ(toString(OpCode::S_CHAT), "S_CHAT");
}

