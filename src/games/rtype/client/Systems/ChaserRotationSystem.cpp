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
    using ::rtype::games::rtype::shared::PlayerIdComponent;
    using ::rtype::games::rtype::shared::TransformComponent;
    using ::rtype::games::rtype::shared::EnemyTypeComponent;
    using ::rtype::games::rtype::shared::EnemyVariant;
    ECS::Entity targetPlayer;
    bool foundPlayer = false;
    float targetX = 0.0f;
    float targetY = 0.0f;

    registry.view<PlayerIdComponent, TransformComponent>().each(
        [&](auto entity, const PlayerIdComponent&,
            const TransformComponent& transform) {
            if (!foundPlayer) {
                targetPlayer = entity;
                targetX = transform.x;
                targetY = transform.y;
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

            if (distance <= EXPLOSION_DISTANCE && !explosion.isExploding) {
                explosion.isExploding = true;
                explosion.explosionTimer = 0.0f;
                anim.currentFrame = 2;
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
