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
#include "../../../../lib/ecs/src/ECS.hpp"

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

TEST_F(LifetimeSystemTest, UpdateParallelPath_ManyEntities) {
    // Create 101 entities to trigger parallel execution path (threshold is 100)
    std::vector<ECS::Entity> entities;
    for (int i = 0; i < 101; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<LifetimeComponent>(e, 5.0f);
        entities.push_back(e);
    }

    lifetimeSystem.update(registry, 1.0f);

    // Verify all entities had their lifetime reduced
    for (auto e : entities) {
        auto& lifetime = registry.getComponent<LifetimeComponent>(e);
        EXPECT_FLOAT_EQ(lifetime.remainingTime, 4.0f);
        EXPECT_FALSE(registry.hasComponent<DestroyTag>(e));
    }

    for (auto e : entities) {
        registry.killEntity(e);
    }
}

TEST_F(LifetimeSystemTest, UpdateParallelPath_WithExpiring) {
    // Create 110 entities, 60 will expire to test parallel path with destruction
    std::vector<ECS::Entity> entities;
    for (int i = 0; i < 110; ++i) {
        auto e = registry.spawnEntity();
        float lifetime = (i < 60) ? 0.5f : 5.0f;
        registry.emplaceComponent<LifetimeComponent>(e, lifetime);
        entities.push_back(e);
    }

    lifetimeSystem.update(registry, 1.0f);

    // Count entities marked for destruction
    int expiredCount = 0;
    int aliveCount = 0;
    for (auto e : entities) {
        if (registry.hasComponent<DestroyTag>(e)) {
            ++expiredCount;
        } else {
            ++aliveCount;
        }
    }
    EXPECT_EQ(expiredCount, 60);
    EXPECT_EQ(aliveCount, 50);

    for (auto e : entities) {
        registry.killEntity(e);
    }
}

TEST_F(LifetimeSystemTest, UpdateParallelPath_WithExistingDestroyTag) {
    // Create 105 entities, some already have DestroyTag
    std::vector<ECS::Entity> entities;
    for (int i = 0; i < 105; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<LifetimeComponent>(e, 0.5f);
        if (i < 20) {
            registry.emplaceComponent<DestroyTag>(e);  // Already marked
        }
        entities.push_back(e);
    }

    // Should not throw - should not try to add duplicate DestroyTag
    EXPECT_NO_THROW(lifetimeSystem.update(registry, 1.0f));

    // All entities should have DestroyTag
    for (auto e : entities) {
        EXPECT_TRUE(registry.hasComponent<DestroyTag>(e));
    }

    for (auto e : entities) {
        registry.killEntity(e);
    }
}

TEST_F(LifetimeSystemTest, UpdateWithNegativeDeltaTimeDoesNothing) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, -1.0f);

    // Lifetime should remain unchanged with negative deltaTime
    auto& lifetime = registry.getComponent<LifetimeComponent>(entity);
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 5.0f);
}

TEST_F(LifetimeSystemTest, UpdateWithZeroDeltaTimeDecreasesNone) {
    registry.emplaceComponent<LifetimeComponent>(entity, 5.0f);

    lifetimeSystem.update(registry, 0.0f);

    auto& lifetime = registry.getComponent<LifetimeComponent>(entity);
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 5.0f);
}
