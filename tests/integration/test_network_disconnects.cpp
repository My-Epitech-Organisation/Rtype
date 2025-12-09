/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_network_disconnects - Integration tests for server/client disconnect handling
*/

#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "../../src/client/network/NetworkClient.hpp"
#include "../../src/server/network/NetworkServer.hpp"

using namespace rtype;
using namespace rtype::client;
using namespace rtype::server;

// Test that server receives onClientConnected and onClientDisconnected events
// and that a client that stops polling (simulates a crash/inactivity) is
// eventually marked as timed out by the server.
TEST(NetworkIntegration, ServerDetectsClientTimeoutAndGracefulDisconnect) {
    // Short timeout for test speed
    NetworkServer::Config serverConfig;
    serverConfig.clientTimeout = std::chrono::milliseconds(250);

    NetworkServer server(serverConfig);
    std::atomic_bool clientConnected{false};
    std::atomic_bool clientDisconnected{false};
    std::uint32_t connectedUserId = 0;

    server.onClientConnected([&](std::uint32_t userId) {
        clientConnected = true;
        connectedUserId = userId;
    });

    server.onClientDisconnected([&](std::uint32_t userId, rtype::network::DisconnectReason reason) {
        (void)userId; // userId parameter may match
        (void)reason; // reason used below for assert
        clientDisconnected = true;
    });

    ASSERT_TRUE(server.start(0));
    const auto port = server.port();

    // Create a client with default config
    NetworkClient::Config clientConfig;
    NetworkClient client(clientConfig);
    std::atomic_bool connectedCallbackCalled{false};

    client.onConnected([&](std::uint32_t myId) {
        (void)myId;
        connectedCallbackCalled = true;
    });

    ASSERT_TRUE(client.connect("127.0.0.1", port));

    // Poll client & server for a short time to establish connection
    auto start = std::chrono::steady_clock::now();
    while (!connectedCallbackCalled && std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
        client.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ASSERT_TRUE(clientConnected);
    ASSERT_TRUE(connectedCallbackCalled);

    // Case 1: Graceful disconnect
    std::promise<void> gracefulDone;
    client.onDisconnected([&](NetworkClient::DisconnectReason reason) {
        (void)reason;
        // We'll let the server detect the graceful disconnect via callback
        gracefulDone.set_value();
    });

    client.disconnect();

    // Poll until server reports disconnection or timeout
    start = std::chrono::steady_clock::now();
    while (!clientDisconnected && std::chrono::steady_clock::now() - start < std::chrono::seconds(3)) {
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ASSERT_TRUE(clientDisconnected);

    // Reset flags and test a non-graceful disconnect (timeout)
    clientConnected = false;
    clientDisconnected = false;
    connectedCallbackCalled = false;

    // Start a new client and connect again
    NetworkClient client2(clientConfig);
    client2.onConnected([&](std::uint32_t myId) { (void)myId; connectedCallbackCalled = true; });
    ASSERT_TRUE(client2.connect("127.0.0.1", port));

    start = std::chrono::steady_clock::now();
    while (!connectedCallbackCalled && std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
        client2.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(connectedCallbackCalled);
    ASSERT_TRUE(clientConnected);

    // Simulate client crash by stopping client polling (client remains alive but inactive)
    // Server should mark it as timed out after serverConfig.clientTimeout
    const auto waitTimeout = serverConfig.clientTimeout + std::chrono::milliseconds(200);
    const auto deadline = std::chrono::steady_clock::now() + waitTimeout;
    while (!clientDisconnected && std::chrono::steady_clock::now() < deadline) {
        // Only poll server - simulate silent client
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ASSERT_TRUE(clientDisconnected);

    server.stop();
}
