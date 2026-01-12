#include <gtest/gtest.h>

#include "../../../lib/common/src/Types.hpp"
#include <sstream>

using namespace rtype;

TEST(EndpointTest, BasicOperations) {
    Endpoint a{"127.0.0.1", 4242};
    Endpoint b{"127.0.0.1", 4242};
    Endpoint c{"192.168.1.1", 4242};

    EXPECT_TRUE(a.isValid());
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_FALSE(Endpoint{}.isValid());

    EXPECT_LT(a, c); // '127...' < '192...' lexicographically

    EXPECT_EQ(a.toString(), "127.0.0.1:4242");

    std::ostringstream os;
    os << a;
    EXPECT_EQ(os.str(), "127.0.0.1:4242");
}

TEST(StringifyEnums, ClientStateAndDisconnectReason) {
    EXPECT_EQ(toString(ClientState::Connecting), "connecting");
    EXPECT_EQ(toString(ClientState::Connected), "connected");
    EXPECT_EQ(toString(DisconnectReason::Timeout), "timeout");
    // Out-of-range safety
    EXPECT_EQ(toString(static_cast<ClientState>(999)), "unknown");
    EXPECT_EQ(toString(static_cast<DisconnectReason>(999)), "unknown");
}

TEST(EndpointTest, MoveAssignmentSelfAssignment) {
    Endpoint a{"192.168.1.1", 8080};
    // Suppress compiler warning about self-assignment
    Endpoint& aRef = a;
    a = std::move(aRef);  // Self-assignment via move
    // Object should still be valid
    EXPECT_EQ(a.address, "192.168.1.1");
    EXPECT_EQ(a.port, 8080);
}

TEST(EndpointTest, MoveAssignmentNormal) {
    Endpoint a{"10.0.0.1", 1234};
    Endpoint b{"172.16.0.1", 5678};
    b = std::move(a);
    EXPECT_EQ(b.address, "10.0.0.1");
    EXPECT_EQ(b.port, 1234);
}
