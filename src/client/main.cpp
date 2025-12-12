/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Client main entry point
*/

#include <exception>
#include <string>

#include "ClientApp.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "Logger/Macros.hpp"

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

        ControllerRumble::cleanup();
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Program exited with an error: ") +
                  std::string(e.what()));
        ControllerRumble::cleanup();
        return 1;
    }
    return 0;
}
