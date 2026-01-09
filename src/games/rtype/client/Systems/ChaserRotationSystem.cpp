/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserRotationSystem.cpp
*/

#include "ChaserRotationSystem.hpp"

#include <cmath>

#include "Logger/Macros.hpp"
#include "../../shared/Components/EnemyTypeComponent.hpp"
#include "../../shared/Components/PlayerIdComponent.hpp"
#include "../../shared/Components/TransformComponent.hpp"
#include "../Components/RotationComponent.hpp"

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
        [&](auto entity, const shared::PlayerIdComponent&, const TransformComponent& transform) {
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

    int chaserCount = 0;
    registry.view<EnemyTypeComponent, TransformComponent, Rotation>().each(
        [&](auto entity, const EnemyTypeComponent& enemyType,
            const TransformComponent& transform, Rotation& rotation) {
            if (enemyType.variant != EnemyVariant::Chaser) {
                return;
            }
            chaserCount++;
            float dx = targetX - transform.x;
            float dy = targetY - transform.y;
            float angleRad = std::atan2(dy, dx);
            float angleDeg = angleRad * 180.0f / 3.14159265f;
            rotation.angle = angleDeg;
        });
}

}  // namespace rtype::games::rtype::client
