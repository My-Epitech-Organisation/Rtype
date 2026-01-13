/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.cpp
*/

#include "ClientApp.hpp"

#include <memory>
#include <utility>

namespace {
rtype::client::NetworkClient::Config createNetworkConfig() {
    rtype::client::NetworkClient::Config cfg;
    cfg.connectionConfig.reliabilityConfig.retransmitTimeout =
        std::chrono::milliseconds(1000);
    cfg.connectionConfig.reliabilityConfig.maxRetries = 15;
    return cfg;
}
}  // namespace

ClientApp::ClientApp(const Config& config)
    : _config(config),
      _registry(std::make_shared<ECS::Registry>()),
      _networkClient(std::make_shared<rtype::client::NetworkClient>(
          createNetworkConfig())),
      _networkSystem(std::make_shared<rtype::client::ClientNetworkSystem>(
          _registry, _networkClient)),
      _graphic(std::make_unique<Graphic>(_registry, _networkClient,
                                         _networkSystem)) {}

void ClientApp::run() { _graphic->loop(); }
