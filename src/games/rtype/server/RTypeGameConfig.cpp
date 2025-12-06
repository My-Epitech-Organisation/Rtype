/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameConfig - Implementation
*/

#include "RTypeGameConfig.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Logger.hpp"

namespace rtype::games::rtype::server {

namespace fs = std::filesystem;
namespace config = ::rtype::game::config;
namespace shared = ::rtype::games::rtype::shared;
namespace server_ns = ::rtype::server;

RTypeGameConfig::RTypeGameConfig() = default;

bool RTypeGameConfig::initialize(const std::string& configDir) {
    _configDir = configDir;
    _initialized = false;

    LOG_INFO("[RTypeConfig] Initializing from: " << configDir);

    const auto serverConfigFile = _configDir / "config.toml";
    if (!loadServerConfig(serverConfigFile)) {
        return false;
    }

    const auto gameConfigDir = _configDir.parent_path() / "game";
    if (fs::exists(gameConfigDir)) {
        if (!loadEntityConfigs(gameConfigDir)) {
            LOG_WARNING("[RTypeConfig] Some entity configs failed to load");
        }
    } else {
        LOG_WARNING("[RTypeConfig] Game config directory not found: "
                    << gameConfigDir.string());
    }

    if (!initializeSaveManager()) {
        return false;
    }

    if (!validateConfiguration()) {
        return false;
    }

    _initialized = true;
    LOG_INFO("[RTypeConfig] Initialization complete");
    return true;
}

bool RTypeGameConfig::reloadConfiguration() {
    LOG_INFO("[RTypeConfig] Reloading configuration...");

    auto backupConfig = _config;

    const auto serverConfigFile = _configDir / "config.toml";
    if (!loadServerConfig(serverConfigFile)) {
        LOG_ERROR("[RTypeConfig] Reload failed, keeping previous config");
        _config = backupConfig;
        return false;
    }

    const auto gameConfigDir = _configDir.parent_path() / "game";
    if (fs::exists(gameConfigDir)) {
        shared::EntityConfigRegistry::getInstance().clear();
        static_cast<void>(loadEntityConfigs(gameConfigDir));
    }

    if (!validateConfiguration()) {
        LOG_ERROR("[RTypeConfig] Validation failed, keeping previous config");
        _config = backupConfig;
        return false;
    }

    LOG_INFO("[RTypeConfig] Configuration reloaded successfully");
    return true;
}

server_ns::GenericServerSettings RTypeGameConfig::getServerSettings()
    const noexcept {
    return {.port = _config.server.port,
            .maxPlayers = _config.server.maxPlayers,
            .tickRate = _config.server.tickrate,
            .mapName = _config.server.mapName};
}

server_ns::GenericGameplaySettings RTypeGameConfig::getGameplaySettings()
    const noexcept {
    return {.difficulty = _config.gameplay.difficulty,
            .startingLives = _config.gameplay.startingLives,
            .playerSpeed = _config.gameplay.playerSpeed,
            .enemySpeedMultiplier = _config.gameplay.enemySpeedMultiplier};
}

std::string RTypeGameConfig::getSavesPath() const noexcept {
    return _config.paths.savesPath;
}

bool RTypeGameConfig::loadServerConfig(const fs::path& configFile) {
    if (!fs::exists(configFile)) {
        _lastError = "Configuration file not found: " + configFile.string();
        LOG_ERROR("[RTypeConfig] " << _lastError);
        return false;
    }

    auto loadedConfig = _configParser.loadFromFile(configFile);
    if (!loadedConfig) {
        _lastError = "Failed to parse configuration file";
        LOG_ERROR("[RTypeConfig] " << _lastError);
        for (const auto& error : _configParser.getLastErrors()) {
            LOG_ERROR("[RTypeConfig]   - " << error.message);
        }
        return false;
    }

    _config = std::move(*loadedConfig);
    LOG_INFO("[RTypeConfig] Loaded config - Port: "
             << _config.server.port
             << ", Max Players: " << _config.server.maxPlayers
             << ", Tick Rate: " << _config.server.tickrate << " Hz");

    return true;
}

bool RTypeGameConfig::loadEntityConfigs(const fs::path& gameConfigDir) {
    auto& registry = shared::EntityConfigRegistry::getInstance();

    bool success = registry.loadFromDirectory(gameConfigDir.string());

    LOG_INFO("[RTypeConfig] Entity configs - Enemies: "
             << registry.getAllEnemies().size()
             << ", Players: " << registry.getAllPlayers().size()
             << ", Projectiles: " << registry.getAllProjectiles().size());

    return success;
}

bool RTypeGameConfig::initializeSaveManager() {
    fs::path savesPath = _config.paths.savesPath;

    if (savesPath.is_relative()) {
        savesPath = _configDir.parent_path().parent_path() / savesPath;
    }

    if (!fs::exists(savesPath)) {
        try {
            fs::create_directories(savesPath);
            LOG_INFO("[RTypeConfig] Created saves directory: "
                     << savesPath.string());
        } catch (const std::exception& e) {
            _lastError =
                "Failed to create saves directory: " + std::string(e.what());
            LOG_ERROR("[RTypeConfig] " << _lastError);
            return false;
        }
    }

    _saveManager = std::make_unique<config::RTypeSaveManager>(savesPath);
    LOG_INFO(
        "[RTypeConfig] Save manager initialized at: " << savesPath.string());

    return true;
}

bool RTypeGameConfig::validateConfiguration() {
    auto errors = _config.validate();

    if (!errors.empty()) {
        _lastError = "Configuration validation failed";
        LOG_ERROR("[RTypeConfig] " << _lastError);
        for (const auto& error : errors) {
            LOG_ERROR("[RTypeConfig]   - " << error.toString());
        }
        return false;
    }

    if (_config.server.port == 0 || _config.server.maxPlayers == 0) {
        _lastError = "Invalid server configuration";
        LOG_ERROR("[RTypeConfig] " << _lastError);
        return false;
    }

    return true;
}

bool RTypeGameConfig::saveGame(const std::string& slotName,
                               const std::vector<uint8_t>& gameStateData) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return false;
    }

    config::RTypeGameState state;
    // TODO(Sam): Implement proper deserialization from gameStateData
    (void)gameStateData;

    auto result = _saveManager->save(state, slotName);
    if (result != config::SaveResult::Success) {
        _lastError = _saveManager->getLastError();
        return false;
    }
    return true;
}

std::vector<uint8_t> RTypeGameConfig::loadGame(const std::string& slotName) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return {};
    }

    auto state = _saveManager->load(slotName);
    if (!state) {
        _lastError = _saveManager->getLastError();
        return {};
    }

    // TODO(Sam): Implement proper serialization
    std::vector<uint8_t> data;
    return data;
}

std::vector<server_ns::GenericSaveInfo> RTypeGameConfig::listSaves() const {
    std::vector<server_ns::GenericSaveInfo> result;

    if (!_saveManager) {
        return result;
    }

    for (const auto& save : _saveManager->listSaves()) {
        result.push_back({.filename = save.filename,
                          .saveName = save.saveName,
                          .timestamp = save.timestamp,
                          .currentLevel = save.currentLevel,
                          .totalScore = save.totalScore,
                          .isValid = save.isValid});
    }

    return result;
}

bool RTypeGameConfig::saveExists(const std::string& slotName) const {
    return _saveManager && _saveManager->saveExists(slotName);
}

bool RTypeGameConfig::deleteSave(const std::string& slotName) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return false;
    }
    return _saveManager->deleteSave(slotName);
}

config::SaveResult RTypeGameConfig::saveRTypeState(
    const config::RTypeGameState& state, const std::string& slotName) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return config::SaveResult::IOError;
    }

    auto result = _saveManager->save(state, slotName);
    if (result != config::SaveResult::Success) {
        _lastError = _saveManager->getLastError();
    }
    return result;
}

std::optional<config::RTypeGameState> RTypeGameConfig::loadRTypeState(
    const std::string& slotName) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return std::nullopt;
    }

    auto state = _saveManager->load(slotName);
    if (!state) {
        _lastError = _saveManager->getLastError();
    }
    return state;
}

bool RTypeGameConfig::createAutosave(const config::RTypeGameState& state) {
    if (!_saveManager) {
        _lastError = "Save manager not initialized";
        return false;
    }

    for (uint32_t i = MAX_AUTOSAVES; i > 1; --i) {
        std::string oldSlot =
            std::string(AUTOSAVE_SLOT) + "_" + std::to_string(i - 1);
        std::string newSlot =
            std::string(AUTOSAVE_SLOT) + "_" + std::to_string(i);

        if (_saveManager->saveExists(oldSlot)) {
            auto oldState = _saveManager->load(oldSlot);
            if (oldState) {
                _saveManager->save(*oldState, newSlot);
            }
        }
    }

    std::string slot = std::string(AUTOSAVE_SLOT) + "_1";
    auto result = _saveManager->save(state, slot);

    if (result != config::SaveResult::Success) {
        _lastError = _saveManager->getLastError();
        return false;
    }

    LOG_DEBUG("[RTypeConfig] Created autosave: " << slot);
    return true;
}

std::unique_ptr<server_ns::IGameConfig> createRTypeGameConfig() {
    return std::make_unique<RTypeGameConfig>();
}

}  // namespace rtype::games::rtype::server
