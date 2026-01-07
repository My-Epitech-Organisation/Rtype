#include <gtest/gtest.h>

#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::network;

TEST(ByteOrderSpecExtra, GenericFourByteLoop) {
    #pragma pack(push, 1)
    struct TwoInts { uint32_t a; uint32_t b; };
    #pragma pack(pop)

    TwoInts v{0x11223344, 0x55667788};
    auto ser = ByteOrderSpec::serializeToNetwork(v);
    ASSERT_EQ(ser.size(), sizeof(TwoInts));
    auto rt = ByteOrderSpec::deserializeFromNetwork<TwoInts>(ser);
    EXPECT_EQ(rt.a, v.a);
    EXPECT_EQ(rt.b, v.b);
}

TEST(ByteOrderSpecExtra, GenericTwelveByteLoop) {
    #pragma pack(push, 1)
    struct ThreeInts { uint32_t a; uint32_t b; uint32_t c; };
    #pragma pack(pop)

    ThreeInts v{0x01020304, 0x05060708, 0x090A0B0C};
    auto ser = ByteOrderSpec::serializeToNetwork(v);
    ASSERT_EQ(ser.size(), sizeof(ThreeInts));
    auto rt = ByteOrderSpec::deserializeFromNetwork<ThreeInts>(ser);
    EXPECT_EQ(rt.a, v.a);
    EXPECT_EQ(rt.b, v.b);
    EXPECT_EQ(rt.c, v.c);
}

TEST(ByteOrderSpecExtra, SpanDeserializeEmptyPayloadThrows) {
    struct Empty {};
    std::vector<std::uint8_t> nonempty = {0xFF};
    EXPECT_THROW(static_cast<void>(ByteOrderSpec::deserializeFromNetwork<Empty>(nonempty)), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ByteOrderSpec::deserializeFromNetwork<Empty>(std::span<const std::uint8_t>(nonempty))), std::runtime_error);
}
