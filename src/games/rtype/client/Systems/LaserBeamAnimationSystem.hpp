/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamAnimationSystem - Multi-phase animation for laser beams
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_LASERBEAMANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_LASERBEAMANIMATIONSYSTEM_HPP_

#include <rtype/engine.hpp>

#include "../Components/LaserBeamAnimationComponent.hpp"

namespace rtype::games::rtype::client {

/**
 * @class LaserBeamAnimationSystem
 * @brief Handles multi-phase animation for laser beam entities
 *
 * This system manages the vertical spritesheet animation with three phases:
 * - Startup: Plays once when laser spawns (frames 0-6)
 * - Loop: Loops while laser is active (frames 7-14)
 * - End: Plays once before destruction (frames 15-17)
 *
 * Unlike the standard AnimationSystem which uses horizontal spritesheets
 * and modifies TextureRect.left, this system handles vertical spritesheets
 * and modifies TextureRect.top.
 */
class LaserBeamAnimationSystem : public ::rtype::engine::ASystem {
   public:
    LaserBeamAnimationSystem();

    /**
     * @brief Update all laser beam animations
     * @param registry ECS registry
     * @param dt Delta time in seconds
     */
    void update(ECS::Registry& registry, float dt) override;

   private:
    /**
     * @brief Advance the animation frame and handle phase transitions
     * @param anim Animation component
     * @return true if entity should be destroyed
     */
    bool advanceFrame(LaserBeamAnimationComponent& anim);
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_LASERBEAMANIMATIONSYSTEM_HPP_
