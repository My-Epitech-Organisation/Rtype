/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - Relationship Management
*/

#include <gtest/gtest.h>
#include "../../../lib/ecs/src/core/Registry/Registry.hpp"
#include <vector>
#include <set>

using namespace ECS;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistryRelationshipTest : public ::testing::Test {
protected:
    Registry registry;
    RelationshipManager& relationships = registry.getRelationshipManager();

    Entity createEntity() {
        return registry.spawnEntity();
    }
};

// ============================================================================
// SET PARENT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, SetParent_BasicRelationship) {
    Entity parent = createEntity();
    Entity child = createEntity();

    bool result = relationships.setParent(child, parent);

    EXPECT_TRUE(result);
    EXPECT_TRUE(relationships.hasParent(child));
    EXPECT_EQ(relationships.getParent(child).value(), parent);
}

TEST_F(RegistryRelationshipTest, SetParent_MultipleChildren) {
    Entity parent = createEntity();
    Entity child1 = createEntity();
    Entity child2 = createEntity();
    Entity child3 = createEntity();

    relationships.setParent(child1, parent);
    relationships.setParent(child2, parent);
    relationships.setParent(child3, parent);

    auto children = relationships.getChildren(parent);

    EXPECT_EQ(children.size(), 3);
    EXPECT_TRUE(std::find(children.begin(), children.end(), child1) != children.end());
    EXPECT_TRUE(std::find(children.begin(), children.end(), child2) != children.end());
    EXPECT_TRUE(std::find(children.begin(), children.end(), child3) != children.end());
}

TEST_F(RegistryRelationshipTest, SetParent_ChangeParent) {
    Entity parent1 = createEntity();
    Entity parent2 = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent1);
    EXPECT_EQ(relationships.getParent(child).value(), parent1);

    relationships.setParent(child, parent2);
    EXPECT_EQ(relationships.getParent(child).value(), parent2);

    // Child should be removed from parent1's children
    auto children1 = relationships.getChildren(parent1);
    EXPECT_TRUE(children1.empty());

    // Child should be in parent2's children
    auto children2 = relationships.getChildren(parent2);
    EXPECT_EQ(children2.size(), 1);
    EXPECT_EQ(children2[0], child);
}

TEST_F(RegistryRelationshipTest, SetParent_CycleDetection_DirectCycle) {
    Entity e1 = createEntity();
    Entity e2 = createEntity();

    relationships.setParent(e2, e1);
    bool result = relationships.setParent(e1, e2);  // Would create cycle

    EXPECT_FALSE(result);
    EXPECT_FALSE(relationships.hasParent(e1));
}

TEST_F(RegistryRelationshipTest, SetParent_CycleDetection_IndirectCycle) {
    Entity e1 = createEntity();
    Entity e2 = createEntity();
    Entity e3 = createEntity();

    relationships.setParent(e2, e1);
    relationships.setParent(e3, e2);
    bool result = relationships.setParent(e1, e3);  // Would create cycle

    EXPECT_FALSE(result);
    EXPECT_FALSE(relationships.hasParent(e1));
}

TEST_F(RegistryRelationshipTest, SetParent_SelfParent_Prevented) {
    Entity e = createEntity();

    bool result = relationships.setParent(e, e);

    EXPECT_FALSE(result);
    EXPECT_FALSE(relationships.hasParent(e));
}

// ============================================================================
// REMOVE PARENT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, RemoveParent_OrphansChild) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);
    relationships.removeParent(child);

    EXPECT_FALSE(relationships.hasParent(child));
    EXPECT_TRUE(relationships.getChildren(parent).empty());
}

TEST_F(RegistryRelationshipTest, RemoveParent_NoParent_NoEffect) {
    Entity child = createEntity();

    // Should not throw
    relationships.removeParent(child);

    EXPECT_FALSE(relationships.hasParent(child));
}

// ============================================================================
// GET PARENT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetParent_HasParent_ReturnsParent) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);

    auto result = relationships.getParent(child);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), parent);
}

TEST_F(RegistryRelationshipTest, GetParent_NoParent_ReturnsNullopt) {
    Entity child = createEntity();

    auto result = relationships.getParent(child);

    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// HAS PARENT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, HasParent_WithParent_ReturnsTrue) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);

    EXPECT_TRUE(relationships.hasParent(child));
}

TEST_F(RegistryRelationshipTest, HasParent_NoParent_ReturnsFalse) {
    Entity entity = createEntity();

    EXPECT_FALSE(relationships.hasParent(entity));
}

// ============================================================================
// GET CHILDREN TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetChildren_HasChildren_ReturnsAll) {
    Entity parent = createEntity();
    std::vector<Entity> expectedChildren;

    for (int i = 0; i < 5; ++i) {
        Entity child = createEntity();
        relationships.setParent(child, parent);
        expectedChildren.push_back(child);
    }

    auto children = relationships.getChildren(parent);

    EXPECT_EQ(children.size(), 5);
    for (Entity expected : expectedChildren) {
        EXPECT_TRUE(std::find(children.begin(), children.end(), expected) != children.end());
    }
}

TEST_F(RegistryRelationshipTest, GetChildren_NoChildren_ReturnsEmpty) {
    Entity parent = createEntity();

    auto children = relationships.getChildren(parent);

    EXPECT_TRUE(children.empty());
}

// ============================================================================
// GET DESCENDANTS TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetDescendants_DeepHierarchy) {
    Entity root = createEntity();
    Entity child1 = createEntity();
    Entity child2 = createEntity();
    Entity grandchild1 = createEntity();
    Entity grandchild2 = createEntity();
    Entity greatgrandchild = createEntity();

    relationships.setParent(child1, root);
    relationships.setParent(child2, root);
    relationships.setParent(grandchild1, child1);
    relationships.setParent(grandchild2, child1);
    relationships.setParent(greatgrandchild, grandchild1);

    auto descendants = relationships.getDescendants(root);

    EXPECT_EQ(descendants.size(), 5);
    EXPECT_TRUE(std::find(descendants.begin(), descendants.end(), child1) != descendants.end());
    EXPECT_TRUE(std::find(descendants.begin(), descendants.end(), child2) != descendants.end());
    EXPECT_TRUE(std::find(descendants.begin(), descendants.end(), grandchild1) != descendants.end());
    EXPECT_TRUE(std::find(descendants.begin(), descendants.end(), grandchild2) != descendants.end());
    EXPECT_TRUE(std::find(descendants.begin(), descendants.end(), greatgrandchild) != descendants.end());
}

TEST_F(RegistryRelationshipTest, GetDescendants_NoDescendants_ReturnsEmpty) {
    Entity leaf = createEntity();

    auto descendants = relationships.getDescendants(leaf);

    EXPECT_TRUE(descendants.empty());
}

// ============================================================================
// GET ANCESTORS TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetAncestors_DeepHierarchy) {
    Entity root = createEntity();
    Entity child = createEntity();
    Entity grandchild = createEntity();
    Entity greatgrandchild = createEntity();

    relationships.setParent(child, root);
    relationships.setParent(grandchild, child);
    relationships.setParent(greatgrandchild, grandchild);

    auto ancestors = relationships.getAncestors(greatgrandchild);

    ASSERT_EQ(ancestors.size(), 3);
    EXPECT_EQ(ancestors[0], grandchild);  // Immediate parent first
    EXPECT_EQ(ancestors[1], child);
    EXPECT_EQ(ancestors[2], root);
}

TEST_F(RegistryRelationshipTest, GetAncestors_NoAncestors_ReturnsEmpty) {
    Entity root = createEntity();

    auto ancestors = relationships.getAncestors(root);

    EXPECT_TRUE(ancestors.empty());
}

// ============================================================================
// GET ROOT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetRoot_DeepHierarchy_ReturnsRoot) {
    Entity root = createEntity();
    Entity child = createEntity();
    Entity grandchild = createEntity();

    relationships.setParent(child, root);
    relationships.setParent(grandchild, child);

    EXPECT_EQ(relationships.getRoot(grandchild), root);
    EXPECT_EQ(relationships.getRoot(child), root);
    EXPECT_EQ(relationships.getRoot(root), root);
}

TEST_F(RegistryRelationshipTest, GetRoot_NoParent_ReturnsSelf) {
    Entity entity = createEntity();

    EXPECT_EQ(relationships.getRoot(entity), entity);
}

// ============================================================================
// IS ANCESTOR TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, IsAncestor_DirectParent_ReturnsTrue) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);

    EXPECT_TRUE(relationships.isAncestor(parent, child));
}

TEST_F(RegistryRelationshipTest, IsAncestor_IndirectAncestor_ReturnsTrue) {
    Entity root = createEntity();
    Entity child = createEntity();
    Entity grandchild = createEntity();

    relationships.setParent(child, root);
    relationships.setParent(grandchild, child);

    EXPECT_TRUE(relationships.isAncestor(root, grandchild));
}

TEST_F(RegistryRelationshipTest, IsAncestor_NotAncestor_ReturnsFalse) {
    Entity e1 = createEntity();
    Entity e2 = createEntity();

    EXPECT_FALSE(relationships.isAncestor(e1, e2));
}

TEST_F(RegistryRelationshipTest, IsAncestor_Self_ReturnsFalse) {
    Entity entity = createEntity();

    EXPECT_FALSE(relationships.isAncestor(entity, entity));
}

// ============================================================================
// REMOVE ENTITY TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, RemoveEntity_RemovesAsParent) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);
    relationships.removeEntity(parent);

    EXPECT_FALSE(relationships.hasParent(child));
}

TEST_F(RegistryRelationshipTest, RemoveEntity_RemovesAsChild) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);
    relationships.removeEntity(child);

    auto children = relationships.getChildren(parent);
    EXPECT_TRUE(children.empty());
}

TEST_F(RegistryRelationshipTest, RemoveEntity_MiddleOfHierarchy) {
    Entity root = createEntity();
    Entity middle = createEntity();
    Entity leaf = createEntity();

    relationships.setParent(middle, root);
    relationships.setParent(leaf, middle);

    relationships.removeEntity(middle);

    // Leaf should now be orphaned
    EXPECT_FALSE(relationships.hasParent(leaf));
    // Root should have no children
    EXPECT_TRUE(relationships.getChildren(root).empty());
}

// ============================================================================
// CHILD COUNT TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, ChildCount_ReturnsCorrectCount) {
    Entity parent = createEntity();

    for (int i = 0; i < 7; ++i) {
        Entity child = createEntity();
        relationships.setParent(child, parent);
    }

    EXPECT_EQ(relationships.childCount(parent), 7);
}

TEST_F(RegistryRelationshipTest, ChildCount_NoChildren_ReturnsZero) {
    Entity parent = createEntity();

    EXPECT_EQ(relationships.childCount(parent), 0);
}

// ============================================================================
// GET DEPTH TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, GetDepth_Root_ReturnsZero) {
    Entity root = createEntity();

    EXPECT_EQ(relationships.getDepth(root), 0);
}

TEST_F(RegistryRelationshipTest, GetDepth_DeepHierarchy_ReturnsCorrectDepth) {
    Entity root = createEntity();
    Entity depth1 = createEntity();
    Entity depth2 = createEntity();
    Entity depth3 = createEntity();

    relationships.setParent(depth1, root);
    relationships.setParent(depth2, depth1);
    relationships.setParent(depth3, depth2);

    EXPECT_EQ(relationships.getDepth(root), 0);
    EXPECT_EQ(relationships.getDepth(depth1), 1);
    EXPECT_EQ(relationships.getDepth(depth2), 2);
    EXPECT_EQ(relationships.getDepth(depth3), 3);
}

// ============================================================================
// CLEAR TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, Clear_RemovesAllRelationships) {
    Entity parent = createEntity();
    Entity child1 = createEntity();
    Entity child2 = createEntity();

    relationships.setParent(child1, parent);
    relationships.setParent(child2, parent);

    relationships.clear();

    EXPECT_FALSE(relationships.hasParent(child1));
    EXPECT_FALSE(relationships.hasParent(child2));
    EXPECT_TRUE(relationships.getChildren(parent).empty());
}

// ============================================================================
// INTEGRATION WITH REGISTRY
// ============================================================================

TEST_F(RegistryRelationshipTest, KillEntity_CleansUpRelationships) {
    Entity parent = createEntity();
    Entity child = createEntity();

    relationships.setParent(child, parent);
    registry.killEntity(parent);

    EXPECT_FALSE(relationships.hasParent(child));
}

TEST_F(RegistryRelationshipTest, GetRelationshipManager_ConstAccess) {
    const Registry& constReg = registry;

    const RelationshipManager& constRelationships = constReg.getRelationshipManager();

    // Should compile and work
    Entity parent = createEntity();
    EXPECT_EQ(constRelationships.childCount(parent), 0);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(RegistryRelationshipTest, StressTest_WideHierarchy) {
    Entity root = createEntity();

    for (int i = 0; i < 100; ++i) {
        Entity child = createEntity();
        relationships.setParent(child, root);
    }

    EXPECT_EQ(relationships.childCount(root), 100);
    EXPECT_EQ(relationships.getDescendants(root).size(), 100);
}

TEST_F(RegistryRelationshipTest, StressTest_DeepHierarchy) {
    Entity current = createEntity();
    Entity root = current;

    for (int i = 0; i < 50; ++i) {
        Entity next = createEntity();
        relationships.setParent(next, current);
        current = next;
    }

    EXPECT_EQ(relationships.getDepth(current), 50);
    EXPECT_EQ(relationships.getRoot(current), root);
    EXPECT_EQ(relationships.getAncestors(current).size(), 50);
}
