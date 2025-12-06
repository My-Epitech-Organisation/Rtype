/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameStateSerializer - Serialization logic for RTypeGameState
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_

#include <optional>
#include <string>
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
     * @param outError Output error message on failure
     * @return Deserialized state, or nullopt on failure
     */
    [[nodiscard]] static std::optional<RTypeGameState> deserialize(
        const std::vector<uint8_t>& data, std::string& outError);

   private:
    static void serializePlayers(std::vector<uint8_t>& data,
                                 const std::vector<PlayerState>& players);

    static void serializeEnemies(std::vector<uint8_t>& data,
                                 const std::vector<EnemyState>& enemies);

    static void serializeProgression(std::vector<uint8_t>& data,
                                     const ProgressionData& progression);

    static void serializeDifficulty(std::vector<uint8_t>& data,
                                    const DifficultySnapshot& difficulty);

    static void deserializePlayers(const std::vector<uint8_t>& data,
                                   size_t& offset,
                                   std::vector<PlayerState>& players);

    static void deserializeEnemies(const std::vector<uint8_t>& data,
                                   size_t& offset,
                                   std::vector<EnemyState>& enemies);

    static void deserializeProgression(const std::vector<uint8_t>& data,
                                       size_t& offset,
                                       ProgressionData& progression);

    static void deserializeDifficulty(const std::vector<uint8_t>& data,
                                      size_t& offset,
                                      DifficultySnapshot& difficulty);
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_GAMESTATESERIALIZER_HPP_
