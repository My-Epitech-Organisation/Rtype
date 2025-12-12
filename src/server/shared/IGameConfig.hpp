/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** IGameConfig - Abstract interface for game configuration
*/

#ifndef SRC_SERVER_SHARED_IGAMECONFIG_HPP_
#define SRC_SERVER_SHARED_IGAMECONFIG_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rtype::server {

/**
 * @brief Generic save information structure
 */
struct GenericSaveInfo {
    std::string filename;
    std::string saveName;
    uint64_t timestamp = 0;
    uint32_t currentLevel = 1;
    uint32_t totalScore = 0;
    bool isValid = false;
};

/**
 * @brief Generic server settings
 */
struct GenericServerSettings {
    uint16_t port = 4000;
    uint32_t maxPlayers = 8;
    uint32_t tickRate = 60;
    std::string mapName = "default";
};

/**
 * @brief Generic gameplay settings
 */
struct GenericGameplaySettings {
    std::string difficulty = "normal";
    uint32_t startingLives = 3;
    float playerSpeed = 200.0F;
    float enemySpeedMultiplier = 1.0F;
};

/**
 * @interface IGameConfig
 * @brief Abstract interface for game-specific configuration management
 *
 * This interface allows the server to work with any game's configuration
 * without knowing the specifics. Each game implements this interface
 * to provide its own configuration loading, entity management, and save system.
 *
 * Example usage:
 * @code
 * // In game-specific code:
 * class RTypeGameConfig : public IGameConfig { ... };
 *
 * // In server:
 * std::unique_ptr<IGameConfig> gameConfig = createGameConfig("rtype");
 * if (gameConfig->initialize("config/server")) {
 *     auto settings = gameConfig->getServerSettings();
 * }
 * @endcode
 */
class IGameConfig {
   public:
    virtual ~IGameConfig() = default;

    // ==================== Lifecycle ====================

    /**
     * @brief Initialize configuration from directory
     * @param configDir Path to configuration directory
     * @return true if initialization succeeded
     */
    [[nodiscard]] virtual bool initialize(const std::string& configDir) = 0;

    /**
     * @brief Reload configuration (hot-reload support)
     * @return true if reload succeeded
     */
    [[nodiscard]] virtual bool reloadConfiguration() = 0;

    /**
     * @brief Check if configuration is loaded and valid
     * @return true if ready to use
     */
    [[nodiscard]] virtual bool isInitialized() const noexcept = 0;

    // ==================== Configuration Access ====================

    /**
     * @brief Get server settings
     * @return Server settings structure
     */
    [[nodiscard]] virtual GenericServerSettings getServerSettings()
        const noexcept = 0;

    /**
     * @brief Get gameplay settings
     * @return Gameplay settings structure
     */
    [[nodiscard]] virtual GenericGameplaySettings getGameplaySettings()
        const noexcept = 0;

    /**
     * @brief Get saves directory path
     * @return Path to saves directory
     */
    [[nodiscard]] virtual std::string getSavesPath() const noexcept = 0;

    // ==================== Save Management ====================

    /**
     * @brief Save current game state
     * @param slotName Name of the save slot
     * @param gameStateData Opaque game state data (game-specific format)
     * @return true if save succeeded
     */
    [[nodiscard]] virtual bool saveGame(
        const std::string& slotName,
        const std::vector<uint8_t>& gameStateData) = 0;

    /**
     * @brief Load game state from slot
     * @param slotName Name of the save slot
     * @return Game state data, or empty on failure
     */
    [[nodiscard]] virtual std::vector<uint8_t> loadGame(
        const std::string& slotName) = 0;

    /**
     * @brief Get list of available saves
     * @return Vector of save info structures
     */
    [[nodiscard]] virtual std::vector<GenericSaveInfo> listSaves() const = 0;

    /**
     * @brief Check if a save exists
     * @param slotName Name of the save slot
     * @return true if save exists
     */
    [[nodiscard]] virtual bool saveExists(
        const std::string& slotName) const = 0;

    /**
     * @brief Delete a save
     * @param slotName Name of the save slot
     * @return true if deletion succeeded
     */
    virtual bool deleteSave(const std::string& slotName) = 0;

    /**
     * @brief Get last error message
     * @return Last error message
     */
    [[nodiscard]] virtual const std::string& getLastError() const noexcept = 0;

    /**
     * @brief Get the game identifier
     * @return Game identifier string (e.g., "rtype", "spaceinvaders")
     */
    [[nodiscard]] virtual std::string getGameId() const noexcept = 0;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_IGAMECONFIG_HPP_
