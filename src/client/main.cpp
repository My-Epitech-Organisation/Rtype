
#include <iostream>
#include "ClientApp.hpp"

int main(int argc, char** argv) {
    auto registry = std::make_shared<ECS::Registry>();
    ClientApp client(registry);
    return 0;
}
