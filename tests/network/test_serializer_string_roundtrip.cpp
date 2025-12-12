#include <gtest/gtest.h>

#include "Serializer.hpp"

using namespace rtype::network;

TEST(SerializerString, RoundTripEmptyAndNormal) {
    std::string s1 = "";
    auto buf1 = Serializer::serialize(s1);
    EXPECT_NO_THROW({ (void)Serializer::deserializeString(buf1); });
    EXPECT_EQ(Serializer::deserializeString(buf1), s1);

    std::string s2 = "Hello, serializer!";
    auto buf2 = Serializer::serialize(s2);
    EXPECT_EQ(Serializer::deserializeString(buf2), s2);
}

TEST(SerializerString, DeserializeInvalidLengthThrows) {
    // Create a buffer with declared length larger than actual data
    auto buf = Serializer::serialize(std::string("abc"));
    // Corrupt the length field to be large
    // length field is first 4 bytes; overwrite with 0xFFFFFFFF
    buf[0] = 0xFF; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0xFF;
    EXPECT_THROW({ (void)Serializer::deserializeString(buf); }, std::runtime_error);
}
