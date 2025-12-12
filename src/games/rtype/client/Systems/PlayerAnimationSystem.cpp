/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PlayerAnimationSystem - Updates player sprite frame based on movement and
* player id
*/

#include "PlayerAnimationSystem.hpp"

#include "../AllComponents.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"

namespace rtype::games::rtype::client {

namespace rs = ::rtype::games::rtype::shared;

PlayerAnimationSystem::PlayerAnimationSystem()
    : ::rtype::engine::ASystem("PlayerAnimationSystem") {}

void PlayerAnimationSystem::update(ECS::Registry& registry, float /*dt*/) {
    registry
        .view<rs::VelocityComponent, TextureRect, Image, rs::NetworkIdComponent,
              PlayerTag>()
        .each([&](auto /*entity*/, const rs::VelocityComponent& vel,
                  TextureRect& tex, Image& img,
                  const rs::NetworkIdComponent& netId, PlayerTag& /*tag*/) {
            // Column selection based on vertical velocity
            int column = 2;  // default: neutral
            if (vel.vy > kHighThreshold) {
                column = 0;  // strong downwards
            } else if (vel.vy > kLowThreshold) {
                column = 1;  // easing downwards
            } else if (vel.vy < -kHighThreshold) {
                column = 4;  // strong upwards
            } else if (vel.vy < -kLowThreshold) {
                column = 3;  // easing upwards
            }

            // Row selection based on player id to colorize per user
            int row = 0;
            if (kColorRows > 0) {
                row = static_cast<int>(netId.networkId % kColorRows);
            }

            tex.rect = sf::IntRect({kFrameWidth * column, kFrameHeight * row},
                                   {kFrameWidth, kFrameHeight});

            img.sprite.setTextureRect(tex.rect);
        });
}

}  // namespace rtype::games::rtype::client
