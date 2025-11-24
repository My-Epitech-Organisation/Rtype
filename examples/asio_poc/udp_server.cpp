/*
** EPITECH PROJECT, 2025
** Rtype - ASIO PoC
** File description:
** UDP Server example using ASIO standalone
*/

#include <asio.hpp>
#include <iostream>
#include <memory>
#include <string>

class UdpServer {
public:
    UdpServer(asio::io_context& io_context, unsigned short port)
        : socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
        std::cout << "UDP Server listening on port " << port << std::endl;
        startReceive();
    }

private:
    void startReceive() {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                handleReceive(ec, bytes_recvd);
            });
    }

    void handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            std::string received(recv_buffer_.data(), bytes_transferred);
            std::cout << "Received: '" << received << "' from "
                      << remote_endpoint_.address().to_string()
                      << ":" << remote_endpoint_.port() << std::endl;

            // Echo back
            std::string response = "Echo: " + received;
            socket_.async_send_to(
                asio::buffer(response), remote_endpoint_,
                [this, response](std::error_code ec, std::size_t bytes_sent) {
                    if (!ec) {
                        std::cout << "Sent response: '" << response
                                  << "' (" << bytes_sent << " bytes)" << std::endl;
                    } else {
                        std::cerr << "Send error: " << ec.message() << std::endl;
                    }
                    startReceive();
                });
        } else {
            std::cerr << "Receive error: " << error.message() << std::endl;
            startReceive();
        }
    }

    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;
};

int main(int argc, char* argv[]) {
    try {
        unsigned short port = 4242;

        if (argc > 1) {
            port = static_cast<unsigned short>(std::atoi(argv[1]));
        }

        std::cout << "=== ASIO Standalone UDP Server PoC ===" << std::endl;
        std::cout << "Starting server on port " << port << "..." << std::endl;

        asio::io_context io_context;
        UdpServer server(io_context, port);

        std::cout << "Server running. Press Ctrl+C to stop." << std::endl;
        io_context.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
