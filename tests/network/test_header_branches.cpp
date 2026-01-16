#include <gtest/gtest.h>

#include "protocol/Header.hpp"

using namespace rtype::network;

TEST(HeaderBranches, CreateAndFlags) {
    Header h = Header::create(OpCode::C_CONNECT, 1, 42, 5);
    // create sets default values and flags depending on reliability
    ASSERT_TRUE(h.hasValidMagic());
    ASSERT_TRUE(h.hasValidReserved());
    h.setReliable(false);
    ASSERT_FALSE(h.isReliable());
    h.setReliable(true);
    ASSERT_TRUE(h.isReliable());
    h.setAck(123);
    ASSERT_TRUE(h.isAck());
}

TEST(HeaderBranches, CreateServerAndConnect) {
    Header s = Header::createServer(OpCode::PONG, 1, 0);
    ASSERT_TRUE(s.isFromServer());
    Header c = Header::createConnect(1);
    ASSERT_TRUE(c.isFromUnassigned());
}

TEST(HeaderBranches, SetCompressedTrueAndFalse) {
    Header h = Header::create(OpCode::C_INPUT, 1, 42, 0);
    ASSERT_FALSE(h.isCompressed());
    h.setCompressed(true);
    ASSERT_TRUE(h.isCompressed());
    h.setCompressed(false);
    ASSERT_FALSE(h.isCompressed());
}

TEST(HeaderBranches, HasValidReservedAllBranches) {
    Header h = Header::create(OpCode::C_INPUT, 1, 42, 0);
    ASSERT_TRUE(h.hasValidReserved());
    
    // Test each reserved byte being non-zero
    h.reserved[0] = 1;
    ASSERT_FALSE(h.hasValidReserved());
    h.reserved[0] = 0;
    
    h.reserved[1] = 1;
    ASSERT_FALSE(h.hasValidReserved());
    h.reserved[1] = 0;
    
    h.reserved[2] = 1;
    ASSERT_FALSE(h.hasValidReserved());
}
