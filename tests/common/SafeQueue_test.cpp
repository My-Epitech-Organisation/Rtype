#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <algorithm>
#include "common/SafeQueue/SafeQueue.hpp"

TEST(SafeQueueTest, PushAndPop) {
    SafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    auto val = queue.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 1);
    val = queue.pop();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 2);
    val = queue.pop();
    ASSERT_FALSE(val.has_value());
}

TEST(SafeQueueTest, ConcurrentPushPop) {
    SafeQueue<int> queue;
    const int numThreads = 10;
    const int itemsPerThread = 100;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Producers
    for (int i = 0; i < numThreads; ++i) {
        producers.emplace_back([&queue, i, itemsPerThread]() {
            for (int j = 0; j < itemsPerThread; ++j) {
                queue.push(i * itemsPerThread + j);
            }
        });
    }

    // Consumers
    std::vector<int> consumed;
    std::mutex consumedMutex;
    for (int i = 0; i < numThreads; ++i) {
        consumers.emplace_back([&queue, &consumed, &consumedMutex, itemsPerThread]() {
            for (int j = 0; j < itemsPerThread; ++j) {
                auto val = queue.pop();
                while (!val) {
                    std::this_thread::yield();
                    val = queue.pop();
                }
                std::lock_guard<std::mutex> lock(consumedMutex);
                consumed.push_back(*val);
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    EXPECT_EQ(consumed.size(), numThreads * itemsPerThread);
    std::sort(consumed.begin(), consumed.end());
    for (int i = 0; i < numThreads * itemsPerThread; ++i) {
        EXPECT_EQ(consumed[i], i);
    }
}
