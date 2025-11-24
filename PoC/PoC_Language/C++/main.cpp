/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** main
*/

#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include "ECS.hpp"
#include "Network.hpp"

int main() {
    std::cout << "R-Type C++ PoC: ECS and Networking Demo" << std::endl;

    Registry registry;

    Entity player = registry.createEntity();
    registry.addComponent<TransformComponent>(player, 0.0f, 0.0f);
    registry.addComponent<VelocityComponent>(player, 1.0f, 0.5f);

    Entity enemy = registry.createEntity();
    registry.addComponent<TransformComponent>(enemy, 10.0f, 5.0f);
    registry.addComponent<VelocityComponent>(enemy, -0.5f, 0.0f);

    for (int i = 0; i < 5; ++i) {
        registry.update(1.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    UdpSocket socket;
    if (!socket.create()) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    if (!socket.bindSocket(12345)) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    std::cout << "UDP Socket bound to port 12345" << std::endl;

    if (socket.sendTo("Hello from R-Type!", "127.0.0.1", 12345)) {
        std::cout << "Sent message to self" << std::endl;
    }

    ReceiveResult result = socket.receiveFrom();
    if (!result.message.empty()) {
        std::cout << "Received: " << result.message << " from "
                  << result.senderIp << ":" << result.senderPort << std::endl;
    } else {
        std::cout << "No message received (expected in this simple demo)" << std::endl;
    }

    std::cout << "C++ PoC completed successfully!" << std::endl;
    return 0;
}
