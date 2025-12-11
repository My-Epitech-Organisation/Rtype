/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameConfig - R-Type specific game configuration implementation
*/

#ifndef SRC_GAMES_RTYPE_SERVER_RTYPEGAMECONFIG_HPP_
#define SRC_GAMES_RTYPE_SERVER_RTYPEGAMECONFIG_HPP_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "games/rtype/shared/Config/GameConfig/RTypeGameConfig.hpp"
#include "games/rtype/shared/Config/GameState/RTypeGameState.hpp"
#include "games/rtype/shared/Config/Parser/RTypeConfigParser.hpp"
#include "games/rtype/shared/Config/SaveManager/RTypeSaveManager.hpp"
#include "server/shared/IGameConfig.hpp"

namespace rtype::games::rtype::server {

namespace server_ns = ::rtype::server;

/**
 * @class RTypeGameConfig
 * @brief R-Type specific implementation of IGameConfig
 *
 * Provides R-Type specific configuration loading, entity management,
 * and save system integration for the generic server.
 */
class RTypeGameConfig : public server_ns::IGameConfig {
   public:
    RTypeGameConfig();
    ~RTypeGameConfig() override = default;

    RTypeGameConfig(const RTypeGameConfig&) = delete;
    RTypeGameConfig& operator=(const RTypeGameConfig&) = delete;
    RTypeGameConfig(RTypeGameConfig&&) = default;
    RTypeGameConfig& operator=(RTypeGameConfig&&) = default;

    [[nodiscard]] bool initialize(const std::string& configDir) override;
    [[nodiscard]] bool reloadConfiguration() override;
    [[nodiscard]] bool isInitialized() const noexcept override {
        return _initialized;
    }

    [[nodiscard]] server_ns::GenericServerSettings getServerSettings()
        const noexcept override;
    [[nodiscard]] server_ns::GenericGameplaySettings getGameplaySettings()
        const noexcept override;
    [[nodiscard]] std::string getSavesPath() const noexcept override;

    [[nodiscard]] bool saveGame(
        const std::string& slotName,
        const std::vector<uint8_t>& gameStateData) override;
    [[nodiscard]] std::vector<uint8_t> loadGame(
        const std::string& slotName) override;
    [[nodiscard]] std::vector<server_ns::GenericSaveInfo> listSaves()
        const override;
    [[nodiscard]] bool saveExists(const std::string& slotName) const override;
    bool deleteSave(const std::string& slotName) override;

    [[nodiscard]] const std::string& getLastError() const noexcept override {
        return _lastError;
    }

    [[nodiscard]] std::string getGameId() const noexcept override {
        return "rtype";
    }

    /**
     * @brief Get R-Type specific configuration
     * @return Reference to R-Type config
     */
    [[nodiscard]] const game::config::RTypeGameConfig& getRTypeConfig()
        const noexcept {
        return _config;
    }

    /**
     * @brief Get entity configuration registry
     * @return Reference to entity registry
     */
    [[nodiscard]] shared::EntityConfigRegistry& getEntityRegistry() noexcept {
        return shared::EntityConfigRegistry::getInstance();
    }

    /**
     * @brief Save R-Type game state directly
     * @param state Game state to save
     * @param slotName Save slot name
     * @return Save result
     */
    [[nodiscard]] game::config::SaveResult saveRTypeState(
        const game::config::RTypeGameState& state, const std::string& slotName);

    /**
     * @brief Load R-Type game state directly
     * @param slotName Save slot name
     * @return Loaded state or nullopt
     */
    [[nodiscard]] std::optional<game::config::RTypeGameState> loadRTypeState(
        const std::string& slotName);

    /**
     * @brief Create autosave with rotation
     * @param state Current game state
     * @return true if successful
     */
    bool createAutosave(const game::config::RTypeGameState& state);

   private:
    [[nodiscard]] bool loadServerConfig(
        const std::filesystem::path& configFile);
    [[nodiscard]] bool loadEntityConfigs(
        const std::filesystem::path& gameConfigDir);
    [[nodiscard]] bool initializeSaveManager();
    [[nodiscard]] bool validateConfiguration();

    bool _initialized = false;
    std::filesystem::path _configDir;
    game::config::RTypeGameConfig _config;
    game::config::RTypeConfigParser _configParser;
    std::unique_ptr<game::config::RTypeSaveManager> _saveManager;
    std::string _lastError;

    static constexpr const std::string AUTOSAVE_SLOT = "autosave";
    static constexpr uint32_t MAX_AUTOSAVES = 3;
};

/**
 * @brief Factory function to create R-Type game config
 * @return Unique pointer to IGameConfig implementation
 */
[[nodiscard]] std::unique_ptr<server_ns::IGameConfig> createRTypeGameConfig();

}  // namespace rtype::games::rtype::server

#endif  // SRC_GAMES_RTYPE_SERVER_RTYPEGAMECONFIG_HPP_
