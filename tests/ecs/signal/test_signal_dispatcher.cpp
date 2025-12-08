#include <gtest/gtest.h>
#include <atomic>

#include "../../../lib/rtype_ecs/src/signal/SignalDispatcher.hpp"
#include "../../../lib/rtype_ecs/src/core/Entity.hpp"

using namespace ECS;

TEST(SignalDispatcherTest, RegisterAndDispatchConstruct) {
    SignalDispatcher dispatcher;
    std::atomic<int> callCount{0};

    dispatcher.registerConstruct(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.registerConstruct(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.dispatchConstruct(typeid(int), {1, 0});

    EXPECT_EQ(callCount.load(), 2);
}

TEST(SignalDispatcherTest, RegisterAndDispatchDestroy) {
    SignalDispatcher dispatcher;
    std::atomic<int> callCount{0};

    dispatcher.registerDestroy(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.registerDestroy(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.dispatchDestroy(typeid(int), {2, 0});

    EXPECT_EQ(callCount.load(), 2);
}

TEST(SignalDispatcherTest, ClearCallbacks) {
    SignalDispatcher dispatcher;
    std::atomic<int> callCount{0};

    dispatcher.registerConstruct(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.clearCallbacks(typeid(int));

    dispatcher.dispatchConstruct(typeid(int), {3, 0});
    EXPECT_EQ(callCount.load(), 0);

    dispatcher.registerDestroy(typeid(int),
        [&callCount](Entity e) { (void)e; callCount.fetch_add(1); });

    dispatcher.clearAllCallbacks();

    dispatcher.dispatchDestroy(typeid(int), {4, 0});
    EXPECT_EQ(callCount.load(), 0);
}
