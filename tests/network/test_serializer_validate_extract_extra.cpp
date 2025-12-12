/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional tests for Serializer validate/extract and byte order error paths
*/

#include <gtest/gtest.h>

#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"

using namespace rtype::network;

TEST(SerializerValidateExtractExtra, ClientUserIdValidationFail) {
    // Using PING (no payload) to test client userId validation
    Header h = Header::create(OpCode::PING, kServerUserId, 1, 0);
    auto raw = ByteOrderSpec::serializeToNetwork(h);
    auto res = Serializer::validateAndExtractPacket(std::span<const uint8_t>(raw), false);
    EXPECT_TRUE(res.isErr());
}

TEST(SerializerToNetworkByteOrder, BufferSizeMismatchThrows) {
    std::vector<uint8_t> badBuffer{0x01, 0x02}; // too small for uint32_t
    EXPECT_THROW(Serializer::toNetworkByteOrder<uint32_t>(badBuffer), std::runtime_error);
}

TEST(SerializerFromNetworkByteOrder, BufferSizeMismatchThrows) {
    std::vector<uint8_t> badBuffer{0x01}; // too small for uint16_t
    EXPECT_THROW(Serializer::fromNetworkByteOrder<uint16_t>(badBuffer), std::runtime_error);
}

TEST(SerializerValidateExtractExtra, HeaderValidationInvalidOpCode) {
    Header h = Header::create(OpCode::C_INPUT, kMinClientUserId, 1, 0);
    h.opcode = 0xFF; // invalid opcode
    auto raw = ByteOrderSpec::serializeToNetwork(h);
    auto res = Serializer::validateAndExtractPacket(std::span<const uint8_t>(raw), false);
    EXPECT_TRUE(res.isErr());
}
