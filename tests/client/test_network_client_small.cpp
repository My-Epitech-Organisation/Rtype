#include <gtest/gtest.h>

#include "client/network/NetworkClient.hpp"
#include "transport/IAsyncSocket.hpp"
#include "core/Error.hpp"

using namespace rtype::client;
using namespace rtype::network;

class FakeSocket : public rtype::network::IAsyncSocket {
  public:
    FakeSocket() = default;

    rtype::network::Result<void> bind(std::uint16_t /*port*/) override {
        bindCalled = true;
        return bindResult;
    }

    bool isOpen() const noexcept override { return openFlag; }

    std::uint16_t localPort() const noexcept override { return localPort_; }

    void asyncSendTo(const rtype::network::Buffer& /*data*/, const rtype::network::Endpoint& /*dest*/, rtype::network::SendCallback handler) override {
        if (handler) handler(rtype::network::Result<std::size_t>::ok(0));
        sendCalled = true;
    }

    void asyncReceiveFrom(std::shared_ptr<rtype::network::Buffer> /*buffer*/, std::shared_ptr<rtype::network::Endpoint> /*sender*/, rtype::network::ReceiveCallback /*handler*/) override {
    }

    void cancel() override { cancelCalled = true; }
    void close() override { closeCalled = true; }

    // Test helpers
    rtype::network::Result<void> bindResult = rtype::network::Ok();
    bool openFlag = false;
    std::uint16_t localPort_ = 12345;
    bool bindCalled = false;
    bool sendCalled = false;
    bool cancelCalled = false;
    bool closeCalled = false;
};

TEST(NetworkClientSmallTest, RequestLobbyListBindFailure) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    raw->openFlag = false;
    raw->bindResult = rtype::network::Err(rtype::network::NetworkError::AddressInUse);

    NetworkClient::Config cfg;
    NetworkClient client(cfg, std::move(sock), false);

    bool res = client.requestLobbyList("127.0.0.1", 4242);
    EXPECT_FALSE(res);
    EXPECT_TRUE(raw->bindCalled);
}

TEST(NetworkClientSmallTest, RequestLobbyListBindAndSend) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    raw->openFlag = false;
    raw->bindResult = Ok();

    NetworkClient::Config cfg;
    NetworkClient client(cfg, std::move(sock), false);

    bool res = client.requestLobbyList("127.0.0.1", 4242);
    EXPECT_TRUE(res);
    EXPECT_TRUE(raw->bindCalled);
    EXPECT_TRUE(raw->sendCalled);
}

TEST(NetworkClientSmallTest, SendInputWhenNotConnected) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    raw->openFlag = true; // socket is open but not connected

    NetworkClient::Config cfg;
    NetworkClient client(cfg, std::move(sock), false);

    EXPECT_FALSE(client.sendInput(0x1));
}
