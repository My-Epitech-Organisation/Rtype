/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserRotationSystem.cpp
*/

#include "ChaserRotationSystem.hpp"

#include <cmath>

#include "../../shared/Components/EnemyTypeComponent.hpp"
#include "../../shared/Components/PlayerIdComponent.hpp"
#include "../../shared/Components/TransformComponent.hpp"
#include "../Components/AnnimationComponent.hpp"
#include "../Components/ChaserExplosionComponent.hpp"
#include "../Components/ImageComponent.hpp"
#include "../Components/RotationComponent.hpp"
#include "../Components/TextureRectComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

ChaserRotationSystem::ChaserRotationSystem()
    : ::rtype::engine::ASystem("ChaserRotationSystem") {}

void ChaserRotationSystem::update(ECS::Registry& registry, float dt) {
    using namespace ::rtype::games::rtype::shared;
    ECS::Entity targetPlayer;
    bool foundPlayer = false;
    float targetX = 0.0f;
    float targetY = 0.0f;

    registry.view<shared::PlayerIdComponent, TransformComponent>().each(
        [&](auto entity, const shared::PlayerIdComponent&,
            const TransformComponent& transform) {
            if (!foundPlayer) {
                targetPlayer = entity;
                targetX = transform.x + 33.0f;
                targetY = transform.y + 34.0f;
                foundPlayer = true;
            }
        });

    if (!foundPlayer) {
        return;
    }

    constexpr float EXPLOSION_DISTANCE = 150.0f;
    int chaserCount = 0;
    registry
        .view<EnemyTypeComponent, TransformComponent, Rotation, Animation,
              ChaserExplosion>()
        .each([&](auto entity, const EnemyTypeComponent& enemyType,
                  const TransformComponent& transform, Rotation& rotation,
                  Animation& anim, ChaserExplosion& explosion) {
            if (enemyType.variant != EnemyVariant::Chaser) {
                return;
            }
            chaserCount++;
            float dx = targetX - transform.x;
            float dy = targetY - transform.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Check if chaser should start exploding
            if (distance <= EXPLOSION_DISTANCE && !explosion.isExploding) {
                // Mark as exploding and trigger animation to frame 2 (start of
                // explosion)
                explosion.isExploding = true;
                explosion.explosionTimer = 0.0f;
                anim.currentFrame =
                    2;  // Jump to frame 2 (first explosion frame)
                anim.elapsedTime = 0.0f;
                LOG_DEBUG("[ChaserRotation] Chaser "
                          << entity.id << " starting explosion at distance "
                          << distance);
            }

            float angleRad = std::atan2(dy, dx);
            float angleDeg = angleRad * 180.0f / 3.14159265f;
            rotation.angle = angleDeg;
        });
}

}  // namespace rtype::games::rtype::client
