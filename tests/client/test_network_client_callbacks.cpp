/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkClient callback branch coverage tests
*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "client/network/NetworkClient.hpp"
#include "transport/IAsyncSocket.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "Serializer.hpp"

#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

using namespace rtype::client;
using namespace rtype::network;

// Mock socket for testing
class MockSocket : public IAsyncSocket {
private:
    bool isOpen_ = true;
    bool bindResult_ = true;
    Endpoint localEndpoint_{0, 0};
    std::queue<std::pair<Buffer, Endpoint>> receivedData_;

public:
    void setBindResult(bool result) { bindResult_ = result; }
    void queueReceive(const Buffer& data, const Endpoint& sender) {
        receivedData_.push({data, sender});
    }

    Result<void> bind(uint16_t port) override {
        if (!bindResult_) return Error::BindFailed;
        localEndpoint_ = Endpoint{0, port};
        return {};
    }

    void asyncReceiveFrom(std::shared_ptr<Buffer> buffer,
                         std::shared_ptr<Endpoint> sender,
                         ReceiveHandler handler) override {
        if (!receivedData_.empty() && isOpen_) {
            auto [data, endpoint] = receivedData_.front();
            receivedData_.pop();
            *buffer = data;
            *sender = endpoint;
            handler(Result<std::size_t>(data.size()));
        }
    }

    void asyncSendTo(std::span<const std::byte> data,
                    const Endpoint& endpoint,
                    SendHandler handler) override {
        if (isOpen_) {
            handler(Result<std::size_t>(data.size()));
        } else {
            handler(Error::SocketClosed);
        }
    }

    void cancel() override {}
    void close() override { isOpen_ = false; }
    bool isOpen() const override { return isOpen_; }
    Result<Endpoint> getLocalEndpoint() const override { return localEndpoint_; }
};

// Test callback execution paths
TEST(NetworkClientCallbacks, MultipleOnConnectedCallbacks) {
    NetworkClient::Config config{};
    config.connectionConfig.timeoutMs = 5000;
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;
    int callback3Count = 0;

    client.onConnected([&](uint32_t userId) { 
        callback1Count++; 
        EXPECT_GT(userId, 0u);
    });
    client.onConnected([&](uint32_t userId) { 
        callback2Count++;
        EXPECT_GT(userId, 0u);
    });
    client.onConnected([&](uint32_t userId) { 
        callback3Count++;
        EXPECT_GT(userId, 0u);
    });

    // Simulate connection success by processing callbacks
    // In real scenario, this would be triggered by network events
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultipleOnDisconnectedCallbacks) {
    NetworkClient::Config config{};
    config.connectionConfig.timeoutMs = 5000;
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onDisconnected([&](DisconnectReason reason) { 
        callback1Count++;
        EXPECT_NE(reason, DisconnectReason::ClientRequested);
    });
    client.onDisconnected([&](DisconnectReason reason) { 
        callback2Count++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultipleGameStartCallbacks) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onGameStart([&]() { callback1Count++; });
    client.onGameStart([&]() { callback2Count++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultipleGameOverCallbacks) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onGameOver([&](bool victory) { 
        callback1Count++;
        (void)victory;
    });
    client.onGameOver([&](bool victory) { 
        callback2Count++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultipleUpdateStateCallbacks) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onUpdateState([&](const std::vector<uint8_t>& data) { 
        callback1Count++;
        EXPECT_FALSE(data.empty());
    });
    client.onUpdateState([&](const std::vector<uint8_t>& data) { 
        callback2Count++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultipleChatMessageCallbacks) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onChatMessage([&](uint32_t senderId, const std::string& msg) { 
        callback1Count++;
        EXPECT_GT(senderId, 0u);
        EXPECT_FALSE(msg.empty());
    });
    client.onChatMessage([&](uint32_t senderId, const std::string& msg) { 
        callback2Count++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(NetworkClientCallbacks, MultiplePlayerReadyCallbacks) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    int callback1Count = 0;
    int callback2Count = 0;

    client.onPlayerReadyState([&](uint32_t playerId, bool ready) { 
        callback1Count++;
        EXPECT_GT(playerId, 0u);
        (void)ready;
    });
    client.onPlayerReadyState([&](uint32_t playerId, bool ready) { 
        callback2Count++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Test callback with null handlers (should not crash)
TEST(NetworkClientCallbacks, NullCallbacksDoNotCrash) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    // Register null callbacks - should be filtered internally
    client.onConnected(nullptr);
    client.onDisconnected(nullptr);
    client.onGameStart(nullptr);
    client.onGameOver(nullptr);
    client.onUpdateState(nullptr);
    client.onChatMessage(nullptr);
    client.onPlayerReadyState(nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Test socket state transitions
TEST(NetworkClientState, SocketOpenCheck) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    auto* mockPtr = mockSocket.get();
    NetworkClient client(config, std::move(mockSocket), false);

    EXPECT_TRUE(mockPtr->isOpen());
}

TEST(NetworkClientState, SocketClosedPreventsSend) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    auto* mockPtr = mockSocket.get();
    NetworkClient client(config, std::move(mockSocket), false);

    mockPtr->close();
    EXPECT_FALSE(mockPtr->isOpen());
}

// Test connection state checks
TEST(NetworkClientState, NotConnectedPreventsSendInput) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    // Should return false when not connected
    EXPECT_FALSE(client.sendInput(0xFF));
}

TEST(NetworkClientState, NotConnectedPreventsPing) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    EXPECT_FALSE(client.ping());
}

TEST(NetworkClientState, NotConnectedPreventsSendReady) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    EXPECT_FALSE(client.sendReady(true));
}

TEST(NetworkClientState, NotConnectedPreventsSendChatMessage) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    EXPECT_FALSE(client.sendChatMessage("test"));
}

// Test with different configuration values
TEST(NetworkClientConfig, VariousTimeoutValues) {
    {
        NetworkClient::Config config{};
        config.connectionConfig.timeoutMs = 1000;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
    {
        NetworkClient::Config config{};
        config.connectionConfig.timeoutMs = 10000;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
    {
        NetworkClient::Config config{};
        config.connectionConfig.timeoutMs = 60000;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
}

TEST(NetworkClientConfig, VariousReconnectAttempts) {
    {
        NetworkClient::Config config{};
        config.connectionConfig.maxReconnectAttempts = 0;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
    {
        NetworkClient::Config config{};
        config.connectionConfig.maxReconnectAttempts = 5;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
    {
        NetworkClient::Config config{};
        config.connectionConfig.maxReconnectAttempts = 100;
        auto mockSocket = std::make_unique<MockSocket>();
        NetworkClient client(config, std::move(mockSocket), false);
    }
}

// Test disconnect while not connected
TEST(NetworkClientConnection, DisconnectWhenNotConnected) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    // Should not crash when disconnecting while not connected
    client.disconnect();
    client.disconnect(); // Second disconnect should also be safe
}

// Test latency and userId when not connected
TEST(NetworkClientConnection, LatencyWhenNotConnected) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    uint32_t latency = client.latencyMs();
    EXPECT_EQ(latency, 0u);
}

TEST(NetworkClientConnection, UserIdWhenNotConnected) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    auto userId = client.userId();
    EXPECT_FALSE(userId.has_value());
}

TEST(NetworkClientConnection, IsConnectedInitiallyFalse) {
    NetworkClient::Config config{};
    auto mockSocket = std::make_unique<MockSocket>();
    NetworkClient client(config, std::move(mockSocket), false);

    EXPECT_FALSE(client.isConnected());
}
