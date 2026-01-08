#include <gtest/gtest.h>

#include "server/shared/NetworkUtils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace rtype::server;

TEST(NetworkUtils, Port0IsAvailable) {
    // Port 0 should be considered available (OS-assigned)
    EXPECT_TRUE(isUdpPortAvailable(0));
}

TEST(NetworkUtils, PortUnavailableWhenBound) {
    // Create a temporary UDP socket and bind to an ephemeral port
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_GE(fd, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0; // ask OS for a free port

    ASSERT_EQ(::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), 0);

    // Retrieve the assigned port
    socklen_t len = sizeof(addr);
    ASSERT_EQ(::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len), 0);
    uint16_t port = ntohs(addr.sin_port);

    // Now the port should be unavailable
    EXPECT_FALSE(isUdpPortAvailable(port));

    ::close(fd);

    // After closing, it should become available again (timing/race could affect this on some OSes)
    EXPECT_TRUE(isUdpPortAvailable(port));
}
