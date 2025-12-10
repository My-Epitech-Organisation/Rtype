/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_collision
*/

#include <gtest/gtest.h>

#include "ECS.hpp"
#include "games/rtype/server/Systems/Collision/CollisionSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "rtype/engine.hpp"

using namespace rtype::games::rtype;

namespace {

struct CollisionFixture : public ::testing::Test {
    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
    }

    std::unique_ptr<ECS::Registry> registry;
    server::CollisionSystem system{1920.0F, 1080.0F};  // Default screen dimensions
};

}  // namespace

TEST_F(CollisionFixture, OverlapMarksDestroyOnEnemyAndProjectile) {
    // Use positions well within world bounds (0,0 to 1920,1080)
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F,
                                                            100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    // Add ProjectileComponent with Player owner so it can hit enemies
    // Constructor: (damage, ownerId, ownerType, projType)
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);

    auto enemy = registry->spawnEntity();
    // Position enemy overlapping with projectile
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionFixture, NoOverlapLeavesEntitiesIntact) {
    // Use positions well within world bounds (0,0 to 1920,1080)
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F,
                                                            100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    // Add ProjectileComponent with Player owner so it can hit enemies
    // Constructor: (damage, ownerId, ownerType, projType)
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);

    auto enemy = registry->spawnEntity();
    // Position enemy far away, no overlap
    registry->emplaceComponent<shared::TransformComponent>(enemy, 500.0F,
                                                           500.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionFixture, ProjectileHitsPlayerMarksBothDestroyed) {
    // Use positions well within world bounds (0,0 to 1920,1080)
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    // Add ProjectileComponent with Enemy owner so it can hit players
    // Constructor: (damage, ownerId, ownerType, projType)
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Enemy,
        shared::ProjectileType::EnemyBullet);

    auto player = registry->spawnEntity();
    // Position player overlapping with projectile
    registry->emplaceComponent<shared::TransformComponent>(player, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
}
