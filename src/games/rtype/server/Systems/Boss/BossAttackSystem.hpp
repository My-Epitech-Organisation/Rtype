/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossAttackSystem - Executes boss attack patterns
*/

#pragma once

#include <functional>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../../../shared/Components/BossPatternComponent.hpp"

namespace rtype::games::rtype::server {

/**
 * @class BossAttackSystem
 * @brief System that executes boss attack patterns
 *
 * Manages attack pattern timing, projectile spawning, and pattern cycling.
 * Each pattern type has specific execution logic:
 * - CircularShot: Spawns projectiles in a circle around the boss
 * - SpreadFan: Spawns projectiles in a fan toward target
 * - LaserSweep: Continuous damage in a sweeping arc
 * - MinionSpawn: Spawns enemy minions
 * - TailSweep: Physical sweep attack
 */
class BossAttackSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;
    using ProjectileSpawner = std::function<uint32_t(
        ECS::Registry&, float, float, float, float, int32_t, uint32_t)>;
    using MinionSpawner =
        std::function<void(ECS::Registry&, const std::string&, float, float)>;

    /**
     * @brief Construct with spawner callbacks
     * @param emitter Function to emit game events
     * @param projSpawner Function to spawn projectiles
     * @param minionSpawner Function to spawn minion enemies
     */
    BossAttackSystem(EventEmitter emitter, ProjectileSpawner projSpawner,
                     MinionSpawner minionSpawner);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    void updatePatternState(ECS::Registry& registry,
                            shared::BossPatternComponent& patterns,
                            float deltaTime);

    void executeCircularShot(ECS::Registry& registry, ECS::Entity boss,
                             shared::BossPatternComponent& patterns,
                             float deltaTime);

    void executeSpreadFan(ECS::Registry& registry, ECS::Entity boss,
                          shared::BossPatternComponent& patterns,
                          float deltaTime);

    void executeLaserSweep(ECS::Registry& registry, ECS::Entity boss,
                           shared::BossPatternComponent& patterns,
                           float deltaTime);

    void executeMinionSpawn(ECS::Registry& registry, ECS::Entity boss,
                            shared::BossPatternComponent& patterns,
                            float deltaTime);

    void executeTailSweep(ECS::Registry& registry, ECS::Entity boss,
                          shared::BossPatternComponent& patterns,
                          float deltaTime);

    void findNearestPlayer(ECS::Registry& registry, float bossX, float bossY,
                           float& targetX, float& targetY);

    EventEmitter _emitEvent;
    ProjectileSpawner _spawnProjectile;
    MinionSpawner _spawnMinion;
};

}  // namespace rtype::games::rtype::server
