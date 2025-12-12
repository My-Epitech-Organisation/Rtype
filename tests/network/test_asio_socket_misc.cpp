#include <gtest/gtest.h>

#include <asio/io_context.hpp>

#include "transport/AsioUdpSocket.hpp"

using namespace rtype::network;

TEST(AsioUdpSocketMisc, IsOpenLocalPortAndCloseCancelNoCrash) {
    asio::io_context ctx;
    AsioUdpSocket socket(ctx);

    EXPECT_TRUE(socket.isOpen());
    EXPECT_EQ(socket.localPort(), 0u);
    EXPECT_NO_THROW(socket.cancel());
    EXPECT_NO_THROW(socket.close());
}
