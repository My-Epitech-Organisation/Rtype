#include <gtest/gtest.h>

#include "../../../lib/ecs/src/core/Registry/Registry.hpp"
#include "../../../lib/ecs/src/core/CommandBuffer.hpp"

using namespace ECS;

struct TestComp {
    int value{0};
};

TEST(CommandBufferTest, EmplaceAndRemoveComponentDeferred) {
    Registry reg;
    CommandBuffer cb(reg);

    auto placeholder = cb.spawnEntityDeferred();
    EXPECT_EQ(cb.pendingCount(), 1u);

    cb.emplaceComponentDeferred<TestComp>(placeholder, 42);
    EXPECT_EQ(cb.pendingCount(), 2u);

    cb.flush();
    EXPECT_EQ(cb.pendingCount(), 0u);

    // After flush, there should be one component of type TestComp
    EXPECT_EQ(reg.countComponents<TestComp>(), 1u);

    // Now remove component deferred
    cb.removeComponentDeferred<TestComp>(placeholder);
    EXPECT_EQ(cb.pendingCount(), 1u);
    cb.flush();
    EXPECT_EQ(reg.countComponents<TestComp>(), 0u);
}

TEST(CommandBufferTest, ClearPendingCommands) {
    Registry reg;
    CommandBuffer cb(reg);

    auto p1 = cb.spawnEntityDeferred();
    cb.emplaceComponentDeferred<TestComp>(p1, 1);
    EXPECT_EQ(cb.pendingCount(), 2u);

    cb.clear();
    EXPECT_EQ(cb.pendingCount(), 0u);

    // Ensure nothing was applied
    EXPECT_EQ(reg.countComponents<TestComp>(), 0u);
}

TEST(CommandBufferTest, SpawnAndDestroyInSameFlush) {
    Registry reg;
    CommandBuffer cb(reg);

    auto placeholder = cb.spawnEntityDeferred();
    cb.emplaceComponentDeferred<TestComp>(placeholder, 55);
    cb.destroyEntityDeferred(placeholder);

    // Both commands should be pending
    EXPECT_EQ(cb.pendingCount(), 3u);

    cb.flush();

    // Created entity should have been destroyed in same flush
    EXPECT_EQ(reg.countComponents<TestComp>(), 0u);
}

TEST(CommandBufferTest, DestroyRealEntityAfterFlush) {
    Registry reg;
    CommandBuffer cb(reg);

    auto placeholder = cb.spawnEntityDeferred();
    cb.emplaceComponentDeferred<TestComp>(placeholder, 77);
    cb.flush();

    // The first real entity should be index 0
    Entity real{0u, 0u};
    EXPECT_TRUE(reg.isAlive(real));

    // Destroy the real entity
    cb.destroyEntityDeferred(real);
    cb.flush();

    EXPECT_FALSE(reg.isAlive(real));
}

