/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** TcpTest - TCP Echo Server to demonstrate Head-of-Line Blocking
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <cstring>

class TcpEchoServer {
private:
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    bool running = false;

public:
    explicit TcpEchoServer(int port) {
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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

        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        std::cout << "TCP Echo Server listening on port " << port << std::endl;
    }

    ~TcpEchoServer() {
        if (server_fd >= 0) {
            close(server_fd);
        }
    }

    void start() {
        running = true;
        std::cout << "Server started. Waiting for connections..." << std::endl;

        while (running) {
            int client_socket;
            if ((client_socket = accept(server_fd, (struct sockaddr *)&address, reinterpret_cast<socklen_t*>(&addrlen))) < 0) {
                if (running) {
                    perror("accept");
                }
                continue;
            }

            std::cout << "New client connected" << std::endl;
            std::thread client_thread(&TcpEchoServer::handleClient, this, client_socket);
            client_thread.detach();
        }
    }

    void stop() {
        running = false;
        close(server_fd);
        server_fd = -1;
    }

private:
    void handleClient(int client_socket) {
        char buffer[1024] = {0};
        int packet_count = 0;

        while (running) {
            ssize_t valread = read(client_socket, buffer, 1024);
            if (valread <= 0) {
                if (valread < 0) {
                    perror("read");
                }
                break;
            }

            packet_count++;
            std::string received_data(buffer, valread);

            std::string response = "Echo[" + std::to_string(packet_count) + "]: " + received_data;
            send(client_socket, response.c_str(), response.length(), 0);

            std::cout << "Processed packet " << packet_count << ": " << received_data.substr(0, 50) << "..." << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        close(client_socket);
        std::cout << "Client disconnected" << std::endl;
    }
};

class TcpEchoClient {
private:
    int sock;
    struct sockaddr_in serv_addr;

public:
    TcpEchoClient(const char* server_ip, int port) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address");
            exit(EXIT_FAILURE);
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection Failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "Connected to server" << std::endl;
    }

    ~TcpEchoClient() {
        if (sock >= 0) {
            close(sock);
        }
    }

    void sendPackets(int num_packets, bool simulate_drop = false) {
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 1; i <= num_packets; ++i) {
            auto packet_start = std::chrono::high_resolution_clock::now();

            std::string message = "Packet " + std::to_string(i) + " - " + std::string(100, 'X');

            if (simulate_drop && i == 5) {
                std::cout << "SIMULATING DROP: Skipping packet " << i << " at "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(
                                 packet_start - start_time).count() << "ms" << std::endl;
                continue;
            }

            send(sock, message.c_str(), message.length(), 0);

            char buffer[1024] = {0};
            ssize_t valread = read(sock, buffer, 1024);

            auto packet_end = std::chrono::high_resolution_clock::now();
            auto packet_latency = std::chrono::duration_cast<std::chrono::microseconds>(packet_end - packet_start);

            if (valread > 0) {
                std::cout << "Packet " << i << " - Latency: " << packet_latency.count() << "μs - "
                          << std::string(buffer, valread).substr(0, 60) << "..." << std::endl;
            } else {
                std::cout << "Packet " << i << " - TIMEOUT/ERROR after " << packet_latency.count() << "μs" << std::endl;
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
        std::cout << "Usage: " << argv[0] << " <server|client> [simulate_drop]" << std::endl;
        std::cout << "  server: Run as TCP echo server" << std::endl;
        std::cout << "  client: Run as TCP echo client (connects to localhost:8080)" << std::endl;
        std::cout << "  simulate_drop: For client mode, simulate dropping packet 5" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    bool simulate_drop = (argc > 2 && std::string(argv[2]) == "simulate_drop");

    if (mode == "server") {
        TcpEchoServer server(8080);
        server.start();
    } else if (mode == "client") {
        TcpEchoClient client("127.0.0.1", 8080);
        std::cout << "Sending 10 packets..." << std::endl;
        if (simulate_drop) {
            std::cout << "Will simulate dropping packet 5 to demonstrate head-of-line blocking" << std::endl;
        }
        client.sendPackets(10, simulate_drop);
        std::cout << "Test completed" << std::endl;
    } else {
        std::cout << "Invalid mode. Use 'server' or 'client'" << std::endl;
        return 1;
    }

    return 0;
}
