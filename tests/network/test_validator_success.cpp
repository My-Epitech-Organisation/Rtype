#include <gtest/gtest.h>

#include "protocol/Validator.hpp"
#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(ValidatorSuccess, ValidateAndExtractPacketSuccess) {
    Header h = Header::create(OpCode::PING, kMinClientUserId, 1, 0);
    auto headerBuf = ByteOrderSpec::serializeToNetwork(h);
    // no payload
    auto res = Validator::validatePacket(std::span<const uint8_t>(headerBuf), false);
    EXPECT_TRUE(res.isOk());
    auto extract = Serializer::validateAndExtractPacket(std::span<const uint8_t>(headerBuf), false);
    EXPECT_TRUE(extract.isOk());
    auto [hdr, payload] = extract.value();
    EXPECT_EQ(hdr.getOpCode(), h.getOpCode());
    EXPECT_EQ(payload.size(), 0u);
}
