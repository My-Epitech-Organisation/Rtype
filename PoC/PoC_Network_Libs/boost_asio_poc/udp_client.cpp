/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Boost.Asio UDP Client PoC
*/

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <cstdlib>

using boost::asio::ip::udp;

class UdpClient {
public:
    UdpClient(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : socket_(io_context, udp::endpoint(udp::v4(), 0)) {

        udp::resolver resolver(io_context);
        endpoint_ = *resolver.resolve(udp::v4(), host, port).begin();

        std::cout << "Connected to " << endpoint_.address().to_string()
                  << ":" << endpoint_.port() << std::endl;
    }

    void send_message(const std::string& message) {
        std::cout << "Sending: \"" << message << "\"" << std::endl;

        socket_.async_send_to(
            boost::asio::buffer(message), endpoint_,
            [this, message](boost::system::error_code ec, std::size_t /*bytes_sent*/) {
                if (!ec) {
                    start_receive();
                } else {
                    std::cerr << "Send error: " << ec.message() << std::endl;
                }
            });
    }

private:
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                handle_receive(ec, bytes_recvd);
            });
    }

    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            std::string response(recv_buffer_.data(), bytes_transferred);
            std::cout << "Received: \"" << response << "\"" << std::endl;
        } else {
            std::cerr << "Receive error: " << error.message() << std::endl;
        }
    }

    udp::socket socket_;
    udp::endpoint endpoint_;
    std::array<char, 1024> recv_buffer_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;
        UdpClient client(io_context, argv[1], argv[2]);

        std::cout << "Boost.Asio UDP Client PoC" << std::endl;

        // Send test messages
        std::vector<std::string> test_messages = {
            "Hello from Boost.Asio!",
            "Test message 2",
            "Benchmark test"
        };

        for (const auto& msg : test_messages) {
            client.send_message(msg);
            io_context.run_one();
            io_context.restart();
        }

        std::cout << "All messages sent successfully!" << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
