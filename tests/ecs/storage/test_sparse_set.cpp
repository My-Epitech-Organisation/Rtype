/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for SparseSet container
*/

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../../lib/rtype_ecs/src/core/Entity.hpp"
#include "../../../lib/rtype_ecs/src/storage/SparseSet.hpp"

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x_, float y_) : x(x_), y(y_) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;

    Velocity() = default;
    Velocity(float dx_, float dy_) : dx(dx_), dy(dy_) {}
};

struct Health {
    int current = 100;
    int max = 100;

    Health() = default;
    Health(int c, int m) : current(c), max(m) {}
};

// Non-trivial component with std::string
struct Name {
    std::string value;

    Name() = default;
    Name(const std::string& v) : value(v) {}
};

// ============================================================================
// SPARSE SET BASIC TESTS
// ============================================================================

class SparseSetTest : public ::testing::Test {
protected:
    SparseSet<Position> positions;
    SparseSet<Health> healths;
    SparseSet<Name> names;

    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SparseSetTest, Constructor_EmptySet) {
    EXPECT_EQ(positions.size(), 0);
    EXPECT_TRUE(positions.begin() == positions.end());
}

TEST_F(SparseSetTest, Emplace_SingleComponent) {
    Entity entity(0, 0);

    auto& pos = positions.emplace(entity, 10.0f, 20.0f);

    EXPECT_EQ(positions.size(), 1);
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
}

TEST_F(SparseSetTest, Emplace_MultipleComponents) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    positions.emplace(e1, 1.0f, 2.0f);
    positions.emplace(e2, 3.0f, 4.0f);
    positions.emplace(e3, 5.0f, 6.0f);

    EXPECT_EQ(positions.size(), 3);
}

TEST_F(SparseSetTest, Emplace_DuplicateEntity_ReplacesComponent) {
    Entity entity(0, 0);

    positions.emplace(entity, 10.0f, 20.0f);
    positions.emplace(entity, 100.0f, 200.0f);

    EXPECT_EQ(positions.size(), 1);
    EXPECT_EQ(positions.get(entity).x, 100.0f);
    EXPECT_EQ(positions.get(entity).y, 200.0f);
}

// ============================================================================
// CONTAINS TESTS
// ============================================================================

TEST_F(SparseSetTest, Contains_ExistingEntity) {
    Entity entity(5, 0);
    positions.emplace(entity, 1.0f, 2.0f);

    EXPECT_TRUE(positions.contains(entity));
}

TEST_F(SparseSetTest, Contains_NonExistingEntity) {
    Entity entity(5, 0);

    EXPECT_FALSE(positions.contains(entity));
}

TEST_F(SparseSetTest, Contains_DifferentGeneration) {
    Entity entity_v1(5, 0);
    Entity entity_v2(5, 1);  // Same index, different generation

    positions.emplace(entity_v1, 1.0f, 2.0f);

    EXPECT_TRUE(positions.contains(entity_v1));
    EXPECT_FALSE(positions.contains(entity_v2));
}

TEST_F(SparseSetTest, Contains_AfterRemoval) {
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);
    positions.remove(entity);

    EXPECT_FALSE(positions.contains(entity));
}

// ============================================================================
// GET TESTS
// ============================================================================

TEST_F(SparseSetTest, Get_ReturnsCorrectComponent) {
    Entity entity(0, 0);
    positions.emplace(entity, 42.0f, 84.0f);

    auto& pos = positions.get(entity);

    EXPECT_EQ(pos.x, 42.0f);
    EXPECT_EQ(pos.y, 84.0f);
}

TEST_F(SparseSetTest, Get_CanModifyComponent) {
    Entity entity(0, 0);
    positions.emplace(entity, 0.0f, 0.0f);

    positions.get(entity).x = 100.0f;
    positions.get(entity).y = 200.0f;

    EXPECT_EQ(positions.get(entity).x, 100.0f);
    EXPECT_EQ(positions.get(entity).y, 200.0f);
}

TEST_F(SparseSetTest, Get_ThrowsForMissingEntity) {
    Entity entity(0, 0);

    EXPECT_THROW(positions.get(entity), std::runtime_error);
}

TEST_F(SparseSetTest, GetConst_ReturnsCorrectComponent) {
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);

    const auto& const_positions = positions;
    const auto& pos = const_positions.get(entity);

    EXPECT_EQ(pos.x, 1.0f);
    EXPECT_EQ(pos.y, 2.0f);
}

TEST_F(SparseSetTest, GetConst_ThrowsForMissingEntity) {
    const auto& const_positions = positions;
    Entity entity(0, 0);

    EXPECT_THROW(const_positions.get(entity), std::runtime_error);
}

// ============================================================================
// REMOVE TESTS
// ============================================================================

TEST_F(SparseSetTest, Remove_ExistingEntity) {
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);

    positions.remove(entity);

    EXPECT_EQ(positions.size(), 0);
    EXPECT_FALSE(positions.contains(entity));
}

TEST_F(SparseSetTest, Remove_NonExistingEntity_NoError) {
    Entity entity(0, 0);

    EXPECT_NO_THROW(positions.remove(entity));
    EXPECT_EQ(positions.size(), 0);
}

TEST_F(SparseSetTest, Remove_SwapAndPop_MaintainsOtherComponents) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    positions.emplace(e1, 1.0f, 1.0f);
    positions.emplace(e2, 2.0f, 2.0f);
    positions.emplace(e3, 3.0f, 3.0f);

    // Remove middle entity
    positions.remove(e2);

    EXPECT_EQ(positions.size(), 2);
    EXPECT_TRUE(positions.contains(e1));
    EXPECT_FALSE(positions.contains(e2));
    EXPECT_TRUE(positions.contains(e3));

    // Verify remaining components are correct
    EXPECT_EQ(positions.get(e1).x, 1.0f);
    EXPECT_EQ(positions.get(e3).x, 3.0f);
}

TEST_F(SparseSetTest, Remove_AllEntities) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    positions.emplace(e1, 1.0f, 2.0f);
    positions.emplace(e2, 3.0f, 4.0f);

    positions.remove(e1);
    positions.remove(e2);

    EXPECT_EQ(positions.size(), 0);
}

// ============================================================================
// CLEAR TESTS
// ============================================================================

TEST_F(SparseSetTest, Clear_RemovesAllComponents) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    positions.emplace(e1, 1.0f, 1.0f);
    positions.emplace(e2, 2.0f, 2.0f);
    positions.emplace(e3, 3.0f, 3.0f);

    positions.clear();

    EXPECT_EQ(positions.size(), 0);
    EXPECT_FALSE(positions.contains(e1));
    EXPECT_FALSE(positions.contains(e2));
    EXPECT_FALSE(positions.contains(e3));
}

TEST_F(SparseSetTest, Clear_EmptySet_NoError) {
    EXPECT_NO_THROW(positions.clear());
    EXPECT_EQ(positions.size(), 0);
}

// ============================================================================
// ITERATION TESTS
// ============================================================================

TEST_F(SparseSetTest, Iteration_ForLoop) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    positions.emplace(e1, 1.0f, 0.0f);
    positions.emplace(e2, 2.0f, 0.0f);
    positions.emplace(e3, 3.0f, 0.0f);

    float sum = 0.0f;
    for (const auto& pos : positions) {
        sum += pos.x;
    }

    EXPECT_EQ(sum, 6.0f);
}

TEST_F(SparseSetTest, Iteration_RangeBasedModification) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    positions.emplace(e1, 1.0f, 0.0f);
    positions.emplace(e2, 2.0f, 0.0f);

    for (auto& pos : positions) {
        pos.x *= 10.0f;
    }

    EXPECT_EQ(positions.get(e1).x, 10.0f);
    EXPECT_EQ(positions.get(e2).x, 20.0f);
}

TEST_F(SparseSetTest, GetPacked_ReturnsEntityList) {
    Entity e1(10, 0);
    Entity e2(20, 0);
    Entity e3(30, 0);

    positions.emplace(e1, 1.0f, 0.0f);
    positions.emplace(e2, 2.0f, 0.0f);
    positions.emplace(e3, 3.0f, 0.0f);

    const auto& packed = positions.getPacked();

    EXPECT_EQ(packed.size(), 3);
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e1) != packed.end());
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e2) != packed.end());
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e3) != packed.end());
}

// ============================================================================
// RESERVE AND SHRINK TESTS
// ============================================================================

TEST_F(SparseSetTest, Reserve_IncreasesCapacity) {
    positions.reserve(1000);

    // Should be able to add 1000 entities without reallocation
    for (std::uint32_t i = 0; i < 1000; ++i) {
        Entity entity(i, 0);
        positions.emplace(entity, static_cast<float>(i), 0.0f);
    }

    EXPECT_EQ(positions.size(), 1000);
}

TEST_F(SparseSetTest, ShrinkToFit_ReducesMemory) {
    // Add many entities
    for (std::uint32_t i = 0; i < 100; ++i) {
        Entity entity(i, 0);
        positions.emplace(entity, static_cast<float>(i), 0.0f);
    }

    // Remove most entities
    for (std::uint32_t i = 10; i < 100; ++i) {
        Entity entity(i, 0);
        positions.remove(entity);
    }

    positions.shrinkToFit();

    // Should still work correctly
    EXPECT_EQ(positions.size(), 10);
    for (std::uint32_t i = 0; i < 10; ++i) {
        Entity entity(i, 0);
        EXPECT_TRUE(positions.contains(entity));
    }
}

// ============================================================================
// NON-TRIVIAL COMPONENT TESTS
// ============================================================================

TEST_F(SparseSetTest, NonTrivialComponent_StringStorage) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    names.emplace(e1, "Player");
    names.emplace(e2, "Enemy");

    EXPECT_EQ(names.get(e1).value, "Player");
    EXPECT_EQ(names.get(e2).value, "Enemy");
}

TEST_F(SparseSetTest, NonTrivialComponent_Removal) {
    Entity entity(0, 0);
    names.emplace(entity, "TestEntity");

    names.remove(entity);

    EXPECT_FALSE(names.contains(entity));
    EXPECT_EQ(names.size(), 0);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(SparseSetTest, Stress_LargeNumberOfEntities) {
    constexpr std::uint32_t COUNT = 10000;

    for (std::uint32_t i = 0; i < COUNT; ++i) {
        Entity entity(i, 0);
        positions.emplace(entity, static_cast<float>(i), static_cast<float>(i * 2));
    }

    EXPECT_EQ(positions.size(), COUNT);

    // Verify all entities
    for (std::uint32_t i = 0; i < COUNT; ++i) {
        Entity entity(i, 0);
        EXPECT_TRUE(positions.contains(entity));
        EXPECT_EQ(positions.get(entity).x, static_cast<float>(i));
    }
}

TEST_F(SparseSetTest, Stress_RepeatedAddRemove) {
    Entity entity(0, 0);

    for (int i = 0; i < 1000; ++i) {
        positions.emplace(entity, static_cast<float>(i), 0.0f);
        EXPECT_TRUE(positions.contains(entity));
        positions.remove(entity);
        EXPECT_FALSE(positions.contains(entity));
    }

    EXPECT_EQ(positions.size(), 0);
}

TEST_F(SparseSetTest, Stress_SparseIndices) {
    // Test with very sparse entity indices
    std::vector<std::uint32_t> indices = {0, 100, 500, 1000, 5000, 10000, 50000};

    for (auto idx : indices) {
        Entity entity(idx, 0);
        positions.emplace(entity, static_cast<float>(idx), 0.0f);
    }

    EXPECT_EQ(positions.size(), indices.size());

    for (auto idx : indices) {
        Entity entity(idx, 0);
        EXPECT_TRUE(positions.contains(entity));
        EXPECT_EQ(positions.get(entity).x, static_cast<float>(idx));
    }
}

// ============================================================================
// INTERFACE COMPLIANCE TESTS
// ============================================================================

TEST_F(SparseSetTest, ISparseSet_PolymorphicRemove) {
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);

    ISparseSet* base_ptr = &positions;

    EXPECT_TRUE(base_ptr->contains(entity));
    base_ptr->remove(entity);
    EXPECT_FALSE(base_ptr->contains(entity));
}

TEST_F(SparseSetTest, ISparseSet_PolymorphicClear) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    positions.emplace(e1, 1.0f, 2.0f);
    positions.emplace(e2, 3.0f, 4.0f);

    ISparseSet* base_ptr = &positions;

    EXPECT_EQ(base_ptr->size(), 2);
    base_ptr->clear();
    EXPECT_EQ(base_ptr->size(), 0);
}

TEST_F(SparseSetTest, ISparseSet_GetPacked) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    positions.emplace(e1, 1.0f, 2.0f);
    positions.emplace(e2, 3.0f, 4.0f);

    ISparseSet* base_ptr = &positions;
    const auto& packed = base_ptr->getPacked();

    EXPECT_EQ(packed.size(), 2);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS
// ============================================================================

TEST_F(SparseSetTest, Emplace_WithDefaultValues) {
    Entity entity(0, 0);
    positions.emplace(entity);  // Using default constructor

    EXPECT_TRUE(positions.contains(entity));
    EXPECT_EQ(positions.get(entity).x, 0.0f);
    EXPECT_EQ(positions.get(entity).y, 0.0f);
}

TEST_F(SparseSetTest, Contains_NullEntity) {
    Entity null_entity;  // Null entity
    EXPECT_FALSE(positions.contains(null_entity));
}

TEST_F(SparseSetTest, Remove_NullEntity_NoError) {
    Entity null_entity;
    EXPECT_NO_THROW(positions.remove(null_entity));
}

TEST_F(SparseSetTest, Get_AfterUpdate) {
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);

    auto& pos = positions.get(entity);
    pos.x = 100.0f;
    pos.y = 200.0f;

    EXPECT_EQ(positions.get(entity).x, 100.0f);
    EXPECT_EQ(positions.get(entity).y, 200.0f);
}

TEST_F(SparseSetTest, Size_AfterMultipleOperations) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    EXPECT_EQ(positions.size(), 0);

    positions.emplace(e1, 1.0f, 0.0f);
    EXPECT_EQ(positions.size(), 1);

    positions.emplace(e2, 2.0f, 0.0f);
    EXPECT_EQ(positions.size(), 2);

    positions.emplace(e3, 3.0f, 0.0f);
    EXPECT_EQ(positions.size(), 3);

    positions.remove(e2);
    EXPECT_EQ(positions.size(), 2);

    positions.remove(e1);
    EXPECT_EQ(positions.size(), 1);

    positions.remove(e3);
    EXPECT_EQ(positions.size(), 0);
}

TEST_F(SparseSetTest, Iteration_EmptySet) {
    int count = 0;
    for ([[maybe_unused]] const auto& pos : positions) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

TEST_F(SparseSetTest, GetPacked_Empty) {
    const auto& packed = positions.getPacked();
    EXPECT_TRUE(packed.empty());
}

TEST_F(SparseSetTest, Iteration_SingleElement) {
    Entity entity(0, 0);
    positions.emplace(entity, 42.0f, 0.0f);

    int count = 0;
    for (const auto& pos : positions) {
        EXPECT_EQ(pos.x, 42.0f);
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(SparseSetTest, SwapAndPop_Order) {
    // Test that swap-and-pop maintains correct component-entity associations
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    positions.emplace(e1, 1.0f, 10.0f);
    positions.emplace(e2, 2.0f, 20.0f);
    positions.emplace(e3, 3.0f, 30.0f);

    // Remove middle element - should swap with last
    positions.remove(e2);

    // e1 and e3 should still have correct values
    EXPECT_TRUE(positions.contains(e1));
    EXPECT_TRUE(positions.contains(e3));
    EXPECT_EQ(positions.get(e1).x, 1.0f);
    EXPECT_EQ(positions.get(e3).x, 3.0f);
}

TEST_F(SparseSetTest, HighGeneration_Entities) {
    Entity e_gen0(0, 0);
    Entity e_gen100(0, 100);
    Entity e_gen4095(0, 4095);  // Max generation

    positions.emplace(e_gen0, 1.0f, 0.0f);
    EXPECT_TRUE(positions.contains(e_gen0));
    EXPECT_FALSE(positions.contains(e_gen100));

    positions.remove(e_gen0);
    positions.emplace(e_gen100, 2.0f, 0.0f);
    EXPECT_TRUE(positions.contains(e_gen100));

    positions.remove(e_gen100);
    positions.emplace(e_gen4095, 3.0f, 0.0f);
    EXPECT_TRUE(positions.contains(e_gen4095));
}

TEST_F(SparseSetTest, NonTrivialComponent_LongString) {
    Entity entity(0, 0);
    std::string longName(1000, 'a');  // 1000 character string

    names.emplace(entity, longName);

    EXPECT_EQ(names.get(entity).value, longName);
}

TEST_F(SparseSetTest, Reserve_ThenClear) {
    positions.reserve(1000);

    for (std::uint32_t i = 0; i < 100; ++i) {
        Entity entity(i, 0);
        positions.emplace(entity, static_cast<float>(i), 0.0f);
    }

    positions.clear();
    EXPECT_EQ(positions.size(), 0);

    // Should still be able to add after clear
    Entity entity(0, 0);
    positions.emplace(entity, 1.0f, 2.0f);
    EXPECT_TRUE(positions.contains(entity));
}
