/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#include "CollisionSystem.hpp"

#include <vector>

#include "../../../shared/Components.hpp"
#include "../../../shared/Systems/Collision/AABB.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BoundingBoxComponent;
using shared::CollisionPair;
using shared::DestroyTag;
using shared::EnemyProjectileTag;
using shared::EnemyTag;
using shared::HealthComponent;
using shared::PlayerProjectileTag;
using shared::PlayerTag;
using shared::ProjectileComponent;
using shared::ProjectileOwner;
using shared::ProjectileTag;
using shared::QuadTreeSystem;
using shared::TransformComponent;
using shared::collision::overlaps;
using shared::collision::Rect;

CollisionSystem::CollisionSystem(float worldWidth, float worldHeight)
    : ASystem("CollisionSystem") {
    Rect worldBounds(0, 0, worldWidth, worldHeight);
    _quadTreeSystem = std::make_unique<QuadTreeSystem>(worldBounds, 10, 5);
}

void CollisionSystem::update(ECS::Registry& registry, float deltaTime) {
    _quadTreeSystem->update(registry, deltaTime);
    auto collisionPairs = _quadTreeSystem->queryCollisionPairs(registry);
    for (const auto& pair : collisionPairs) {
        ECS::Entity entityA = pair.entityA;
        ECS::Entity entityB = pair.entityB;

        if (!registry.isAlive(entityA) || !registry.isAlive(entityB)) {
            continue;
        }
        if (registry.hasComponent<DestroyTag>(entityA) ||
            registry.hasComponent<DestroyTag>(entityB)) {
            continue;
        }

        if (!registry.hasComponent<TransformComponent>(entityA) ||
            !registry.hasComponent<TransformComponent>(entityB) ||
            !registry.hasComponent<BoundingBoxComponent>(entityA) ||
            !registry.hasComponent<BoundingBoxComponent>(entityB)) {
            continue;
        }
        const auto& transformA =
            registry.getComponent<TransformComponent>(entityA);
        const auto& transformB =
            registry.getComponent<TransformComponent>(entityB);
        const auto& boxA = registry.getComponent<BoundingBoxComponent>(entityA);
        const auto& boxB = registry.getComponent<BoundingBoxComponent>(entityB);

        if (!overlaps(transformA, boxA, transformB, boxB)) {
            continue;
        }

        bool aIsProjectile = registry.hasComponent<ProjectileTag>(entityA);
        bool bIsProjectile = registry.hasComponent<ProjectileTag>(entityB);
        bool aIsEnemy = registry.hasComponent<EnemyTag>(entityA);
        bool bIsEnemy = registry.hasComponent<EnemyTag>(entityB);
        bool aIsPlayer = registry.hasComponent<PlayerTag>(entityA);
        bool bIsPlayer = registry.hasComponent<PlayerTag>(entityB);

        if (aIsProjectile && (bIsEnemy || bIsPlayer)) {
            handleProjectileCollision(registry, entityA, entityB, bIsPlayer);
        } else if (bIsProjectile && (aIsEnemy || aIsPlayer)) {
            handleProjectileCollision(registry, entityB, entityA, aIsPlayer);
        }
    }
}

void CollisionSystem::handleProjectileCollision(ECS::Registry& registry,
                                                ECS::Entity projectile,
                                                ECS::Entity target,
                                                bool isTargetPlayer) {
    if (registry.hasComponent<DestroyTag>(projectile) ||
        registry.hasComponent<DestroyTag>(target)) {
        return;
    }
    ProjectileOwner projOwner = ProjectileOwner::Neutral;
    int32_t damage = 25;
    bool piercing = false;

    if (registry.hasComponent<ProjectileComponent>(projectile)) {
        const auto& projComp =
            registry.getComponent<ProjectileComponent>(projectile);
        projOwner = projComp.owner;
        damage = projComp.damage;
        piercing = projComp.piercing;
    } else {
        if (registry.hasComponent<PlayerProjectileTag>(projectile)) {
            projOwner = ProjectileOwner::Player;
        } else if (registry.hasComponent<EnemyProjectileTag>(projectile)) {
            projOwner = ProjectileOwner::Enemy;
        }
    }
    bool canHit = false;
    if (projOwner == ProjectileOwner::Neutral) {
        canHit = true;
    } else if (projOwner == ProjectileOwner::Player && !isTargetPlayer) {
        canHit = true;
    } else if (projOwner == ProjectileOwner::Enemy && isTargetPlayer) {
        canHit = true;
    }

    if (!canHit) {
        return;
    }

    LOG_DEBUG("[CollisionSystem] Collision detected! Projectile "
              << projectile.id << " hit target " << target.id
              << " (isPlayer=" << isTargetPlayer << ")");

    if (registry.hasComponent<HealthComponent>(target)) {
        auto& health = registry.getComponent<HealthComponent>(target);
        health.takeDamage(damage);
        if (!health.isAlive()) {
            LOG_DEBUG("[CollisionSystem] Target " << target.id
                                                  << " destroyed (no health)");
            registry.emplaceComponent<DestroyTag>(target, DestroyTag{});
        }
    } else {
        LOG_DEBUG("[CollisionSystem] Target "
                  << target.id << " destroyed (no HealthComponent)");
        registry.emplaceComponent<DestroyTag>(target, DestroyTag{});
    }
    if (!piercing) {
        LOG_DEBUG("[CollisionSystem] Projectile "
                  << projectile.id << " destroyed (non-piercing)");
        registry.emplaceComponent<DestroyTag>(projectile, DestroyTag{});
    } else if (registry.hasComponent<ProjectileComponent>(projectile)) {
        auto& projComp = registry.getComponent<ProjectileComponent>(projectile);
        if (projComp.registerHit()) {
            LOG_DEBUG("[CollisionSystem] Projectile "
                      << projectile.id << " destroyed (max hits)");
            registry.emplaceComponent<DestroyTag>(projectile, DestroyTag{});
        }
    }
}

}  // namespace rtype::games::rtype::server
