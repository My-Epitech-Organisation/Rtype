/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargedProjectileAnimationSystem - Handles charged shot animations
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEDPROJECTILEANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEDPROJECTILEANIMATIONSYSTEM_HPP_

#include "ASystem.hpp"

namespace rtype::games::rtype::client {

/**
 * @class ChargedProjectileAnimationSystem
 * @brief System that handles animation for charged shot projectiles
 *
 * Manages the special animation sequence for charged shots:
 * - Spawn phase: frames 0-4 (energy growing)
 * - Loop phase: frames 4-5 (pulsating energy)
 */
class ChargedProjectileAnimationSystem : public ::rtype::engine::ASystem {
   public:
    ChargedProjectileAnimationSystem();

    /**
     * @brief Update charged projectile animations
     * @param registry ECS registry
     * @param dt Delta time
     */
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEDPROJECTILEANIMATIONSYSTEM_HPP_
