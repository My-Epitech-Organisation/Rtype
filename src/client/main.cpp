
#include <iostream>

#include "ClientApp.hpp"

auto main(int argc, char** argv) -> int {
    (void)argc;
    (void)argv;
    auto registry = std::make_shared<ECS::Registry>();
    ClientApp client(registry);
    client.run();
    return 0;
}
