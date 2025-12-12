#include <gtest/gtest.h>

#include <vector>

#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::network;

// Small trivially-copyable non-RFC type to exercise generic byte swapping
#pragma pack(push, 1)
struct NonRfc6 {
    std::uint32_t a;
    std::uint16_t b;
};
#pragma pack(pop)

static_assert(sizeof(NonRfc6) == 6, "NonRfc6 should be 6 bytes");

TEST(ByteOrderSpecBranches, GenericToFromNetwork) {
    NonRfc6 n;
    n.a = 0x01020304; // bytes: 01 02 03 04
    n.b = 0x0506;     // bytes: 05 06

    NonRfc6 network = ByteOrderSpec::toNetwork(n);
    NonRfc6 host = ByteOrderSpec::fromNetwork(network);
    ASSERT_EQ(host.a, n.a);
    ASSERT_EQ(host.b, n.b);
}

TEST(ByteOrderSpecBranches, DeserializeVectorTooSmall) {
    // Header is 16 bytes; pass a short vector to cause exception
    std::vector<std::uint8_t> smallBuf(8, 0);
    EXPECT_THROW(ByteOrderSpec::deserializeFromNetwork<Header>(smallBuf), std::runtime_error);
}

TEST(ByteOrderSpecBranches, DeserializeSpanTooSmall) {
    std::vector<std::uint8_t> smallBuf(8, 0);
    std::span<const std::uint8_t> s(smallBuf);
    EXPECT_THROW(ByteOrderSpec::deserializeFromNetwork<Header>(s), std::runtime_error);
}
