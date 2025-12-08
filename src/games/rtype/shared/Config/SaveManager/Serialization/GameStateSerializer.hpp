/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameStateSerializer - Serialization logic for RTypeGameState
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../GameState/RTypeGameState.hpp"
#include "../Types/SaveTypes.hpp"
#include "BinarySerializer.hpp"

namespace rtype::game::config {

/**
 * @class GameStateSerializer
 * @brief Handles serialization and deserialization of RTypeGameState
 */
class GameStateSerializer {
   public:
    /**
     * @brief Serialize game state to binary data
     * @param state Game state to serialize
     * @return Binary data
     */
    [[nodiscard]] static std::vector<uint8_t> serialize(
        const RTypeGameState& state);

    /**
     * @brief Deserialize binary data to game state
     * @param data Binary data
     * @return Pair of optional state and optional error message
     */
    [[nodiscard]] static std::pair<std::optional<RTypeGameState>,
                                   std::optional<std::string>>
    deserialize(const std::vector<uint8_t>& data);

   private:
    static void serializePlayers(
        std::shared_ptr<std::vector<uint8_t>> data,
        const std::vector<PlayerState>& players);

    static void serializeEnemies(std::shared_ptr<std::vector<uint8_t>> data,
                                 const std::vector<EnemyState>& enemies);

    static void serializeProgression(
        std::shared_ptr<std::vector<uint8_t>> data,
        const ProgressionData& progression);

    static void serializeDifficulty(
        std::shared_ptr<std::vector<uint8_t>> data,
        const DifficultySnapshot& difficulty);

    static void deserializePlayers(
        std::shared_ptr<const std::vector<uint8_t>> data,
        std::shared_ptr<size_t> offset,
        std::shared_ptr<std::vector<PlayerState>> players);

    static void deserializeEnemies(
        std::shared_ptr<const std::vector<uint8_t>> data,
        std::shared_ptr<size_t> offset,
        std::shared_ptr<std::vector<EnemyState>> enemies);

    static void deserializeProgression(
        std::shared_ptr<const std::vector<uint8_t>> data,
        std::shared_ptr<size_t> offset,
        std::shared_ptr<ProgressionData> progression);

    static void deserializeDifficulty(
        std::shared_ptr<const std::vector<uint8_t>> data,
        std::shared_ptr<size_t> offset,
        std::shared_ptr<DifficultySnapshot> difficulty);
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_
