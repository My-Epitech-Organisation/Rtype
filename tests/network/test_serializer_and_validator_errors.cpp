#include <gtest/gtest.h>

#include <vector>

#include "Serializer.hpp"
#include "protocol/Validator.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::network;

TEST(SerializerErrorBranches, ToNetworkByteOrderSizeMismatchThrows)
{
    uint32_t value = 0x12345678;
    auto buf = Serializer::serialize(value);
    // Remove a byte to make the buffer size incorrect
    buf.pop_back();
    EXPECT_THROW({ (void)Serializer::toNetworkByteOrder<uint32_t>(buf); }, std::runtime_error);
}

TEST(SerializerErrorBranches, FromNetworkByteOrderSizeMismatchThrows)
{
    uint32_t value = 0x12345678;
    auto buf = Serializer::serialize(value);
    // Increase size to make mismatch
    buf.push_back(0);
    EXPECT_THROW({ (void)Serializer::fromNetworkByteOrder<uint32_t>(buf); }, std::runtime_error);
}

TEST(SerializerErrorBranches, DeserializePrimitiveSizeMismatchThrows)
{
    std::vector<uint8_t> smallBuf(1, 0x01);
    EXPECT_THROW({ (void)Serializer::deserialize<uint32_t>(smallBuf); }, std::runtime_error);
}

TEST(ValidatorErrorBranches, SafeDeserializeBoundsFailure)
{
    std::vector<uint8_t> tiny(1, 0);
    auto res = Validator::safeDeserialize<Header>(std::span<const uint8_t>(tiny));
    EXPECT_TRUE(res.isErr());
}

TEST(ValidatorErrorBranches, RGetUsersPayloadTooSmall)
{
    std::vector<uint8_t> payload(0); // empty
    auto res = Validator::validateRGetUsersPayload(std::span<const uint8_t>(payload));
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::PacketTooSmall);
}

TEST(ValidatorErrorBranches, RGetUsersCountMismatch)
{
    std::vector<uint8_t> payload(1); // count = 1 but missing 4 bytes for UID
    payload[0] = 1;
    auto res = Validator::validateRGetUsersPayload(std::span<const uint8_t>(payload));
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), NetworkError::MalformedPacket);
}

TEST(ValidatorErrorBranches, ValidateServerUserId)
{
    auto ok = Validator::validateServerUserId(kServerUserId);
    EXPECT_TRUE(ok.isOk());
    auto err = Validator::validateServerUserId(123);
    EXPECT_TRUE(err.isErr());
}
