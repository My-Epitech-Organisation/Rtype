/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_network_packet_loss - Integration test for packet loss simulation via proxy
*/

#include <gtest/gtest.h>
#include <asio.hpp>
#include <chrono>
#include <thread>
#include <atomic>
#include <random>
#include <memory>

#include "../../src/client/network/NetworkClient.hpp"
#include "../../src/server/network/NetworkServer.hpp"

using namespace rtype;
using namespace rtype::client;
using namespace rtype::server;
using asio::ip::udp;

class UdpProxy {
public:
    UdpProxy(uint16_t proxyPort, const std::string& serverHost, uint16_t serverPort, double dropRate = 0.0)
        : ioCtx_(), socket_(ioCtx_, udp::endpoint(udp::v4(), proxyPort)),
          serverEndpoint_(asio::ip::make_address(serverHost), serverPort),
          dropRate_(dropRate), running_(false) {}

    void start() {
        running_ = true;
        socket_.non_blocking(true);  // Set non-blocking mode
        thread_ = std::thread([this]() { this->run(); });
    }

    void stop() {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }

    // Set drop rate (0.0 .. 1.0)
    void setDropRate(double r) { dropRate_ = r; }

private:
    void run() {
        std::array<uint8_t, 4096> buffer;
        udp::endpoint remote;
        udp::endpoint lastClientEndpoint;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        while (running_) {
            try {
                asio::error_code ec;
                std::size_t len = socket_.receive_from(asio::buffer(buffer), remote, 0, ec);

                if (ec == asio::error::would_block) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }
                if (ec) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                // If the packet came from the real server, forward to the last known client endpoint.
                bool fromServer = (remote.address() == serverEndpoint_.address() &&
                                   remote.port() == serverEndpoint_.port());

                udp::endpoint target;
                if (fromServer) {
                    // Forward to last client endpoint
                    target = lastClientEndpoint;
                } else {
                    // Packet from client; remember it and forward to server
                    lastClientEndpoint = remote;
                    target = serverEndpoint_;
                }

                if (target.address().is_unspecified() || target.port() == 0) {
                    // Unknown forwarding target; drop
                    continue;
                }

                // Simulate drop probability
                if (dropRate_ > 0.0 && dist(rng) < dropRate_) {
                    // drop packet
                    continue;
                }

                socket_.send_to(asio::buffer(buffer.data(), len), target);
            } catch (const std::exception& e) {
                (void)e; // ignore and continue while running
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    asio::io_context ioCtx_;
    udp::socket socket_;
    udp::endpoint serverEndpoint_;
    double dropRate_;
    std::atomic<bool> running_;
    std::thread thread_;
public:
    // Returns the actual local port this proxy is bound to (0-based ephemeral if 0 originally)
    uint16_t local_port() const {
        try {
            return socket_.local_endpoint().port();
        } catch (...) {
            return 0;
        }
    }
};

// Deterministic Drop Proxy - drops every Nth packet (both directions)
class DropNthProxy {
public:
    DropNthProxy(uint16_t proxyPort, const std::string& serverHost, uint16_t serverPort, uint32_t nth = 2)
        : ioCtx_(), socket_(ioCtx_, udp::endpoint(udp::v4(), proxyPort)), serverEndpoint_(asio::ip::make_address(serverHost), serverPort), nth_(nth), count_(0), running_(false) {}

    void start() {
        running_ = true;
        socket_.non_blocking(true);  // Set non-blocking mode
        thread_ = std::thread([this]() { this->run(); });
    }
    void stop() {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }
    uint16_t local_port() const {
        try { return socket_.local_endpoint().port(); } catch (...) { return 0; }
    }

private:
    void run() {
        std::array<uint8_t, 4096> buffer;
        udp::endpoint remote;
        udp::endpoint lastClientEndpoint;

        while (running_) {
            try {
                asio::error_code ec;
                std::size_t len = socket_.receive_from(asio::buffer(buffer), remote, 0, ec);
                
                if (ec == asio::error::would_block) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }
                if (ec) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                bool fromServer = (remote.address() == serverEndpoint_.address() && remote.port() == serverEndpoint_.port());
                udp::endpoint target;
                if (fromServer) {
                    target = lastClientEndpoint;
                } else {
                    lastClientEndpoint = remote;
                    target = serverEndpoint_;
                }

                if (target.address().is_unspecified() || target.port() == 0) {
                    continue;
                }

                count_++;
                if (nth_ > 0 && (count_ % nth_ == 0)) {
                    // drop this packet
                    continue;
                }

                socket_.send_to(asio::buffer(buffer.data(), len), target);
            } catch (const std::exception& e) {
                (void)e;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    asio::io_context ioCtx_;
    udp::socket socket_;
    udp::endpoint serverEndpoint_;
    uint32_t nth_;
    uint64_t count_;
    std::atomic<bool> running_;
    std::thread thread_;
};

// Ensure reliable packets (spawn) are delivered despite packet loss
TEST(NetworkIntegration, ReliableSpawnDeliveredDespitePacketLoss) {
    NetworkServer::Config serverConfig;
    serverConfig.clientTimeout = std::chrono::milliseconds(500);

    NetworkServer server(serverConfig);
    std::atomic_bool serverConnected{false};
    std::uint32_t userId{0};
    server.onClientConnected([&](std::uint32_t id) {
        serverConnected = true;
        userId = id;
    });

    ASSERT_TRUE(server.start(0));
    const uint16_t serverPort = server.port();

    // Start proxy that forwards between client (via proxyPort) and server (serverPort)
    // Use deterministic drop (drop every 3rd packet)
    DropNthProxy proxy(0, "127.0.0.1", serverPort, 3);
    proxy.start();

    NetworkClient::Config clientConfig;
    NetworkClient client(clientConfig);
    std::atomic_bool clientConnected{false};
    std::atomic_bool spawnReceived{false};

    client.onConnected([&](std::uint32_t id) { (void)id; clientConnected = true; });
    client.onEntitySpawn([&](EntitySpawnEvent ev) {
        (void)ev;
        spawnReceived = true;
    });

    const auto clientConnectPort = proxy.local_port();
    ASSERT_TRUE(client.connect("127.0.0.1", clientConnectPort));

    // Poll until connected
    auto start = std::chrono::steady_clock::now();
    while (!clientConnected && std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
        client.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(clientConnected);
    ASSERT_TRUE(serverConnected);

    // Ask server to spawn an entity reliably
    constexpr std::uint32_t entityId = 10001;
    server.spawnEntity(entityId, network::EntityType::Bydos, 100.0f, 200.0f);

    // Wait for client to receive spawn (with a generous timeout; retransmits will happen automatically)
    start = std::chrono::steady_clock::now();
    const auto deadline = start + std::chrono::seconds(5);
    while (!spawnReceived && std::chrono::steady_clock::now() < deadline) {
        client.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(spawnReceived);

    // Cleanup
    client.disconnect();
    proxy.stop();
    server.stop();
}
