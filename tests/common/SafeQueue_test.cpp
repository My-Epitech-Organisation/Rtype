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

TEST(SafeQueueTest, SizeReturnsCorrectCount) {
    SafeQueue<int> queue;
    EXPECT_EQ(queue.size(), 0);
    queue.push(1);
    EXPECT_EQ(queue.size(), 1);
    queue.push(2);
    EXPECT_EQ(queue.size(), 2);
    queue.pop();
    EXPECT_EQ(queue.size(), 1);
    queue.pop();
    EXPECT_EQ(queue.size(), 0);
    queue.pop();  // Should not change size when empty
    EXPECT_EQ(queue.size(), 0);
}

TEST(SafeQueueTest, SizeIsConcurrentlySafe) {
    SafeQueue<int> queue;
    std::atomic<int> operationsCompleted{0};
    const int numThreads = 5;
    const int operationsPerThread = 50;

    auto worker = [&queue, &operationsCompleted]() {
        for (int i = 0; i < 50; ++i) {
            queue.push(i);
            queue.size();  // Test concurrent size calls
            queue.pop();
        }
        operationsCompleted++;
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) t.join();

    EXPECT_EQ(operationsCompleted, numThreads);
    EXPECT_EQ(queue.size(), 0);  // Should be empty after all operations
}

TEST(SafeQueueTest, MovePushPreservesObject) {
    SafeQueue<std::string> queue;

    std::string original = "Hello, World!";
    std::string movedFrom = original;

    queue.push(std::move(movedFrom));

    EXPECT_TRUE(movedFrom.empty());  // Should be moved from

    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, original);  // Should contain the original content
}

TEST(SafeQueueTest, ComplexTypeHandling) {
    struct ComplexType {
        int id;
        std::string name;
        std::vector<int> data;

        bool operator==(const ComplexType& other) const {
            return id == other.id && name == other.name && data == other.data;
        }
    };

    SafeQueue<ComplexType> queue;

    ComplexType original{42, "test", {1, 2, 3}};
    queue.push(original);

    EXPECT_EQ(queue.size(), 1);

    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, original);
    EXPECT_EQ(queue.size(), 0);
}
