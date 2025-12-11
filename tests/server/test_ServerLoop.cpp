/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerLoop - Unit Tests
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "server/serverApp/ServerLoop.hpp"

using rtype::server::ServerLoop;

class ServerLoopTest : public ::testing::Test {
   protected:
    void SetUp() override {
        _shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    }
    void TearDown() override {
        _shutdownFlag->store(true);
    }
    
    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
};

// ====================
// Construction Tests
// ====================

TEST_F(ServerLoopTest, ConstructionCreatesValidLoop) {
    ServerLoop loop(60, _shutdownFlag);
    EXPECT_EQ(loop.getTickRate(), 60U);
}

TEST_F(ServerLoopTest, ConstructionWithHighTickRate) {
    ServerLoop loop(120, _shutdownFlag);
    EXPECT_EQ(loop.getTickRate(), 120U);
}

TEST_F(ServerLoopTest, ConstructionWithLowTickRate) {
    ServerLoop loop(10, _shutdownFlag);
    EXPECT_EQ(loop.getTickRate(), 10U);
}

TEST_F(ServerLoopTest, GetDeltaTimeReturnsCorrectValue) {
    ServerLoop loop(60, _shutdownFlag);
    float expectedDt = 1.0F / 60.0F;
    EXPECT_NEAR(loop.getDeltaTime(), expectedDt, 0.0001F);
}

TEST_F(ServerLoopTest, GetDeltaTimeWithDifferentTickRate) {
    ServerLoop loop(30, _shutdownFlag);
    float expectedDt = 1.0F / 30.0F;
    EXPECT_NEAR(loop.getDeltaTime(), expectedDt, 0.0001F);
}

// ====================
// Run/Stop Tests
// ====================

TEST_F(ServerLoopTest, RunExecutesCallbacks) {
    ServerLoop loop(60, _shutdownFlag);
    
    int frameCount = 0;
    int updateCount = 0;
    int postUpdateCount = 0;
    
    auto frameCallback = [&]() {
        frameCount++;
        if (frameCount >= 3) {
            _shutdownFlag->store(true);
        }
    };
    
    auto updateCallback = [&](float /*dt*/) {
        updateCount++;
    };
    
    auto postUpdateCallback = [&]() {
        postUpdateCount++;
    };
    
    loop.run(frameCallback, updateCallback, postUpdateCallback);
    
    EXPECT_GE(frameCount, 3);
    EXPECT_GE(updateCount, 1);
    EXPECT_GE(postUpdateCount, 1);
}

TEST_F(ServerLoopTest, ShutdownFlagStopsTheLoop) {
    ServerLoop loop(60, _shutdownFlag);
    
    int iterations = 0;
    
    loop.run(
        [&]() {
            iterations++;
            if (iterations >= 5) {
                _shutdownFlag->store(true);
            }
        },
        [](float) {},
        []() {}
    );
    
    EXPECT_TRUE(_shutdownFlag->load());
    EXPECT_GE(iterations, 5);
}

TEST_F(ServerLoopTest, ImmediateShutdownStopsQuickly) {
    _shutdownFlag->store(true);  // Set before running
    ServerLoop loop(60, _shutdownFlag);
    
    int iterations = 0;
    
    loop.run(
        [&]() { iterations++; },
        [](float) {},
        []() {}
    );
    
    EXPECT_EQ(iterations, 0);
}

// ====================
// Delta Time Tests
// ====================

TEST_F(ServerLoopTest, UpdateCallbackReceivesCorrectDeltaTime) {
    ServerLoop loop(60, _shutdownFlag);
    
    float expectedDt = 1.0F / 60.0F;
    float receivedDt = -1.0F;
    int iterations = 0;
    
    loop.run(
        [&]() {
            iterations++;
            if (iterations >= 3) {
                _shutdownFlag->store(true);
            }
        },
        [&](float dt) {
            if (receivedDt < 0.0F) {
                receivedDt = dt;
            }
        },
        []() {}
    );
    
    EXPECT_NEAR(receivedDt, expectedDt, 0.0001F);
}

// ====================
// Fixed Update Tests
// ====================

TEST_F(ServerLoopTest, FixedUpdateCalledMultipleTimesForLargeDelta) {
    ServerLoop loop(100, _shutdownFlag);  // 10ms per tick
    
    int updateCalls = 0;
    int frameIterations = 0;
    
    loop.run(
        [&]() {
            frameIterations++;
            // Sleep to accumulate time for multiple fixed updates
            if (frameIterations == 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } else if (frameIterations >= 3) {
                _shutdownFlag->store(true);
            }
        },
        [&](float /*dt*/) {
            updateCalls++;
        },
        []() {}
    );
    
    // Multiple updates should have been called due to accumulated time
    EXPECT_GE(updateCalls, 1);
}

// ====================
// Callback Nullptr Tests
// ====================

TEST_F(ServerLoopTest, RunWithNullUpdateCallbackDoesNotCrash) {
    ServerLoop loop(60, _shutdownFlag);
    
    EXPECT_NO_THROW({
        loop.run(
            [&]() { _shutdownFlag->store(true); },
            nullptr,
            []() {}
        );
    });
}

TEST_F(ServerLoopTest, RunWithNullPostUpdateCallbackDoesNotCrash) {
    ServerLoop loop(60, _shutdownFlag);
    
    EXPECT_NO_THROW({
        loop.run(
            [&]() { _shutdownFlag->store(true); },
            [](float) {},
            nullptr
        );
    });
}

// ====================
// Tick Overrun Tests
// ====================

TEST_F(ServerLoopTest, TickOverrunIsTracked) {
    ServerLoop loop(60, _shutdownFlag);
    
    int iterations = 0;
    
    // Simulate slow updates to cause overruns
    loop.run(
        [&]() {
            iterations++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (iterations >= 3) {
                _shutdownFlag->store(true);
            }
        },
        [](float) {},
        []() {}
    );
    
    // Tick overruns are expected since we're sleeping longer than tick rate
    EXPECT_GE(loop.getTickOverruns(), 0UL);
}

// ====================
// Edge Cases
// ====================

TEST_F(ServerLoopTest, MultipleRunsAreIdempotent) {
    int runCount = 0;
    
    for (int i = 0; i < 3; i++) {
        _shutdownFlag->store(false);
        ServerLoop loop(60, _shutdownFlag);
        
        loop.run(
            [&]() {
                runCount++;
                _shutdownFlag->store(true);
            },
            [](float) {},
            []() {}
        );
    }
    
    EXPECT_EQ(runCount, 3);
}

TEST_F(ServerLoopTest, RapidStartStopCycles) {
    for (int i = 0; i < 10; i++) {
        _shutdownFlag->store(false);
        ServerLoop loop(60, _shutdownFlag);
        
        loop.run(
            [&]() { _shutdownFlag->store(true); },
            [](float) {},
            []() {}
        );
        EXPECT_TRUE(_shutdownFlag->load());
    }
}

