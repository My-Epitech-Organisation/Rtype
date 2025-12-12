#include <gtest/gtest.h>

#include "Serializer.hpp"

using namespace rtype::network;

#pragma pack(push, 1)
struct PackedStruct {
    uint32_t a;
    uint16_t b;
};
#pragma pack(pop)

TEST(SerializerTemplate, SerializeDeserializeRoundTrip) {
    PackedStruct in{0x11223344, 0x5566};
    auto buf = Serializer::serialize(in);
    auto out = Serializer::deserialize<PackedStruct>(buf);
    ASSERT_EQ(in.a, out.a);
    ASSERT_EQ(in.b, out.b);
}

TEST(SerializerTemplate, DeserializeSizeMismatchThrows) {
    PackedStruct in{0x1, 0x2};
    auto buf = Serializer::serialize(in);
    // remove one byte to cause mismatch
    buf.pop_back();
    EXPECT_THROW(Serializer::deserialize<PackedStruct>(buf), std::runtime_error);
}

TEST(SerializerByteOrder, ToFromNetworkBufferSizeMismatch) {
    PackedStruct in{0xAABBCCDD, 0xEEFF};
    auto buf = Serializer::serialize(in);
    // Mismatched size - remove a byte
    buf.pop_back();
    EXPECT_THROW(Serializer::toNetworkByteOrder<PackedStruct>(buf), std::runtime_error);
    // Add the byte back
    buf.push_back(0);
    auto nb = Serializer::toNetworkByteOrder<PackedStruct>(buf);
    // now call fromNetwork with wrong size
    nb.pop_back();
    EXPECT_THROW(Serializer::fromNetworkByteOrder<PackedStruct>(nb), std::runtime_error);
}
