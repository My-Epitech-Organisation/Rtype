/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** LobbyManager - Implementation
*/

#include "server/lobby/LobbyManager.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Logger/Logger.hpp"
#include "server/lobby/LobbyDiscoveryServer.hpp"

namespace fs = std::filesystem;

namespace rtype::server {

LobbyManager::LobbyManager(const Config& config)
    : config_(config),
      rng_(std::random_device()()),
      charDist_(0, 35),
      banManager_(std::make_shared<BanManager>()) {
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

        auto lobby =
            std::make_unique<Lobby>(code, lobbyConfig, this, banManager_);

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
        info.levelId = lobby->getConfig().levelId;

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

std::vector<Lobby*> LobbyManager::getAllLobbies() const {
    std::vector<Lobby*> result;
    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    for (const auto& lobby : lobbies_) {
        if (lobby && lobby->isRunning()) {
            result.push_back(lobby.get());
        }
    }

    return result;
}

std::vector<std::string> LobbyManager::getAvailableLevels() const {
    std::vector<std::string> levels;
    const fs::path levelsDir = fs::path("config") / "game" / "levels";

    if (fs::exists(levelsDir) && fs::is_directory(levelsDir)) {
        for (const auto& entry : fs::directory_iterator(levelsDir)) {
            if (entry.path().extension() == ".toml") {
                levels.push_back(entry.path().stem().string());
            }
        }
    }
    std::sort(levels.begin(), levels.end());
    return levels;
}

std::string LobbyManager::createLobby(bool isPrivate,
                                      const std::string& levelId) {
    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    if (lobbies_.size() >= config_.maxInstances) {
        rtype::Logger::instance().warning(
            std::format("Cannot create lobby: max instances ({}) reached",
                        config_.maxInstances));
        return "";
    }

    std::string code;
    if (isPrivate) {
        code = generateLobbyCode();
    } else {
        int nextNum = 1;
        while (lobbyByCode_.find(std::format("{:06}", nextNum)) !=
               lobbyByCode_.end()) {
            ++nextNum;
        }
        code = std::format("{:06}", nextNum);
    }

    std::uint16_t port =
        config_.basePort + 1 + static_cast<std::uint16_t>(lobbies_.size());

    Lobby::Config lobbyConfig;
    lobbyConfig.port = port;
    lobbyConfig.maxPlayers = config_.maxPlayers;
    lobbyConfig.tickRate = config_.tickRate;
    lobbyConfig.configPath = config_.configPath;
    lobbyConfig.emptyTimeout = config_.emptyTimeout;
    lobbyConfig.levelId = levelId;

    auto lobby = std::make_unique<Lobby>(code, lobbyConfig, this, banManager_);

    if (!lobby->start()) {
        rtype::Logger::instance().error(
            std::format("Failed to start dynamically created lobby {}", code));
        return "";
    }

    lobbyByCode_[code] = lobby.get();
    lobbies_.push_back(std::move(lobby));

    rtype::Logger::instance().info(
        std::format("Dynamically created lobby {} ({}) on port {}", code,
                    isPrivate ? "private" : "public", port));

    return code;
}

bool LobbyManager::deleteLobby(const std::string& code) {
    std::lock_guard<std::mutex> lock(lobbiesMutex_);

    auto it = lobbyByCode_.find(code);
    if (it == lobbyByCode_.end()) {
        rtype::Logger::instance().warning(
            std::format("Cannot delete lobby: code {} not found", code));
        return false;
    }

    Lobby* lobbyPtr = it->second;
    auto lobbyIt = std::find_if(lobbies_.begin(), lobbies_.end(),
                                [lobbyPtr](const std::unique_ptr<Lobby>& l) {
                                    return l.get() == lobbyPtr;
                                });

    if (lobbyIt != lobbies_.end()) {
        rtype::Logger::instance().info(std::format(
            "Deleting lobby {} on port {}", code, (*lobbyIt)->getPort()));

        (*lobbyIt)->stop();
        lobbies_.erase(lobbyIt);
    }

    lobbyByCode_.erase(it);

    rtype::Logger::instance().info(
        std::format("Lobby {} deleted successfully", code));
    return true;
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
