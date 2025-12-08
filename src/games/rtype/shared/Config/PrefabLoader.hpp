/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PrefabLoader - Loads prefabs from EntityConfig into PrefabManager
*/

#pragma once

#include <string>

#include <rtype/ecs.hpp>

#include "../Components.hpp"
#include "../Components/Tags.hpp"
#include "EntityConfig/EntityConfig.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class PrefabLoader
 * @brief Loads prefabs from EntityConfig TOML files into PrefabManager
 *
 * This bridges the configuration system with the ECS prefab system.
 * Call registerAllPrefabs() after loading configs to populate PrefabManager.
 *
 * Usage:
 * @code
 *   EntityConfigRegistry::getInstance().loadFromDirectory("config/game");
 *   PrefabLoader::registerAllPrefabs(prefabManager);
 *
 *   auto enemy = prefabManager.instantiate("enemy_basic", [](auto& r, auto e) {
 *       r.getComponent<TransformComponent>(e) = {800.0f, 300.0f, 0.0f};
 *   });
 * @endcode
 */
class PrefabLoader {
   public:
    /**
     * @brief Register all entity prefabs from loaded configs
     * @param prefabs PrefabManager to populate
     */
    static void registerAllPrefabs(ECS::PrefabManager& prefabs);

    /**
     * @brief Register enemy prefabs from config
     */
    static void registerEnemyPrefabs(ECS::PrefabManager& prefabs);

    /**
     * @brief Register projectile prefabs from config
     */
    static void registerProjectilePrefabs(ECS::PrefabManager& prefabs);

    /**
     * @brief Register player prefabs from config
     */
    static void registerPlayerPrefabs(ECS::PrefabManager& prefabs);

    /**
     * @brief Register power-up prefabs from config
     */
    static void registerPowerUpPrefabs(ECS::PrefabManager& prefabs);

    /**
     * @brief Helper to get config values (for systems that need stats)
     */
    [[nodiscard]] static float getPlayerSpeed(const std::string& playerId);

    [[nodiscard]] static float getPlayerFireRate(const std::string& playerId);

    [[nodiscard]] static int32_t getEnemyScore(const std::string& enemyId);

    [[nodiscard]] static int32_t getProjectileDamage(
        const std::string& projectileId);
};

}  // namespace rtype::games::rtype::shared
