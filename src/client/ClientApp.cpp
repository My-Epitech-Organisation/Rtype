/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.cpp
*/

#include "ClientApp.hpp"

#include <memory>
#include <utility>

ClientApp::ClientApp(const Config& config)
    : _config(config),
      _registry(std::make_shared<ECS::Registry>()),
      _networkClient(std::make_shared<rtype::client::NetworkClient>()),
      _networkSystem(std::make_shared<rtype::client::ClientNetworkSystem>(
          _registry, _networkClient)),
      _graphic(std::make_unique<Graphic>(_registry, _networkClient,
                                         _networkSystem)) {}

void ClientApp::run() { _graphic->loop(); }
