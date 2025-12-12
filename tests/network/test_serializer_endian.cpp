#include <gtest/gtest.h>

#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"

using namespace rtype::network;

TEST(SerializerEndian, ToFromNetworkPrimitive) {
    uint32_t value = 0x12345678;
    auto native = Serializer::serialize<uint32_t>(value);
    auto network = Serializer::toNetworkByteOrder<uint32_t>(native);
    auto restored_native = Serializer::fromNetworkByteOrder<uint32_t>(network);
    auto final = Serializer::deserialize<uint32_t>(restored_native);
    EXPECT_EQ(final, value);
}

TEST(SerializerEndian, ToNetworkByteOrderWrongSizeThrows) {
    uint32_t value = 0x11223344;
    auto buf = Serializer::serialize<uint32_t>(value);
    buf.pop_back();
    EXPECT_THROW({ (void)Serializer::toNetworkByteOrder<uint32_t>(buf); }, std::runtime_error);
}
