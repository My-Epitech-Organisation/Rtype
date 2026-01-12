#include <gtest/gtest.h>

#include "common/src/ArgParser/NumberParser.hpp"

using namespace rtype;

TEST(NumberParserBranches, UnsignedNegativeIsError) {
    auto res = parseNumber<uint32_t>("-1", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, UnsignedNonNumberIsError) {
    auto res = parseNumber<uint32_t>("abc", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, UnsignedPartialParseIsError) {
    auto res = parseNumber<uint32_t>("12abc", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, UnsignedOutOfRangeIsError) {
    // pick a value larger than uint8_t max but still numeric
    auto res = parseNumber<uint8_t>("1000", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, SignedPartialParseIsError) {
    auto res = parseNumber<int32_t>("42xyz", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, SignedOutOfRangeIsError) {
    auto res = parseNumber<int8_t>("1000", "num");
    EXPECT_FALSE(res.has_value());
}

TEST(NumberParserBranches, ValidUnsignedAndSignedReturnValue) {
    auto u = parseNumber<uint32_t>("42", "num");
    auto s = parseNumber<int32_t>("-7", "num");
    ASSERT_TRUE(u.has_value());
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(u.value(), 42u);
    EXPECT_EQ(s.value(), -7);
}

TEST(NumberParserBranches, MinMaxChecksFail) {
    // min=10 max=20, input 5 => should fail
    auto res = parseNumber<int>("5", "num", 10, 20);
    EXPECT_FALSE(res.has_value());
}
