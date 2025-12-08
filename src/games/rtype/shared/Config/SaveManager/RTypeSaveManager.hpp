/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeSaveManager - Binary save/load system for R-Type
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_RTYPESAVEMANAGER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_RTYPESAVEMANAGER_HPP_

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../GameState/RTypeGameState.hpp"
#include "Types/SaveTypes.hpp"

namespace rtype::game::config {

/**
 * @class RTypeSaveManager
 * @brief Manages binary save/load operations for R-Type game state
 *
 * Features:
 * - Binary serialization for compact saves
 * - Version control for save compatibility
 * - Checksum validation for corruption detection
 * - Safe file writing (temp file + rename)
 * - Support for multiple save slots
 *
 * Binary Format:
 * - Header: magic (4B) + version (4B) + timestamp (8B) + checksum (4B) +
 * dataSize (4B)
 * - Data: serialized game state
 *
 * Example usage:
 * @code
 * RTypeSaveManager manager("saves/");
 *
 * // Save game
 * RTypeGameState state = RTypeGameState::createNew();
 * state.progression.currentLevel = 3;
 * auto result = manager.save(state, "slot1");
 *
 * // Load game
 * auto loaded = manager.load("slot1");
 * if (loaded) {
 *     // Use loaded state
 * }
 * @endcode
 */
class RTypeSaveManager {
   public:
    using VersionMigrationCallback =
        std::function<bool(RTypeGameState&, uint32_t oldVersion)>;

    /**
     * @brief Construct with save directory path
     * @param saveDirectory Directory for save files
     * @param fileExtension Extension for save files (default: ".rtsave")
     */
    explicit RTypeSaveManager(std::filesystem::path saveDirectory = "saves",
                              std::string fileExtension = ".rtsave");

    ~RTypeSaveManager() = default;

    RTypeSaveManager(const RTypeSaveManager&) = delete;
    RTypeSaveManager& operator=(const RTypeSaveManager&) = delete;
    RTypeSaveManager(RTypeSaveManager&&) = default;
    RTypeSaveManager& operator=(RTypeSaveManager&&) = default;

    /**
     * @brief Save game state to file
     * @param state Game state to save
     * @param slotName Name of save slot
     * @return Result of save operation
     */
    SaveResult save(const RTypeGameState& state, const std::string& slotName);

    /**
     * @brief Load game state from file
     * @param slotName Name of save slot
     * @return Loaded game state, or nullopt on failure
     */
    [[nodiscard]] std::optional<RTypeGameState> load(
        const std::string& slotName);

    /**
     * @brief Delete a save file
     * @param slotName Name of save slot
     * @return true if deleted successfully
     */
    bool deleteSave(const std::string& slotName);

    /**
     * @brief Check if a save file exists
     * @param slotName Name of save slot
     * @return true if save exists
     */
    [[nodiscard]] bool saveExists(const std::string& slotName) const;

    /**
     * @brief List all available saves
     * @return Vector of save information
     */
    [[nodiscard]] std::vector<SaveInfo> listSaves() const;

    /**
     * @brief Get information about a specific save
     * @param slotName Name of save slot
     * @return Save information, or nullopt if not found
     */
    [[nodiscard]] std::optional<SaveInfo> getSaveInfo(
        const std::string& slotName) const;

    /**
     * @brief Get the last operation result
     * @return Last result
     */
    [[nodiscard]] SaveResult getLastResult() const noexcept {
        return _lastResult;
    }

    /**
     * @brief Get the last error message
     * @return Error message
     */
    [[nodiscard]] const std::string& getLastError() const noexcept {
        return _lastError;
    }

    /**
     * @brief Set callback for version migration
     * @param callback Function to migrate old save formats
     */
    void setMigrationCallback(VersionMigrationCallback callback) {
        _migrationCallback = std::move(callback);
    }

    /**
     * @brief Create a backup of a save file
     * @param slotName Name of save slot
     * @param backupName Name for backup (default: adds .bak)
     * @return true if backup created successfully
     */
    bool createBackup(const std::string& slotName,
                      const std::string& backupName = "");

    /**
     * @brief Restore from backup
     * @param slotName Name of save slot
     * @param backupName Name of backup to restore
     * @return true if restored successfully
     */
    bool restoreBackup(const std::string& slotName,
                       const std::string& backupName = "");

   private:
    [[nodiscard]] std::filesystem::path getFilePath(
        const std::string& slotName) const;

    std::filesystem::path _saveDirectory;
    std::string _fileExtension;
    SaveResult _lastResult = SaveResult::Success;
    std::string _lastError;
    VersionMigrationCallback _migrationCallback;
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_RTYPESAVEMANAGER_HPP_
