/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for EventManager (Pub/Sub system)
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "EventManager.hpp"

using namespace rtype::engine;
using namespace rtype::engine::events;

// ============================================================================
// Mock Events for Testing
// ============================================================================

struct TestEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()
    int value = 0;
    std::string message;

    TestEvent() = default;
    explicit TestEvent(int v, std::string msg = "")
        : value(v), message(std::move(msg)) {}
};

struct AnotherTestEvent : public IEvent {
    IMPLEMENT_EVENT_TYPE()
    float data = 0.0f;

    AnotherTestEvent() = default;
    explicit AnotherTestEvent(float d) : data(d) {}
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST(EventManagerTest, SubscribeAndPublish_BasicCallback) {
    EventManager eventManager;
    int callbackInvocations = 0;

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });

    EXPECT_EQ(callbackInvocations, 0);

    TestEvent event{42, "test"};
    eventManager.publish(event);

    EXPECT_EQ(callbackInvocations, 1);
}

TEST(EventManagerTest, SubscribeAndPublish_EventDataPassed) {
    EventManager eventManager;
    int receivedValue = 0;
    std::string receivedMessage;

    eventManager.subscribe<TestEvent>(
        [&receivedValue, &receivedMessage](const TestEvent& event) {
            receivedValue = event.value;
            receivedMessage = event.message;
        });

    TestEvent event{123, "hello"};
    eventManager.publish(event);

    EXPECT_EQ(receivedValue, 123);
    EXPECT_EQ(receivedMessage, "hello");
}

TEST(EventManagerTest, PublishWithoutSubscribers_NoError) {
    EventManager eventManager;

    TestEvent event{42, "test"};
    EXPECT_NO_THROW(eventManager.publish(event));
}

// ============================================================================
// Multiple Subscribers Tests
// ============================================================================

TEST(EventManagerTest, MultipleSubscribers_AllInvoked) {
    EventManager eventManager;
    int counter1 = 0;
    int counter2 = 0;
    int counter3 = 0;

    eventManager.subscribe<TestEvent>(
        [&counter1](const TestEvent& /*event*/) { ++counter1; });
    eventManager.subscribe<TestEvent>(
        [&counter2](const TestEvent& /*event*/) { ++counter2; });
    eventManager.subscribe<TestEvent>(
        [&counter3](const TestEvent& /*event*/) { ++counter3; });

    TestEvent event{42, "test"};
    eventManager.publish(event);

    EXPECT_EQ(counter1, 1);
    EXPECT_EQ(counter2, 1);
    EXPECT_EQ(counter3, 1);
}

TEST(EventManagerTest, MultipleSubscribers_InvocationOrder) {
    EventManager eventManager;
    std::vector<int> invocationOrder;

    eventManager.subscribe<TestEvent>(
        [&invocationOrder](const TestEvent& /*event*/) {
            invocationOrder.push_back(1);
        });
    eventManager.subscribe<TestEvent>(
        [&invocationOrder](const TestEvent& /*event*/) {
            invocationOrder.push_back(2);
        });
    eventManager.subscribe<TestEvent>(
        [&invocationOrder](const TestEvent& /*event*/) {
            invocationOrder.push_back(3);
        });

    TestEvent event{42, "test"};
    eventManager.publish(event);

    ASSERT_EQ(invocationOrder.size(), 3);
    EXPECT_EQ(invocationOrder[0], 1);
    EXPECT_EQ(invocationOrder[1], 2);
    EXPECT_EQ(invocationOrder[2], 3);
}

// ============================================================================
// Multiple Event Types Tests
// ============================================================================

TEST(EventManagerTest, MultipleEventTypes_IndependentSubscriptions) {
    EventManager eventManager;
    int testEventCount = 0;
    int anotherEventCount = 0;

    eventManager.subscribe<TestEvent>(
        [&testEventCount](const TestEvent& /*event*/) { ++testEventCount; });
    eventManager.subscribe<AnotherTestEvent>(
        [&anotherEventCount](const AnotherTestEvent& /*event*/) {
            ++anotherEventCount;
        });

    TestEvent testEvent{42, "test"};
    AnotherTestEvent anotherEvent{3.14f};

    eventManager.publish(testEvent);
    EXPECT_EQ(testEventCount, 1);
    EXPECT_EQ(anotherEventCount, 0);

    eventManager.publish(anotherEvent);
    EXPECT_EQ(testEventCount, 1);
    EXPECT_EQ(anotherEventCount, 1);

    eventManager.publish(testEvent);
    EXPECT_EQ(testEventCount, 2);
    EXPECT_EQ(anotherEventCount, 1);
}

// ============================================================================
// Unsubscribe Tests
// ============================================================================

TEST(EventManagerTest, UnsubscribeAll_RemovesAllSubscribers) {
    EventManager eventManager;
    int callbackInvocations = 0;

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });
    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });

    TestEvent event{42, "test"};
    eventManager.publish(event);
    EXPECT_EQ(callbackInvocations, 2);

    eventManager.unsubscribeAll<TestEvent>();
    eventManager.publish(event);
    EXPECT_EQ(callbackInvocations, 2);  // No additional invocations
}

TEST(EventManagerTest, UnsubscribeAll_OnlyAffectsSpecificEventType) {
    EventManager eventManager;
    int testEventCount = 0;
    int anotherEventCount = 0;

    eventManager.subscribe<TestEvent>(
        [&testEventCount](const TestEvent& /*event*/) { ++testEventCount; });
    eventManager.subscribe<AnotherTestEvent>(
        [&anotherEventCount](const AnotherTestEvent& /*event*/) {
            ++anotherEventCount;
        });

    TestEvent testEvent{42, "test"};
    AnotherTestEvent anotherEvent{3.14f};

    eventManager.unsubscribeAll<TestEvent>();

    eventManager.publish(testEvent);
    eventManager.publish(anotherEvent);

    EXPECT_EQ(testEventCount, 0);
    EXPECT_EQ(anotherEventCount, 1);
}

TEST(EventManagerTest, Clear_RemovesAllSubscriptions) {
    EventManager eventManager;
    int testEventCount = 0;
    int anotherEventCount = 0;

    eventManager.subscribe<TestEvent>(
        [&testEventCount](const TestEvent& /*event*/) { ++testEventCount; });
    eventManager.subscribe<AnotherTestEvent>(
        [&anotherEventCount](const AnotherTestEvent& /*event*/) {
            ++anotherEventCount;
        });

    eventManager.clear();

    TestEvent testEvent{42, "test"};
    AnotherTestEvent anotherEvent{3.14f};

    eventManager.publish(testEvent);
    eventManager.publish(anotherEvent);

    EXPECT_EQ(testEventCount, 0);
    EXPECT_EQ(anotherEventCount, 0);
}

// ============================================================================
// Subscriber Count Tests
// ============================================================================

TEST(EventManagerTest, SubscriberCount_ReturnsCorrectValue) {
    EventManager eventManager;

    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 0);

    eventManager.subscribe<TestEvent>(
        [](const TestEvent& /*event*/) { /* no-op */ });
    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 1);

    eventManager.subscribe<TestEvent>(
        [](const TestEvent& /*event*/) { /* no-op */ });
    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 2);

    eventManager.subscribe<TestEvent>(
        [](const TestEvent& /*event*/) { /* no-op */ });
    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 3);
}

TEST(EventManagerTest, SubscriberCount_AfterUnsubscribe) {
    EventManager eventManager;

    eventManager.subscribe<TestEvent>(
        [](const TestEvent& /*event*/) { /* no-op */ });
    eventManager.subscribe<TestEvent>(
        [](const TestEvent& /*event*/) { /* no-op */ });

    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 2);

    eventManager.unsubscribeAll<TestEvent>();
    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(), 0);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(EventManagerTest, ThreadSafety_ConcurrentSubscribe) {
    EventManager eventManager;
    constexpr int numThreads = 10;
    constexpr int subscribesPerThread = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&eventManager]() {
            for (int i = 0; i < subscribesPerThread; ++i) {
                eventManager.subscribe<TestEvent>(
                    [](const TestEvent& /*event*/) { /* no-op */ });
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(eventManager.subscriberCount<TestEvent>(),
              numThreads * subscribesPerThread);
}

TEST(EventManagerTest, ThreadSafety_ConcurrentPublish) {
    EventManager eventManager;
    std::atomic<int> callbackInvocations{0};

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });

    constexpr int numThreads = 10;
    constexpr int publishesPerThread = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&eventManager]() {
            for (int i = 0; i < publishesPerThread; ++i) {
                TestEvent event{i, "concurrent"};
                eventManager.publish(event);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(callbackInvocations.load(),
              numThreads * publishesPerThread);
}

TEST(EventManagerTest, ThreadSafety_MixedOperations) {
    EventManager eventManager;
    std::atomic<int> callbackInvocations{0};

    auto callback = [&callbackInvocations](const TestEvent& /*event*/) {
        ++callbackInvocations;
    };

    eventManager.subscribe<TestEvent>(callback);

    constexpr int numThreads = 8;
    std::vector<std::thread> threads;

    // Mix of subscribe, publish, and query operations
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&eventManager, t, callback]() {
            if (t % 3 == 0) {
                // Subscribe threads
                for (int i = 0; i < 50; ++i) {
                    eventManager.subscribe<TestEvent>(callback);
                }
            } else if (t % 3 == 1) {
                // Publish threads
                for (int i = 0; i < 50; ++i) {
                    TestEvent event{i, "mixed"};
                    eventManager.publish(event);
                }
            } else {
                // Query threads
                for (int i = 0; i < 50; ++i) {
                    [[maybe_unused]] auto count =
                        eventManager.subscriberCount<TestEvent>();
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // At least the initial callback was invoked
    EXPECT_GT(callbackInvocations.load(), 0);
}

// ============================================================================
// Common Events Tests
// ============================================================================

TEST(EventManagerTest, CollisionEvent_DataIntegrity) {
    EventManager eventManager;
    ECS::Entity receivedA{0, 0};
    ECS::Entity receivedB{0, 0};
    float receivedForce = 0.0f;

    eventManager.subscribe<CollisionEvent>(
        [&](const CollisionEvent& event) {
            receivedA = event.entityA;
            receivedB = event.entityB;
            receivedForce = event.impactForce;
        });

    ECS::Entity entityA{1, 0};
    ECS::Entity entityB{2, 0};
    CollisionEvent collision{entityA, entityB, 42.5f};
    eventManager.publish(collision);

    EXPECT_EQ(receivedA.id, 1);
    EXPECT_EQ(receivedB.id, 2);
    EXPECT_FLOAT_EQ(receivedForce, 42.5f);
}

TEST(EventManagerTest, DamageEvent_DataIntegrity) {
    EventManager eventManager;
    ECS::Entity receivedEntity{0, 0};
    int32_t receivedDamage = 0;
    int32_t receivedBefore = 0;
    int32_t receivedAfter = 0;

    eventManager.subscribe<DamageEvent>([&](const DamageEvent& event) {
        receivedEntity = event.entity;
        receivedDamage = event.damage;
        receivedBefore = event.healthBefore;
        receivedAfter = event.healthAfter;
    });

    ECS::Entity entity{42, 0};
    DamageEvent damage{entity, 25, 100, 75, 1};
    eventManager.publish(damage);

    EXPECT_EQ(receivedEntity.id, 42);
    EXPECT_EQ(receivedDamage, 25);
    EXPECT_EQ(receivedBefore, 100);
    EXPECT_EQ(receivedAfter, 75);
}

TEST(EventManagerTest, NetworkInputEvent_DataIntegrity) {
    EventManager eventManager;
    uint32_t receivedUserId = 0;
    float receivedDeltaX = 0.0f;
    float receivedDeltaY = 0.0f;
    bool receivedPressed = false;

    eventManager.subscribe<NetworkInputEvent>(
        [&](const NetworkInputEvent& event) {
            receivedUserId = event.userId;
            receivedDeltaX = event.deltaX;
            receivedDeltaY = event.deltaY;
            receivedPressed = event.isPressed;
        });

    NetworkInputEvent input{123, 1, 5.0f, -3.0f, true};
    eventManager.publish(input);

    EXPECT_EQ(receivedUserId, 123);
    EXPECT_FLOAT_EQ(receivedDeltaX, 5.0f);
    EXPECT_FLOAT_EQ(receivedDeltaY, -3.0f);
    EXPECT_TRUE(receivedPressed);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST(EventManagerTest, PublishMultipleTimes_CountersIncrement) {
    EventManager eventManager;
    int callbackInvocations = 0;

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });

    for (int i = 0; i < 10; ++i) {
        TestEvent event{i, "iteration"};
        eventManager.publish(event);
    }

    EXPECT_EQ(callbackInvocations, 10);
}

TEST(EventManagerTest, ResubscribeAfterUnsubscribe_Works) {
    EventManager eventManager;
    int callbackInvocations = 0;

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });

    TestEvent event{42, "test"};
    eventManager.publish(event);
    EXPECT_EQ(callbackInvocations, 1);

    eventManager.unsubscribeAll<TestEvent>();
    eventManager.publish(event);
    EXPECT_EQ(callbackInvocations, 1);

    eventManager.subscribe<TestEvent>(
        [&callbackInvocations](const TestEvent& /*event*/) {
            ++callbackInvocations;
        });
    eventManager.publish(event);
    EXPECT_EQ(callbackInvocations, 2);
}

TEST(EventManagerTest, LambdaWithCapture_WorksCorrectly) {
    EventManager eventManager;
    std::vector<int> values;

    eventManager.subscribe<TestEvent>([&values](const TestEvent& event) {
        values.push_back(event.value);
    });

    for (int i = 0; i < 5; ++i) {
        TestEvent event{i, "capture"};
        eventManager.publish(event);
    }

    ASSERT_EQ(values.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(values[i], i);
    }
}
