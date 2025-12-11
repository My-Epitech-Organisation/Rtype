/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_LifetimeSystem - Unit tests for LifetimeSystem
*/

#include <gtest/gtest.h>
#include "../../../src/games/rtype/shared/Components/LifetimeComponent.hpp"
#include "../../../src/games/rtype/shared/Components/Tags.hpp"
#include "../../../src/games/rtype/shared/Systems/Lifetime/LifetimeSystem.hpp"
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

    // Entity should be marked for destruction with DestroyTag
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(LifetimeSystemTest, UpdateDestroysEntityWhenLifetimeBecomesNegative) {
    registry.emplaceComponent<LifetimeComponent>(entity, 0.5f);

    lifetimeSystem.update(registry, 1.0f);

    // Entity should be marked for destruction with DestroyTag
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(LifetimeSystemTest, UpdateDoesNotDestroyEntityWithPositiveLifetime) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, 1.0f);

    EXPECT_TRUE(registry.isAlive(entity));
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
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

TEST_F(LifetimeSystemTest, UpdateMultipleEntities) {
    auto entity2 = registry.spawnEntity();
    auto entity3 = registry.spawnEntity();
    
    registry.emplaceComponent<LifetimeComponent>(entity, 1.0f);
    registry.emplaceComponent<LifetimeComponent>(entity2, 2.0f);
    registry.emplaceComponent<LifetimeComponent>(entity3, 0.5f);

    lifetimeSystem.update(registry, 0.6f);

    // entity should still have lifetime
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
    // entity2 should still have lifetime
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity2));
    // entity3 should be destroyed (0.5 - 0.6 < 0)
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity3));
    
    registry.killEntity(entity2);
    registry.killEntity(entity3);
}

TEST_F(LifetimeSystemTest, DoesNotAddDuplicateDestroyTag) {
    registry.emplaceComponent<LifetimeComponent>(entity, 0.1f);
    registry.emplaceComponent<DestroyTag>(entity);  // Already has destroy tag

    // Should not throw or double-add
    lifetimeSystem.update(registry, 1.0f);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(LifetimeSystemTest, LifetimeExactlyZero) {
    registry.emplaceComponent<LifetimeComponent>(entity, 1.0f);

    lifetimeSystem.update(registry, 1.0f);

    // 1.0 - 1.0 = 0.0, which is <= 0, so should be destroyed
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}
