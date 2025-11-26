/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for TagSparseSet container (empty/tag components)
*/

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "../../../src/engine/ecs/core/Entity.hpp"
#include "../../../src/engine/ecs/storage/TagSparseSet.hpp"

using namespace ECS;

// ============================================================================
// TAG COMPONENT DEFINITIONS
// ============================================================================

// Empty tag components (markers)
struct PlayerTag {};
struct EnemyTag {};
struct DeadTag {};
struct FrozenTag {};
struct InvisibleTag {};

// Verify they are empty types
static_assert(std::is_empty_v<PlayerTag>, "PlayerTag must be empty");
static_assert(std::is_empty_v<EnemyTag>, "EnemyTag must be empty");
static_assert(std::is_empty_v<DeadTag>, "DeadTag must be empty");

// ============================================================================
// TAG SPARSE SET BASIC TESTS
// ============================================================================

class TagSparseSetTest : public ::testing::Test {
protected:
    TagSparseSet<PlayerTag> players;
    TagSparseSet<EnemyTag> enemies;
    TagSparseSet<DeadTag> dead;

    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TagSparseSetTest, Constructor_EmptySet) {
    EXPECT_EQ(players.size(), 0);
}

TEST_F(TagSparseSetTest, Emplace_SingleTag) {
    Entity entity(0, 0);

    players.emplace(entity);

    EXPECT_EQ(players.size(), 1);
    EXPECT_TRUE(players.contains(entity));
}

TEST_F(TagSparseSetTest, Emplace_MultipleTags) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    players.emplace(e1);
    players.emplace(e2);
    players.emplace(e3);

    EXPECT_EQ(players.size(), 3);
    EXPECT_TRUE(players.contains(e1));
    EXPECT_TRUE(players.contains(e2));
    EXPECT_TRUE(players.contains(e3));
}

TEST_F(TagSparseSetTest, Emplace_Idempotent) {
    Entity entity(0, 0);

    players.emplace(entity);
    players.emplace(entity);  // Duplicate
    players.emplace(entity);  // Duplicate

    EXPECT_EQ(players.size(), 1);
}

TEST_F(TagSparseSetTest, Emplace_IgnoresArguments) {
    Entity entity(0, 0);

    // TagSparseSet should ignore any arguments passed to emplace
    players.emplace(entity, 1, 2, 3, "ignored");

    EXPECT_EQ(players.size(), 1);
    EXPECT_TRUE(players.contains(entity));
}

// ============================================================================
// CONTAINS TESTS
// ============================================================================

TEST_F(TagSparseSetTest, Contains_ExistingEntity) {
    Entity entity(5, 0);
    players.emplace(entity);

    EXPECT_TRUE(players.contains(entity));
}

TEST_F(TagSparseSetTest, Contains_NonExistingEntity) {
    Entity entity(5, 0);

    EXPECT_FALSE(players.contains(entity));
}

TEST_F(TagSparseSetTest, Contains_DifferentGeneration) {
    Entity entity_v1(5, 0);
    Entity entity_v2(5, 1);  // Same index, different generation

    players.emplace(entity_v1);

    EXPECT_TRUE(players.contains(entity_v1));
    EXPECT_FALSE(players.contains(entity_v2));
}

TEST_F(TagSparseSetTest, Contains_AfterRemoval) {
    Entity entity(0, 0);
    players.emplace(entity);
    players.remove(entity);

    EXPECT_FALSE(players.contains(entity));
}

// ============================================================================
// GET TESTS (returns dummy instance)
// ============================================================================

TEST_F(TagSparseSetTest, Get_ReturnsDummyInstance) {
    Entity entity(0, 0);
    players.emplace(entity);

    // Get should return a reference to dummy (no crash)
    auto& tag = players.get(entity);
    (void)tag;  // Suppress unused warning

    SUCCEED();  // If we get here, the test passes
}

TEST_F(TagSparseSetTest, Get_ThrowsForMissingEntity) {
    Entity entity(0, 0);

    EXPECT_THROW(players.get(entity), std::runtime_error);
}

TEST_F(TagSparseSetTest, GetConst_ReturnsDummyInstance) {
    Entity entity(0, 0);
    players.emplace(entity);

    const auto& const_players = players;
    const auto& tag = const_players.get(entity);
    (void)tag;

    SUCCEED();
}

TEST_F(TagSparseSetTest, GetConst_ThrowsForMissingEntity) {
    const auto& const_players = players;
    Entity entity(0, 0);

    EXPECT_THROW(const_players.get(entity), std::runtime_error);
}

// ============================================================================
// REMOVE TESTS
// ============================================================================

TEST_F(TagSparseSetTest, Remove_ExistingEntity) {
    Entity entity(0, 0);
    players.emplace(entity);

    players.remove(entity);

    EXPECT_EQ(players.size(), 0);
    EXPECT_FALSE(players.contains(entity));
}

TEST_F(TagSparseSetTest, Remove_NonExistingEntity_NoError) {
    Entity entity(0, 0);

    EXPECT_NO_THROW(players.remove(entity));
    EXPECT_EQ(players.size(), 0);
}

TEST_F(TagSparseSetTest, Remove_SwapAndPop_MaintainsOtherTags) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    players.emplace(e1);
    players.emplace(e2);
    players.emplace(e3);

    // Remove middle entity
    players.remove(e2);

    EXPECT_EQ(players.size(), 2);
    EXPECT_TRUE(players.contains(e1));
    EXPECT_FALSE(players.contains(e2));
    EXPECT_TRUE(players.contains(e3));
}

TEST_F(TagSparseSetTest, Remove_AllEntities) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    players.emplace(e1);
    players.emplace(e2);

    players.remove(e1);
    players.remove(e2);

    EXPECT_EQ(players.size(), 0);
}

// ============================================================================
// CLEAR TESTS
// ============================================================================

TEST_F(TagSparseSetTest, Clear_RemovesAllTags) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    players.emplace(e1);
    players.emplace(e2);
    players.emplace(e3);

    players.clear();

    EXPECT_EQ(players.size(), 0);
    EXPECT_FALSE(players.contains(e1));
    EXPECT_FALSE(players.contains(e2));
    EXPECT_FALSE(players.contains(e3));
}

TEST_F(TagSparseSetTest, Clear_EmptySet_NoError) {
    EXPECT_NO_THROW(players.clear());
    EXPECT_EQ(players.size(), 0);
}

// ============================================================================
// GET PACKED TESTS
// ============================================================================

TEST_F(TagSparseSetTest, GetPacked_ReturnsEntityList) {
    Entity e1(10, 0);
    Entity e2(20, 0);
    Entity e3(30, 0);

    players.emplace(e1);
    players.emplace(e2);
    players.emplace(e3);

    const auto& packed = players.getPacked();

    EXPECT_EQ(packed.size(), 3);
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e1) != packed.end());
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e2) != packed.end());
    EXPECT_TRUE(std::find(packed.begin(), packed.end(), e3) != packed.end());
}

TEST_F(TagSparseSetTest, GetPacked_EmptySet) {
    const auto& packed = players.getPacked();
    EXPECT_TRUE(packed.empty());
}

// ============================================================================
// RESERVE AND SHRINK TESTS
// ============================================================================

TEST_F(TagSparseSetTest, Reserve_IncreasesCapacity) {
    players.reserve(1000);

    // Should be able to add 1000 entities without reallocation
    for (std::uint32_t i = 0; i < 1000; ++i) {
        Entity entity(i, 0);
        players.emplace(entity);
    }

    EXPECT_EQ(players.size(), 1000);
}

TEST_F(TagSparseSetTest, ShrinkToFit_ReducesMemory) {
    // Add many entities
    for (std::uint32_t i = 0; i < 100; ++i) {
        Entity entity(i, 0);
        players.emplace(entity);
    }

    // Remove most entities
    for (std::uint32_t i = 10; i < 100; ++i) {
        Entity entity(i, 0);
        players.remove(entity);
    }

    players.shrinkToFit();

    // Should still work correctly
    EXPECT_EQ(players.size(), 10);
    for (std::uint32_t i = 0; i < 10; ++i) {
        Entity entity(i, 0);
        EXPECT_TRUE(players.contains(entity));
    }
}

// ============================================================================
// MULTIPLE TAG TYPES TESTS
// ============================================================================

TEST_F(TagSparseSetTest, MultipleTags_SameEntity) {
    Entity entity(0, 0);

    players.emplace(entity);
    dead.emplace(entity);

    EXPECT_TRUE(players.contains(entity));
    EXPECT_TRUE(dead.contains(entity));
    EXPECT_FALSE(enemies.contains(entity));
}

TEST_F(TagSparseSetTest, MultipleTags_DifferentEntities) {
    Entity player(0, 0);
    Entity enemy(1, 0);

    players.emplace(player);
    enemies.emplace(enemy);

    EXPECT_TRUE(players.contains(player));
    EXPECT_FALSE(players.contains(enemy));
    EXPECT_TRUE(enemies.contains(enemy));
    EXPECT_FALSE(enemies.contains(player));
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(TagSparseSetTest, Stress_LargeNumberOfEntities) {
    constexpr std::uint32_t COUNT = 10000;

    for (std::uint32_t i = 0; i < COUNT; ++i) {
        Entity entity(i, 0);
        players.emplace(entity);
    }

    EXPECT_EQ(players.size(), COUNT);

    // Verify all entities
    for (std::uint32_t i = 0; i < COUNT; ++i) {
        Entity entity(i, 0);
        EXPECT_TRUE(players.contains(entity));
    }
}

TEST_F(TagSparseSetTest, Stress_RepeatedAddRemove) {
    Entity entity(0, 0);

    for (int i = 0; i < 1000; ++i) {
        players.emplace(entity);
        EXPECT_TRUE(players.contains(entity));
        players.remove(entity);
        EXPECT_FALSE(players.contains(entity));
    }

    EXPECT_EQ(players.size(), 0);
}

TEST_F(TagSparseSetTest, Stress_SparseIndices) {
    // Test with very sparse entity indices
    std::vector<std::uint32_t> indices = {0, 100, 500, 1000, 5000, 10000, 50000};

    for (auto idx : indices) {
        Entity entity(idx, 0);
        players.emplace(entity);
    }

    EXPECT_EQ(players.size(), indices.size());

    for (auto idx : indices) {
        Entity entity(idx, 0);
        EXPECT_TRUE(players.contains(entity));
    }
}

// ============================================================================
// INTERFACE COMPLIANCE TESTS
// ============================================================================

TEST_F(TagSparseSetTest, ISparseSet_PolymorphicRemove) {
    Entity entity(0, 0);
    players.emplace(entity);

    ISparseSet* base_ptr = &players;

    EXPECT_TRUE(base_ptr->contains(entity));
    base_ptr->remove(entity);
    EXPECT_FALSE(base_ptr->contains(entity));
}

TEST_F(TagSparseSetTest, ISparseSet_PolymorphicClear) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    players.emplace(e1);
    players.emplace(e2);

    ISparseSet* base_ptr = &players;

    EXPECT_EQ(base_ptr->size(), 2);
    base_ptr->clear();
    EXPECT_EQ(base_ptr->size(), 0);
}

TEST_F(TagSparseSetTest, ISparseSet_GetPacked) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    players.emplace(e1);
    players.emplace(e2);

    ISparseSet* base_ptr = &players;
    const auto& packed = base_ptr->getPacked();

    EXPECT_EQ(packed.size(), 2);
}

TEST_F(TagSparseSetTest, ISparseSet_ShrinkToFit) {
    Entity e1(0, 0);
    players.emplace(e1);

    ISparseSet* base_ptr = &players;
    EXPECT_NO_THROW(base_ptr->shrinkToFit());
}

// ============================================================================
// MEMORY EFFICIENCY TESTS
// ============================================================================

TEST_F(TagSparseSetTest, NoDataStorage_OnlyEntityIds) {
    // This is more of a design verification
    // TagSparseSet should not store any component data, only entity IDs

    Entity e1(0, 0);
    Entity e2(1, 0);

    players.emplace(e1);
    players.emplace(e2);

    // Verify size matches number of entities
    EXPECT_EQ(players.size(), 2);
    EXPECT_EQ(players.getPacked().size(), 2);
}
