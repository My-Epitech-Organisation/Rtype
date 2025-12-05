/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PrefabLoader - Loads prefabs from EntityConfig into PrefabManager
*/

#pragma once

#include <string>

#include "../Components.hpp"
#include "../Components/Tags.hpp"
#include "EntityConfig/EntityConfig.hpp"
#include "engine/ecs/ECS.hpp"

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
    static void registerAllPrefabs(ECS::PrefabManager& prefabs) {
        registerEnemyPrefabs(prefabs);
        registerProjectilePrefabs(prefabs);
        registerPlayerPrefabs(prefabs);
        registerPowerUpPrefabs(prefabs);
    }

    /**
     * @brief Register enemy prefabs from config
     */
    static void registerEnemyPrefabs(ECS::PrefabManager& prefabs) {
        auto& configRegistry = EntityConfigRegistry::getInstance();

        for (const auto& [id, config] : configRegistry.getAllEnemies()) {
            prefabs.registerPrefab(
                "enemy_" + id,
                [cfg = config](ECS::Registry& registry, ECS::Entity entity) {
                    registry.emplaceComponent<TransformComponent>(entity, 0.0f,
                                                                  0.0f, 0.0f);
                    float velX = (cfg.behavior == AIBehavior::MoveLeft)
                                     ? -cfg.speed
                                     : 0.0f;
                    registry.emplaceComponent<VelocityComponent>(entity, velX,
                                                                 0.0f);
                    registry.emplaceComponent<HealthComponent>(
                        entity, cfg.health, cfg.health);
                    registry.emplaceComponent<AIComponent>(
                        entity, cfg.behavior, cfg.speed, 0.0f, 0.0f, 0.0f);
                    registry.emplaceComponent<BoundingBoxComponent>(
                        entity, cfg.hitboxWidth, cfg.hitboxHeight);
                    registry.emplaceComponent<EnemyTag>(entity);
                });
        }
    }

    /**
     * @brief Register projectile prefabs from config
     */
    static void registerProjectilePrefabs(ECS::PrefabManager& prefabs) {
        auto& configRegistry = EntityConfigRegistry::getInstance();

        for (const auto& [id, config] : configRegistry.getAllProjectiles()) {
            prefabs.registerPrefab(
                "projectile_" + id,
                [cfg = config](ECS::Registry& registry, ECS::Entity entity) {
                    registry.emplaceComponent<TransformComponent>(entity, 0.0f,
                                                                  0.0f, 0.0f);

                    registry.emplaceComponent<VelocityComponent>(
                        entity, cfg.speed, 0.0f);
                    registry.emplaceComponent<BoundingBoxComponent>(
                        entity, cfg.hitboxWidth, cfg.hitboxHeight);
                    registry.emplaceComponent<HealthComponent>(
                        entity, cfg.damage, cfg.damage);
                    registry.emplaceComponent<ProjectileTag>(entity);
                });
        }
    }

    /**
     * @brief Register player prefabs from config
     */
    static void registerPlayerPrefabs(ECS::PrefabManager& prefabs) {
        auto& configRegistry = EntityConfigRegistry::getInstance();

        for (const auto& [id, config] : configRegistry.getAllPlayers()) {
            prefabs.registerPrefab(
                "player_" + id,
                [cfg = config](ECS::Registry& registry, ECS::Entity entity) {
                    registry.emplaceComponent<TransformComponent>(entity, 0.0f,
                                                                  0.0f, 0.0f);
                    registry.emplaceComponent<VelocityComponent>(entity, 0.0f,
                                                                 0.0f);
                    registry.emplaceComponent<HealthComponent>(
                        entity, cfg.health, cfg.health);
                    registry.emplaceComponent<BoundingBoxComponent>(
                        entity, cfg.hitboxWidth, cfg.hitboxHeight);
                    registry.emplaceComponent<PlayerTag>(entity);
                });
        }
    }

    /**
     * @brief Register power-up prefabs from config
     */
    static void registerPowerUpPrefabs(ECS::PrefabManager& prefabs) {
        auto& configRegistry = EntityConfigRegistry::getInstance();

        for (const auto& [id, config] : configRegistry.getAllPowerUps()) {
            prefabs.registerPrefab(
                "powerup_" + id,
                [cfg = config](ECS::Registry& registry, ECS::Entity entity) {
                    registry.emplaceComponent<TransformComponent>(entity, 0.0f,
                                                                  0.0f, 0.0f);
                    registry.emplaceComponent<VelocityComponent>(
                        entity, -50.0f, 0.0f);
                    registry.emplaceComponent<BoundingBoxComponent>(
                        entity, cfg.hitboxWidth, cfg.hitboxHeight);
                    registry.emplaceComponent<PickupTag>(entity);
                });
        }
    }

    /**
     * @brief Helper to get config values (for systems that need stats)
     */
    [[nodiscard]] static float getPlayerSpeed(const std::string& playerId) {
        const auto* config =
            EntityConfigRegistry::getInstance().getPlayer(playerId);
        return config ? config->speed : 200.0f;
    }

    [[nodiscard]] static float getPlayerFireRate(const std::string& playerId) {
        const auto* config =
            EntityConfigRegistry::getInstance().getPlayer(playerId);
        return config ? config->fireRate : 5.0f;
    }

    [[nodiscard]] static int32_t getEnemyScore(const std::string& enemyId) {
        const auto* config =
            EntityConfigRegistry::getInstance().getEnemy(enemyId);
        return config ? config->scoreValue : 100;
    }

    [[nodiscard]] static int32_t getProjectileDamage(
        const std::string& projectileId) {
        const auto* config =
            EntityConfigRegistry::getInstance().getProjectile(projectileId);
        return config ? config->damage : 10;
    }
};

}  // namespace rtype::games::rtype::shared
