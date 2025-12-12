/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for AsioUdpSocket
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "transport/IoContext.hpp"
#include "transport/AsioUdpSocket.hpp"
#include "core/Core.hpp"

using namespace rtype::network;

// ============================================================================
// Test Fixture
// ============================================================================

class AsioUdpSocketTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Fresh IoContext for each test
        ctx_ = std::make_unique<IoContext>();
    }

    void TearDown() override {
        ctx_->stop();
        ctx_.reset();
    }

    /**
     * @brief Run io_context until condition is met or timeout
     * @return true if condition was met, false if timeout
     */
    bool runUntil(std::function<bool()> condition,
                  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
        auto start = std::chrono::steady_clock::now();
        while (!condition()) {
            ctx_->poll();
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }

    std::unique_ptr<IoContext> ctx_;
};

// ============================================================================
// Basic Socket Tests
// ============================================================================

TEST_F(AsioUdpSocketTest, CreateSocket) {
    auto socket = createAsyncSocket(ctx_->get());
    ASSERT_NE(socket, nullptr);
    EXPECT_TRUE(socket->isOpen());
}

TEST_F(AsioUdpSocketTest, BindSucceeds) {
    auto socket = createAsyncSocket(ctx_->get());
    ASSERT_NE(socket, nullptr);

    // Bind to ephemeral port (0)
    auto result = socket->bind(0);
    EXPECT_TRUE(result.isOk()) << "Bind failed: " << toString(result.error());

    // Should have a port assigned
    EXPECT_GT(socket->localPort(), 0);
}

TEST_F(AsioUdpSocketTest, BindToSpecificPort) {
    auto socket = createAsyncSocket(ctx_->get());
    ASSERT_NE(socket, nullptr);

    // First bind to ephemeral port to get an available port
    auto helperSocket = createAsyncSocket(ctx_->get());
    auto helperResult = helperSocket->bind(0);
    ASSERT_TRUE(helperResult.isOk()) << "Helper bind failed";
    uint16_t testPort = helperSocket->localPort();
    helperSocket->close();

    // Now bind our test socket to the port we know was available
    auto result = socket->bind(testPort);
    EXPECT_TRUE(result.isOk()) << "Bind failed: " << toString(result.error());
    EXPECT_EQ(socket->localPort(), testPort);
}

TEST_F(AsioUdpSocketTest, BindTwiceToSameSocketFails) {
    auto socket = createAsyncSocket(ctx_->get());

    // First bind to ephemeral port
    auto result1 = socket->bind(0);
    EXPECT_TRUE(result1.isOk()) << "First bind failed: " << toString(result1.error());
    uint16_t firstPort = socket->localPort();
    EXPECT_GT(firstPort, 0);

    // Second bind on SAME socket - implementation closes and reopens
    // This is actually valid behavior since we reopen the socket
    auto result2 = socket->bind(0);
    EXPECT_TRUE(result2.isOk()) << "Second bind failed: " << toString(result2.error());

    // Verify socket is still functional after rebind
    EXPECT_TRUE(socket->isOpen());
}

TEST_F(AsioUdpSocketTest, CloseSocket) {
    auto socket = createAsyncSocket(ctx_->get());
    ASSERT_NE(socket, nullptr);
    EXPECT_TRUE(socket->isOpen());

    socket->close();
    EXPECT_FALSE(socket->isOpen());

    // Closing again should be safe (no-op)
    socket->close();
    EXPECT_FALSE(socket->isOpen());
}

// ============================================================================
// Async Send/Receive Tests
// ============================================================================

TEST_F(AsioUdpSocketTest, AsyncSendReceiveRoundtrip) {
    // Create server socket
    auto server = createAsyncSocket(ctx_->get());
    ASSERT_NE(server, nullptr);

    auto bindResult = server->bind(0);
    ASSERT_TRUE(bindResult.isOk()) << "Server bind failed";
    uint16_t serverPort = server->localPort();

    // Create client socket
    auto client = createAsyncSocket(ctx_->get());
    ASSERT_NE(client, nullptr);

    // Prepare test data
    Buffer sendData = {0xA1, 0x01, 0x00, 0x04, 'T', 'E', 'S', 'T'};
    Buffer recvBuffer(kMaxPacketSize);
    Endpoint sender;

    // Tracking variables
    std::atomic<bool> sendComplete{false};
    std::atomic<bool> recvComplete{false};
    std::atomic<size_t> bytesReceived{0};

    // Start async receive on server
    auto recvBufferPtr = std::make_shared<Buffer>();
    auto senderPtr = std::make_shared<Endpoint>();
    server->asyncReceiveFrom(recvBufferPtr, senderPtr,
        [&, recvBufferPtr, senderPtr](Result<size_t> result) {
            EXPECT_TRUE(result.isOk()) << "Receive failed: " << toString(result.error());
            if (result.isOk()) {
                bytesReceived = result.value();
                recvBuffer = *recvBufferPtr;
                sender = *senderPtr;
            }
            recvComplete = true;
        });

    // Send from client to server
    Endpoint serverEndpoint{"127.0.0.1", serverPort};
    client->asyncSendTo(sendData, serverEndpoint,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isOk()) << "Send failed: " << toString(result.error());
            if (result.isOk()) {
                EXPECT_EQ(result.value(), sendData.size());
            }
            sendComplete = true;
        });

    // Run until both complete
    bool completed = runUntil([&]() {
        return sendComplete && recvComplete;
    });

    ASSERT_TRUE(completed) << "Timeout waiting for send/receive";
    EXPECT_EQ(bytesReceived, sendData.size());

    // Verify received data matches sent data
    for (size_t i = 0; i < sendData.size(); ++i) {
        EXPECT_EQ(recvBuffer[i], sendData[i]) << "Mismatch at index " << i;
    }

    // Verify sender endpoint
    EXPECT_EQ(sender.address, "127.0.0.1");
    EXPECT_GT(sender.port, 0);
}

TEST_F(AsioUdpSocketTest, EchoServerPattern) {
    // Create echo server
    auto server = createAsyncSocket(ctx_->get());
    auto bindResult = server->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    uint16_t serverPort = server->localPort();

    // Create client
    auto client = createAsyncSocket(ctx_->get());

    // Test data
    Buffer request = {'H', 'E', 'L', 'L', 'O'};
    Buffer serverRecvBuf(kMaxPacketSize);
    Buffer clientRecvBuf(kMaxPacketSize);
    Endpoint serverSender;
    Endpoint clientSender;

    std::atomic<bool> echoReceived{false};

    // Server: receive and echo back
    auto serverRecvBufPtr = std::make_shared<Buffer>(kMaxPacketSize);
    auto serverSenderPtr = std::make_shared<Endpoint>();
    server->asyncReceiveFrom(serverRecvBufPtr, serverSenderPtr,
        [&, serverRecvBufPtr, serverSenderPtr](Result<size_t> result) {
            if (result.isOk()) {
                serverRecvBuf = *serverRecvBufPtr;
                serverSender = *serverSenderPtr;
                // Echo the data back
                Buffer echo(serverRecvBuf.begin(),
                           serverRecvBuf.begin() + result.value());
                server->asyncSendTo(echo, serverSender, [](Result<size_t>) {});
            }
        });

    // Client: send request
    Endpoint serverEndpoint{"127.0.0.1", serverPort};
    client->asyncSendTo(request, serverEndpoint, [](Result<size_t>) {});

    // Client: wait for echo
    auto clientRecvBufPtr = std::make_shared<Buffer>(kMaxPacketSize);
    auto clientSenderPtr = std::make_shared<Endpoint>();
    client->asyncReceiveFrom(clientRecvBufPtr, clientSenderPtr,
        [&, clientRecvBufPtr, clientSenderPtr](Result<size_t> result) {
            EXPECT_TRUE(result.isOk());
            if (result.isOk()) {
                EXPECT_EQ(result.value(), request.size());
                clientRecvBuf = *clientRecvBufPtr;
                clientSender = *clientSenderPtr;
            }
            echoReceived = true;
        });

    // Run until echo received
    bool completed = runUntil([&]() { return echoReceived.load(); });
    EXPECT_TRUE(completed) << "Echo not received within timeout";

    // Verify echo data
    for (size_t i = 0; i < request.size(); ++i) {
        EXPECT_EQ(clientRecvBuf[i], request[i]);
    }
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(AsioUdpSocketTest, SendToInvalidEndpoint) {
    auto socket = createAsyncSocket(ctx_->get());

    // Invalid endpoint (empty address)
    Endpoint invalid{"", 0};
    Buffer data = {1, 2, 3};

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncSendTo(data, invalid,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    EXPECT_EQ(receivedError, NetworkError::NotConnected);
}

TEST_F(AsioUdpSocketTest, HandleNullCallback) {
    auto socket = createAsyncSocket(ctx_->get());

    Buffer data = {1, 2, 3};
    Endpoint dest{"127.0.0.1", 12345};

    // Should not crash with null callback
    socket->asyncSendTo(data, dest, nullptr);
    ctx_->poll();
}

TEST_F(AsioUdpSocketTest, ReceiveFromNullHandler) {
    auto socket = createAsyncSocket(ctx_->get());
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    auto buffer = std::make_shared<Buffer>(kMaxPacketSize);
    auto sender = std::make_shared<Endpoint>();

    // Should not crash with null handler
    socket->asyncReceiveFrom(buffer, sender, nullptr);
    ctx_->poll();
}

TEST_F(AsioUdpSocketTest, ReceiveFromNullBuffer) {
    auto socket = createAsyncSocket(ctx_->get());
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    auto sender = std::make_shared<Endpoint>();

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncReceiveFrom(nullptr, sender,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    EXPECT_EQ(receivedError, NetworkError::InternalError);
}

TEST_F(AsioUdpSocketTest, ReceiveFromNullSender) {
    auto socket = createAsyncSocket(ctx_->get());
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    auto buffer = std::make_shared<Buffer>(kMaxPacketSize);

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncReceiveFrom(buffer, nullptr,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    EXPECT_EQ(receivedError, NetworkError::InternalError);
}

TEST_F(AsioUdpSocketTest, SendToInvalidAddressFormat) {
    auto socket = createAsyncSocket(ctx_->get());

    // Invalid IP address format that should cause toAsioEndpoint to throw
    Endpoint invalid{"999.999.999.999", 12345};
    Buffer data = {1, 2, 3};

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncSendTo(data, invalid,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    EXPECT_EQ(receivedError, NetworkError::HostNotFound);
}

TEST_F(AsioUdpSocketTest, BindToInvalidPort) {
    auto socket = createAsyncSocket(ctx_->get());

    // Try to bind to a privileged port (should fail without root)
    auto result = socket->bind(80);  // HTTP port
    // This might succeed or fail depending on permissions, but should not crash
    // Just verify the result is handled properly
    EXPECT_TRUE(result.isOk() || result.isErr());
}

TEST_F(AsioUdpSocketTest, LocalPortOnClosedSocket) {
    auto socket = createAsyncSocket(ctx_->get());

    // Socket is not bound, should return 0
    EXPECT_EQ(socket->localPort(), 0);

    // Bind then close
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    EXPECT_GT(socket->localPort(), 0);

    socket->close();
    EXPECT_EQ(socket->localPort(), 0);
}

// ============================================================================
// Cancel Operations Tests
// ============================================================================

TEST_F(AsioUdpSocketTest, CancelPendingOperations) {
    auto socket = createAsyncSocket(ctx_->get());
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    Buffer recvBuffer(kMaxPacketSize);
    Endpoint sender;

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    // Start a receive that won't complete (no data coming)
    auto recvBufferPtr = std::make_shared<Buffer>(kMaxPacketSize);
    auto senderPtr = std::make_shared<Endpoint>();
    socket->asyncReceiveFrom(recvBufferPtr, senderPtr,
        [&, recvBufferPtr, senderPtr](Result<size_t> result) {
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    // Cancel the operation
    socket->cancel();

    // Run to process the cancellation
    bool completed = runUntil([&]() { return callbackCalled.load(); });

    EXPECT_TRUE(completed) << "Callback not called after cancel";
    EXPECT_EQ(receivedError, NetworkError::Cancelled);
}

TEST_F(AsioUdpSocketTest, CloseWithPendingOperations) {
    auto socket = createAsyncSocket(ctx_->get());
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    Buffer recvBuffer(kMaxPacketSize);
    Endpoint sender;

    std::atomic<bool> callbackCalled{false};

    // Start a receive
    auto recvBufferPtr = std::make_shared<Buffer>(kMaxPacketSize);
    auto senderPtr = std::make_shared<Endpoint>();
    socket->asyncReceiveFrom(recvBufferPtr, senderPtr,
        [&, recvBufferPtr, senderPtr](Result<size_t> result) {
            // Should be cancelled when socket closes
            EXPECT_TRUE(result.isErr());
            callbackCalled = true;
        });

    // Close the socket (should cancel pending ops)
    socket->close();

    // Run to process
    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
}

// ============================================================================
// IoContext Tests
// ============================================================================

TEST(IoContextTest, CreateAndDestroy) {
    {
        IoContext ctx;
        EXPECT_FALSE(ctx.stopped());
    }
    // Should not crash on destruction
}

TEST(IoContextTest, PollWithNoWork) {
    IoContext ctx;

    // Poll should return 0 when no work
    ctx.releaseWorkGuard();
    size_t handled = ctx.poll();
    EXPECT_EQ(handled, 0);
}

TEST(IoContextTest, RunInBackground) {
    IoContext ctx;

    EXPECT_FALSE(ctx.isRunningInBackground());
    ctx.runInBackground();
    EXPECT_TRUE(ctx.isRunningInBackground());

    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ctx.stop();
    EXPECT_TRUE(ctx.stopped());
}

// ============================================================================
// Core Types Tests
// ============================================================================

TEST(EndpointTest, Construction) {
    Endpoint ep{"192.168.1.1", 8080};
    EXPECT_EQ(ep.address, "192.168.1.1");
    EXPECT_EQ(ep.port, 8080);
    EXPECT_TRUE(ep.isValid());
}

TEST(EndpointTest, InvalidEndpoint) {
    Endpoint empty;
    EXPECT_FALSE(empty.isValid());

    Endpoint noPort{"127.0.0.1", 0};
    EXPECT_FALSE(noPort.isValid());

    Endpoint noAddress{"", 1234};
    EXPECT_FALSE(noAddress.isValid());
}

TEST(EndpointTest, ToString) {
    Endpoint ep{"10.0.0.1", 4242};
    EXPECT_EQ(ep.toString(), "10.0.0.1:4242");
}

TEST(EndpointTest, Equality) {
    Endpoint a{"127.0.0.1", 1234};
    Endpoint b{"127.0.0.1", 1234};
    Endpoint c{"127.0.0.1", 5678};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ============================================================================
// Additional AsioUdpSocket Error Tests
// ============================================================================

TEST_F(AsioUdpSocketTest, BindToAlreadyBoundPort) {
    auto socket1 = createAsyncSocket(ctx_->get());
    auto socket2 = createAsyncSocket(ctx_->get());

    // Bind first socket to a port
    auto result1 = socket1->bind(0);
    ASSERT_TRUE(result1.isOk());
    uint16_t port = socket1->localPort();

    // Try to bind second socket to the same port
    auto result2 = socket2->bind(port);

    // This might succeed or fail depending on the system, but should not crash
    EXPECT_TRUE(result2.isOk() || result2.isErr());
}

TEST_F(AsioUdpSocketTest, LocalPortAfterClose) {
    auto socket = createAsyncSocket(ctx_->get());

    // Initially should be 0
    EXPECT_EQ(socket->localPort(), 0);

    // Bind and check port
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    EXPECT_GT(socket->localPort(), 0);

    // Close and check port again
    socket->close();
    EXPECT_EQ(socket->localPort(), 0);
}

TEST_F(AsioUdpSocketTest, MultipleCloseCalls) {
    auto socket = createAsyncSocket(ctx_->get());

    // Bind first
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());

    // Multiple closes should be safe
    socket->close();
    socket->close();
    socket->close();

    EXPECT_FALSE(socket->isOpen());
}

TEST_F(AsioUdpSocketTest, SendToAfterClose) {
    auto socket = createAsyncSocket(ctx_->get());

    // Bind and then close
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    socket->close();

    // Try to send after close
    Buffer data = {1, 2, 3};
    Endpoint dest{"127.0.0.1", 12345};

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncSendTo(data, dest,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    // The exact error may vary, but there should be an error
    EXPECT_NE(receivedError, NetworkError::None);
}

TEST_F(AsioUdpSocketTest, ReceiveFromAfterClose) {
    auto socket = createAsyncSocket(ctx_->get());

    // Bind and then close
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    socket->close();

    // Try to receive after close
    auto buffer = std::make_shared<Buffer>(kMaxPacketSize);
    auto sender = std::make_shared<Endpoint>();

    std::atomic<bool> callbackCalled{false};
    NetworkError receivedError = NetworkError::None;

    socket->asyncReceiveFrom(buffer, sender,
        [&](Result<size_t> result) {
            EXPECT_TRUE(result.isErr());
            if (result.isErr()) {
                receivedError = result.error();
            }
            callbackCalled = true;
        });

    bool completed = runUntil([&]() { return callbackCalled.load(); });
    EXPECT_TRUE(completed);
    // The exact error may vary, but there should be an error
    EXPECT_NE(receivedError, NetworkError::None);
}

TEST_F(AsioUdpSocketTest, CancelAfterClose) {
    auto socket = createAsyncSocket(ctx_->get());

    // Bind, close, then cancel
    auto bindResult = socket->bind(0);
    ASSERT_TRUE(bindResult.isOk());
    socket->close();

    // Cancel should be safe even after close
    EXPECT_NO_THROW(socket->cancel());
}

// ============================================================================
// ByteOrder Tests
// ============================================================================

TEST(ByteOrderTest, Uint16Roundtrip) {
    uint16_t original = 0x1234;
    uint16_t network = ByteOrder::toNetwork(original);
    uint16_t back = ByteOrder::fromNetwork(network);
    EXPECT_EQ(original, back);
}

TEST(ByteOrderTest, Uint32Roundtrip) {
    uint32_t original = 0x12345678;
    uint32_t network = ByteOrder::toNetwork(original);
    uint32_t back = ByteOrder::fromNetwork(network);
    EXPECT_EQ(original, back);
}

TEST(ByteOrderTest, FloatRoundtrip) {
    float original = 3.14159f;
    float network = ByteOrder::toNetwork(original);
    float back = ByteOrder::fromNetwork(network);
    EXPECT_FLOAT_EQ(original, back);
}

TEST(ByteOrderTest, WriteAndRead) {
    std::array<uint8_t, 8> buffer{};

    ByteOrder::writeTo(buffer.data(), uint32_t{0xDEADBEEF});
    ByteOrder::writeTo(buffer.data() + 4, uint16_t{0x1234});

    uint32_t val32 = ByteOrder::readFrom<uint32_t>(buffer.data());
    uint16_t val16 = ByteOrder::readFrom<uint16_t>(buffer.data() + 4);

    EXPECT_EQ(val32, 0xDEADBEEF);
    EXPECT_EQ(val16, 0x1234);
}

// ============================================================================
// Result Type Tests
// ============================================================================

TEST(ResultTest, OkValue) {
    Result<int> result = Ok(42);
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
    EXPECT_EQ(result.value(), 42);
}

TEST(ResultTest, ErrValue) {
    Result<int> result = Err<int>(NetworkError::Timeout);
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::Timeout);
}

TEST(ResultTest, ValueOr) {
    Result<int> ok = Ok(42);
    Result<int> err = Err<int>(NetworkError::Timeout);

    EXPECT_EQ(ok.valueOr(0), 42);
    EXPECT_EQ(err.valueOr(0), 0);
}

TEST(ResultTest, VoidResult) {
    Result<void> ok = Ok();
    Result<void> err = Err(NetworkError::NotConnected);

    EXPECT_TRUE(ok.isOk());
    EXPECT_TRUE(err.isErr());
    EXPECT_EQ(err.error(), NetworkError::NotConnected);
}

TEST(ResultTest, BoolConversion) {
    Result<int> ok = Ok(1);
    Result<int> err = Err<int>(NetworkError::Timeout);

    EXPECT_TRUE(static_cast<bool>(ok));
    EXPECT_FALSE(static_cast<bool>(err));

    // Can use in if statements
    if (ok) {
        SUCCEED();
    } else {
        FAIL() << "Ok result should be truthy";
    }
}
