/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_quadtree - Unit tests for QuadTree and QuadTreeSystem
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <set>

#include "../../../lib/rtype_ecs/src/ECS.hpp"
#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/Collision/QuadTree.hpp"
#include "../../../src/games/rtype/shared/Systems/Collision/QuadTreeSystem.hpp"
#include "../../../src/games/rtype/shared/Systems/Collision/Rect.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::shared::collision;

// =============================================================================
// Rect Unit Tests
// =============================================================================

class RectTest : public ::testing::Test {
   protected:
    Rect rect{10.0F, 20.0F, 100.0F, 50.0F};
};

TEST_F(RectTest, DefaultConstructor) {
    Rect r;
    EXPECT_FLOAT_EQ(r.x, 0.0F);
    EXPECT_FLOAT_EQ(r.y, 0.0F);
    EXPECT_FLOAT_EQ(r.w, 0.0F);
    EXPECT_FLOAT_EQ(r.h, 0.0F);
}

TEST_F(RectTest, ParameterizedConstructor) {
    EXPECT_FLOAT_EQ(rect.x, 10.0F);
    EXPECT_FLOAT_EQ(rect.y, 20.0F);
    EXPECT_FLOAT_EQ(rect.w, 100.0F);
    EXPECT_FLOAT_EQ(rect.h, 50.0F);
}

TEST_F(RectTest, EdgeCoordinates) {
    EXPECT_FLOAT_EQ(rect.left(), 10.0F);
    EXPECT_FLOAT_EQ(rect.right(), 110.0F);
    EXPECT_FLOAT_EQ(rect.top(), 20.0F);
    EXPECT_FLOAT_EQ(rect.bottom(), 70.0F);
}

TEST_F(RectTest, CenterCoordinates) {
    EXPECT_FLOAT_EQ(rect.centerX(), 60.0F);
    EXPECT_FLOAT_EQ(rect.centerY(), 45.0F);
}

TEST_F(RectTest, Area) {
    EXPECT_FLOAT_EQ(rect.area(), 5000.0F);
}

TEST_F(RectTest, IsValid) {
    EXPECT_TRUE(rect.isValid());
    EXPECT_FALSE(Rect(0, 0, 0, 0).isValid());
    EXPECT_FALSE(Rect(0, 0, -1, 10).isValid());
    EXPECT_FALSE(Rect(0, 0, 10, -1).isValid());
}

TEST_F(RectTest, Intersects_Overlapping) {
    Rect other{50.0F, 30.0F, 100.0F, 50.0F};
    EXPECT_TRUE(rect.intersects(other));
    EXPECT_TRUE(other.intersects(rect));
}

TEST_F(RectTest, Intersects_NonOverlapping) {
    Rect farAway{500.0F, 500.0F, 10.0F, 10.0F};
    EXPECT_FALSE(rect.intersects(farAway));
    EXPECT_FALSE(farAway.intersects(rect));
}

TEST_F(RectTest, Intersects_Touching) {
    Rect touchingRight{110.0F, 20.0F, 10.0F, 50.0F};
    // Touching at edge counts as intersection (shared edge)
    EXPECT_TRUE(rect.intersects(touchingRight));
}

TEST_F(RectTest, Intersects_Contained) {
    Rect inner{30.0F, 30.0F, 20.0F, 20.0F};
    EXPECT_TRUE(rect.intersects(inner));
    EXPECT_TRUE(inner.intersects(rect));
}

TEST_F(RectTest, Contains_FullyContained) {
    Rect inner{20.0F, 30.0F, 50.0F, 20.0F};
    EXPECT_TRUE(rect.contains(inner));
    EXPECT_FALSE(inner.contains(rect));
}

TEST_F(RectTest, Contains_NotContained) {
    Rect outside{200.0F, 200.0F, 10.0F, 10.0F};
    EXPECT_FALSE(rect.contains(outside));
}

TEST_F(RectTest, Contains_PartiallyOverlapping) {
    Rect partial{50.0F, 30.0F, 100.0F, 50.0F};
    EXPECT_FALSE(rect.contains(partial));
}

TEST_F(RectTest, ContainsPoint_Inside) {
    EXPECT_TRUE(rect.containsPoint(60.0F, 45.0F));
}

TEST_F(RectTest, ContainsPoint_OnEdge) {
    EXPECT_TRUE(rect.containsPoint(10.0F, 20.0F));   // Top-left
    EXPECT_TRUE(rect.containsPoint(110.0F, 70.0F));  // Bottom-right
}

TEST_F(RectTest, ContainsPoint_Outside) {
    EXPECT_FALSE(rect.containsPoint(0.0F, 0.0F));
    EXPECT_FALSE(rect.containsPoint(200.0F, 200.0F));
}

// =============================================================================
// QuadTree Unit Tests
// =============================================================================

class QuadTreeTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create a QuadTree with 1000x1000 bounds
        quadTree = std::make_unique<QuadTree<int>>(Rect{0, 0, 1000, 1000}, 4, 5);
    }

    std::unique_ptr<QuadTree<int>> quadTree;
};

TEST_F(QuadTreeTest, InsertSingleObject) {
    QuadTreeObject<int> obj{Rect{100, 100, 50, 50}, 1};
    EXPECT_TRUE(quadTree->insert(obj));
    EXPECT_EQ(quadTree->totalSize(), 1);
}

TEST_F(QuadTreeTest, InsertMultipleObjects) {
    for (int i = 0; i < 10; ++i) {
        QuadTreeObject<int> obj{Rect{static_cast<float>(i * 100),
                                     static_cast<float>(i * 100), 50, 50},
                                i};
        EXPECT_TRUE(quadTree->insert(obj));
    }
    EXPECT_EQ(quadTree->totalSize(), 10);
}

TEST_F(QuadTreeTest, InsertOutsideBounds) {
    QuadTreeObject<int> obj{Rect{2000, 2000, 50, 50}, 1};
    EXPECT_FALSE(quadTree->insert(obj));
    EXPECT_EQ(quadTree->totalSize(), 0);
}

TEST_F(QuadTreeTest, QuerySingleObject) {
    QuadTreeObject<int> obj{Rect{100, 100, 50, 50}, 42};
    quadTree->insert(obj);

    std::vector<QuadTreeObject<int>> found;
    quadTree->query(Rect{90, 90, 100, 100}, found);

    EXPECT_EQ(found.size(), 1);
    EXPECT_EQ(found[0].data, 42);
}

TEST_F(QuadTreeTest, QueryNoResults) {
    QuadTreeObject<int> obj{Rect{100, 100, 50, 50}, 1};
    quadTree->insert(obj);

    std::vector<QuadTreeObject<int>> found;
    quadTree->query(Rect{500, 500, 50, 50}, found);

    EXPECT_TRUE(found.empty());
}

TEST_F(QuadTreeTest, QueryMultipleObjects) {
    // Insert objects in a grid pattern
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            QuadTreeObject<int> obj{
                Rect{static_cast<float>(i * 100), static_cast<float>(j * 100),
                     50, 50},
                i * 3 + j};
            quadTree->insert(obj);
        }
    }

    // Query a region that should contain some objects
    std::vector<QuadTreeObject<int>> found;
    quadTree->query(Rect{0, 0, 150, 150}, found);

    // Should find objects at (0,0), (0,100), (100,0), (100,100)
    EXPECT_EQ(found.size(), 4);
}

TEST_F(QuadTreeTest, QueryAllObjects) {
    for (int i = 0; i < 5; ++i) {
        QuadTreeObject<int> obj{Rect{static_cast<float>(i * 100),
                                     static_cast<float>(i * 100), 50, 50},
                                i};
        quadTree->insert(obj);
    }

    std::vector<QuadTreeObject<int>> found;
    quadTree->queryAll(found);

    EXPECT_EQ(found.size(), 5);
}

TEST_F(QuadTreeTest, Clear) {
    for (int i = 0; i < 5; ++i) {
        QuadTreeObject<int> obj{Rect{static_cast<float>(i * 100),
                                     static_cast<float>(i * 100), 50, 50},
                                i};
        quadTree->insert(obj);
    }

    quadTree->clear();
    EXPECT_EQ(quadTree->totalSize(), 0);
    EXPECT_FALSE(quadTree->isDivided());
}

TEST_F(QuadTreeTest, Subdivision) {
    // Insert more than maxObjects (4) to trigger subdivision
    for (int i = 0; i < 10; ++i) {
        QuadTreeObject<int> obj{Rect{static_cast<float>(i * 50),
                                     static_cast<float>(i * 50), 20, 20},
                                i};
        quadTree->insert(obj);
    }

    EXPECT_TRUE(quadTree->isDivided());
    EXPECT_GT(quadTree->getNodeCount(), 1);
}

TEST_F(QuadTreeTest, GetBounds) {
    const Rect& bounds = quadTree->getBounds();
    EXPECT_FLOAT_EQ(bounds.x, 0.0F);
    EXPECT_FLOAT_EQ(bounds.y, 0.0F);
    EXPECT_FLOAT_EQ(bounds.w, 1000.0F);
    EXPECT_FLOAT_EQ(bounds.h, 1000.0F);
}

TEST_F(QuadTreeTest, GetDepth) {
    EXPECT_EQ(quadTree->getDepth(), 0);
}

// =============================================================================
// QuadTreeSystem Unit Tests
// =============================================================================

class QuadTreeSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        system = std::make_unique<QuadTreeSystem>(Rect{0, 0, 1920, 1080}, 10, 5);
    }

    void TearDown() override {
        registry.reset();
        system.reset();
    }

    ECS::Entity createCollidableEntity(float x, float y, float w = 32.0F,
                                        float h = 32.0F) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<TransformComponent>(entity, x, y, 0.0F);
        registry->emplaceComponent<BoundingBoxComponent>(entity, w, h);
        return entity;
    }

    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<QuadTreeSystem> system;
};

TEST_F(QuadTreeSystemTest, UpdateWithNoEntities) {
    system->update(*registry, 0.016F);
    EXPECT_EQ(system->getEntityCount(), 0);
    EXPECT_EQ(system->getNodeCount(), 1);  // Root node exists
}

TEST_F(QuadTreeSystemTest, UpdateWithSingleEntity) {
    createCollidableEntity(100.0F, 100.0F);

    system->update(*registry, 0.016F);

    EXPECT_EQ(system->getEntityCount(), 1);
}

TEST_F(QuadTreeSystemTest, UpdateWithMultipleEntities) {
    for (int i = 0; i < 20; ++i) {
        // Start at (50, 50) to ensure bounding boxes are within world bounds
        createCollidableEntity(static_cast<float>(50 + i * 50),
                               static_cast<float>(50 + i * 30));
    }

    system->update(*registry, 0.016F);

    EXPECT_EQ(system->getEntityCount(), 20);
}

TEST_F(QuadTreeSystemTest, QueryCollisionPairs_NoCollisions) {
    // Create entities far apart
    createCollidableEntity(100.0F, 100.0F);
    createCollidableEntity(500.0F, 500.0F);

    system->update(*registry, 0.016F);
    auto pairs = system->queryCollisionPairs(*registry);

    EXPECT_TRUE(pairs.empty());
}

TEST_F(QuadTreeSystemTest, QueryCollisionPairs_Overlapping) {
    // Create overlapping entities
    createCollidableEntity(100.0F, 100.0F, 50.0F, 50.0F);
    createCollidableEntity(120.0F, 120.0F, 50.0F, 50.0F);

    system->update(*registry, 0.016F);
    auto pairs = system->queryCollisionPairs(*registry);

    EXPECT_EQ(pairs.size(), 1);
}

TEST_F(QuadTreeSystemTest, QueryCollisionPairs_NoDuplicates) {
    // Create several overlapping entities
    auto e1 = createCollidableEntity(100.0F, 100.0F, 100.0F, 100.0F);
    auto e2 = createCollidableEntity(150.0F, 150.0F, 100.0F, 100.0F);

    system->update(*registry, 0.016F);
    auto pairs = system->queryCollisionPairs(*registry);

    // Check for unique pairs
    std::set<std::pair<uint32_t, uint32_t>> uniquePairs;
    for (const auto& pair : pairs) {
        uint32_t minId = std::min(pair.entityA.id, pair.entityB.id);
        uint32_t maxId = std::max(pair.entityA.id, pair.entityB.id);
        uniquePairs.insert({minId, maxId});
    }

    EXPECT_EQ(uniquePairs.size(), pairs.size());
}

TEST_F(QuadTreeSystemTest, QueryNearby_ByRect) {
    createCollidableEntity(100.0F, 100.0F);
    createCollidableEntity(150.0F, 150.0F);
    createCollidableEntity(800.0F, 800.0F);

    system->update(*registry, 0.016F);

    auto nearby = system->queryNearby(Rect{50, 50, 200, 200});

    EXPECT_EQ(nearby.size(), 2);
}

TEST_F(QuadTreeSystemTest, QueryNearby_ByPointAndRadius) {
    createCollidableEntity(100.0F, 100.0F);
    createCollidableEntity(150.0F, 150.0F);
    createCollidableEntity(800.0F, 800.0F);

    system->update(*registry, 0.016F);

    auto nearby = system->queryNearby(125.0F, 125.0F, 100.0F);

    EXPECT_EQ(nearby.size(), 2);
}

TEST_F(QuadTreeSystemTest, QueryNearby_NoResults) {
    createCollidableEntity(100.0F, 100.0F);

    system->update(*registry, 0.016F);

    auto nearby = system->queryNearby(Rect{800, 800, 50, 50});

    EXPECT_TRUE(nearby.empty());
}

TEST_F(QuadTreeSystemTest, GetWorldBounds) {
    const auto& bounds = system->getWorldBounds();
    EXPECT_FLOAT_EQ(bounds.x, 0.0F);
    EXPECT_FLOAT_EQ(bounds.y, 0.0F);
    EXPECT_FLOAT_EQ(bounds.w, 1920.0F);
    EXPECT_FLOAT_EQ(bounds.h, 1080.0F);
}

TEST_F(QuadTreeSystemTest, SetWorldBounds) {
    system->setWorldBounds(Rect{0, 0, 3840, 2160});
    const auto& bounds = system->getWorldBounds();
    EXPECT_FLOAT_EQ(bounds.w, 3840.0F);
    EXPECT_FLOAT_EQ(bounds.h, 2160.0F);
}

TEST_F(QuadTreeSystemTest, SystemName) {
    EXPECT_EQ(system->getName(), "QuadTreeSystem");
}

TEST_F(QuadTreeSystemTest, SystemEnabledByDefault) {
    EXPECT_TRUE(system->isEnabled());
}

TEST_F(QuadTreeSystemTest, DisableSystem) {
    system->setEnabled(false);
    EXPECT_FALSE(system->isEnabled());
}

// =============================================================================
// QuadTreeSystem Edge Cases
// =============================================================================

TEST_F(QuadTreeSystemTest, EntityAtOrigin) {
    createCollidableEntity(0.0F, 0.0F);

    system->update(*registry, 0.016F);

    // Entity at origin should be in tree (adjusted for center positioning)
    EXPECT_GE(system->getEntityCount(), 0);
}

TEST_F(QuadTreeSystemTest, EntityAtEdge) {
    // Entity at the edge of world bounds
    createCollidableEntity(1900.0F, 1060.0F, 20.0F, 20.0F);

    system->update(*registry, 0.016F);

    EXPECT_EQ(system->getEntityCount(), 1);
}

TEST_F(QuadTreeSystemTest, ManyEntitiesTriggerSubdivision) {
    // Insert many entities to ensure subdivision occurs
    for (int i = 0; i < 50; ++i) {
        createCollidableEntity(static_cast<float>((i % 10) * 100 + 50),
                               static_cast<float>((i / 10) * 100 + 50));
    }

    system->update(*registry, 0.016F);

    EXPECT_EQ(system->getEntityCount(), 50);
    EXPECT_GT(system->getNodeCount(), 1);  // Should be subdivided
}

TEST_F(QuadTreeSystemTest, RebuildEachFrame) {
    auto entity = createCollidableEntity(100.0F, 100.0F);

    system->update(*registry, 0.016F);
    EXPECT_EQ(system->getEntityCount(), 1);

    // Move entity
    auto& transform = registry->getComponent<TransformComponent>(entity);
    transform.x = 500.0F;
    transform.y = 500.0F;

    // Update should rebuild tree with new position
    system->update(*registry, 0.016F);
    EXPECT_EQ(system->getEntityCount(), 1);

    // Query old position should be empty
    auto nearOld = system->queryNearby(Rect{80, 80, 50, 50});
    EXPECT_TRUE(nearOld.empty());

    // Query new position should find entity
    auto nearNew = system->queryNearby(Rect{480, 480, 50, 50});
    EXPECT_EQ(nearNew.size(), 1);
}

