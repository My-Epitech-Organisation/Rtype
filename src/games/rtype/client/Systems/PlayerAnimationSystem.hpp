/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PlayerAnimationSystem - Updates player sprite frame based on movement and
* player id
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_

#include <memory>

#include <rtype/engine.hpp>

namespace rtype::games::rtype::client {

/**
 * @class PlayerAnimationSystem
 * @brief Updates player sprite frames based on velocity and player ID
 * 
 * Sprite sheet layout:
 * - Each frame: 33x17 pixels
 * - 5 columns (states): strong down, ease down, neutral, ease up, strong up
 * - 5 rows: different player colors (selected by network ID % 5)
 * 
 * Velocity thresholds:
 * - High: ±140.0 pixels/sec (strong tilt animation)
 * - Low: ±40.0 pixels/sec (slight tilt animation)
 */
class PlayerAnimationSystem : public ::rtype::engine::ASystem {
   public:
    PlayerAnimationSystem();
    ~PlayerAnimationSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;

    /// Width of each sprite frame in pixels
    static constexpr int kFrameWidth = 33;
    /// Height of each sprite frame in pixels
    static constexpr int kFrameHeight = 17;
    /// Number of animation states per row (columns)
    static constexpr int kStatesPerRow = 5;
    /// Number of color variants (rows) in sprite sheet
    static constexpr int kColorRows = 5;
    /// Velocity threshold for slight tilt animation (pixels/sec)
    static constexpr float kLowThreshold = 40.0F;
    /// Velocity threshold for strong tilt animation (pixels/sec)
    static constexpr float kHighThreshold = 140.0F;

   private:
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_
