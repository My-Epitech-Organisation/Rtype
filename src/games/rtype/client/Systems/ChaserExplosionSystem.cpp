/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserExplosionSystem.cpp
*/

#include "ChaserExplosionSystem.hpp"

#include "../../shared/Components/EnemyTypeComponent.hpp"
#include "../Components/AnnimationComponent.hpp"
#include "../Components/ChaserExplosionComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

ChaserExplosionSystem::ChaserExplosionSystem()
    : ::rtype::engine::ASystem("ChaserExplosionSystem") {}

void ChaserExplosionSystem::update(ECS::Registry& registry, float dt) {
    using ::rtype::games::rtype::shared::EnemyTypeComponent;
    using ::rtype::games::rtype::shared::EnemyVariant;

    registry.view<EnemyTypeComponent, ChaserExplosion, Animation>().each(
        [&](auto entity, const EnemyTypeComponent& enemyType,
            ChaserExplosion& explosion, Animation& anim) {
            if (enemyType.variant != EnemyVariant::Chaser) {
                return;
            }

            if (!explosion.isExploding) {
                anim.currentFrame = 1;
                anim.elapsedTime = 0.0f;
            } else {
                if (anim.currentFrame >= anim.frameCount) {
                    anim.currentFrame = anim.frameCount;
                    anim.elapsedTime = 0.0f;
                }
            }
        });
}

}  // namespace rtype::games::rtype::client
