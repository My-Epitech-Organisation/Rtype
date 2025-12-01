#include <iostream>

#include "ClientApp.hpp"

ClientApp::ClientApp(const std::shared_ptr<ECS::Registry> &registry)
    : _graphic(std::move(registry))
{
}
