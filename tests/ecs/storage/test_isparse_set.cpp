/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for ISparseSet interface
*/

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "../../../src/engine/ecs/core/Entity.hpp"
#include "../../../src/engine/ecs/storage/ISparseSet.hpp"
#include "../../../src/engine/ecs/storage/SparseSet.hpp"
#include "../../../src/engine/ecs/storage/TagSparseSet.hpp"

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct TestComponent {
    int value;
    TestComponent() : value(0) {}
    TestComponent(int v) : value(v) {}
};

struct EmptyTag {};

// ============================================================================
// ISPARSE SET INTERFACE TESTS
// ============================================================================

class ISparseSetTest : public ::testing::Test {
protected:
    std::unique_ptr<ISparseSet> sparse_set;
    std::unique_ptr<ISparseSet> tag_set;

    void SetUp() override {
        sparse_set = std::make_unique<SparseSet<TestComponent>>();
        tag_set = std::make_unique<TagSparseSet<EmptyTag>>();
    }

    void TearDown() override {
        sparse_set.reset();
        tag_set.reset();
    }

    // Helper to cast back to concrete type for emplacing
    SparseSet<TestComponent>& getSparseSet() {
        return static_cast<SparseSet<TestComponent>&>(*sparse_set);
    }

    TagSparseSet<EmptyTag>& getTagSet() {
        return static_cast<TagSparseSet<EmptyTag>&>(*tag_set);
    }
};

// ============================================================================
// CONTAINS TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Contains_EmptySet) {
    Entity entity(0, 0);

    EXPECT_FALSE(sparse_set->contains(entity));
    EXPECT_FALSE(tag_set->contains(entity));
}

TEST_F(ISparseSetTest, Contains_ExistingEntity) {
    Entity entity(0, 0);

    getSparseSet().emplace(entity, 42);
    getTagSet().emplace(entity);

    EXPECT_TRUE(sparse_set->contains(entity));
    EXPECT_TRUE(tag_set->contains(entity));
}

TEST_F(ISparseSetTest, Contains_NonExistingEntity) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 42);
    getTagSet().emplace(e1);

    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_FALSE(tag_set->contains(e2));
}

TEST_F(ISparseSetTest, Contains_DifferentGeneration) {
    Entity e_v1(5, 0);
    Entity e_v2(5, 1);  // Same index, different generation

    getSparseSet().emplace(e_v1, 42);
    getTagSet().emplace(e_v1);

    EXPECT_TRUE(sparse_set->contains(e_v1));
    EXPECT_FALSE(sparse_set->contains(e_v2));
    EXPECT_TRUE(tag_set->contains(e_v1));
    EXPECT_FALSE(tag_set->contains(e_v2));
}

// ============================================================================
// REMOVE TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Remove_ExistingEntity) {
    Entity entity(0, 0);

    getSparseSet().emplace(entity, 42);
    getTagSet().emplace(entity);

    sparse_set->remove(entity);
    tag_set->remove(entity);

    EXPECT_FALSE(sparse_set->contains(entity));
    EXPECT_FALSE(tag_set->contains(entity));
}

TEST_F(ISparseSetTest, Remove_NonExistingEntity) {
    Entity entity(0, 0);

    // Should not throw
    EXPECT_NO_THROW(sparse_set->remove(entity));
    EXPECT_NO_THROW(tag_set->remove(entity));
}

TEST_F(ISparseSetTest, Remove_MaintainsOtherEntities) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    getTagSet().emplace(e1);
    getTagSet().emplace(e2);
    getTagSet().emplace(e3);

    sparse_set->remove(e2);
    tag_set->remove(e2);

    EXPECT_TRUE(sparse_set->contains(e1));
    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_TRUE(sparse_set->contains(e3));

    EXPECT_TRUE(tag_set->contains(e1));
    EXPECT_FALSE(tag_set->contains(e2));
    EXPECT_TRUE(tag_set->contains(e3));
}

// ============================================================================
// CLEAR TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Clear_RemovesAllEntities) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    getTagSet().emplace(e1);
    getTagSet().emplace(e2);
    getTagSet().emplace(e3);

    sparse_set->clear();
    tag_set->clear();

    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_FALSE(sparse_set->contains(e1));
    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_FALSE(sparse_set->contains(e3));

    EXPECT_EQ(tag_set->size(), 0);
    EXPECT_FALSE(tag_set->contains(e1));
    EXPECT_FALSE(tag_set->contains(e2));
    EXPECT_FALSE(tag_set->contains(e3));
}

TEST_F(ISparseSetTest, Clear_EmptySet) {
    EXPECT_NO_THROW(sparse_set->clear());
    EXPECT_NO_THROW(tag_set->clear());
    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(tag_set->size(), 0);
}

// ============================================================================
// SIZE TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Size_EmptySet) {
    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(tag_set->size(), 0);
}

TEST_F(ISparseSetTest, Size_AfterEmplace) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    EXPECT_EQ(sparse_set->size(), 1);

    getSparseSet().emplace(e2, 2);
    EXPECT_EQ(sparse_set->size(), 2);

    getTagSet().emplace(e1);
    EXPECT_EQ(tag_set->size(), 1);

    getTagSet().emplace(e2);
    EXPECT_EQ(tag_set->size(), 2);
}

TEST_F(ISparseSetTest, Size_AfterRemove) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getTagSet().emplace(e1);
    getTagSet().emplace(e2);

    sparse_set->remove(e1);
    tag_set->remove(e1);

    EXPECT_EQ(sparse_set->size(), 1);
    EXPECT_EQ(tag_set->size(), 1);
}

TEST_F(ISparseSetTest, Size_AfterClear) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getTagSet().emplace(e1);
    getTagSet().emplace(e2);

    sparse_set->clear();
    tag_set->clear();

    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(tag_set->size(), 0);
}

// ============================================================================
// SHRINK TO FIT TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, ShrinkToFit_NoError) {
    Entity e1(0, 0);
    getSparseSet().emplace(e1, 1);
    getTagSet().emplace(e1);

    EXPECT_NO_THROW(sparse_set->shrinkToFit());
    EXPECT_NO_THROW(tag_set->shrinkToFit());

    // Data should still be intact
    EXPECT_TRUE(sparse_set->contains(e1));
    EXPECT_TRUE(tag_set->contains(e1));
}

TEST_F(ISparseSetTest, ShrinkToFit_EmptySet) {
    EXPECT_NO_THROW(sparse_set->shrinkToFit());
    EXPECT_NO_THROW(tag_set->shrinkToFit());
}

// ============================================================================
// GET PACKED TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, GetPacked_EmptySet) {
    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_tag = tag_set->getPacked();

    EXPECT_TRUE(packed_sparse.empty());
    EXPECT_TRUE(packed_tag.empty());
}

TEST_F(ISparseSetTest, GetPacked_ContainsAllEntities) {
    Entity e1(10, 0);
    Entity e2(20, 0);
    Entity e3(30, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    getTagSet().emplace(e1);
    getTagSet().emplace(e2);
    getTagSet().emplace(e3);

    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_tag = tag_set->getPacked();

    EXPECT_EQ(packed_sparse.size(), 3);
    EXPECT_EQ(packed_tag.size(), 3);

    // Check all entities are present
    auto contains_entity = [](const std::vector<Entity>& vec, Entity e) {
        return std::find(vec.begin(), vec.end(), e) != vec.end();
    };

    EXPECT_TRUE(contains_entity(packed_sparse, e1));
    EXPECT_TRUE(contains_entity(packed_sparse, e2));
    EXPECT_TRUE(contains_entity(packed_sparse, e3));

    EXPECT_TRUE(contains_entity(packed_tag, e1));
    EXPECT_TRUE(contains_entity(packed_tag, e2));
    EXPECT_TRUE(contains_entity(packed_tag, e3));
}

TEST_F(ISparseSetTest, GetPacked_UpdatesAfterRemove) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);

    getTagSet().emplace(e1);
    getTagSet().emplace(e2);

    sparse_set->remove(e1);
    tag_set->remove(e1);

    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_tag = tag_set->getPacked();

    EXPECT_EQ(packed_sparse.size(), 1);
    EXPECT_EQ(packed_tag.size(), 1);
}

// ============================================================================
// POLYMORPHISM TESTS
// ============================================================================

TEST_F(ISparseSetTest, Polymorphism_DifferentConcreteTypes) {
    std::vector<std::unique_ptr<ISparseSet>> pools;

    pools.push_back(std::make_unique<SparseSet<TestComponent>>());
    pools.push_back(std::make_unique<TagSparseSet<EmptyTag>>());

    Entity entity(0, 0);

    // Emplace using concrete types
    static_cast<SparseSet<TestComponent>&>(*pools[0]).emplace(entity, 42);
    static_cast<TagSparseSet<EmptyTag>&>(*pools[1]).emplace(entity);

    // Use interface methods
    for (auto& pool : pools) {
        EXPECT_TRUE(pool->contains(entity));
        EXPECT_EQ(pool->size(), 1);
    }

    // Clear all pools through interface
    for (auto& pool : pools) {
        pool->clear();
        EXPECT_EQ(pool->size(), 0);
    }
}

TEST_F(ISparseSetTest, Polymorphism_HeterogeneousContainer) {
    std::vector<ISparseSet*> pools;

    SparseSet<TestComponent> component_pool;
    TagSparseSet<EmptyTag> tag_pool;

    pools.push_back(&component_pool);
    pools.push_back(&tag_pool);

    Entity entity(5, 0);

    component_pool.emplace(entity, 100);
    tag_pool.emplace(entity);

    // All pools contain the entity
    for (auto* pool : pools) {
        EXPECT_TRUE(pool->contains(entity));
    }

    // Remove from all pools
    for (auto* pool : pools) {
        pool->remove(entity);
    }

    // All pools are now empty
    for (auto* pool : pools) {
        EXPECT_FALSE(pool->contains(entity));
        EXPECT_EQ(pool->size(), 0);
    }
}

// ============================================================================
// VIRTUAL DESTRUCTOR TEST
// ============================================================================

TEST_F(ISparseSetTest, VirtualDestructor_SafeDeletion) {
    // Test that deleting through base pointer is safe
    ISparseSet* base_ptr = new SparseSet<TestComponent>();

    Entity entity(0, 0);
    static_cast<SparseSet<TestComponent>*>(base_ptr)->emplace(entity, 42);

    EXPECT_TRUE(base_ptr->contains(entity));

    // Should not leak or crash
    EXPECT_NO_THROW(delete base_ptr);
}

TEST_F(ISparseSetTest, VirtualDestructor_TagSet) {
    ISparseSet* base_ptr = new TagSparseSet<EmptyTag>();

    Entity entity(0, 0);
    static_cast<TagSparseSet<EmptyTag>*>(base_ptr)->emplace(entity);

    EXPECT_TRUE(base_ptr->contains(entity));

    EXPECT_NO_THROW(delete base_ptr);
}
