/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** LobbyManager - Implementation
*/

#include "server/lobby/LobbyManager.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Logger/Logger.hpp"
#include "server/lobby/LobbyDiscoveryServer.hpp"

namespace rtype::server {

LobbyManager::LobbyManager(const Config& config)
    : config_(config), rng_(std::random_device()()), charDist_(0, 35) {
    if (config_.instanceCount == 0) {
        throw std::invalid_argument("Instance count must be at least 1");
    }

    if (config_.instanceCount > config_.maxInstances) {
        throw std::invalid_argument("Instance count exceeds maximum allowed");
    }

    rtype::Logger::instance().info(std::format(
        "Creating lobby manager with {} instances", config_.instanceCount));
}

LobbyManager::~LobbyManager() { stop(); }

std::string LobbyManager::generateLobbyCode() {
    const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string code;
    code.reserve(6);

    do {
        code.clear();
        for (int i = 0; i < 6; ++i) {
            code += chars[charDist_(rng_)];
        }
    } while (lobbyByCode_.find(code) != lobbyByCode_.end());

    return code;
}

bool LobbyManager::start() {
    if (running_) {
        rtype::Logger::instance().warning(
            std::format("Manager already running"));
        return false;
    }

    rtype::Logger::instance().info(
        std::format("Starting {} lobby instances...", config_.instanceCount));

    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    for (std::uint32_t i = 0; i < config_.instanceCount; ++i) {
        std::string code = generateLobbyCode();
        std::uint16_t port = config_.basePort + 1 + i;

        Lobby::Config lobbyConfig;
        lobbyConfig.port = port;
        lobbyConfig.maxPlayers = config_.maxPlayers;
        lobbyConfig.tickRate = config_.tickRate;
        lobbyConfig.configPath = config_.configPath;
        lobbyConfig.emptyTimeout = config_.emptyTimeout;

        auto lobby = std::make_unique<Lobby>(code, lobbyConfig);

        if (!lobby->start()) {
            rtype::Logger::instance().error(
                std::format("Failed to start lobby {} on port {}", code, port));
            for (auto& l : lobbies_) {
                l->stop();
            }
            lobbies_.clear();
            lobbyByCode_.clear();
            return false;
        }

        lobbyByCode_[code] = lobby.get();
        lobbies_.push_back(std::move(lobby));

        rtype::Logger::instance().info(
            std::format("Lobby {} started on port {}", code, port));
    }

    running_ = true;

    cleanupThread_ =
        std::make_unique<std::thread>(&LobbyManager::cleanupLoop, this);

    discoveryThread_ =
        std::make_unique<std::thread>(&LobbyManager::discoveryLoop, this);

    rtype::Logger::instance().info(
        std::format("All lobbies started successfully"));
    return true;
}

void LobbyManager::stop() {
    if (!running_) {
        return;
    }

    rtype::Logger::instance().info(std::format("Stopping lobby manager..."));

    running_ = false;

    if (cleanupThread_ && cleanupThread_->joinable()) {
        cleanupThread_->join();
    }

    if (discoveryThread_ && discoveryThread_->joinable()) {
        discoveryThread_->join();
    }

    std::lock_guard<std::mutex> lock(lobbiesMutex_);
    for (auto& lobby : lobbies_) {
        lobby->stop();
    }
    lobbies_.clear();
    lobbyByCode_.clear();

    rtype::Logger::instance().info(std::format("Lobby manager stopped"));
}

void LobbyManager::run() {
    if (!running_) {
        rtype::Logger::instance().error(
            std::format("Cannot run: manager not started"));
        return;
    }

    rtype::Logger::instance().info(
        std::format("Lobby manager running. Press Ctrl+C to stop."));

    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    rtype::Logger::instance().info(
        std::format("Lobby manager shutting down..."));
}

std::vector<LobbyManager::LobbyInfo> LobbyManager::getActiveLobbyList() const {
    std::vector<LobbyInfo> result;

    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    for (const auto& lobby : lobbies_) {
        if (!lobby->isRunning()) {
            continue;
        }

        bool isActive = !lobby->isEmpty() ||
                        lobby->getTimeSinceEmpty() < std::chrono::seconds(30);

        LobbyInfo info;
        info.code = lobby->getCode();
        info.port = lobby->getPort();
        info.playerCount = lobby->getPlayerCount();
        info.maxPlayers = lobby->getMaxPlayers();
        info.isActive = isActive;

        result.push_back(info);
    }

    return result;
}

bool LobbyManager::verifyLobbyCode(const std::string& code,
                                   std::uint16_t port) const {
    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    auto it = lobbyByCode_.find(code);
    if (it == lobbyByCode_.end()) {
        return false;
    }

    return it->second->getPort() == port;
}

Lobby* LobbyManager::findLobbyByCode(const std::string& code) const {
    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    auto it = lobbyByCode_.find(code);
    if (it != lobbyByCode_.end()) {
        return it->second;
    }

    return nullptr;
}

void LobbyManager::cleanupLoop() {
    rtype::Logger::instance().info(std::format("Cleanup thread started"));

    auto lastCleanup = std::chrono::steady_clock::now();
    const auto cleanupInterval = std::chrono::seconds(30);

    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (!running_) break;

        auto now = std::chrono::steady_clock::now();
        if (now - lastCleanup < cleanupInterval) {
            continue;
        }

        lastCleanup = now;

        std::lock_guard<std::mutex> lock(lobbiesMutex_);

        for (auto& lobby : lobbies_) {
            if (lobby->isEmpty()) {
                auto timeSinceEmpty = lobby->getTimeSinceEmpty();

                if (timeSinceEmpty >= config_.emptyTimeout) {
                    rtype::Logger::instance().info(std::format(
                        "Lobby {} has been empty for {} seconds (timeout: {}s)",
                        lobby->getCode(), timeSinceEmpty.count(),
                        config_.emptyTimeout.count()));

                    // For now, just log. Could implement auto-restart or other
                    // logic here
                }
            } else {
                lobby->updateActivity();
            }
        }
    }

    rtype::Logger::instance().info(std::format("Cleanup thread stopped"));
}

void LobbyManager::discoveryLoop() {
    rtype::Logger::instance().info(std::format(
        "Discovery service thread started on port {}", config_.basePort));

    LobbyDiscoveryServer discoveryServer(config_.basePort, *this);
    discoveryServer_ = &discoveryServer;

    if (!discoveryServer.start()) {
        rtype::Logger::instance().error(
            std::format("Failed to start discovery server"));
        return;
    }

    while (running_) {
        discoveryServer.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    discoveryServer.stop();
    discoveryServer_ = nullptr;

    rtype::Logger::instance().info(
        std::format("Discovery service thread stopped"));
}

}  // namespace rtype::server
