/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_LifetimeSystem - Unit tests for LifetimeSystem
*/

#include <gtest/gtest.h>
#include "../../../src/games/rtype/shared/Systems/Lifetime/LifetimeSystem.hpp"
#include "../../../src/games/rtype/shared/Components/LifetimeComponent.hpp"
#include "../../../../lib/rtype_ecs/src/ECS.hpp"

using namespace rtype::games::rtype::shared;

class LifetimeSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        entity = registry.spawnEntity();
    }

    void TearDown() override {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
    }

    ECS::Registry registry;
    ECS::Entity entity;
    LifetimeSystem lifetimeSystem;
};

TEST_F(LifetimeSystemTest, UpdateDecrementsLifetime) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, 1.0f);

    auto& lifetime = registry.getComponent<LifetimeComponent>(entity);
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 4.0f);
}

TEST_F(LifetimeSystemTest, UpdateDestroysEntityWhenLifetimeExpires) {
    registry.emplaceComponent<LifetimeComponent>(entity, 0.5f);

    lifetimeSystem.update(registry, 0.5f);

    EXPECT_FALSE(registry.isAlive(entity));
}

TEST_F(LifetimeSystemTest, UpdateDestroysEntityWhenLifetimeBecomesNegative) {
    registry.emplaceComponent<LifetimeComponent>(entity, 0.5f);

    lifetimeSystem.update(registry, 1.0f);

    EXPECT_FALSE(registry.isAlive(entity));
}

TEST_F(LifetimeSystemTest, UpdateDoesNotDestroyEntityWithPositiveLifetime) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, 1.0f);

    EXPECT_TRUE(registry.isAlive(entity));
}

TEST_F(LifetimeSystemTest, UpdateWithZeroDeltaTime) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, 0.0f);

    auto& lifetime = registry.getComponent<LifetimeComponent>(entity);
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 5.0f);
    EXPECT_TRUE(registry.isAlive(entity));
}

TEST_F(LifetimeSystemTest, UpdateWithNegativeDeltaTime) {
    // Assuming negative delta time should NOT decrease lifetime (or should be ignored)
    // If the implementation doesn't guard, it will increase lifetime.
    // I will modify the implementation to guard against negative delta time as it is a safer practice.
    // But first let's see what the current implementation does (it subtracts, so it adds time).
    // I'll assume we want to guard it like ProjectileSystem.
    
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, -1.0f);

    auto& lifetime = registry.getComponent<LifetimeComponent>(entity);
    // If guarded, it should remain 5.0. If not, it becomes 6.0.
    // I will update the implementation to guard it.
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 5.0f); 
}
