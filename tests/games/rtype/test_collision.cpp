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
