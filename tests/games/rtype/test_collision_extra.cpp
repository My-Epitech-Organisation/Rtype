#include <gtest/gtest.h>

#include "ECS.hpp"
#include "games/rtype/server/Systems/Collision/CollisionSystem.hpp"
#include "games/rtype/shared/Components.hpp"
#include "rtype/engine.hpp"

using namespace rtype::games::rtype;

namespace {

struct CollisionExtraFixture : public ::testing::Test {
    CollisionExtraFixture()
        : system([](const rtype::engine::GameEvent&) {}, 1920.0F, 1080.0F) {}

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
    }

    std::unique_ptr<ECS::Registry> registry;
    server::CollisionSystem system;
};

}  // namespace

TEST_F(CollisionExtraFixture, PickupSpeedBoostAppliesSpeed) {
    bool eventEmitted = false;
    rtype::engine::GameEvent emitted;
    server::CollisionSystem systemWithEmitter(
        [&](const rtype::engine::GameEvent& ev) { eventEmitted = true; emitted = ev; });

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::NetworkIdComponent>(player, 42);
    registry->emplaceComponent<shared::ShootCooldownComponent>(player, 0.5F);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 100.0F, 100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent pu{};
    pu.type = shared::PowerUpType::SpeedBoost;
    pu.duration = 3.0F;
    pu.magnitude = 0.5F;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, pu);


    systemWithEmitter.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_FLOAT_EQ(active.speedMultiplier, 1.0F + pu.magnitude);
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
    EXPECT_TRUE(eventEmitted);
    EXPECT_EQ(emitted.type, rtype::engine::GameEventType::PowerUpApplied);
}

TEST_F(CollisionExtraFixture, PickupShieldAddsInvincibleTag) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 200.0F, 200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 200.0F, 200.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent pu{};
    pu.type = shared::PowerUpType::Shield;
    pu.duration = 2.0F;
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, pu);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    EXPECT_TRUE(registry->hasComponent<shared::InvincibleTag>(player));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionExtraFixture, PickupRapidFireAdjustsCooldown) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 300.0F, 300.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::ShootCooldownComponent>(player, 0.5F);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 300.0F, 300.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent pu{};
    pu.type = shared::PowerUpType::RapidFire;
    pu.duration = 5.0F;
    pu.magnitude = 1.0F; // doubles fire rate
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, pu);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::ActivePowerUpComponent>(player));
    auto& active = registry->getComponent<shared::ActivePowerUpComponent>(player);
    EXPECT_TRUE(active.hasOriginalCooldown);
    auto& cd = registry->getComponent<shared::ShootCooldownComponent>(player);
    EXPECT_LE(cd.cooldownTime, 0.5F);
}

TEST_F(CollisionExtraFixture, PickupHealthBoostIncreasesHealth) {
    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 400.0F, 400.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 50, 100);

    auto pickup = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(pickup, 400.0F, 400.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(pickup, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PickupTag>(pickup);
    shared::PowerUpComponent pu{};
    pu.type = shared::PowerUpType::HealthBoost;
    pu.duration = 1.0F;
    pu.magnitude = 0.3F; // +30 health
    registry->emplaceComponent<shared::PowerUpComponent>(pickup, pu);

    system.update(*registry, 0.0F);

    auto& health = registry->getComponent<shared::HealthComponent>(player);
    EXPECT_GT(health.current, 50);
    EXPECT_LE(health.current, health.max);
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(pickup));
}

TEST_F(CollisionExtraFixture, ObstacleDamageDestroysIfConfigured) {
    auto obstacle = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(obstacle, 500.0F, 500.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(obstacle, 10.0F, 10.0F);
    registry->emplaceComponent<shared::ObstacleTag>(obstacle);
    shared::DamageOnContactComponent dmg{};
    dmg.damage = 50;
    dmg.destroySelf = true;
    registry->emplaceComponent<shared::DamageOnContactComponent>(obstacle, dmg);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 502.0F, 500.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(player, 10.0F, 10.0F);
    registry->emplaceComponent<shared::PlayerTag>(player);
    registry->emplaceComponent<shared::HealthComponent>(player, 40, 100);

    system.update(*registry, 0.0F);

    // Player should be destroyed (health <= 0) and obstacle destroyed (destroySelf=true)
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(player));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(obstacle));
}
