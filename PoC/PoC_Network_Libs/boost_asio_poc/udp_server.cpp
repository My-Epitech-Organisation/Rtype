/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Boost.Asio UDP Server PoC
*/

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <cstdlib>

using boost::asio::ip::udp;

class UdpServer {
public:
    UdpServer(boost::asio::io_context& io_context, unsigned short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)) {
        std::cout << "Server listening on port " << port << std::endl;
        start_receive();
    }

private:
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                handle_receive(ec, bytes_recvd);
            });
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            std::string message(recv_buffer_.data(), bytes_transferred);
            std::cout << "Received: \"" << message << "\" from "
                      << remote_endpoint_.address().to_string() << ":"
                      << remote_endpoint_.port() << std::endl;

            // Echo back
            std::string response = "Echo: " + message;
            socket_.async_send_to(
                boost::asio::buffer(response), remote_endpoint_,
                [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                    start_receive();
                });
        } else {
            std::cerr << "Receive error: " << error.message() << std::endl;
            start_receive();
        }
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
            return 1;
        }

        unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));

        boost::asio::io_context io_context;
        UdpServer server(io_context, port);

        std::cout << "Boost.Asio UDP Server PoC running..." << std::endl;
        io_context.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
