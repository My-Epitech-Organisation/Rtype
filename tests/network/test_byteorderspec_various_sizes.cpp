#include <gtest/gtest.h>

#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::network;

#pragma pack(push, 1)
struct S2 { uint16_t a; };
struct S3 { uint8_t a; uint16_t b; };
struct S4 { uint32_t a; };
struct S6 { uint32_t a; uint16_t b; };
struct S8 { uint32_t a; uint32_t b; };
#pragma pack(pop)

#pragma pack(push, 1)
struct S5 { uint32_t a; uint8_t b; };
struct S7 { uint32_t a; uint16_t b; uint8_t c; };
#pragma pack(pop)

TEST(ByteOrderSpecVariousSizes, RoundTrips) {
    S2 s2{0x2233};
    S3 s3{0x11, 0x4455};
    S4 s4{0x11223344};
    S6 s6{0x55667788, 0x99AA};
    S8 s8{0x0A0B0C0D, 0x0E0F1011};
    S5 s5{0x11223344, 0x77};
    S7 s7{0x21222324, 0x8899, 0xAA};

    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s2)).a, s2.a);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s3)).b, s3.b);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s4)).a, s4.a);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s6)).b, s6.b);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s8)).a, s8.a);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s5)).b, s5.b);
    ASSERT_EQ(ByteOrderSpec::fromNetwork(ByteOrderSpec::toNetwork(s7)).b, s7.b);
}
