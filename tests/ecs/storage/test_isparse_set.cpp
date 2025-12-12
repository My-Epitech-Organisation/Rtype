/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for ISparseSet interface
*/

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "../../../lib/ecs/src/core/Entity.hpp"
#include "../../../lib/ecs/src/storage/ISparseSet.hpp"
#include "../../../lib/ecs/src/storage/SparseSet.hpp"

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct TestComponent {
    int value;
    TestComponent() : value(0) {}
    TestComponent(int v) : value(v) {}
};

struct AnotherComponent {
    float x;
    AnotherComponent() : x(0.0f) {}
    AnotherComponent(float val) : x(val) {}
};

// ============================================================================
// ISPARSE SET INTERFACE TESTS
// ============================================================================

class ISparseSetTest : public ::testing::Test {
protected:
    std::unique_ptr<ISparseSet> sparse_set;
    std::unique_ptr<ISparseSet> another_set;

    void SetUp() override {
        sparse_set = std::make_unique<SparseSet<TestComponent>>();
        another_set = std::make_unique<SparseSet<AnotherComponent>>();
    }

    void TearDown() override {
        sparse_set.reset();
        another_set.reset();
    }

    // Helper to cast back to concrete type for emplacing
    SparseSet<TestComponent>& getSparseSet() {
        return static_cast<SparseSet<TestComponent>&>(*sparse_set);
    }

    SparseSet<AnotherComponent>& getAnotherSet() {
        return static_cast<SparseSet<AnotherComponent>&>(*another_set);
    }
};

// ============================================================================
// CONTAINS TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Contains_EmptySet) {
    Entity entity(0, 0);

    EXPECT_FALSE(sparse_set->contains(entity));
    EXPECT_FALSE(another_set->contains(entity));
}

TEST_F(ISparseSetTest, Contains_ExistingEntity) {
    Entity entity(0, 0);

    getSparseSet().emplace(entity, 42);
    getAnotherSet().emplace(entity, 3.14f);

    EXPECT_TRUE(sparse_set->contains(entity));
    EXPECT_TRUE(another_set->contains(entity));
}

TEST_F(ISparseSetTest, Contains_NonExistingEntity) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 42);
    getAnotherSet().emplace(e1, 3.14f);

    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_FALSE(another_set->contains(e2));
}

TEST_F(ISparseSetTest, Contains_DifferentGeneration) {
    Entity e_v1(5, 0);
    Entity e_v2(5, 1);  // Same index, different generation

    getSparseSet().emplace(e_v1, 42);
    getAnotherSet().emplace(e_v1, 3.14f);

    EXPECT_TRUE(sparse_set->contains(e_v1));
    EXPECT_FALSE(sparse_set->contains(e_v2));
    EXPECT_TRUE(another_set->contains(e_v1));
    EXPECT_FALSE(another_set->contains(e_v2));
}

// ============================================================================
// REMOVE TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Remove_ExistingEntity) {
    Entity entity(0, 0);

    getSparseSet().emplace(entity, 42);
    getAnotherSet().emplace(entity, 3.14f);

    sparse_set->remove(entity);
    another_set->remove(entity);

    EXPECT_FALSE(sparse_set->contains(entity));
    EXPECT_FALSE(another_set->contains(entity));
}

TEST_F(ISparseSetTest, Remove_NonExistingEntity) {
    Entity entity(0, 0);

    // Should not throw
    EXPECT_NO_THROW(sparse_set->remove(entity));
    EXPECT_NO_THROW(another_set->remove(entity));
}

TEST_F(ISparseSetTest, Remove_MaintainsOtherEntities) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);
    getAnotherSet().emplace(e3, 3.0f);

    sparse_set->remove(e2);
    another_set->remove(e2);

    EXPECT_TRUE(sparse_set->contains(e1));
    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_TRUE(sparse_set->contains(e3));

    EXPECT_TRUE(another_set->contains(e1));
    EXPECT_FALSE(another_set->contains(e2));
    EXPECT_TRUE(another_set->contains(e3));
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

    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);
    getAnotherSet().emplace(e3, 3.0f);

    sparse_set->clear();
    another_set->clear();

    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_FALSE(sparse_set->contains(e1));
    EXPECT_FALSE(sparse_set->contains(e2));
    EXPECT_FALSE(sparse_set->contains(e3));

    EXPECT_EQ(another_set->size(), 0);
    EXPECT_FALSE(another_set->contains(e1));
    EXPECT_FALSE(another_set->contains(e2));
    EXPECT_FALSE(another_set->contains(e3));
}

TEST_F(ISparseSetTest, Clear_EmptySet) {
    EXPECT_NO_THROW(sparse_set->clear());
    EXPECT_NO_THROW(another_set->clear());
    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(another_set->size(), 0);
}

// ============================================================================
// SIZE TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, Size_EmptySet) {
    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(another_set->size(), 0);
}

TEST_F(ISparseSetTest, Size_AfterEmplace) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    EXPECT_EQ(sparse_set->size(), 1);

    getSparseSet().emplace(e2, 2);
    EXPECT_EQ(sparse_set->size(), 2);

    getAnotherSet().emplace(e1, 1.0f);
    EXPECT_EQ(another_set->size(), 1);

    getAnotherSet().emplace(e2, 2.0f);
    EXPECT_EQ(another_set->size(), 2);
}

TEST_F(ISparseSetTest, Size_AfterRemove) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);

    sparse_set->remove(e1);
    another_set->remove(e1);

    EXPECT_EQ(sparse_set->size(), 1);
    EXPECT_EQ(another_set->size(), 1);
}

TEST_F(ISparseSetTest, Size_AfterClear) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);

    sparse_set->clear();
    another_set->clear();

    EXPECT_EQ(sparse_set->size(), 0);
    EXPECT_EQ(another_set->size(), 0);
}

// ============================================================================
// SHRINK TO FIT TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, ShrinkToFit_NoError) {
    Entity e1(0, 0);
    getSparseSet().emplace(e1, 1);
    getAnotherSet().emplace(e1, 1.0f);

    EXPECT_NO_THROW(sparse_set->shrinkToFit());
    EXPECT_NO_THROW(another_set->shrinkToFit());

    // Data should still be intact
    EXPECT_TRUE(sparse_set->contains(e1));
    EXPECT_TRUE(another_set->contains(e1));
}

TEST_F(ISparseSetTest, ShrinkToFit_EmptySet) {
    EXPECT_NO_THROW(sparse_set->shrinkToFit());
    EXPECT_NO_THROW(another_set->shrinkToFit());
}

// ============================================================================
// GET PACKED TESTS (ISparseSet Interface)
// ============================================================================

TEST_F(ISparseSetTest, GetPacked_EmptySet) {
    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_another = another_set->getPacked();

    EXPECT_TRUE(packed_sparse.empty());
    EXPECT_TRUE(packed_another.empty());
}

TEST_F(ISparseSetTest, GetPacked_ContainsAllEntities) {
    Entity e1(10, 0);
    Entity e2(20, 0);
    Entity e3(30, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);
    getAnotherSet().emplace(e3, 3.0f);

    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_another = another_set->getPacked();

    EXPECT_EQ(packed_sparse.size(), 3);
    EXPECT_EQ(packed_another.size(), 3);

    // Check all entities are present
    auto contains_entity = [](const std::vector<Entity>& vec, Entity e) {
        return std::find(vec.begin(), vec.end(), e) != vec.end();
    };

    EXPECT_TRUE(contains_entity(packed_sparse, e1));
    EXPECT_TRUE(contains_entity(packed_sparse, e2));
    EXPECT_TRUE(contains_entity(packed_sparse, e3));

    EXPECT_TRUE(contains_entity(packed_another, e1));
    EXPECT_TRUE(contains_entity(packed_another, e2));
    EXPECT_TRUE(contains_entity(packed_another, e3));
}

TEST_F(ISparseSetTest, GetPacked_UpdatesAfterRemove) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);

    getAnotherSet().emplace(e1, 1.0f);
    getAnotherSet().emplace(e2, 2.0f);

    sparse_set->remove(e1);
    another_set->remove(e1);

    const auto& packed_sparse = sparse_set->getPacked();
    const auto& packed_another = another_set->getPacked();

    EXPECT_EQ(packed_sparse.size(), 1);
    EXPECT_EQ(packed_another.size(), 1);
}

// ============================================================================
// POLYMORPHISM TESTS
// ============================================================================

TEST_F(ISparseSetTest, Polymorphism_DifferentConcreteTypes) {
    std::vector<std::unique_ptr<ISparseSet>> pools;

    pools.push_back(std::make_unique<SparseSet<TestComponent>>());
    pools.push_back(std::make_unique<SparseSet<AnotherComponent>>());

    Entity entity(0, 0);

    // Emplace using concrete types
    static_cast<SparseSet<TestComponent>&>(*pools[0]).emplace(entity, 42);
    static_cast<SparseSet<AnotherComponent>&>(*pools[1]).emplace(entity, 3.14f);

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
    SparseSet<AnotherComponent> another_pool;

    pools.push_back(&component_pool);
    pools.push_back(&another_pool);

    Entity entity(5, 0);

    component_pool.emplace(entity, 100);
    another_pool.emplace(entity, 3.14f);

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

TEST_F(ISparseSetTest, VirtualDestructor_AnotherSet) {
    ISparseSet* base_ptr = new SparseSet<AnotherComponent>();

    Entity entity(0, 0);
    static_cast<SparseSet<AnotherComponent>*>(base_ptr)->emplace(entity, 3.14f);

    EXPECT_TRUE(base_ptr->contains(entity));

    EXPECT_NO_THROW(delete base_ptr);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS
// ============================================================================

TEST_F(ISparseSetTest, ShrinkToFit_AfterManyRemovals) {
    Entity entities[100];
    for (int i = 0; i < 100; ++i) {
        entities[i] = Entity(i, 0);
        getSparseSet().emplace(entities[i], i);
    }

    // Remove most entities
    for (int i = 10; i < 100; ++i) {
        sparse_set->remove(entities[i]);
    }

    EXPECT_NO_THROW(sparse_set->shrinkToFit());
    EXPECT_EQ(sparse_set->size(), 10);
}

TEST_F(ISparseSetTest, GetPacked_ConstAccess) {
    Entity e1(0, 0);
    Entity e2(1, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);

    const ISparseSet* const_ptr = sparse_set.get();
    const auto& packed = const_ptr->getPacked();

    EXPECT_EQ(packed.size(), 2);
}

TEST_F(ISparseSetTest, Contains_AfterClear) {
    Entity entity(0, 0);
    getSparseSet().emplace(entity, 42);

    sparse_set->clear();

    EXPECT_FALSE(sparse_set->contains(entity));
}

TEST_F(ISparseSetTest, Size_MaxEntities) {
    // Test with a reasonable number of entities
    const int COUNT = 1000;
    for (int i = 0; i < COUNT; ++i) {
        Entity entity(i, 0);
        getSparseSet().emplace(entity, i);
    }

    EXPECT_EQ(sparse_set->size(), COUNT);
}

TEST_F(ISparseSetTest, Remove_FirstElement) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    sparse_set->remove(e1);

    EXPECT_FALSE(sparse_set->contains(e1));
    EXPECT_TRUE(sparse_set->contains(e2));
    EXPECT_TRUE(sparse_set->contains(e3));
    EXPECT_EQ(sparse_set->size(), 2);
}

TEST_F(ISparseSetTest, Remove_LastElement) {
    Entity e1(0, 0);
    Entity e2(1, 0);
    Entity e3(2, 0);

    getSparseSet().emplace(e1, 1);
    getSparseSet().emplace(e2, 2);
    getSparseSet().emplace(e3, 3);

    sparse_set->remove(e3);

    EXPECT_TRUE(sparse_set->contains(e1));
    EXPECT_TRUE(sparse_set->contains(e2));
    EXPECT_FALSE(sparse_set->contains(e3));
    EXPECT_EQ(sparse_set->size(), 2);
}

TEST_F(ISparseSetTest, Clear_MultipleTimes) {
    Entity entity(0, 0);

    for (int i = 0; i < 5; ++i) {
        getSparseSet().emplace(entity, i);
        EXPECT_EQ(sparse_set->size(), 1);

        sparse_set->clear();
        EXPECT_EQ(sparse_set->size(), 0);
    }
}
