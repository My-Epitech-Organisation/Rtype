#include <gtest/gtest.h>
#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(OpCodeExhaustive, ValidityAndCategory) {
    for (int i = 0; i < 256; ++i) {
        uint8_t raw = static_cast<uint8_t>(i);
        bool valid = isValidOpCode(raw);
        OpCode op = static_cast<OpCode>(raw);
        auto str = toString(op);
        if (!valid) {
            ASSERT_EQ(str, "UNKNOWN");
        }
        // Category function for known ranges
        if (isValidOpCode(raw)) {
            auto cat = getCategory(op);
            ASSERT_FALSE(cat.empty());
        }
    }
}
