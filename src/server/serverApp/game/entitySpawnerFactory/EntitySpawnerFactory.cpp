/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntitySpawnerFactory - Implementation
*/

#include "EntitySpawnerFactory.hpp"

#include <algorithm>

namespace rtype::server {

std::unordered_map<std::string, EntitySpawnerFactory::Creator>&
EntitySpawnerFactory::getRegistry() {
    static std::unordered_map<std::string, Creator> registry;
    return registry;
}

std::mutex& EntitySpawnerFactory::getMutex() {
    static std::mutex mutex;
    return mutex;
}

bool EntitySpawnerFactory::registerSpawner(const std::string& gameId,
                                           Creator creator) {
    if (gameId.empty() || !creator) {
        return false;
    }

    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    if (registry.find(gameId) != registry.end()) {
        return false;
    }

    registry[gameId] = std::move(creator);
    return true;
}

bool EntitySpawnerFactory::unregisterSpawner(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    auto it = registry.find(gameId);
    if (it == registry.end()) {
        return false;
    }

    registry.erase(it);
    return true;
}

std::unique_ptr<IEntitySpawner> EntitySpawnerFactory::create(
    const std::string& gameId, std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<ServerNetworkSystem> networkSystem,
    GameEngineOpt gameEngine, GameConfigOpt gameConfig) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& factoryRegistry = getRegistry();

    auto it = factoryRegistry.find(gameId);
    if (it == factoryRegistry.end()) {
        return nullptr;
    }

    return it->second(std::move(registry), std::move(networkSystem),
                      gameEngine, gameConfig);
}

bool EntitySpawnerFactory::isRegistered(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();
    return registry.find(gameId) != registry.end();
}

std::vector<std::string> EntitySpawnerFactory::getRegisteredSpawners() {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    std::vector<std::string> spawners;
    spawners.reserve(registry.size());

    for (const auto& [gameId, _] : registry) {
        spawners.push_back(gameId);
    }

    std::sort(spawners.begin(), spawners.end());
    return spawners;
}

void EntitySpawnerFactory::clearRegistry() {
    std::lock_guard<std::mutex> lock(getMutex());
    getRegistry().clear();
}

}  // namespace rtype::server
