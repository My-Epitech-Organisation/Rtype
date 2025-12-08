#include <gtest/gtest.h>

#include "../../../lib/rtype_ecs/src/system/SystemScheduler.hpp"
#include "../../../lib/rtype_ecs/src/core/Registry/Registry.hpp"

using namespace ECS;

TEST(SystemSchedulerTest, AddAndRunSystemsWithDependencies) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    std::vector<std::string> calls;

    scheduler.addSystem("A", [&calls](Registry&) { calls.push_back("A"); });
    scheduler.addSystem("B", [&calls](Registry&) { calls.push_back("B"); }, {"A"});

    // Should run A then B
    scheduler.run();
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_EQ(calls[0], "A");
    EXPECT_EQ(calls[1], "B");
}

TEST(SystemSchedulerTest, RunSystemByName) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    bool calledA = false;
    scheduler.addSystem("A", [&calledA](Registry&) { calledA = true; });

    scheduler.runSystem("A");
    EXPECT_TRUE(calledA);

    // Running unknown system should throw
    EXPECT_THROW(scheduler.runSystem("Unknown"), std::runtime_error);
}

TEST(SystemSchedulerTest, DuplicateSystemRegistrationThrows) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    scheduler.addSystem("A", [](Registry&) {});
    EXPECT_THROW(scheduler.addSystem("A", [](Registry&) {}), std::runtime_error);
}

TEST(SystemSchedulerTest, EnableDisableSystem) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    bool called = false;
    scheduler.addSystem("A", [&called](Registry&) { called = true; });

    // Disabled -> not called
    scheduler.setSystemEnabled("A", false);
    EXPECT_FALSE(scheduler.isSystemEnabled("A"));
    scheduler.run();
    EXPECT_FALSE(called);

    // Enable -> called
    scheduler.setSystemEnabled("A", true);
    EXPECT_TRUE(scheduler.isSystemEnabled("A"));
    scheduler.run();
    EXPECT_TRUE(called);

    // setSystemEnabled for unknown should throw
    EXPECT_THROW(scheduler.setSystemEnabled("Nope", false), std::runtime_error);
    EXPECT_THROW(scheduler.isSystemEnabled("Nope"), std::runtime_error);
}

TEST(SystemSchedulerTest, MissingDependencyThrows) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    // B depends on non-existent X
    scheduler.addSystem("A", [](Registry&) {});
    scheduler.addSystem("B", [](Registry&) {}, {"X"});

    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(SystemSchedulerTest, DetectsCycle) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    scheduler.addSystem("A", [](Registry&) {}, {"B"});
    scheduler.addSystem("B", [](Registry&) {}, {"A"});

    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(SystemSchedulerTest, ClearRemovesSystemsAndOrder) {
    Registry registry;
    SystemScheduler scheduler(std::ref(registry));

    scheduler.addSystem("A", [](Registry&) {});
    scheduler.addSystem("B", [](Registry&) {}, {"A"});

    scheduler.clear();
    // No systems -> run should not throw
    EXPECT_NO_THROW(scheduler.run());
    auto order = scheduler.getExecutionOrder();
    EXPECT_TRUE(order.empty());
}
