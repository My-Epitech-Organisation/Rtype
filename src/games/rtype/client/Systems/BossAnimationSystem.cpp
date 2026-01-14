#include "BossAnimationSystem.hpp"

#include <cmath>
#include <unordered_map>

#include "../../shared/Components/TransformComponent.hpp"
#include "../../shared/Components/VelocityComponent.hpp"
#include "../Components/BossVisualComponent.hpp"
#include "../Components/RotationComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

using shared::TransformComponent;
using shared::VelocityComponent;

namespace {
constexpr float PI = 3.14159265F;
constexpr float RAD_TO_DEG = 180.0F / PI;
constexpr float MIN_VELOCITY_THRESHOLD = 0.5F;
constexpr float ANGLE_WRAP = 360.0F;
constexpr float ANGLE_HALF = 180.0F;

struct EntityPositionData {
    float prevX = 0.0F;
    float prevY = 0.0F;
    bool initialized = false;
};

std::unordered_map<std::uint32_t, EntityPositionData> s_positionTracker;
}  // namespace

BossAnimationSystem::BossAnimationSystem()
    : ::rtype::engine::ASystem("BossAnimationSystem") {}

void BossAnimationSystem::update(ECS::Registry& registry, float dt) {
    static bool loggedOnce = false;
    static int entityCount = 0;
    static int callCount = 0;
    callCount++;

    if (callCount == 1 || callCount == 60 || callCount == 120) {
        LOG_INFO_CAT(::rtype::LogCategory::ECS,
                     "[BossAnimSystem] update called, callCount=" << callCount);
    }

    registry.view<BossVisualComponent, TransformComponent, TextureRect, Image>()
        .each([&registry, dt](auto entity, BossVisualComponent& visual,
                              TransformComponent& transform,
                              TextureRect& texRect, Image& image) {
            entityCount++;

            if (!loggedOnce) {
                LOG_INFO_CAT(
                    ::rtype::LogCategory::ECS,
                    "[BossAnimSystem] Entity "
                        << entity.id << " - texture: " << visual.moveTexture
                        << ", frameCount: " << visual.frameCount
                        << ", enableRotation: " << visual.enableRotation
                        << ", frameW: " << visual.frameWidth
                        << ", frameH: " << visual.frameHeight);
            }

            visual.updateAnimation(dt);

            if (visual.enableRotation &&
                registry.hasComponent<Rotation>(entity)) {
                auto& rot = registry.getComponent<Rotation>(entity);
                auto& posData = s_positionTracker[entity.id];

                if (posData.initialized) {
                    float vx = transform.x - posData.prevX;
                    float vy = transform.y - posData.prevY;

                    if (std::abs(vx) > MIN_VELOCITY_THRESHOLD ||
                        std::abs(vy) > MIN_VELOCITY_THRESHOLD) {
                        float targetAngle = std::atan2(vy, vx) * RAD_TO_DEG;
                        targetAngle += ANGLE_HALF;

                        while (targetAngle > ANGLE_HALF)
                            targetAngle -= ANGLE_WRAP;
                        while (targetAngle < -ANGLE_HALF)
                            targetAngle += ANGLE_WRAP;

                        float diff = targetAngle - rot.angle;
                        while (diff > ANGLE_HALF) diff -= ANGLE_WRAP;
                        while (diff < -ANGLE_HALF) diff += ANGLE_WRAP;

                        rot.angle += diff * visual.rotationSmoothing;
                    }
                }

                posData.prevX = transform.x;
                posData.prevY = transform.y;
                posData.initialized = true;
            }

            const std::string& expectedTexture = visual.getCurrentTexture();
            if (image.textureName != expectedTexture) {
                image.textureName = expectedTexture;
            }

            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;
            visual.getTextureRect(x, y, w, h);

            texRect.rect.left = x;
            texRect.rect.top = y;
            texRect.rect.width = w;
            texRect.rect.height = h;
        });

    if (!loggedOnce && entityCount > 0) {
        LOG_INFO_CAT(::rtype::LogCategory::ECS,
                     "[BossAnimSystem] Processing "
                         << entityCount
                         << " entities with BossVisualComponent");
        loggedOnce = true;
    }
    entityCount = 0;
}

}  // namespace rtype::games::rtype::client
