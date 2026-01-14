/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamSystem - Handles continuous laser beam weapon logic
*/

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../../../shared/Components/BoundingBoxComponent.hpp"
#include "../../../shared/Components/DamageOnContactComponent.hpp"
#include "../../../shared/Components/LaserBeamComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"
#include "../../../shared/Config/GameConfig/RTypeGameConfig.hpp"

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
     * @param config Laser weapon configuration
     */
    explicit LaserBeamSystem(EventEmitter emitter,
                             const game::config::LaserConfig& config = {});

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

    // Static helpers for beam state management (ECS-pure pattern)
    // These methods modify the component data directly, keeping
    // LaserBeamComponent as a data-only struct.
    // Public for unit testing and potential external use.

    /**
     * @brief Start firing a laser beam (state transition)
     * @param beam The beam component to modify
     */
    static void startFiringBeam(shared::LaserBeamComponent& beam);

    /**
     * @brief Stop firing a laser beam (player released)
     * @param beam The beam component to modify
     */
    static void stopFiringBeam(shared::LaserBeamComponent& beam);

    /**
     * @brief Force stop a laser beam (max duration reached)
     * @param beam The beam component to modify
     */
    static void forceStopBeam(shared::LaserBeamComponent& beam);

    /**
     * @brief Update beam state and timers
     * @param beam The beam component to modify
     * @param deltaTime Frame delta time
     * @return true if beam reached max duration and should emit destroy event
     */
    static bool updateBeamState(shared::LaserBeamComponent& beam,
                                float deltaTime);

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
     * @brief Emit beam spawn event
     */
    void emitBeamSpawn(uint32_t beamNetworkId, float x, float y,
                       uint32_t ownerNetworkId);

    /**
     * @brief Emit beam destroy event
     */
    void emitBeamDestroy(uint32_t beamNetworkId);

    /**
     * @brief Rebuild player cache for O(1) lookups
     * @param registry ECS registry
     */
    void rebuildPlayerCache(ECS::Registry& registry);

    EventEmitter _emitEvent;
    game::config::LaserConfig _config;

    std::unordered_map<uint32_t, ECS::Entity> _playerCache;

    static constexpr uint32_t kLaserBeamNetworkIdStart = 200000;

    uint32_t _nextBeamNetworkId{kLaserBeamNetworkIdStart};
};

}  // namespace rtype::games::rtype::server
