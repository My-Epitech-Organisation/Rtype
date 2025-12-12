/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngineFactory - Implementation
*/

#include "GameEngineFactory.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

namespace rtype::engine {

std::unordered_map<std::string, GameEngineFactory::Creator>&
GameEngineFactory::getRegistry() {
    static std::unordered_map<std::string, Creator> registry;
    return registry;
}

std::mutex& GameEngineFactory::getMutex() {
    static std::mutex mutex;
    return mutex;
}

std::string& GameEngineFactory::getDefaultGameId() {
    static std::string defaultGameId;
    return defaultGameId;
}

bool GameEngineFactory::registerGame(const std::string& gameId, Creator creator) {
    if (gameId.empty() || !creator) {
        return false;
    }

    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    if (registry.find(gameId) != registry.end()) {
        return false;
    }

    registry[gameId] = std::move(creator);

    if (getDefaultGameId().empty()) {
        getDefaultGameId() = gameId;
    }

    return true;
}

bool GameEngineFactory::unregisterGame(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();
    auto it = registry.find(gameId);

    if (it == registry.end()) {
        return false;
    }

    registry.erase(it);
    if (getDefaultGameId() == gameId) {
        getDefaultGameId().clear();
        if (!registry.empty()) {
            getDefaultGameId() = registry.begin()->first;
        }
    }

    return true;
}

std::unique_ptr<IGameEngine> GameEngineFactory::create(
    const std::string& gameId,
    std::shared_ptr<ECS::Registry> registry) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& factoryRegistry = getRegistry();

    auto it = factoryRegistry.find(gameId);
    if (it == factoryRegistry.end()) {
        return nullptr;
    }

    return it->second(std::move(registry));
}

bool GameEngineFactory::isRegistered(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();
    return registry.find(gameId) != registry.end();
}

std::vector<std::string> GameEngineFactory::getRegisteredGames() {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    std::vector<std::string> games;
    games.reserve(registry.size());

    for (const auto& [gameId, _] : registry) {
        games.push_back(gameId);
    }

    std::sort(games.begin(), games.end());
    return games;
}

std::size_t GameEngineFactory::getRegisteredCount() {
    std::lock_guard<std::mutex> lock(getMutex());
    return getRegistry().size();
}

void GameEngineFactory::clearRegistry() {
    std::lock_guard<std::mutex> lock(getMutex());
    getRegistry().clear();
    getDefaultGameId().clear();
}

std::string GameEngineFactory::getDefaultGame() {
    std::lock_guard<std::mutex> lock(getMutex());
    return getDefaultGameId();
}

bool GameEngineFactory::setDefaultGame(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto& registry = getRegistry();

    if (registry.find(gameId) == registry.end()) {
        return false;
    }

    getDefaultGameId() = gameId;
    return true;
}

std::unique_ptr<IGameEngine> createGameEngine(
    std::shared_ptr<ECS::Registry> registry) {
    auto defaultGame = GameEngineFactory::getDefaultGame();

    if (!defaultGame.empty()) {
        return GameEngineFactory::create(defaultGame, std::move(registry));
    }

    auto registeredGames = GameEngineFactory::getRegisteredGames();
    if (!registeredGames.empty()) {
        return GameEngineFactory::create(registeredGames[0], std::move(registry));
    }

    return nullptr;
}

}  // namespace rtype::engine

