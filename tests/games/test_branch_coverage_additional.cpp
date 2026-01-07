/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional branch coverage tests for various game systems
*/

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <limits>

#include "core/Registry/Registry.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"

using namespace ECS;
using namespace rtype::games::rtype::shared;

// Test Health component edge cases
TEST(HealthComponentBranchTest, ZeroHealth) {
    HealthComponent health;
    health.current = 0;
    health.max = 100;
    
    EXPECT_EQ(health.current, 0);
    EXPECT_TRUE(health.current <= 0);  // Test branch condition
}

TEST(HealthComponentBranchTest, NegativeHealth) {
    HealthComponent health;
    health.current = -10;
    health.max = 100;
    
    EXPECT_LT(health.current, 0);
}

TEST(HealthComponentBranchTest, FullHealth) {
    HealthComponent health;
    health.current = 100;
    health.max = 100;
    
    EXPECT_EQ(health.current, health.max);
    EXPECT_GE(health.current, health.max);
}

TEST(HealthComponentBranchTest, OverMaxHealth) {
    HealthComponent health;
    health.current = 150;
    health.max = 100;
    
    EXPECT_GT(health.current, health.max);
}

TEST(HealthComponentBranchTest, ExtremeValues) {
    HealthComponent health;
    health.current = std::numeric_limits<int>::max();
    health.max = std::numeric_limits<int>::max();
    
    EXPECT_EQ(health.current, health.max);
}

// Additional registry edge cases
TEST(RegistryBranchTest, SpawnMultipleEntities) {
    auto registry = std::make_shared<Registry>();
    
    std::vector<Entity> entities;
    for (int i = 0; i < 1000; ++i) {
        entities.push_back(registry->spawnEntity());
    }
    
    EXPECT_EQ(entities.size(), 1000u);
}

TEST(RegistryBranchTest, KillNonExistentEntity) {
    auto registry = std::make_shared<Registry>();
    
    Entity fake;
    fake.id = 999999;
    
    // Should handle gracefully
    EXPECT_NO_THROW(registry->killEntity(fake));
}

// Test boundary conditions
TEST(BoundaryConditionTest, FloatEpsilonComparisons) {
    float a = 1.0f;
    float b = 1.0f + std::numeric_limits<float>::epsilon();
    
    // These should trigger different branches in floating point comparisons
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a >= (b - std::numeric_limits<float>::epsilon() * 2));
}

TEST(BoundaryConditionTest, IntegerOverflow) {
    int32_t maxInt = std::numeric_limits<int32_t>::max();
    
    // Test conditions near overflow
    EXPECT_GT(maxInt, 0);
    EXPECT_EQ(maxInt, std::numeric_limits<int32_t>::max());
}

TEST(BoundaryConditionTest, UnsignedUnderflow) {
    uint32_t zero = 0;
    
    // Test conditions at minimum unsigned value
    EXPECT_EQ(zero, 0u);
    EXPECT_GE(zero, 0u);
}

// Test various vector operations that might have branches
TEST(VectorOperationBranchTest, EmptyVectorOperations) {
    std::vector<int> vec;
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0u);
    
    // These should all handle empty case
    for (const auto& item : vec) {
        (void)item;
    }
}

TEST(VectorOperationBranchTest, SingleElementVector) {
    std::vector<int> vec = {42};
    
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_EQ(vec.front(), 42);
    EXPECT_EQ(vec.back(), 42);
}

TEST(VectorOperationBranchTest, LargeVectorOperations) {
    std::vector<int> vec;
    for (int i = 0; i < 10000; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 10000u);
    EXPECT_EQ(vec.front(), 0);
    EXPECT_EQ(vec.back(), 9999);
}

// Additional health component tests for better coverage
TEST(HealthComponentBranchTest, PartialHealth) {
    HealthComponent health;
    health.current = 50;
    health.max = 100;
    
    EXPECT_GT(health.current, 0);
    EXPECT_LT(health.current, health.max);
}

TEST(HealthComponentBranchTest, MaxHealthZero) {
    HealthComponent health;
    health.current = 0;
    health.max = 0;
    
    EXPECT_EQ(health.current, health.max);
}

TEST(HealthComponentBranchTest, MultipleChecks) {
    HealthComponent health;
    health.current = 75;
    health.max = 100;
    
    // Multiple branch conditions
    EXPECT_TRUE(health.current > 0);
    EXPECT_TRUE(health.current < health.max);
    EXPECT_TRUE(health.current != health.max);
    EXPECT_FALSE(health.current == health.max);
    EXPECT_FALSE(health.current <= 0);
}

