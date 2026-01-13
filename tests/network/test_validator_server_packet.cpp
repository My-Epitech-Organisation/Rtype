#include <gtest/gtest.h>

#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Validator.hpp"

using namespace rtype::network;

TEST(ValidatorServerPacketTest, InvalidServerUserIdIsRejected) {
    // Create a header that claims to be from server but uses a client id
    Header header = Header::createServer(OpCode::S_ENTITY_SPAWN, 1, 0);
    // Force an invalid user id (client id)
    header.userId = 42;

    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    auto res = Validator::validatePacket(std::span<const std::uint8_t>(bytes), true);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::InvalidUserId);
}

TEST(ValidatorServerPacketTest, ValidServerPacketAccepted) {
    // Proper server header with server user id
    Header header = Header::createServer(OpCode::S_ENTITY_SPAWN, 1, 0);
    auto bytes = ByteOrderSpec::serializeToNetwork(header);

    auto res = Validator::validatePacket(std::span<const std::uint8_t>(bytes), true);
    EXPECT_TRUE(res.isOk());
}
