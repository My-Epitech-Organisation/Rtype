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
    auto view = registry.view<shared::ForcePodComponent, ForcePodVisual, Image,
                              shared::TransformComponent>();
    
    view.each([this, &registry, deltaTime](
                  ECS::Entity entity, shared::ForcePodComponent& pod,
                  ForcePodVisual& visual, Image& image,
                  shared::TransformComponent& transform) {
        updateGlowEffect(registry, pod, visual, image, deltaTime);
        
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
    ForcePodVisual& visual, Image& image, float deltaTime) {
    
    if (pod.state == shared::ForcePodState::Attached) {
        visual.glowIntensity += deltaTime * 2.0F;
        if (visual.glowIntensity > 1.0F) {
            visual.glowIntensity = 0.0F;
        }
        
        float brightness = 200 + static_cast<int>(55.0F * std::sin(visual.glowIntensity * 3.14159F * 2.0F));
        visual.tintColor = sf::Color(
            static_cast<std::uint8_t>(std::min(255.0F, brightness * 0.4F)),
            static_cast<std::uint8_t>(std::min(255.0F, brightness * 0.8F)),
            static_cast<std::uint8_t>(std::min(255.0F, brightness)),
            255);
        
        image.sprite.setColor(visual.tintColor);
    } else if (pod.state == shared::ForcePodState::Detached) {
        visual.tintColor = sf::Color(255, 255, 255, 255);
        image.sprite.setColor(visual.tintColor);
    } else if (pod.state == shared::ForcePodState::Returning) {
        visual.tintColor = sf::Color(100, 255, 100, 255);
        image.sprite.setColor(visual.tintColor);
    }
}

void ForcePodVisualSystem::createTrailParticle(ECS::Registry& registry, float x,
                                              float y) {
    VisualCueFactory::createFlash(registry, {x, y},
                                 sf::Color(100, 200, 255, 180), 20.f, 0.2f, 8);
}

}  // namespace rtype::games::rtype::client
