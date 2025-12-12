#include <gtest/gtest.h>

#include <asio/io_context.hpp>

#include "transport/AsioUdpSocket.hpp"

using namespace rtype::network;

TEST(AsioUdpSocketErrorCases, AsyncSendToInvalidHostTriggersHostNotFound) {
    asio::io_context ctx;
    AsioUdpSocket socket(ctx);

    Buffer data = {0x01, 0x02};
    Endpoint badDest{"not.a.real.host", 12345};

    std::promise<NetworkError> prom;
    auto fut = prom.get_future();

    socket.asyncSendTo(data, badDest, [&prom](Result<std::size_t> res) mutable {
        if (res.isErr()) {
            prom.set_value(res.error());
        } else {
            prom.set_value(NetworkError::None);
        }
    });

    // Run the context to process posted error callback
    ctx.run_for(std::chrono::milliseconds(50));

    auto err = fut.get();
    EXPECT_EQ(err, NetworkError::HostNotFound);
}

TEST(AsioUdpSocketErrorCases, AsyncReceiveFromNullArgsReturnsImmediateError) {
    asio::io_context ctx;
    AsioUdpSocket socket(ctx);

    std::promise<NetworkError> prom;
    auto fut = prom.get_future();

    socket.asyncReceiveFrom(nullptr, nullptr, [&prom](Result<std::size_t> res) mutable {
        if (res.isErr()) prom.set_value(res.error());
        else prom.set_value(NetworkError::None);
    });

    // No io_context run required - handler is called synchronously
    auto err = fut.get();
    EXPECT_EQ(err, NetworkError::InternalError);
}

TEST(AsioUdpSocketErrorCases, AsyncSendToNoHandlerReturnsNoCrash) {
    asio::io_context ctx;
    AsioUdpSocket socket(ctx);

    Buffer data = {0x01};
    Endpoint dest{"127.0.0.1", 0};

    // Empty handler should be a no-op
    std::function<void(Result<std::size_t>)> empty;
    EXPECT_NO_THROW(socket.asyncSendTo(data, dest, empty));
}

TEST(AsioUdpSocketErrorCases, AsyncSendToInvalidDestReturnsNotConnected) {
    asio::io_context ctx;
    AsioUdpSocket socket(ctx);

    Buffer data = {0xAA};
    Endpoint invalidDest{"127.0.0.1", 0};

    std::promise<NetworkError> prom;
    auto fut = prom.get_future();

    socket.asyncSendTo(data, invalidDest, [&prom](Result<std::size_t> res) mutable {
        if (res.isErr()) prom.set_value(res.error());
        else prom.set_value(NetworkError::None);
    });

    ctx.run_for(std::chrono::milliseconds(10));
    auto err = fut.get();
    EXPECT_EQ(err, NetworkError::NotConnected);
}
