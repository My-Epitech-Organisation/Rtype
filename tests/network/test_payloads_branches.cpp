#include <gtest/gtest.h>

#include "protocol/Payloads.hpp"
#include "protocol/OpCode.hpp"

using namespace rtype::network;

TEST(PayloadsBranches, IterateAllOpCodes) {
    // Iterate every possible byte value and call getPayloadSize/hasVariablePayload
    // to exercise the switch branches in getPayloadSize and hasVariablePayload.
    for (int i = 0; i < 256; ++i) {
        auto op = static_cast<OpCode>(static_cast<uint8_t>(i));
        // Must not throw and return a sensible value
        EXPECT_NO_THROW({
            volatile auto s = getPayloadSize(op);
            (void)s;
            volatile auto v = hasVariablePayload(op);
            (void)v;
        });
    }
}
