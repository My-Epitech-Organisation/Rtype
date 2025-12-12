#include <gtest/gtest.h>

#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"

using namespace rtype::network;

TEST(SerializerValidateExtract, ServerUserIdValidationFail) {
    Header h = Header::createServer(OpCode::PONG, 1, 0);
    // Tamper with userId to simulate false server claim
    h.userId = 123; 
    auto raw = ByteOrderSpec::serializeToNetwork(h);
    auto res = Serializer::validateAndExtractPacket(std::span<const uint8_t>(raw), true);
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::InvalidUserId);
}
