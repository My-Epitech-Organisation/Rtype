/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_time
*/

#include "../../src/engine/core/Time.hpp"
#include <gtest/gtest.h>

using namespace rtype::engine::core;

TEST(TimeTest, DefaultConstructorInitializesZero) {
    Time time;

    EXPECT_DOUBLE_EQ(time.deltaTime(), 0.0);
    EXPECT_DOUBLE_EQ(time.totalTime(), 0.0);
}

TEST(TimeTest, UpdateSetsDeltaTime) {
    Time time;

    time.update();

    EXPECT_GT(time.deltaTime(), 0.0);
}

TEST(TimeTest, UpdateIncrementsTotalTime) {
    Time time;
    EXPECT_DOUBLE_EQ(time.totalTime(), 0.0);

    time.update();
    double afterFirst = time.totalTime();
    EXPECT_GT(afterFirst, 0.0);

    time.update();
    EXPECT_GT(time.totalTime(), afterFirst);
}

TEST(TimeTest, MultipleUpdatesAccumulateTotalTime) {
    Time time;

    for (int i = 0; i < 10; ++i) {
        time.update();
    }

    // With ~60 FPS (0.016s per frame), 10 frames should be ~0.16s
    EXPECT_GT(time.totalTime(), 0.1);
    EXPECT_LT(time.totalTime(), 0.3);
}

TEST(TimeTest, DeltaTimeIsConsistent) {
    Time time;

    time.update();
    double delta1 = time.deltaTime();

    time.update();
    double delta2 = time.deltaTime();

    // In this placeholder implementation, delta should be consistent
    EXPECT_DOUBLE_EQ(delta1, delta2);
}

TEST(TimeTest, DeltaTimeIsReasonable) {
    Time time;

    time.update();

    // Delta time should be between 1ms and 100ms (10-1000 FPS)
    EXPECT_GT(time.deltaTime(), 0.001);
    EXPECT_LT(time.deltaTime(), 0.1);
}
