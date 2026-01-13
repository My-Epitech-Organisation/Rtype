/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameState - Serializable game state for save/load
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_GAMESTATE_RTYPEGAMESTATE_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_GAMESTATE_RTYPEGAMESTATE_HPP_

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace rtype::game::config {

/**
 * @brief Save file format version for compatibility checking
 */
constexpr uint32_t SAVE_FORMAT_VERSION = 1;

/**
 * @brief Magic number for save file validation
 */
constexpr uint32_t SAVE_MAGIC_NUMBER = 0x52545950;  // "RTYP"

/**
 * @brief Power-up type enumeration
 */
enum class PowerUpType : uint8_t {
    None = 0,
    SpeedBoost,
    Shield,
    DoubleDamage,
    RapidFire,
    ExtraLife,
    Bomb,
    ForcePod
};

/**
 * @brief Player state for save/load
 */
struct PlayerState {
    uint32_t playerId = 0;
    float positionX = 0.0F;
    float positionY = 0.0F;
    float rotation = 0.0F;
    int32_t health = 100;
    int32_t maxHealth = 100;
    int32_t lives = 3;
    uint32_t score = 0;
    PowerUpType activePowerUp = PowerUpType::None;
    float powerUpTimeRemaining = 0.0F;
    uint32_t weaponLevel = 1;

    bool operator==(const PlayerState&) const = default;
};

/**
 * @brief Enemy state for save/load (minimal for checkpoint)
 */
struct EnemyState {
    uint32_t enemyId = 0;
    uint8_t enemyType = 0;
    float positionX = 0.0F;
    float positionY = 0.0F;
    int32_t health = 0;

    bool operator==(const EnemyState&) const = default;
};

/**
 * @brief Checkpoint data
 */
struct CheckpointData {
    uint32_t checkpointId = 0;
    uint32_t waveNumber = 1;
    float waveProgress = 0.0F;  // 0.0 to 1.0

    bool operator==(const CheckpointData&) const = default;
};

/**
 * @brief Game progression data
 */
struct ProgressionData {
    uint32_t currentLevel = 1;
    uint32_t currentWave = 1;
    uint32_t totalWaves = 10;
    uint32_t enemiesDefeated = 0;
    uint32_t totalScore = 0;
    float playTimeSeconds = 0.0F;
    CheckpointData lastCheckpoint;

    bool operator==(const ProgressionData&) const = default;
};

/**
 * @brief Difficulty settings snapshot
 */
struct DifficultySnapshot {
    std::string difficultyLevel = "normal";
    float enemyHealthMultiplier = 1.0F;
    float enemySpeedMultiplier = 1.0F;
    float playerDamageMultiplier = 1.0F;
    uint32_t startingLives = 3;

    bool operator==(const DifficultySnapshot&) const = default;
};

/**
 * @brief Save file header for version checking
 */
struct SaveHeader {
    uint32_t magic = SAVE_MAGIC_NUMBER;
    uint32_t version = SAVE_FORMAT_VERSION;
    uint64_t timestamp = 0;  // Unix timestamp
    uint32_t checksum = 0;   // Simple checksum for corruption detection
    uint32_t dataSize = 0;   // Size of data following header

    bool operator==(const SaveHeader&) const = default;
};

/**
 * @class RTypeGameState
 * @brief Complete game state for save/load operations
 *
 * Contains all data needed to restore a game session:
 * - Player states (position, health, score, power-ups)
 * - Progression (level, wave, checkpoint)
 * - Difficulty settings at time of save
 *
 * Example usage:
 * @code
 * RTypeGameState state;
 * state.players.push_back(PlayerState{
 *     .playerId = 1,
 *     .positionX = 100.0F,
 *     .health = 80,
 *     .score = 5000
 * });
 * state.progression.currentLevel = 3;
 * state.progression.currentWave = 5;
 * @endcode
 */
class RTypeGameState {
   public:
    /// @brief Save file header
    SaveHeader header;

    /// @brief Player states (supports multiplayer)
    std::vector<PlayerState> players;

    /// @brief Enemy states (for checkpoint restoration)
    std::vector<EnemyState> enemies;

    /// @brief Game progression data
    ProgressionData progression;

    /// @brief Difficulty settings at save time
    DifficultySnapshot difficulty;

    /// @brief Save slot name/description
    std::string saveName;

    /**
     * @brief Create a new game state with default values
     * @return Default game state
     */
    [[nodiscard]] static RTypeGameState createNew();

    /**
     * @brief Update header timestamp to current time
     */
    void updateTimestamp();

    /**
     * @brief Calculate checksum of the state data
     * @return Checksum value
     */
    [[nodiscard]] uint32_t calculateChecksum() const;

    /**
     * @brief Validate the game state
     * @return true if state is valid
     */
    [[nodiscard]] bool isValid() const;

    bool operator==(const RTypeGameState&) const = default;
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_GAMESTATE_RTYPEGAMESTATE_HPP_
