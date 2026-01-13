/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_forcepod_collision - Integration tests for Force Pod collision
*/

#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include "games/rtype/server/Systems/Collision/CollisionSystem.hpp"
#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/ForcePodComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/ProjectileComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;

class ForcePodCollisionTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<CollisionSystem> collisionSystem;
    std::vector<::rtype::engine::GameEvent> events;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        
        auto emitter = [this](const ::rtype::engine::GameEvent& event) {
            events.push_back(event);
        };
        
        collisionSystem = std::make_unique<CollisionSystem>(emitter);
    }

    void TearDown() override {
        collisionSystem.reset();
        registry.reset();
        events.clear();
    }
};

TEST_F(ForcePodCollisionTest, AttachedPodBlocksEnemyProjectile) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 1000);
    registry->emplaceComponent<TransformComponent>(forcePod, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile);
    registry->emplaceComponent<EnemyProjectileTag>(projectile);
    registry->emplaceComponent<TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile, 16.0F, 8.0F);
    registry->emplaceComponent<ProjectileComponent>(projectile, 25, 0,
                                                    ProjectileOwner::Enemy,
                                                    ProjectileType::BasicBullet);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<DestroyTag>(forcePod));
}

TEST_F(ForcePodCollisionTest, AttachedPodDestroysWeakEnemy) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 2000);
    registry->emplaceComponent<TransformComponent>(forcePod, 200.0F, 200.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity enemy = registry->spawnEntity();
    registry->emplaceComponent<EnemyTag>(enemy);
    registry->emplaceComponent<TransformComponent>(enemy, 200.0F, 200.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(enemy, 48.0F, 48.0F);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<DestroyTag>(enemy));
    EXPECT_FALSE(registry->hasComponent<DestroyTag>(forcePod));
}

TEST_F(ForcePodCollisionTest, AttachedPodDoesNotDestroyEnemyWithHealth) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 3000);
    registry->emplaceComponent<TransformComponent>(forcePod, 300.0F, 300.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity enemy = registry->spawnEntity();
    registry->emplaceComponent<EnemyTag>(enemy);
    registry->emplaceComponent<TransformComponent>(enemy, 300.0F, 300.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(enemy, 48.0F, 48.0F);
    registry->emplaceComponent<HealthComponent>(enemy, 100, 100);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<DestroyTag>(enemy));
    EXPECT_FALSE(registry->hasComponent<DestroyTag>(forcePod));
}

TEST_F(ForcePodCollisionTest, DetachedPodDoesNotBlockProjectile) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Detached,
                                                  0.0F, 0.0F, 4000);
    registry->emplaceComponent<TransformComponent>(forcePod, 400.0F, 400.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile);
    registry->emplaceComponent<EnemyProjectileTag>(projectile);
    registry->emplaceComponent<TransformComponent>(projectile, 400.0F, 400.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile, 16.0F, 8.0F);
    registry->emplaceComponent<ProjectileComponent>(projectile, 25, 0,
                                                    ProjectileOwner::Enemy,
                                                    ProjectileType::BasicBullet);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<DestroyTag>(projectile));
}

TEST_F(ForcePodCollisionTest, ReturningPodDoesNotBlockProjectile) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Returning,
                                                  0.0F, 0.0F, 5000);
    registry->emplaceComponent<TransformComponent>(forcePod, 500.0F, 500.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile);
    registry->emplaceComponent<EnemyProjectileTag>(projectile);
    registry->emplaceComponent<TransformComponent>(projectile, 500.0F, 500.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile, 16.0F, 8.0F);
    registry->emplaceComponent<ProjectileComponent>(projectile, 25, 0,
                                                    ProjectileOwner::Enemy,
                                                    ProjectileType::BasicBullet);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<DestroyTag>(projectile));
}

TEST_F(ForcePodCollisionTest, PodDoesNotBlockPlayerProjectile) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 6000);
    registry->emplaceComponent<TransformComponent>(forcePod, 600.0F, 600.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile);
    registry->emplaceComponent<PlayerProjectileTag>(projectile);
    registry->emplaceComponent<TransformComponent>(projectile, 600.0F, 600.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile, 16.0F, 8.0F);
    registry->emplaceComponent<ProjectileComponent>(projectile, 25, 0,
                                                    ProjectileOwner::Player,
                                                    ProjectileType::BasicBullet);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<DestroyTag>(projectile));
}

TEST_F(ForcePodCollisionTest, PodBlocksMultipleProjectiles) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 7000);
    registry->emplaceComponent<TransformComponent>(forcePod, 700.0F, 700.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile1 = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile1);
    registry->emplaceComponent<EnemyProjectileTag>(projectile1);
    registry->emplaceComponent<TransformComponent>(projectile1, 700.0F, 700.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile1, 16.0F, 8.0F);

    ECS::Entity projectile2 = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile2);
    registry->emplaceComponent<EnemyProjectileTag>(projectile2);
    registry->emplaceComponent<TransformComponent>(projectile2, 705.0F, 705.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile2, 16.0F, 8.0F);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<DestroyTag>(projectile1));
    EXPECT_TRUE(registry->hasComponent<DestroyTag>(projectile2));
    EXPECT_FALSE(registry->hasComponent<DestroyTag>(forcePod));
}

TEST_F(ForcePodCollisionTest, NoCollisionWhenNotOverlapping) {
    ECS::Entity forcePod = registry->spawnEntity();
    registry->emplaceComponent<ForcePodTag>(forcePod);
    registry->emplaceComponent<ForcePodComponent>(forcePod, ForcePodState::Attached,
                                                  0.0F, 0.0F, 8000);
    registry->emplaceComponent<TransformComponent>(forcePod, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F, 32.0F);

    ECS::Entity projectile = registry->spawnEntity();
    registry->emplaceComponent<ProjectileTag>(projectile);
    registry->emplaceComponent<EnemyProjectileTag>(projectile);
    registry->emplaceComponent<TransformComponent>(projectile, 500.0F, 500.0F, 0.0F);
    registry->emplaceComponent<BoundingBoxComponent>(projectile, 16.0F, 8.0F);

    collisionSystem->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<DestroyTag>(forcePod));
}
