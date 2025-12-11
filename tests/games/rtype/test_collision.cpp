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
    CollisionFixture()
        : system([](const rtype::engine::GameEvent&) {}, 1920.0F, 1080.0F) {}

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
    }

    std::unique_ptr<ECS::Registry> registry;
    server::CollisionSystem system;
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

TEST_F(CollisionFixture, ProjectileHitsPlayerWithHealthReducesHealth) {
    bool eventEmitted = false;
    uint32_t eventNetworkId = 0;
    int32_t eventHealthCurrent = 0;
    int32_t eventHealthMax = 0;

    server::CollisionSystem systemWithCallback(
        [&](const rtype::engine::GameEvent& event) {
            if (event.type ==
                rtype::engine::GameEventType::EntityHealthChanged) {
                eventEmitted = true;
                eventNetworkId = event.entityNetworkId;
                eventHealthCurrent = event.healthCurrent;
                eventHealthMax = event.healthMax;
            }
        });

    auto projectile = registry->spawnEntity();
    // Use positions well within world bounds (0,0 to 1920,1080)
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    // Add ProjectileComponent with Enemy owner (damage=1) so it can hit players
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 1, 0U, shared::ProjectileOwner::Enemy,
        shared::ProjectileType::EnemyBullet);

    auto player = registry->spawnEntity();
    // Position player overlapping with projectile
    registry->emplaceComponent<shared::TransformComponent>(player, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 42);

    systemWithCallback.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::HealthComponent>(player));
    auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 99);  // Damage of 1 applied
    EXPECT_TRUE(health.isAlive());
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(player));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));

    EXPECT_TRUE(eventEmitted);
    EXPECT_EQ(eventNetworkId, 42);
    EXPECT_EQ(eventHealthCurrent, 99);
    EXPECT_EQ(eventHealthMax, 100);
}

TEST_F(CollisionFixture, ProjectileKillsPlayerAtLowHealth) {
    bool eventEmitted = false;
    int32_t eventHealthCurrent = 0;

    server::CollisionSystem systemWithCallback(
        [&](const rtype::engine::GameEvent& event) {
            if (event.type ==
                rtype::engine::GameEventType::EntityHealthChanged) {
                eventEmitted = true;
                eventHealthCurrent = event.healthCurrent;
            }
        });

    auto projectile = registry->spawnEntity();
    // Use positions well within world bounds (0,0 to 1920,1080)
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    // Add ProjectileComponent with Enemy owner (damage=1) so it can hit players
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 1, 0U, shared::ProjectileOwner::Enemy,
        shared::ProjectileType::EnemyBullet);

    auto player = registry->spawnEntity();
    // Position player overlapping with projectile
    registry->emplaceComponent<shared::TransformComponent>(player, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 1, 100);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 99);

    systemWithCallback.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::HealthComponent>(player));
    auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 0);
    EXPECT_FALSE(health.isAlive());
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));

    EXPECT_TRUE(eventEmitted);
    EXPECT_EQ(eventHealthCurrent, 0);
}

TEST_F(CollisionFixture, PlayerProjectileDoesNotHitPlayer) {
    // Player projectiles should not hit players (friendly fire off)
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    system.update(*registry, 0.0F);

    // Player projectile should NOT hit player
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(player));
    auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage
}

TEST_F(CollisionFixture, EnemyProjectileDoesNotHitEnemy) {
    // Enemy projectiles should not hit enemies (friendly fire off)
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Enemy,
        shared::ProjectileType::EnemyBullet);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);

    system.update(*registry, 0.0F);

    // Enemy projectile should NOT hit enemy
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
    auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_EQ(health.current, 100);  // No damage
}

TEST_F(CollisionFixture, NeutralProjectileHitsEveryone) {
    // Neutral projectiles should hit both players and enemies
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Neutral,
        shared::ProjectileType::BasicBullet);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    system.update(*registry, 0.0F);

    // Neutral projectile should hit player
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
}

TEST_F(CollisionFixture, PiercingProjectileDoesNotGetDestroyed) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    shared::ProjectileComponent projComp(10, 0U, shared::ProjectileOwner::Player,
                                          shared::ProjectileType::LaserBeam);
    projComp.piercing = true;
    projComp.maxHits = 3;
    registry->emplaceComponent<shared::ProjectileComponent>(projectile, projComp);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    // Piercing projectile should NOT be destroyed after first hit
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionFixture, ProjectileWithDestroyTagSkipped) {
    // Projectiles already marked for destruction should be skipped
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 10, 0U, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);
    registry->emplaceComponent<shared::DestroyTag>(projectile);  // Already destroyed

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);

    system.update(*registry, 0.0F);

    // Enemy should NOT be damaged since projectile was already destroyed
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
    auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_EQ(health.current, 100);
}

TEST_F(CollisionFixture, EnemyWithHealthTakesDamage) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 200.0F,
                                                            200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 25, 0U, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 202.0F, 200.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);

    system.update(*registry, 0.0F);

    auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_EQ(health.current, 75);  // 100 - 25 damage
    EXPECT_TRUE(health.isAlive());
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
}

