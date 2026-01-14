/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BossSerpentAnimationSystem.cpp
**
** Handles animation for boss serpent entities using 4 separate sprite sheets:
** - boss_serpent_head: Head idle/movement (5 frames, 135x369 each)
** - boss_serpent_attack: Head attack animation (5 frames, 135x369 each)
** - boss_serpent_body: Body segments (5 frames, 135x369 each)
** - boss_serpent_tail: Tail segment (5 frames, 135x369 each)
*/

#include "BossSerpentAnimationSystem.hpp"

#include <cmath>
#include <unordered_map>

#include "../../shared/Components/TransformComponent.hpp"
#include "../../shared/Components/VelocityComponent.hpp"
#include "../Components/BossSerpentComponent.hpp"
#include "../Components/RotationComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/TextureRectComponent.hpp"

namespace rtype::games::rtype::client {

using shared::TransformComponent;
using shared::VelocityComponent;

struct EntityPositionData {
    float prevX = 0.0f;
    float prevY = 0.0f;
    bool initialized = false;
};

static std::unordered_map<std::uint32_t, EntityPositionData> s_positionTracker;

BossSerpentAnimationSystem::BossSerpentAnimationSystem()
    : ::rtype::engine::ASystem("BossSerpentAnimationSystem") {}

void BossSerpentAnimationSystem::update(ECS::Registry& registry, float dt) {
    registry.view<BossSerpentVisual, TransformComponent, TextureRect, Image>()
        .each([&registry, dt](auto entity, BossSerpentVisual& visual,
                              TransformComponent& transform,
                              TextureRect& texRect, Image& image) {
            visual.updateAnimation(dt);
            if (registry.hasComponent<Rotation>(entity)) {
                auto& rot = registry.getComponent<Rotation>(entity);
                auto& posData = s_positionTracker[entity.id];
                if (posData.initialized) {
                    float vx = transform.x - posData.prevX;
                    float vy = transform.y - posData.prevY;
                    if (std::abs(vx) > 0.5f || std::abs(vy) > 0.5f) {
                        float targetAngle = std::atan2(vy, vx) * 180.0f / 3.14159265f;
                        targetAngle += 180.0f;
                        while (targetAngle > 180.0f) targetAngle -= 360.0f;
                        while (targetAngle < -180.0f) targetAngle += 360.0f;
                        float diff = targetAngle - rot.angle;
                        while (diff > 180.0f) diff -= 360.0f;
                        while (diff < -180.0f) diff += 360.0f;
                        rot.angle += diff * 0.15f;
                    }
                }
                posData.prevX = transform.x;
                posData.prevY = transform.y;
                posData.initialized = true;
            }
            const char* expectedTexture = visual.getTextureName();
            if (image.textureName != expectedTexture) {
                image.textureName = expectedTexture;
            }
            int x = 0, y = 0, w = 0, h = 0;
            visual.getTextureRect(x, y, w, h);

            texRect.rect.left = x;
            texRect.rect.top = y;
            texRect.rect.width = w;
            texRect.rect.height = h;
        });

    registry.view<BossSerpentBodyVisual, TransformComponent, TextureRect, Image>()
        .each([&registry, dt](auto entity, BossSerpentBodyVisual& bodyVisual,
                              TransformComponent& transform,
                              TextureRect& texRect, Image& image) {
            bodyVisual.updateAnimation(dt);
            if (registry.hasComponent<Rotation>(entity)) {
                auto& rot = registry.getComponent<Rotation>(entity);
                auto& posData = s_positionTracker[entity.id];

                if (posData.initialized) {
                    float vx = transform.x - posData.prevX;
                    float vy = transform.y - posData.prevY;

                    if (std::abs(vx) > 0.5f || std::abs(vy) > 0.5f) {
                        float targetAngle = std::atan2(vy, vx) * 180.0f / 3.14159265f;
                        targetAngle += 180.0f;

                        while (targetAngle > 180.0f) targetAngle -= 360.0f;
                        while (targetAngle < -180.0f) targetAngle += 360.0f;

                        float diff = targetAngle - rot.angle;
                        while (diff > 180.0f) diff -= 360.0f;
                        while (diff < -180.0f) diff += 360.0f;
                        rot.angle += diff * 0.12f;
                    }
                }

                posData.prevX = transform.x;
                posData.prevY = transform.y;
                posData.initialized = true;
            }
            const char* expectedTexture = bodyVisual.getTextureName();
            if (image.textureName != expectedTexture) {
                image.textureName = expectedTexture;
            }
            int x = 0, y = 0, w = 0, h = 0;
            bodyVisual.getTextureRect(x, y, w, h);

            texRect.rect.left = x;
            texRect.rect.top = y;
            texRect.rect.width = w;
            texRect.rect.height = h;
        });
}

}  // namespace rtype::games::rtype::client
