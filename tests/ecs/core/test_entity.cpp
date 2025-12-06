/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Entity struct
*/

#include <gtest/gtest.h>
#include <unordered_set>
#include <unordered_map>
#include "../../../lib/rtype_ecs/src/core/Entity.hpp"

using namespace ECS;

// ============================================================================
// ENTITY CONSTRUCTION TESTS
// ============================================================================

class EntityTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EntityTest, DefaultConstructor_CreatesNullEntity) {
    Entity entity;
    EXPECT_TRUE(entity.isNull());
    EXPECT_EQ(entity.id, Entity::_NullID);
}

TEST_F(EntityTest, RawIdConstructor_SetsCorrectId) {
    Entity entity(42);
    EXPECT_EQ(entity.id, 42);
    EXPECT_FALSE(entity.isNull());
}

TEST_F(EntityTest, IndexGenerationConstructor_PacksCorrectly) {
    Entity entity(100, 5);

    EXPECT_EQ(entity.index(), 100);
    EXPECT_EQ(entity.generation(), 5);
    EXPECT_FALSE(entity.isNull());
}

TEST_F(EntityTest, IndexGenerationConstructor_ZeroValues) {
    Entity entity(0, 0);

    EXPECT_EQ(entity.index(), 0);
    EXPECT_EQ(entity.generation(), 0);
    EXPECT_FALSE(entity.isNull());
}

// ============================================================================
// INDEX AND GENERATION EXTRACTION TESTS
// ============================================================================

TEST_F(EntityTest, Index_ExtractsLower20Bits) {
    // Max index value (2^20 - 1 = 1,048,575)
    Entity entity(Entity::_IndexMask, 0);
    EXPECT_EQ(entity.index(), Entity::_IndexMask);
}

TEST_F(EntityTest, Generation_ExtractsUpper12Bits) {
    // Max generation value (2^12 - 1 = 4,095)
    Entity entity(0, Entity::_GenerationMask);
    EXPECT_EQ(entity.generation(), Entity::_GenerationMask);
}

TEST_F(EntityTest, IndexAndGeneration_BothMaxValues) {
    Entity entity(Entity::_IndexMask, Entity::_GenerationMask);

    EXPECT_EQ(entity.index(), Entity::_IndexMask);
    EXPECT_EQ(entity.generation(), Entity::_GenerationMask);
}

TEST_F(EntityTest, IndexOverflow_MaskedCorrectly) {
    // If index exceeds 20 bits, it should be masked
    std::uint32_t overflow_index = Entity::_IndexMask + 1;
    Entity entity(overflow_index, 0);

    // Should wrap around to 0
    EXPECT_EQ(entity.index(), 0);
}

TEST_F(EntityTest, GenerationOverflow_MaskedCorrectly) {
    // If generation exceeds 12 bits, it should be masked
    std::uint32_t overflow_gen = Entity::_GenerationMask + 1;
    Entity entity(0, overflow_gen);

    // Should wrap around to 0
    EXPECT_EQ(entity.generation(), 0);
}

// ============================================================================
// NULL AND TOMBSTONE TESTS
// ============================================================================

TEST_F(EntityTest, IsNull_TrueForDefaultEntity) {
    Entity entity;
    EXPECT_TRUE(entity.isNull());
}

TEST_F(EntityTest, IsNull_FalseForValidEntity) {
    Entity entity(0, 0);
    EXPECT_FALSE(entity.isNull());
}

TEST_F(EntityTest, IsNull_FalseForMaxValidEntity) {
    Entity entity(Entity::_IndexMask, Entity::_GenerationMask - 1);
    EXPECT_FALSE(entity.isNull());
}

TEST_F(EntityTest, IsTombstone_TrueWhenGenerationIsMax) {
    Entity entity(0, Entity::_MaxGeneration);
    EXPECT_TRUE(entity.isTombstone());
}

TEST_F(EntityTest, IsTombstone_FalseForNormalEntity) {
    Entity entity(42, 5);
    EXPECT_FALSE(entity.isTombstone());
}

TEST_F(EntityTest, IsTombstone_FalseWhenGenerationBelowMax) {
    Entity entity(0, Entity::_MaxGeneration - 1);
    EXPECT_FALSE(entity.isTombstone());
}

// ============================================================================
// COMPARISON OPERATOR TESTS
// ============================================================================

TEST_F(EntityTest, Equality_SameEntity) {
    Entity e1(100, 5);
    Entity e2(100, 5);

    EXPECT_TRUE(e1 == e2);
    EXPECT_FALSE(e1 != e2);
}

TEST_F(EntityTest, Inequality_DifferentIndex) {
    Entity e1(100, 5);
    Entity e2(101, 5);

    EXPECT_FALSE(e1 == e2);
    EXPECT_TRUE(e1 != e2);
}

TEST_F(EntityTest, Inequality_DifferentGeneration) {
    Entity e1(100, 5);
    Entity e2(100, 6);

    EXPECT_FALSE(e1 == e2);
    EXPECT_TRUE(e1 != e2);
}

TEST_F(EntityTest, ThreeWayComparison_LessThan) {
    Entity e1(50, 0);
    Entity e2(100, 0);

    EXPECT_TRUE(e1 < e2);
    EXPECT_FALSE(e2 < e1);
}

TEST_F(EntityTest, ThreeWayComparison_GenerationAffectsOrdering) {
    Entity e1(0, 1);
    Entity e2(0, 2);

    EXPECT_TRUE(e1 < e2);
}

// ============================================================================
// HASH FUNCTION TESTS
// ============================================================================

TEST_F(EntityTest, Hash_DifferentEntitiesHaveDifferentHashes) {
    std::hash<Entity> hasher;

    Entity e1(100, 5);
    Entity e2(101, 5);
    Entity e3(100, 6);

    // Different entities should (generally) have different hashes
    EXPECT_NE(hasher(e1), hasher(e2));
    EXPECT_NE(hasher(e1), hasher(e3));
}

TEST_F(EntityTest, Hash_SameEntityHasSameHash) {
    std::hash<Entity> hasher;

    Entity e1(100, 5);
    Entity e2(100, 5);

    EXPECT_EQ(hasher(e1), hasher(e2));
}

TEST_F(EntityTest, Hash_WorksInUnorderedSet) {
    std::unordered_set<Entity> entities;

    Entity e1(1, 0);
    Entity e2(2, 0);
    Entity e3(1, 1);  // Same index, different generation

    entities.insert(e1);
    entities.insert(e2);
    entities.insert(e3);

    EXPECT_EQ(entities.size(), 3);
    EXPECT_TRUE(entities.count(e1) == 1);
    EXPECT_TRUE(entities.count(e2) == 1);
    EXPECT_TRUE(entities.count(e3) == 1);
}

TEST_F(EntityTest, Hash_WorksInUnorderedMap) {
    std::unordered_map<Entity, std::string> entity_names;

    Entity player(1, 0);
    Entity enemy(2, 0);

    entity_names[player] = "Player";
    entity_names[enemy] = "Enemy";

    EXPECT_EQ(entity_names[player], "Player");
    EXPECT_EQ(entity_names[enemy], "Enemy");
}

// ============================================================================
// CONSTEXPR TESTS
// ============================================================================

TEST_F(EntityTest, Constexpr_DefaultConstruction) {
    constexpr Entity entity;
    static_assert(entity.isNull(), "Default entity should be null");
}

TEST_F(EntityTest, Constexpr_IndexExtraction) {
    constexpr Entity entity(42, 7);
    static_assert(entity.index() == 42, "Index should be 42");
    static_assert(entity.generation() == 7, "Generation should be 7");
}

TEST_F(EntityTest, Constexpr_IsNullCheck) {
    constexpr Entity null_entity;
    constexpr Entity valid_entity(0, 0);

    static_assert(null_entity.isNull(), "Default should be null");
    static_assert(!valid_entity.isNull(), "Valid entity should not be null");
}

// ============================================================================
// BIT LAYOUT VERIFICATION TESTS
// ============================================================================

TEST_F(EntityTest, BitLayout_Constants) {
    EXPECT_EQ(Entity::_IndexBits, 20);
    EXPECT_EQ(Entity::_GenerationBits, 12);
    EXPECT_EQ(Entity::_IndexBits + Entity::_GenerationBits, 32);
}

TEST_F(EntityTest, BitLayout_Masks) {
    EXPECT_EQ(Entity::_IndexMask, (1 << 20) - 1);  // 0xFFFFF
    EXPECT_EQ(Entity::_GenerationMask, (1 << 12) - 1);  // 0xFFF
}

TEST_F(EntityTest, BitLayout_MaxGeneration) {
    EXPECT_EQ(Entity::_MaxGeneration, Entity::_GenerationMask);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(EntityTest, EdgeCase_EntityWithMaxIndex) {
    Entity entity(Entity::_IndexMask, 0);

    EXPECT_EQ(entity.index(), Entity::_IndexMask);
    EXPECT_EQ(entity.generation(), 0);
    EXPECT_FALSE(entity.isNull());
    EXPECT_FALSE(entity.isTombstone());
}

TEST_F(EntityTest, EdgeCase_EntityBeforeTombstone) {
    Entity entity(0, Entity::_MaxGeneration - 1);

    EXPECT_FALSE(entity.isTombstone());
    EXPECT_EQ(entity.generation(), Entity::_MaxGeneration - 1);
}

TEST_F(EntityTest, EdgeCase_CopyConstruction) {
    Entity original(123, 45);
    Entity copy(original);

    EXPECT_EQ(original, copy);
    EXPECT_EQ(original.id, copy.id);
}

TEST_F(EntityTest, EdgeCase_Assignment) {
    Entity e1(100, 5);
    Entity e2(200, 10);

    e1 = e2;

    EXPECT_EQ(e1, e2);
    EXPECT_EQ(e1.index(), 200);
    EXPECT_EQ(e1.generation(), 10);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS
// ============================================================================

TEST_F(EntityTest, RawIdConstructor_WithNullId) {
    Entity entity(Entity::_NullID);
    EXPECT_TRUE(entity.isNull());
    EXPECT_EQ(entity.id, Entity::_NullID);
}

TEST_F(EntityTest, IndexGeneration_BothZero_IsValid) {
    Entity entity(0, 0);
    EXPECT_FALSE(entity.isNull());
    EXPECT_EQ(entity.index(), 0);
    EXPECT_EQ(entity.generation(), 0);
}

TEST_F(EntityTest, IsTombstone_WithNonZeroIndex) {
    Entity entity(42, Entity::_MaxGeneration);
    EXPECT_TRUE(entity.isTombstone());
    EXPECT_EQ(entity.index(), 42);
}

TEST_F(EntityTest, ThreeWayComparison_Equal) {
    Entity e1(10, 5);
    Entity e2(10, 5);
    EXPECT_TRUE((e1 <=> e2) == 0);
}

TEST_F(EntityTest, ThreeWayComparison_GreaterThan) {
    Entity e1(10, 0);
    Entity e2(5, 0);
    EXPECT_TRUE(e1 > e2);
    EXPECT_FALSE(e1 < e2);
}

TEST_F(EntityTest, Hash_NullEntity) {
    Entity null_entity;
    std::hash<Entity> hasher;
    std::size_t hash = hasher(null_entity);
    EXPECT_EQ(hash, std::hash<uint32_t>{}(Entity::_NullID));
}

TEST_F(EntityTest, Hash_ZeroEntity) {
    Entity zero_entity(0, 0);
    std::hash<Entity> hasher;
    std::size_t hash = hasher(zero_entity);
    EXPECT_EQ(hash, std::hash<uint32_t>{}(0));
}

TEST_F(EntityTest, Equality_NullEntities) {
    Entity e1;
    Entity e2;
    EXPECT_EQ(e1, e2);
}

TEST_F(EntityTest, Inequality_NullVsValid) {
    Entity null_entity;
    Entity valid_entity(0, 0);
    EXPECT_NE(null_entity, valid_entity);
}

TEST_F(EntityTest, Hash_CollisionResistance) {
    // Test that different entities have different hashes
    std::unordered_set<std::size_t> hashes;
    std::hash<Entity> hasher;

    for (uint32_t i = 0; i < 100; ++i) {
        Entity e(i, 0);
        hashes.insert(hasher(e));
    }

    // All hashes should be unique for sequential entities
    EXPECT_EQ(hashes.size(), 100);
}

TEST_F(EntityTest, MoveConstruction) {
    Entity original(123, 45);
    Entity moved(std::move(original));

    EXPECT_EQ(moved.index(), 123);
    EXPECT_EQ(moved.generation(), 45);
}

TEST_F(EntityTest, MoveAssignment) {
    Entity e1(100, 5);
    Entity e2(200, 10);

    e1 = std::move(e2);

    EXPECT_EQ(e1.index(), 200);
    EXPECT_EQ(e1.generation(), 10);
}

TEST_F(EntityTest, Constexpr_IsTombstoneCheck) {
    constexpr Entity tombstone(0, Entity::_MaxGeneration);
    static_assert(tombstone.isTombstone(), "Should be tombstone");

    constexpr Entity normal(0, 0);
    static_assert(!normal.isTombstone(), "Should not be tombstone");
}

TEST_F(EntityTest, BitLayout_PackingVerification) {
    // Verify that index and generation are packed correctly
    Entity e(0b11111111111111111111, 0b111111111111);  // Max index, max generation

    EXPECT_EQ(e.index(), Entity::_IndexMask);
    EXPECT_EQ(e.generation(), Entity::_GenerationMask);
}

TEST_F(EntityTest, BitLayout_SpecificValues) {
    Entity e(1234567, 2048);

    uint32_t expectedIndex = 1234567 & Entity::_IndexMask;
    uint32_t expectedGen = 2048 & Entity::_GenerationMask;

    EXPECT_EQ(e.index(), expectedIndex);
    EXPECT_EQ(e.generation(), expectedGen);
}
