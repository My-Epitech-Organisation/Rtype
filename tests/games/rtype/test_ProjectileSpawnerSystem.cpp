/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_ProjectileSpawnerSystem - Unit tests for ProjectileSpawnerSystem
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/server/Systems/Projectile/ProjectileSpawnerSystem.hpp"
#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../../lib/ecs/src/ECS.hpp"

using namespace rtype::games::rtype;

class ProjectileSpawnerSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        lastEvent = rtype::engine::GameEvent{};
        eventEmitted = false;
    }

    rtype::engine::GameEvent lastEvent;
    bool eventEmitted = false;

    server::ProjectileSpawnerSystem createSystem() {
        return server::ProjectileSpawnerSystem(
            [this](const rtype::engine::GameEvent& event) {
                lastEvent = event;
                eventEmitted = true;
            });
    }

    server::ProjectileSpawnerSystem createSystemWithConfig(
        server::ProjectileSpawnConfig config) {
        return server::ProjectileSpawnerSystem(
            [this](const rtype::engine::GameEvent& event) {
                lastEvent = event;
                eventEmitted = true;
            },
            config);
    }
};

// =============================================================================
// Construction Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, ConstructWithDefaultConfig) {
    auto system = createSystem();
    EXPECT_EQ(system.getProjectileCount(), 0U);
}

TEST_F(ProjectileSpawnerSystemTest, ConstructWithCustomConfig) {
    server::ProjectileSpawnConfig config;
    config.playerProjectileOffsetX = 50.0F;
    config.playerProjectileOffsetY = 10.0F;
    auto system = createSystemWithConfig(config);
    EXPECT_EQ(system.getProjectileCount(), 0U);
}

// =============================================================================
// Update Tests (Cooldown)
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, UpdateCooldownsDecrement) {
    auto system = createSystem();
    ECS::Registry registry;

    auto entity = registry.spawnEntity();
    registry.emplaceComponent<shared::ShootCooldownComponent>(entity, 1.0F);
    auto& cooldown = registry.getComponent<shared::ShootCooldownComponent>(entity);
    cooldown.triggerCooldown();

    EXPECT_FALSE(cooldown.canShoot());

    system.update(registry, 0.5F);
    EXPECT_FALSE(cooldown.canShoot());

    system.update(registry, 0.6F);
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(ProjectileSpawnerSystemTest, UpdateWithMultipleCooldowns) {
    auto system = createSystem();
    ECS::Registry registry;

    auto entity1 = registry.spawnEntity();
    auto entity2 = registry.spawnEntity();
    registry.emplaceComponent<shared::ShootCooldownComponent>(entity1, 0.5F);
    registry.emplaceComponent<shared::ShootCooldownComponent>(entity2, 1.0F);

    auto& cooldown1 = registry.getComponent<shared::ShootCooldownComponent>(entity1);
    auto& cooldown2 = registry.getComponent<shared::ShootCooldownComponent>(entity2);
    cooldown1.triggerCooldown();
    cooldown2.triggerCooldown();

    system.update(registry, 0.6F);

    EXPECT_TRUE(cooldown1.canShoot());
    EXPECT_FALSE(cooldown2.canShoot());
}

// =============================================================================
// Spawn Player Projectile Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, SpawnPlayerProjectileCreatesEntity) {
    auto system = createSystem();
    ECS::Registry registry;

    uint32_t networkId = system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    EXPECT_GT(networkId, 0U);
    EXPECT_EQ(system.getProjectileCount(), 1U);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnPlayerProjectileEmitsEvent) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    EXPECT_TRUE(eventEmitted);
    EXPECT_EQ(lastEvent.type, rtype::engine::GameEventType::EntitySpawned);
    EXPECT_EQ(lastEvent.entityType, static_cast<uint8_t>(shared::EntityType::Projectile));
}

TEST_F(ProjectileSpawnerSystemTest, SpawnPlayerProjectileHasCorrectComponents) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    auto view = registry.view<shared::ProjectileTag, shared::TransformComponent,
                               shared::VelocityComponent, shared::ProjectileComponent>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::ProjectileTag&,
                       shared::TransformComponent& transform,
                       shared::VelocityComponent& velocity,
                       shared::ProjectileComponent& proj) {
        count++;
        EXPECT_GT(transform.x, 100.0F);  // Offset applied
        EXPECT_GT(velocity.vx, 0.0F);    // Moving right
        EXPECT_EQ(proj.owner, shared::ProjectileOwner::Player);
    });
    EXPECT_EQ(count, 1);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnPlayerProjectileHasPlayerTag) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    auto view = registry.view<shared::PlayerProjectileTag>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::PlayerProjectileTag&) {
        count++;
    });
    EXPECT_EQ(count, 1);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnMultiplePlayerProjectiles) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 1, 0.0F, 0.0F);
    system.spawnPlayerProjectile(registry, 2, 100.0F, 100.0F);
    system.spawnPlayerProjectile(registry, 3, 200.0F, 200.0F);

    EXPECT_EQ(system.getProjectileCount(), 3U);
}

// =============================================================================
// Spawn Enemy Projectile Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileCreatesEntity) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    uint32_t networkId = system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 100.0F, 300.0F);

    EXPECT_GT(networkId, 0U);
    EXPECT_EQ(system.getProjectileCount(), 1U);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileEmitsEvent) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 100.0F, 300.0F);

    EXPECT_TRUE(eventEmitted);
    EXPECT_EQ(lastEvent.type, rtype::engine::GameEventType::EntitySpawned);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileHasEnemyTag) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 100.0F, 300.0F);

    auto view = registry.view<shared::EnemyProjectileTag>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::EnemyProjectileTag&) {
        count++;
    });
    EXPECT_EQ(count, 1);
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileAimsAtTarget) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    // Enemy at (500, 300), target at (100, 300) - should shoot left
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 100.0F, 300.0F);

    auto view = registry.view<shared::VelocityComponent, shared::EnemyProjectileTag>();
    view.each([](ECS::Entity /*e*/, shared::VelocityComponent& velocity,
                 shared::EnemyProjectileTag&) {
        EXPECT_LT(velocity.vx, 0.0F);  // Moving left towards target
        EXPECT_NEAR(velocity.vy, 0.0F, 0.001F);  // No vertical movement
    });
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileAimsDiagonally) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    // Enemy at (500, 500), target at (100, 100) - should shoot diagonally
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 500.0F, 100.0F, 100.0F);

    auto view = registry.view<shared::VelocityComponent, shared::EnemyProjectileTag>();
    view.each([](ECS::Entity /*e*/, shared::VelocityComponent& velocity,
                 shared::EnemyProjectileTag&) {
        EXPECT_LT(velocity.vx, 0.0F);  // Moving left
        EXPECT_LT(velocity.vy, 0.0F);  // Moving up
    });
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileZeroDistance) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    // Target position accounts for the default offset (-32, 0)
    // Enemy at (500, 300), spawn position will be (468, 300)
    // Set target to spawn position to get zero distance
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 468.0F, 300.0F);

    auto view = registry.view<shared::VelocityComponent, shared::EnemyProjectileTag>();
    view.each([](ECS::Entity /*e*/, shared::VelocityComponent& velocity,
                 shared::EnemyProjectileTag&) {
        // When length is 0, should default to shooting left (-speed)
        EXPECT_LT(velocity.vx, 0.0F);
    });
}

TEST_F(ProjectileSpawnerSystemTest, SpawnEnemyProjectileHasCorrectOwner) {
    auto system = createSystem();
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    system.spawnEnemyProjectile(registry, enemy, 99, 500.0F, 300.0F, 100.0F, 300.0F);

    auto view = registry.view<shared::ProjectileComponent>();
    view.each([](ECS::Entity /*e*/, shared::ProjectileComponent& proj) {
        EXPECT_EQ(proj.owner, shared::ProjectileOwner::Enemy);
        EXPECT_EQ(proj.ownerNetworkId, 99U);
    });
}

// =============================================================================
// Projectile Count Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, DecrementProjectileCount) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 1, 0.0F, 0.0F);
    system.spawnPlayerProjectile(registry, 2, 0.0F, 0.0F);
    EXPECT_EQ(system.getProjectileCount(), 2U);

    system.decrementProjectileCount();
    EXPECT_EQ(system.getProjectileCount(), 1U);

    system.decrementProjectileCount();
    EXPECT_EQ(system.getProjectileCount(), 0U);
}

TEST_F(ProjectileSpawnerSystemTest, DecrementProjectileCountAtZero) {
    auto system = createSystem();
    EXPECT_EQ(system.getProjectileCount(), 0U);

    system.decrementProjectileCount();
    EXPECT_EQ(system.getProjectileCount(), 0U);  // Should not underflow
}

// =============================================================================
// Lifetime Component Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, SpawnedProjectileHasLifetime) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    auto view = registry.view<shared::LifetimeComponent, shared::ProjectileTag>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::LifetimeComponent& lifetime,
                       shared::ProjectileTag&) {
        count++;
        EXPECT_GT(lifetime.remainingTime, 0.0F);
    });
    EXPECT_EQ(count, 1);
}

// =============================================================================
// Bounding Box Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, SpawnedProjectileHasBoundingBox) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    auto view = registry.view<shared::BoundingBoxComponent, shared::ProjectileTag>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::BoundingBoxComponent& bbox,
                       shared::ProjectileTag&) {
        count++;
        EXPECT_GT(bbox.width, 0.0F);
        EXPECT_GT(bbox.height, 0.0F);
    });
    EXPECT_EQ(count, 1);
}

// =============================================================================
// Network ID Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, SpawnedProjectileHasNetworkId) {
    auto system = createSystem();
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 100.0F, 200.0F);

    auto view = registry.view<shared::NetworkIdComponent, shared::ProjectileTag>();
    int count = 0;
    view.each([&count](ECS::Entity /*e*/, shared::NetworkIdComponent& netId,
                       shared::ProjectileTag&) {
        count++;
        EXPECT_TRUE(netId.isValid());
    });
    EXPECT_EQ(count, 1);
}

TEST_F(ProjectileSpawnerSystemTest, NetworkIdsAreUnique) {
    auto system = createSystem();
    ECS::Registry registry;

    uint32_t id1 = system.spawnPlayerProjectile(registry, 1, 0.0F, 0.0F);
    uint32_t id2 = system.spawnPlayerProjectile(registry, 2, 0.0F, 0.0F);
    uint32_t id3 = system.spawnPlayerProjectile(registry, 3, 0.0F, 0.0F);

    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

// =============================================================================
// Custom Config Tests
// =============================================================================

TEST_F(ProjectileSpawnerSystemTest, CustomOffsetApplied) {
    server::ProjectileSpawnConfig config;
    config.playerProjectileOffsetX = 100.0F;
    config.playerProjectileOffsetY = 50.0F;
    auto system = createSystemWithConfig(config);
    ECS::Registry registry;

    system.spawnPlayerProjectile(registry, 42, 0.0F, 0.0F);

    auto view = registry.view<shared::TransformComponent, shared::PlayerProjectileTag>();
    view.each([](ECS::Entity /*e*/, shared::TransformComponent& transform,
                 shared::PlayerProjectileTag&) {
        EXPECT_FLOAT_EQ(transform.x, 100.0F);
        EXPECT_FLOAT_EQ(transform.y, 50.0F);
    });
}

TEST_F(ProjectileSpawnerSystemTest, EnemyCustomOffsetApplied) {
    server::ProjectileSpawnConfig config;
    config.enemyProjectileOffsetX = -50.0F;
    config.enemyProjectileOffsetY = 25.0F;
    auto system = createSystemWithConfig(config);
    ECS::Registry registry;

    auto enemy = registry.spawnEntity();
    system.spawnEnemyProjectile(registry, enemy, 99, 200.0F, 100.0F, 0.0F, 100.0F);

    auto view = registry.view<shared::TransformComponent, shared::EnemyProjectileTag>();
    view.each([](ECS::Entity /*e*/, shared::TransformComponent& transform,
                 shared::EnemyProjectileTag&) {
        EXPECT_FLOAT_EQ(transform.x, 150.0F);  // 200 - 50
        EXPECT_FLOAT_EQ(transform.y, 125.0F);  // 100 + 25
    });
}
