/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Client main entry point
*/

#include <exception>
#include <iostream>

#include "ClientApp.hpp"

auto main(int argc, char** argv) -> int {
    (void)argc;
    (void)argv;

    try {
        ClientApp::Config config;
        // TODO(Noa): Parse command line arguments for server host/port
        // config.defaultServerHost = "127.0.0.1";
        // config.defaultServerPort = 4242;

        ClientApp client(config);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Program exited with an error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
