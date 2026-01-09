/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AnimationSystem.cpp
*/

#include "AnimationSystem.hpp"

#include "Components/AnnimationComponent.hpp"
#include "Components/TextureRectComponent.hpp"

rtype::games::rtype::client::AnimationSystem::AnimationSystem()
    : ::rtype::engine::ASystem("AnimationSystem") {}

void rtype::games::rtype::client::AnimationSystem::update(
    ECS::Registry& registry, float dt) {
    registry.view<Animation, TextureRect>().each([dt](auto _, Animation& anim,
                                                      TextureRect& texRect) {
        anim.elapsedTime += dt;
        if (anim.elapsedTime >= anim.frameDuration) {
            anim.elapsedTime = 0.f;
            if (anim.currentFrame < anim.frameCount) {
                anim.currentFrame++;
            } else if (!anim.oneTime) {
                anim.currentFrame = 1;
            }
            texRect.rect.left = (anim.currentFrame - 1) * texRect.rect.width;
        }
    });
}
