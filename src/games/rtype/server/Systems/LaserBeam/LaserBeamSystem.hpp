/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamSystem - Handles continuous laser beam weapon logic
*/

#pragma once

#include <functional>
#include <unordered_set>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../../../shared/Components/BoundingBoxComponent.hpp"
#include "../../../shared/Components/HealthComponent.hpp"
#include "../../../shared/Components/LaserBeamComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::server {

/**
 * @brief System that handles continuous laser beam weapon
 *
 * The laser beam is a hold-to-fire weapon that:
 * - Creates a beam entity attached to the player
 * - Extends over time while fire button is held
 * - Deals damage per second (DPS) to all enemies in its path
 * - Has maximum duration and cooldown after release
 */
class LaserBeamSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct a new Laser Beam System
     * @param emitter Function to emit game events
     */
    explicit LaserBeamSystem(EventEmitter emitter);

    /**
     * @brief Update laser beam logic
     * @param registry ECS registry
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Handle laser input from player
     *
     * Called by PlayerInputHandler when player has laser weapon selected.
     *
     * @param registry ECS registry
     * @param playerEntity Player entity
     * @param playerNetworkId Player's network ID
     * @param isFiring True if fire button is held, false if released
     */
    void handleLaserInput(ECS::Registry& registry, ECS::Entity playerEntity,
                          uint32_t playerNetworkId, bool isFiring);

    /**
     * @brief Check if player has an active laser beam
     * @param registry ECS registry
     * @param playerNetworkId Player's network ID
     * @return True if player has an active beam
     */
    [[nodiscard]] bool hasActiveLaser(ECS::Registry& registry,
                                      uint32_t playerNetworkId) const;

   private:
    /**
     * @brief Start firing laser for a player
     */
    void startLaser(ECS::Registry& registry, ECS::Entity playerEntity,
                    uint32_t playerNetworkId);

    /**
     * @brief Stop firing laser for a player
     */
    void stopLaser(ECS::Registry& registry, uint32_t playerNetworkId);

    /**
     * @brief Update all active laser beams
     */
    void updateActiveBeams(ECS::Registry& registry, float deltaTime);

    /**
     * @brief Update laser beam positions to follow players
     */
    void updateBeamPositions(ECS::Registry& registry);

    /**
     * @brief Perform collision detection for laser beams
     */
    void performBeamCollisions(ECS::Registry& registry, float deltaTime);

    /**
     * @brief Apply damage to a target entity
     */
    void applyDamage(ECS::Registry& registry, ECS::Entity target,
                     float damage, uint32_t attackerNetworkId);

    /**
     * @brief Emit beam spawn event
     */
    void emitBeamSpawn(uint32_t beamNetworkId, float x, float y,
                       uint32_t ownerNetworkId);

    /**
     * @brief Emit beam destroy event
     */
    void emitBeamDestroy(uint32_t beamNetworkId);

    EventEmitter _emitEvent;
    uint32_t _nextBeamNetworkId{200000};  // Start beam IDs at 200000
    std::unordered_set<uint64_t> _damagedThisFrame;  // Track damaged entities
};

}  // namespace rtype::games::rtype::server
