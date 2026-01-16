/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodVisualSystem - Implementation
*/

#include "ForcePodVisualSystem.hpp"

#include <cmath>

#include "../../shared/Components/TransformComponent.hpp"
#include "../GameScene/VisualCueFactory.hpp"

namespace rtype::games::rtype::client {

ForcePodVisualSystem::ForcePodVisualSystem()
    : ::rtype::engine::ASystem("ForcePodVisualSystem") {}

void ForcePodVisualSystem::update(ECS::Registry& registry, float deltaTime) {
    auto view = registry.view<shared::ForcePodComponent, ForcePodVisual,
                              ColorTint, shared::TransformComponent>();

    view.each([this, &registry, deltaTime](
                  ECS::Entity entity, shared::ForcePodComponent& pod,
                  ForcePodVisual& visual, ColorTint& colorTint,
                  shared::TransformComponent& transform) {
        updateGlowEffect(registry, pod, visual, colorTint, deltaTime);

        if (pod.state == shared::ForcePodState::Detached) {
            visual.showTrail = true;

            static float trailTimer = 0.0F;
            trailTimer += deltaTime;
            if (trailTimer >= 0.05F) {
                createTrailParticle(registry, transform.x, transform.y);
                trailTimer = 0.0F;
            }
        } else {
            visual.showTrail = false;
        }
    });
}

void ForcePodVisualSystem::updateGlowEffect(
    ECS::Registry& registry, const shared::ForcePodComponent& pod,
    ForcePodVisual& visual, ColorTint& colorTint, float deltaTime) {
    if (pod.state == shared::ForcePodState::Attached) {
        visual.glowIntensity += deltaTime * 2.0F;
        if (visual.glowIntensity > 1.0F) {
            visual.glowIntensity = 0.0F;
        }

        float brightness =
            200 + static_cast<int>(
                      55.0F * std::sin(visual.glowIntensity * 3.14159F * 2.0F));
        colorTint.r =
            static_cast<std::uint8_t>(std::min(255.0F, brightness * 0.4F));
        colorTint.g =
            static_cast<std::uint8_t>(std::min(255.0F, brightness * 0.8F));
        colorTint.b = static_cast<std::uint8_t>(std::min(255.0F, brightness));
        colorTint.a = 255;
    } else if (pod.state == shared::ForcePodState::Detached) {
        colorTint.r = 255;
        colorTint.g = 255;
        colorTint.b = 255;
        colorTint.a = 255;
    } else if (pod.state == shared::ForcePodState::Returning) {
        colorTint.r = 100;
        colorTint.g = 255;
        colorTint.b = 100;
        colorTint.a = 255;
    }
}

void ForcePodVisualSystem::createTrailParticle(ECS::Registry& registry, float x,
                                               float y) {
    VisualCueFactory::createFlash(registry, {x, y},
                                  ::rtype::display::Color{100, 200, 255, 180},
                                  20.f, 0.2f, 8);
}

}  // namespace rtype::games::rtype::client
