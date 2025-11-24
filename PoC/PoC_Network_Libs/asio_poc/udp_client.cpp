/*
** EPITECH PROJECT, 2025
** Rtype - ASIO PoC
** File description:
** UDP Client example using ASIO standalone
*/

#include <asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

class UdpClient {
public:
    UdpClient(asio::io_context& io_context, const std::string& host, unsigned short port)
        : socket_(io_context),
          resolver_(io_context),
          remote_endpoint_(*resolver_.resolve(asio::ip::udp::v4(), host, std::to_string(port)).begin()) {

        socket_.open(asio::ip::udp::v4());
        std::cout << "UDP Client connected to " << host << ":" << port << std::endl;
    }

    void send(const std::string& message) {
        socket_.send_to(asio::buffer(message), remote_endpoint_);
        std::cout << "Sent: '" << message << "'" << std::endl;
    }

    std::string receive() {
        std::array<char, 1024> recv_buffer;
        asio::ip::udp::endpoint sender_endpoint;

        size_t len = socket_.receive_from(asio::buffer(recv_buffer), sender_endpoint);

        std::string received(recv_buffer.data(), len);
        std::cout << "Received: '" << received << "' from "
                  << sender_endpoint.address().to_string()
                  << ":" << sender_endpoint.port() << std::endl;

        return received;
    }

    void asyncReceive() {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::string received(recv_buffer_.data(), bytes_recvd);
                    std::cout << "Async received: '" << received << "'" << std::endl;
                }
            });
    }

private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::resolver resolver_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;
};

int main(int argc, char* argv[]) {
    try {
        std::string host = "127.0.0.1";
        unsigned short port = 4242;

        if (argc > 1) {
            host = argv[1];
        }
        if (argc > 2) {
            port = static_cast<unsigned short>(std::atoi(argv[2]));
        }

        std::cout << "=== ASIO Standalone UDP Client PoC ===" << std::endl;
        std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;

        asio::io_context io_context;
        UdpClient client(io_context, host, port);

        // Test 1: Send a simple message
        std::cout << "\n--- Test 1: Simple Echo ---" << std::endl;
        client.send("Hello from ASIO client!");
        std::string response = client.receive();

        // Test 2: Send multiple messages
        std::cout << "\n--- Test 2: Multiple Messages ---" << std::endl;
        for (int i = 1; i <= 3; ++i) {
            std::string msg = "Message #" + std::to_string(i);
            client.send(msg);
            response = client.receive();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Test 3: Send game-like packet
        std::cout << "\n--- Test 3: Game-like Packet ---" << std::endl;
        client.send("PLAYER_INPUT:UP");
        response = client.receive();

        std::cout << "\n✅ All tests completed successfully!" << std::endl;
        std::cout << "ASIO standalone is working correctly." << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
