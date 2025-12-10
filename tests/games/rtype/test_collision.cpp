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
        : system([](const rtype::engine::GameEvent&) {}) {}

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
    }

    std::unique_ptr<ECS::Registry> registry;
    server::CollisionSystem system;
};

}  // namespace

TEST_F(CollisionFixture, OverlapMarksDestroyOnEnemyAndProjectile) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 0.0F,
                                                            0.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 5.0F, 0.0F,
                                                           0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_TRUE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionFixture, NoOverlapLeavesEntitiesIntact) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 0.0F,
                                                            0.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto enemy = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(enemy, 100.0F,
                                                           100.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(enemy, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::EnemyTag>(enemy);

    system.update(*registry, 0.0F);

    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(projectile));
    EXPECT_FALSE(registry->hasComponent<shared::DestroyTag>(enemy));
}

TEST_F(CollisionFixture, ProjectileHitsPlayerMarksBothDestroyed) {
    auto projectile = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(projectile, 0.0F,
                                                            0.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 2.0F, 0.0F,
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
    registry->emplaceComponent<shared::TransformComponent>(projectile, 0.0F,
                                                            0.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 2.0F, 0.0F,
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
    registry->emplaceComponent<shared::TransformComponent>(projectile, 0.0F,
                                                            0.0F, 0.0F);
    registry->emplaceComponent<shared::BoundingBoxComponent>(projectile, 10.0F,
                                                              10.0F);
    registry->emplaceComponent<shared::ProjectileTag>(projectile);

    auto player = registry->spawnEntity();
    registry->emplaceComponent<shared::TransformComponent>(player, 2.0F, 0.0F,
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

