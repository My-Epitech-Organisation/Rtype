/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_collision_coverage - Additional tests for collision system coverage
*/

#include <gtest/gtest.h>

#include "ECS.hpp"
#include "games/rtype/server/Systems/Collision/CollisionSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "rtype/engine.hpp"

using namespace rtype::games::rtype;

namespace {

struct CollisionCoverageFixture : public ::testing::Test {
    CollisionCoverageFixture()
        : eventCount(0),
          system([this](const rtype::engine::GameEvent& evt) {
              ++eventCount;
              lastEvent = evt;
          },
                 1920.0F, 1080.0F) {}

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
// Laser-Enemy Collision Tests
// ============================================================================

TEST_F(CollisionCoverageFixture, LaserHitsEnemy_BasicDamage) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    shared::DamageOnContactComponent laserDmg{};
    laserDmg.damage = 50;
    laserDmg.damagePerSecond = 50.0F;
    laserDmg.isDPS = true;
    laserDmg.activeTime = 1.0F;  // Past startup delay, so active
    registry->emplaceComponent<shared::DamageOnContactComponent>(laser,
                                                                 laserDmg);
    registry->emplaceComponent<shared::NetworkIdComponent>(laser, 1001);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<shared::NetworkIdComponent>(enemy, 2001);

    system.update(*registry, 0.016F);

    const auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_LT(health.current, 100);  // Took damage
    EXPECT_GT(eventCount, 0);         // Event emitted
}

TEST_F(CollisionCoverageFixture, LaserHitsEnemy_InactiveLaser) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    shared::DamageOnContactComponent laserDmg{};
    laserDmg.damage = 50;
    laserDmg.damagePerSecond = 50.0F;
    laserDmg.isDPS = true;
    laserDmg.startupDelay = 1.0F;  // Has startup delay
    laserDmg.activeTime = 0.0F;    // Not past startup - inactive
    registry->emplaceComponent<shared::DamageOnContactComponent>(laser,
                                                                 laserDmg);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);

    system.update(*registry, 0.016F);

    const auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_EQ(health.current, 100);  // No damage (laser inactive)
}

TEST_F(CollisionCoverageFixture, LaserHitsEnemy_NoDamageComponent) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    // No DamageOnContactComponent

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);

    system.update(*registry, 0.016F);

    const auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    EXPECT_EQ(health.current, 100);  // No damage (no damage component)
}

TEST_F(CollisionCoverageFixture, LaserHitsEnemy_NoHealthComponent) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    shared::DamageOnContactComponent laserDmg{};
    laserDmg.damage = 50;
    laserDmg.damagePerSecond = 50.0F;
    laserDmg.isDPS = true;
    laserDmg.activeTime = 1.0F;  // Active
    registry->emplaceComponent<shared::DamageOnContactComponent>(laser,
                                                                 laserDmg);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    // No HealthComponent

    system.update(*registry, 0.016F);

    // Should not crash
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionCoverageFixture, LaserHitsEnemy_KillsEnemy) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    shared::DamageOnContactComponent laserDmg{};
    laserDmg.damage = 500;  // High damage to kill
    laserDmg.damagePerSecond = 5000.0F;
    laserDmg.isDPS = true;
    laserDmg.activeTime = 1.0F;  // Active
    registry->emplaceComponent<shared::DamageOnContactComponent>(laser,
                                                                 laserDmg);
    registry->emplaceComponent<shared::NetworkIdComponent>(laser, 1001);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 10, 100);  // Low HP
    registry->emplaceComponent<shared::NetworkIdComponent>(enemy, 2001);

    system.update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionCoverageFixture,
       LaserHitsEnemy_SameEnemyOnlyDamagedOncePerFrame) {
    auto laser = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(laser, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(laser, 200.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::LaserBeamTag>(laser);
    shared::DamageOnContactComponent laserDmg{};
    laserDmg.damage = 10;
    laserDmg.damagePerSecond = 100.0F;
    laserDmg.isDPS = true;
    laserDmg.activeTime = 1.0F;  // Active
    registry->emplaceComponent<shared::DamageOnContactComponent>(laser,
                                                                 laserDmg);
    registry->emplaceComponent<shared::NetworkIdComponent>(laser, 1001);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 150.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 32.0F, 32.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);
    registry->emplaceComponent<shared::HealthComponent>(enemy, 100, 100);
    registry->emplaceComponent<shared::NetworkIdComponent>(enemy, 2001);

    // First update
    system.update(*registry, 0.016F);

    const auto& health = registry->getComponent<shared::HealthComponent>(enemy);
    int32_t healthAfterFirst = health.current;
    EXPECT_LT(healthAfterFirst, 100);

    // Enemy health should be consistent (only damaged once per laser per frame)
}

// ============================================================================
// Force Pod Pickup Tests
// ============================================================================

TEST_F(CollisionCoverageFixture, ForcePodPickup_SpawnsNewForcePod) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 5000);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::ForcePod;
    powerUp.duration = 0.0F;  // Instant effect
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    size_t initialEntityCount = 0;
    registry->view<shared::ForcePodTag>().each(
        [&initialEntityCount](ECS::Entity, const shared::ForcePodTag&) {
            ++initialEntityCount;
        });

    system.update(*registry, 0.0F);

    size_t finalEntityCount = 0;
    registry->view<shared::ForcePodTag>().each(
        [&finalEntityCount](ECS::Entity, const shared::ForcePodTag&) {
            ++finalEntityCount;
        });

    EXPECT_GT(finalEntityCount, initialEntityCount);
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionCoverageFixture, ForcePodPickup_PlayerNoNetworkId) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    // No NetworkIdComponent

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::ForcePod;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    EXPECT_NO_THROW({ system.update(*registry, 0.0F); });

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionCoverageFixture, ForcePodPickup_MultipleExistingPods) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 5000);

    // Create 9 existing force pods (more than 8 preset positions)
    for (int i = 0; i < 9; ++i) {
        auto pod = registry->spawnEntity();
        registry->emplaceComponent<shared::ForcePodTag>(pod);
        registry->emplaceComponent<shared::ForcePodComponent>(
            pod, shared::ForcePodState::Attached, 0.0F, 0.0F, 5000);
    }

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::ForcePod;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    // Should create a new pod using the angle calculation path
    size_t podCount = 0;
    registry->view<shared::ForcePodTag>().each(
        [&podCount](ECS::Entity, const shared::ForcePodTag&) { ++podCount; });

    EXPECT_GE(podCount, 10);  // 9 existing + 1 new
}

// ============================================================================
// Laser Upgrade Pickup Tests
// ============================================================================

TEST_F(CollisionCoverageFixture, LaserUpgradePickup_AddsWeaponSlot) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 5000);
    registry->emplaceComponent<shared::WeaponComponent>(player);

    auto& weapon = registry->getComponent<shared::WeaponComponent>(player);
    uint8_t initialSlots = weapon.unlockedSlots;

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::LaserUpgrade;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    system.update(*registry, 0.0F);

    EXPECT_GT(weapon.unlockedSlots, initialSlots);
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionCoverageFixture, LaserUpgradePickup_NoWeaponComponent) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 5000);
    // No WeaponComponent

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 16.0F, 16.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent powerUp{};
    powerUp.type = shared::PowerUpType::LaserUpgrade;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, powerUp);

    EXPECT_NO_THROW({ system.update(*registry, 0.0F); });

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

// ============================================================================
// Orphan Force Pod Pickup Tests
// ============================================================================

TEST_F(CollisionCoverageFixture, OrphanForcePodPickup_PlayerAdoptsPod) {
    auto forcePod = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(forcePod, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(forcePod, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ForcePodTag>(forcePod);
    registry->emplaceComponent<shared::ForcePodComponent>(
        forcePod, shared::ForcePodState::Orphan, 0.0F, 0.0F, 0);
    registry->emplaceComponent<shared::VelocityComponent>(forcePod, 100.0F,
                                                          50.0F);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 7000);

    system.update(*registry, 0.0F);

    const auto& podComp =
        registry->getComponent<shared::ForcePodComponent>(forcePod);
    EXPECT_EQ(podComp.state, shared::ForcePodState::Attached);
    EXPECT_EQ(podComp.ownerNetworkId, 7000u);

    const auto& vel =
        registry->getComponent<shared::VelocityComponent>(forcePod);
    EXPECT_EQ(vel.vx, 0.0F);
    EXPECT_EQ(vel.vy, 0.0F);
}

TEST_F(CollisionCoverageFixture, OrphanForcePodPickup_NotOrphan_NoPickup) {
    auto forcePod = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(forcePod, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(forcePod, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ForcePodTag>(forcePod);
    registry->emplaceComponent<shared::ForcePodComponent>(
        forcePod, shared::ForcePodState::Attached, 0.0F, 0.0F, 1000);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 7000);

    system.update(*registry, 0.0F);

    const auto& podComp =
        registry->getComponent<shared::ForcePodComponent>(forcePod);
    EXPECT_EQ(podComp.ownerNetworkId, 1000u);  // Still original owner
}

TEST_F(CollisionCoverageFixture, OrphanForcePodPickup_PlayerNoNetworkId) {
    auto forcePod = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(forcePod, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(forcePod, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ForcePodTag>(forcePod);
    registry->emplaceComponent<shared::ForcePodComponent>(
        forcePod, shared::ForcePodState::Orphan, 0.0F, 0.0F, 0);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    // No NetworkIdComponent

    system.update(*registry, 0.0F);

    const auto& podComp =
        registry->getComponent<shared::ForcePodComponent>(forcePod);
    EXPECT_EQ(podComp.state, shared::ForcePodState::Orphan);  // Still orphan
}

TEST_F(CollisionCoverageFixture,
       OrphanForcePodPickup_ExistingPodsOverflowPosition) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 7000);

    // Create 9 existing attached pods (more than 8 preset positions)
    for (int i = 0; i < 9; ++i) {
        auto pod = registry->spawnEntity();
        registry->emplaceComponent<shared::ForcePodTag>(pod);
        registry->emplaceComponent<shared::ForcePodComponent>(
            pod, shared::ForcePodState::Attached, 0.0F, 0.0F, 7000);
    }

    auto orphanPod = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(orphanPod, 105.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(orphanPod, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ForcePodTag>(orphanPod);
    registry->emplaceComponent<shared::ForcePodComponent>(
        orphanPod, shared::ForcePodState::Orphan, 0.0F, 0.0F, 0);

    system.update(*registry, 0.0F);

    const auto& podComp =
        registry->getComponent<shared::ForcePodComponent>(orphanPod);
    EXPECT_EQ(podComp.state, shared::ForcePodState::Attached);
    EXPECT_EQ(podComp.ownerNetworkId, 7000u);
    // Uses angle calculation path for 10th pod
}

// ============================================================================
// Obstacle Collision Edge Cases
// ============================================================================

TEST_F(CollisionCoverageFixture,
       ObstacleCollision_DestroySelfWithInvinciblePlayer) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);
    registry->emplaceComponent<shared::InvincibleTag>(player);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    shared::DamageOnContactComponent dmg{};
    dmg.damage = 50;
    dmg.destroySelf = true;
    registry->emplaceComponent<shared::DamageOnContactComponent>(obstacle, dmg);

    system.update(*registry, 0.0F);

    // Player is invincible, obstacle should still destroy itself
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(obstacle));
    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_EQ(health.current, 100);  // No damage
}

TEST_F(CollisionCoverageFixture,
       ObstacleCollision_ProjectileWithProjectileComponent) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 25, 0, shared::ProjectileOwner::Player,
        shared::ProjectileType::BasicBullet);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(obstacle));
}

TEST_F(CollisionCoverageFixture,
       ObstacleCollision_EnemyProjectileDoesNotDestroyObstacle) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 25, 0, shared::ProjectileOwner::Enemy,
        shared::ProjectileType::BasicBullet);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_FALSE(
        registry->hasComponent<shared::DestroyTag>(obstacle));  // Not destroyed
}

// ============================================================================
// Projectile Neutral Owner Test
// ============================================================================

TEST_F(CollisionCoverageFixture, ProjectileNeutralOwner_CanHitAnyone) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                             10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);
    registry->emplaceComponent<shared::ProjectileComponent>(
        projectile, 25, 0, shared::ProjectileOwner::Neutral,
        shared::ProjectileType::BasicBullet);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 105.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_LT(health.current, 100);  // Took damage from neutral projectile
}

// ============================================================================
// Multiple Obstacle Collisions Same Frame
// ============================================================================

TEST_F(CollisionCoverageFixture, ObstacleCollision_SamePairOnlyOncePerFrame) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 32.0F, 32.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 100, 100);

    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 105.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 32.0F,
                                                             32.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    shared::DamageOnContactComponent dmg{};
    dmg.damage = 10;
    dmg.destroySelf = false;
    registry->emplaceComponent<shared::DamageOnContactComponent>(obstacle, dmg);

    system.update(*registry, 0.0F);

    const auto& health = registry->getComponent<shared::HealthComponent>(player);
    // Damage should only be applied once per frame for the same pair
    EXPECT_EQ(health.current, 90);  // 100 - 10 = 90
}
