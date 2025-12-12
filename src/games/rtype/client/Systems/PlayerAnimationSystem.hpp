/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PlayerAnimationSystem - Updates player sprite frame based on movement and player id
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_

#include <memory>

#include <rtype/engine.hpp>

namespace rtype::games::rtype::client {

class PlayerAnimationSystem : public ::rtype::engine::ASystem {
   public:
    PlayerAnimationSystem();
    ~PlayerAnimationSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;

    // Sprite sheet layout (used by factory to set initial frame)
    static constexpr int kFrameWidth = 33;
    static constexpr int kFrameHeight = 17;
    static constexpr int kStatesPerRow = 5;
    static constexpr int kColorRows = 5;
    static constexpr float kLowThreshold = 40.0F;
    static constexpr float kHighThreshold = 140.0F;

   private:
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PLAYERANIMATIONSYSTEM_HPP_
