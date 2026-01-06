/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_collision_branches - Additional tests for collision system branch coverage
*/

#include <gtest/gtest.h>

#include "ECS.hpp"
#include "games/rtype/server/Systems/Collision/CollisionSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "rtype/engine.hpp"

using namespace rtype::games::rtype;

namespace {

struct CollisionBranchFixture : public ::testing::Test {
    CollisionBranchFixture()
        : eventCount(0),
          system([this](const rtype::engine::GameEvent& evt) {
              ++eventCount;
              lastEvent = evt;
          }, 1920.0F, 1080.0F) {}

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        eventCount = 0;
    }

    std::unique_ptr<ECS::Registry> registry;
    server::CollisionSystem system;
    int eventCount;
    rtype::engine::GameEvent lastEvent;
};

}  // namespace

// ============================================================================
// Pickup Collision Tests
// ============================================================================

TEST_F(CollisionBranchFixture, PickupWithNoPowerUpComponent) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    // No PowerUpComponent

    system.update(*registry, 0.0F);

    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionBranchFixture, PickupWithPowerUpTypeNone) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::None;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionBranchFixture, SpeedBoostPickup) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 1234);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::SpeedBoost;
    powerUp.duration = 5.0F;
    powerUp.magnitude = 0.5F;  // 50% speed increase
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    const auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_EQ(active.type, shared::PowerUpType::SpeedBoost);
    EXPECT_FLOAT_EQ(active.speedMultiplier, 1.5F);
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionBranchFixture, ShieldPickup) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::Shield;
    powerUp.duration = 8.0F;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::InvincibleTag>(player));
    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    const auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_TRUE(active.shieldActive);
}

TEST_F(CollisionBranchFixture, RapidFirePickup) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::ShootCooldownComponent>(player, 0.5F);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::RapidFire;
    powerUp.duration = 5.0F;
    powerUp.magnitude = 1.0F;  // 2x fire rate
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    const auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_FLOAT_EQ(active.fireRateMultiplier, 2.0F);
    EXPECT_TRUE(active.hasOriginalCooldown);
    
    const auto& cooldown = registry->getComponent<shared::ShootCooldownComponent>(player);
    EXPECT_LT(cooldown.cooldownTime, 0.5F);  // Should be reduced
}

TEST_F(CollisionBranchFixture, DoubleDamagePickup) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::DoubleDamage;
    powerUp.duration = 10.0F;
    powerUp.magnitude = 1.0F;  // 2x damage
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    const auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_FLOAT_EQ(active.damageMultiplier, 2.0F);
}

TEST_F(CollisionBranchFixture, HealthBoostPickup) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 50, 100);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::HealthBoost;
    powerUp.magnitude = 0.3F;  // 30 health
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 80);  // 50 + 30
}

TEST_F(CollisionBranchFixture, PickupReplacesExistingPowerUpWithShield) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::ShootCooldownComponent>(player, 0.5F);
    registry->emplaceComponent<shared::InvincibleTag>(player);
    
    // Add existing powerup with shield
    shared::ActivePowerUpComponent existingPowerUp{};
    existingPowerUp.shieldActive = true;
    existingPowerUp.hasOriginalCooldown = true;
    existingPowerUp.originalCooldown = 0.5F;
    registry->emplaceComponent<shared::ActivePowerUpComponent>(player, existingPowerUp);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::SpeedBoost;
    powerUp.duration = 5.0F;
    powerUp.magnitude = 0.5F;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_FALSE(registry->hasComponent<shared::InvincibleTag>(player));
    const auto& cooldown = registry->getComponent<shared::ShootCooldownComponent>(player);
    EXPECT_FLOAT_EQ(cooldown.cooldownTime, 0.5F);  // Restored original
}

// ============================================================================
// Obstacle Collision Tests
// ============================================================================

TEST_F(CollisionBranchFixture, ObstacleHitsPlayerWithInvincibility) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);
    registry->emplaceComponent<shared::InvincibleTag>(player);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F, 32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    registry->emplaceComponent<shared::DamageOnContactComponent>(obstacle, 20, false);

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage due to invincibility
}

TEST_F(CollisionBranchFixture, ObstacleHitsPlayerNoHealthComponent) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    // No HealthComponent

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F, 32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
}

TEST_F(CollisionBranchFixture, ObstacleDestroysItself) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F, 32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    registry->emplaceComponent<shared::DamageOnContactComponent>(obstacle, 15, true);  // destroySelf = true

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(obstacle));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
}

TEST_F(CollisionBranchFixture, ObstacleNoDamageOnContact) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F, 32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    // No DamageOnContactComponent - uses default damage

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
}

// ============================================================================
// Enemy-Player Collision Tests
// ============================================================================

TEST_F(CollisionBranchFixture, EnemyPlayerCollisionNoDamageComponent) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    // No DamageOnContactComponent

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage without DamageOnContactComponent
}

TEST_F(CollisionBranchFixture, EnemyPlayerCollisionDestroysEnemy) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 5678);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::DamageOnContactComponent>(enemy, 30, true);  // destroySelf = true

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 70);
    EXPECT_GT(eventCount, 0);  // Event emitted
}

// ============================================================================
// Projectile Collision Edge Cases
// ============================================================================

TEST_F(CollisionBranchFixture, ProjectileWithoutProjectileComponent) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::PlayerProjectileTag>(projectile);  // Tag instead of component

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F, 10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionBranchFixture, ProjectileWithEnemyProjectileTag) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::EnemyProjectileTag>(projectile);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
}

TEST_F(CollisionBranchFixture, PiercingProjectileWithMaxHits) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    shared::ProjectileComponent projComp(25, 1, shared::ProjectileOwner::Player, shared::ProjectileType::BasicBullet);
    projComp.piercing = true;
    projComp.maxHits = 2;
    projComp.currentHits = 1;  // One hit away from max
    registry->emplaceComponent<shared::ProjectileComponent>(projectile, projComp);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F, 10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));  // Max hits reached
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionBranchFixture, ProjectileWithInvalidNetworkId) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 0);  // Check if ID 0 triggers event
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(projectile, 25, 0, shared::ProjectileOwner::Enemy, shared::ProjectileType::BasicBullet);

    system.update(*registry, 0.0F);

    // Collision should happen regardless of network ID
    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 75);  // Took damage
}

// ============================================================================
// Already Destroyed Entity Tests
// ============================================================================

TEST_F(CollisionBranchFixture, PickupAlreadyDestroyed) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    registry->emplaceComponent<shared::DestroyTag>(pickup);  // Already destroyed

    system.update(*registry, 0.0F);

    // Should not try to process destroyed entity
    EXPECT_FALSE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
}

TEST_F(CollisionBranchFixture, ObstacleAlreadyDestroyed) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F, 32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    registry->emplaceComponent<shared::DestroyTag>(obstacle);  // Already destroyed

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage
}

TEST_F(CollisionBranchFixture, EnemyAlreadyDestroyed) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 105.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::DestroyTag>(enemy);  // Already destroyed

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage
}

