/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** UdpTest - UDP Echo Server to demonstrate real-time capabilities
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdio>

class UdpEchoServer {
private:
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    bool running = false;

public:
    explicit UdpEchoServer(int port) {
        if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "UDP Echo Server listening on port " << port << std::endl;
    }

    ~UdpEchoServer() {
        if (server_fd >= 0) {
            close(server_fd);
        }
    }

    void start() {
        running = true;
        std::cout << "Server started. Waiting for packets..." << std::endl;

        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        while (running) {
            ssize_t valread = recvfrom(server_fd, buffer, 1024, 0,
                                     (struct sockaddr *)&client_addr, &client_len);
            if (valread <= 0) {
                if (running) {
                    perror("recvfrom");
                }
                continue;
            }

            std::string received_data(buffer, valread);
            std::string response = "Echo: " + received_data;

            sendto(server_fd, response.c_str(), response.length(), 0,
                  (struct sockaddr *)&client_addr, client_len);

            std::cout << "Processed packet: " << received_data.substr(0, 50) << "..." << std::endl;
        }
    }

    void stop() {
        running = false;
        close(server_fd);
        server_fd = -1;
    }
};

class UdpEchoClient {
private:
    int sock;
    struct sockaddr_in serv_addr;
    int packet_count = 0;

public:
    UdpEchoClient(const char* server_ip, int port) {
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address");
            exit(EXIT_FAILURE);
        }

        std::cout << "UDP Client ready to send to " << server_ip << ":" << port << std::endl;
    }

    ~UdpEchoClient() {
        if (sock >= 0) {
            close(sock);
        }
    }

    void sendPackets(int num_packets, bool simulate_loss = false) {
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 1; i <= num_packets; ++i) {
            auto packet_start = std::chrono::high_resolution_clock::now();

            std::string message = "Packet " + std::to_string(i) + " - " + std::string(100, 'X');

            if (simulate_loss && i == 5) {
                std::cout << "SIMULATING LOSS: Skipping packet " << i << " at "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(
                                 packet_start - start_time).count() << "ms" << std::endl;
                continue;
            }

            sendto(sock, message.c_str(), message.length(), 0,
                  (struct sockaddr *)&serv_addr, sizeof(serv_addr));

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            char buffer[1024] = {0};
            ssize_t valread = recvfrom(sock, buffer, 1024, 0, NULL, NULL);

            auto packet_end = std::chrono::high_resolution_clock::now();
            auto packet_latency = std::chrono::duration_cast<std::chrono::microseconds>(packet_end - packet_start);

            if (valread > 0) {
                std::cout << "Packet " << i << " - Latency: " << packet_latency.count() << "μs - "
                          << std::string(buffer, valread).substr(0, 60) << "..." << std::endl;
            } else {
                std::cout << "Packet " << i << " - TIMEOUT/LOSS after " << packet_latency.count() << "μs" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "\nTotal test duration: " << total_duration.count() << "ms" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <server|client> [simulate_loss]" << std::endl;
        std::cout << "  server: Run as UDP echo server" << std::endl;
        std::cout << "  client: Run as UDP echo client (connects to localhost:8081)" << std::endl;
        std::cout << "  simulate_loss: For client mode, simulate dropping packet 5" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    bool simulate_loss = (argc > 2 && std::string(argv[2]) == "simulate_loss");

    if (mode == "server") {
        UdpEchoServer server(8081);
        server.start();
    } else if (mode == "client") {
        UdpEchoClient client("127.0.0.1", 8081);
        std::cout << "Sending 10 packets..." << std::endl;
        if (simulate_loss) {
            std::cout << "Will simulate losing packet 5 to demonstrate UDP behavior" << std::endl;
        }
        client.sendPackets(10, simulate_loss);
        std::cout << "Test completed" << std::endl;
    } else {
        std::cout << "Invalid mode. Use 'server' or 'client'" << std::endl;
        return 1;
    }

    return 0;
}
