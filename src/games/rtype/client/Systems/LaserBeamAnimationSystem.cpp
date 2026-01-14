/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamAnimationSystem - Multi-phase animation for laser beams
*/

#include "LaserBeamAnimationSystem.hpp"

#include "../../shared/Components/Tags.hpp"
#include "../Components/TextureRectComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

using shared::LaserBeamTag;

LaserBeamAnimationSystem::LaserBeamAnimationSystem()
    : ::rtype::engine::ASystem("LaserBeamAnimationSystem") {}

void LaserBeamAnimationSystem::update(ECS::Registry& registry, float dt) {
    registry.view<LaserBeamAnimationComponent, TextureRect, LaserBeamTag>().each(
        [this, &registry, dt](auto entity, LaserBeamAnimationComponent& anim,
                               TextureRect& tex, const LaserBeamTag&) {
            anim.elapsedTime += dt;

            if (anim.elapsedTime >= anim.frameDuration) {
                anim.elapsedTime = 0.0f;

                bool shouldDestroy = advanceFrame(anim);

                // Update texture rect for vertical spritesheet
                tex.rect.top = anim.getTextureTopOffset();

                if (shouldDestroy) {
                    // Mark entity for destruction
                    if (!registry
                             .hasComponent<shared::DestroyTag>(entity)) {
                        registry.emplaceComponent<shared::DestroyTag>(entity);
                    }
                }
            }
        });
}

bool LaserBeamAnimationSystem::advanceFrame(LaserBeamAnimationComponent& anim) {
    switch (anim.phase) {
        case LaserAnimPhase::Startup:
            if (anim.currentFrame <
                LaserBeamAnimationComponent::kStartupLast) {
                anim.currentFrame++;
            } else {
                // Transition to Loop phase
                anim.phase = LaserAnimPhase::Loop;
                anim.currentFrame = LaserBeamAnimationComponent::kLoopFirst;
                LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                              "[LaserBeamAnimation] Transition: Startup -> Loop");
            }
            break;

        case LaserAnimPhase::Loop:
            if (anim.pendingDestroy) {
                // Server sent destroy, transition to End phase
                anim.phase = LaserAnimPhase::End;
                anim.currentFrame = LaserBeamAnimationComponent::kEndFirst;
                LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                              "[LaserBeamAnimation] Transition: Loop -> End");
            } else {
                // Continue looping
                anim.currentFrame++;
                if (anim.currentFrame >
                    LaserBeamAnimationComponent::kLoopLast) {
                    anim.currentFrame = LaserBeamAnimationComponent::kLoopFirst;
                }
            }
            break;

        case LaserAnimPhase::End:
            if (anim.currentFrame < LaserBeamAnimationComponent::kEndLast) {
                anim.currentFrame++;
            } else {
                // Animation complete, mark for destruction
                anim.phase = LaserAnimPhase::Destroyed;
                LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                              "[LaserBeamAnimation] Transition: End -> Destroyed");
                return true;
            }
            break;

        case LaserAnimPhase::Destroyed:
            return true;
    }

    return false;
}

}  // namespace rtype::games::rtype::client
