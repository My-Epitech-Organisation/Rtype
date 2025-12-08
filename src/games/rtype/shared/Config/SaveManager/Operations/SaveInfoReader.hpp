/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SaveInfoReader - Save info reading operations
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_SAVEINFOREADER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_SAVEINFOREADER_HPP_

#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include "../../GameState/RTypeGameState.hpp"
#include "../Serialization/BinarySerializer.hpp"
#include "../Types/SaveTypes.hpp"

namespace rtype::game::config {

/**
 * @class SaveInfoReader
 * @brief Reads save file information without loading full state
 */
class SaveInfoReader {
   public:
    /**
     * @brief Read save info from a file
     * @param filepath Path to the save file
     * @return SaveInfo structure (isValid indicates success)
     */
    [[nodiscard]] static SaveInfo readSaveInfo(
        const std::filesystem::path& filepath);

   private:
    static void readProgressionInfo(
        std::shared_ptr<const std::vector<uint8_t>> data,
        std::shared_ptr<size_t> offset, SaveInfo& info);
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_SAVEINFOREADER_HPP_
