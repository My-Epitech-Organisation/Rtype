/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_forcepod_shooting_system - Tests for Force Pod automatic shooting
*/

#include <gtest/gtest.h>

#include <memory>

#include "games/rtype/server/Systems/ForcePod/ForcePodShootingSystem.hpp"
#include "games/rtype/server/Systems/Projectile/ProjectileSpawnerSystem.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/ForcePodComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;

class ForcePodShootingSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<ForcePodShootingSystem> system;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        // Create system without projectile spawner for basic tests
        system = std::make_unique<ForcePodShootingSystem>(nullptr);
    }

    void TearDown() override {
        system.reset();
        registry.reset();
    }

    ECS::Entity createForcePod(std::uint32_t ownerNetworkId, ForcePodState state,
                                float x, float y) {
        ECS::Entity forcePod = registry->spawnEntity();
        registry->emplaceComponent<ForcePodTag>(forcePod);
        registry->emplaceComponent<ForcePodComponent>(forcePod, state, 50.0F,
                                                      20.0F, ownerNetworkId);
        registry->emplaceComponent<TransformComponent>(forcePod, x, y, 0.0F);
        registry->emplaceComponent<NetworkIdComponent>(forcePod, ownerNetworkId);
        return forcePod;
    }
};

TEST_F(ForcePodShootingSystemTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(system->getName(), "ForcePodShootingSystem");
}

TEST_F(ForcePodShootingSystemTest, UpdateWithNoEntities) {
    system->update(*registry, 0.016F);
}

TEST_F(ForcePodShootingSystemTest, AttachedPodAddsCooldownComponent) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);

    EXPECT_FALSE(registry->hasComponent<ShootCooldownComponent>(forcePod));

    system->update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<ShootCooldownComponent>(forcePod));
}

TEST_F(ForcePodShootingSystemTest, DetachedPodAddsCooldownComponent) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Detached, 100.0F, 200.0F);

    system->update(*registry, 0.016F);

    EXPECT_TRUE(registry->hasComponent<ShootCooldownComponent>(forcePod));
}

TEST_F(ForcePodShootingSystemTest, ReturningPodDoesNotAddCooldown) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Returning, 100.0F, 200.0F);

    system->update(*registry, 0.016F);

    EXPECT_FALSE(registry->hasComponent<ShootCooldownComponent>(forcePod));
}

TEST_F(ForcePodShootingSystemTest, CooldownUpdatesDeltaTime) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);

    // First update adds cooldown and potentially triggers
    system->update(*registry, 0.016F);

    auto& cooldown = registry->getComponent<ShootCooldownComponent>(forcePod);
    float initialRemaining = cooldown.currentCooldown;

    // Second update should reduce cooldown
    system->update(*registry, 0.1F);

    EXPECT_LT(cooldown.currentCooldown, initialRemaining);
}

TEST_F(ForcePodShootingSystemTest, CooldownTriggersShoot) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);

    // Add a cooldown that's ready to fire (currentCooldown = 0)
    registry->emplaceComponent<ShootCooldownComponent>(forcePod, 0.9F);
    auto& cooldown = registry->getComponent<ShootCooldownComponent>(forcePod);
    cooldown.currentCooldown = 0.0F;  // Make it ready to shoot

    EXPECT_TRUE(cooldown.canShoot());

    system->update(*registry, 0.016F);

    // Cooldown should be reset after triggering (since canShoot was true)
    // The system calls triggerCooldown which sets currentCooldown = cooldownTime
    EXPECT_FALSE(cooldown.canShoot());
}

TEST_F(ForcePodShootingSystemTest, MultiplePods) {
    ECS::Entity forcePod1 = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);
    ECS::Entity forcePod2 = createForcePod(2000, ForcePodState::Detached, 300.0F, 400.0F);
    ECS::Entity forcePod3 = createForcePod(3000, ForcePodState::Returning, 500.0F, 600.0F);

    system->update(*registry, 0.016F);

    // Attached and detached should have cooldown
    EXPECT_TRUE(registry->hasComponent<ShootCooldownComponent>(forcePod1));
    EXPECT_TRUE(registry->hasComponent<ShootCooldownComponent>(forcePod2));
    // Returning should not
    EXPECT_FALSE(registry->hasComponent<ShootCooldownComponent>(forcePod3));
}

TEST_F(ForcePodShootingSystemTest, PodWithExistingCooldownUpdates) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);
    registry->emplaceComponent<ShootCooldownComponent>(forcePod, 0.5F);

    auto& cooldown = registry->getComponent<ShootCooldownComponent>(forcePod);
    cooldown.currentCooldown = 0.5F;  // Set the current cooldown
    float initialRemaining = cooldown.currentCooldown;

    system->update(*registry, 0.1F);

    EXPECT_LT(cooldown.currentCooldown, initialRemaining);
}

TEST_F(ForcePodShootingSystemTest, NullProjectileSpawnerDoesNotCrash) {
    ECS::Entity forcePod = createForcePod(1000, ForcePodState::Attached, 100.0F, 200.0F);
    registry->emplaceComponent<ShootCooldownComponent>(forcePod, 0.0F);  // Ready to fire

    // Should not crash with null projectile spawner
    EXPECT_NO_THROW(system->update(*registry, 0.016F));
}
