/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserExplosionSystem.cpp
*/

#include "ChaserExplosionSystem.hpp"

#include "Logger/Macros.hpp"
#include "../../shared/Components/EnemyTypeComponent.hpp"
#include "../Components/AnnimationComponent.hpp"
#include "../Components/ChaserExplosionComponent.hpp"

namespace rtype::games::rtype::client {

ChaserExplosionSystem::ChaserExplosionSystem()
    : ::rtype::engine::ASystem("ChaserExplosionSystem") {}

void ChaserExplosionSystem::update(ECS::Registry& registry, float dt) {
    using namespace ::rtype::games::rtype::shared;

    registry.view<EnemyTypeComponent, ChaserExplosion, Animation>().each(
        [&](auto entity, const EnemyTypeComponent& enemyType,
            ChaserExplosion& explosion, Animation& anim) {
            if (enemyType.variant != EnemyVariant::Chaser) {
                return;
            }
            
            if (!explosion.isExploding) {
                // Keep chaser on frame 1 (normal sprite) until explosion triggers
                anim.currentFrame = 1;
                anim.elapsedTime = 0.0f;
            } else {
                // During explosion, let animation play frames 2-6
                // Once it reaches the end, freeze on final frame
                if (anim.currentFrame >= anim.frameCount) {
                    anim.currentFrame = anim.frameCount;
                    anim.elapsedTime = 0.0f;
                }
            }
        });
}

}  // namespace rtype::games::rtype::client
