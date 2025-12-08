
#include <exception>
#include <iostream>

#include "ClientApp.hpp"

auto main(int argc, char** argv) -> int {
    (void)argc;
    (void)argv;
    auto registry = std::make_shared<ECS::Registry>();
    try {
        ClientApp client(registry);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Program exited with an error: " << e.what() << std::endl;
    }
    return 0;
}
