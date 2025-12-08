/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PrefabLoader - Loads prefabs from EntityConfig into PrefabManager
*/

#include "PrefabLoader.hpp"

namespace rtype::games::rtype::shared {

void PrefabLoader::registerAllPrefabs(ECS::PrefabManager& prefabs) {
    registerEnemyPrefabs(prefabs);
    registerProjectilePrefabs(prefabs);
    registerPlayerPrefabs(prefabs);
    registerPowerUpPrefabs(prefabs);
}

void PrefabLoader::registerEnemyPrefabs(ECS::PrefabManager& prefabs) {
    auto& configRegistry = EntityConfigRegistry::getInstance();

    for (const auto& [id, config] : configRegistry.getAllEnemies()) {
        prefabs.registerPrefab("enemy_" + id, [cfg = config](
                                                  ECS::Registry& registry,
                                                  ECS::Entity entity) {
            registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f,
                                                          0.0f);
            float velX =
                (cfg.behavior == AIBehavior::MoveLeft) ? -cfg.speed : 0.0f;
            registry.emplaceComponent<VelocityComponent>(entity, velX, 0.0f);
            registry.emplaceComponent<HealthComponent>(entity, cfg.health,
                                                       cfg.health);
            registry.emplaceComponent<AIComponent>(entity, cfg.behavior,
                                                   cfg.speed, 0.0f, 0.0f, 0.0f);
            registry.emplaceComponent<BoundingBoxComponent>(
                entity, cfg.hitboxWidth, cfg.hitboxHeight);
            registry.emplaceComponent<EnemyTag>(entity);
        });
    }
}

void PrefabLoader::registerProjectilePrefabs(ECS::PrefabManager& prefabs) {
    auto& configRegistry = EntityConfigRegistry::getInstance();

    for (const auto& [id, config] : configRegistry.getAllProjectiles()) {
        prefabs.registerPrefab(
            "projectile_" + id,
            [cfg = config](ECS::Registry& registry, ECS::Entity entity) {
                registry.emplaceComponent<TransformComponent>(entity, 0.0f,
                                                              0.0f, 0.0f);

                registry.emplaceComponent<VelocityComponent>(entity, cfg.speed,
                                                             0.0f);
                registry.emplaceComponent<BoundingBoxComponent>(
                    entity, cfg.hitboxWidth, cfg.hitboxHeight);
                registry.emplaceComponent<HealthComponent>(entity, cfg.damage,
                                                           cfg.damage);
                registry.emplaceComponent<ProjectileTag>(entity);
            });
    }
}

void PrefabLoader::registerPlayerPrefabs(ECS::PrefabManager& prefabs) {
    auto& configRegistry = EntityConfigRegistry::getInstance();

    for (const auto& [id, config] : configRegistry.getAllPlayers()) {
        prefabs.registerPrefab("player_" + id, [cfg = config](
                                                   ECS::Registry& registry,
                                                   ECS::Entity entity) {
            registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f,
                                                          0.0f);
            registry.emplaceComponent<VelocityComponent>(entity, 0.0f, 0.0f);
            registry.emplaceComponent<HealthComponent>(entity, cfg.health,
                                                       cfg.health);
            registry.emplaceComponent<BoundingBoxComponent>(
                entity, cfg.hitboxWidth, cfg.hitboxHeight);
            registry.emplaceComponent<PlayerTag>(entity);
        });
    }
}

void PrefabLoader::registerPowerUpPrefabs(ECS::PrefabManager& prefabs) {
    auto& configRegistry = EntityConfigRegistry::getInstance();

    for (const auto& [id, config] : configRegistry.getAllPowerUps()) {
        prefabs.registerPrefab("powerup_" + id, [cfg = config](
                                                    ECS::Registry& registry,
                                                    ECS::Entity entity) {
            registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f,
                                                          0.0f);
            registry.emplaceComponent<VelocityComponent>(entity, -50.0f, 0.0f);
            registry.emplaceComponent<BoundingBoxComponent>(
                entity, cfg.hitboxWidth, cfg.hitboxHeight);
            registry.emplaceComponent<PickupTag>(entity);
        });
    }
}

float PrefabLoader::getPlayerSpeed(const std::string& playerId) {
    auto config = EntityConfigRegistry::getInstance().getPlayer(playerId);
    return config ? config->get().speed : 200.0f;
}

float PrefabLoader::getPlayerFireRate(const std::string& playerId) {
    auto config = EntityConfigRegistry::getInstance().getPlayer(playerId);
    return config ? config->get().fireRate : 5.0f;
}

int32_t PrefabLoader::getEnemyScore(const std::string& enemyId) {
    auto config = EntityConfigRegistry::getInstance().getEnemy(enemyId);
    return config ? config->get().scoreValue : 100;
}

int32_t PrefabLoader::getProjectileDamage(const std::string& projectileId) {
    auto config =
        EntityConfigRegistry::getInstance().getProjectile(projectileId);
    return config ? config->get().damage : 10;
}

}  // namespace rtype::games::rtype::shared
