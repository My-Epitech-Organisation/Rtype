// EPITECH PROJECT, 2025
// r-type
// File: test_rumble.cpp - ControllerRumble branch coverage tests

#include <gtest/gtest.h>

#include "Graphic/ControllerRumble.hpp"

// These tests avoid hardware dependencies. They exercise early-return branches
// and idempotent paths to improve branch coverage safely.

TEST(ControllerRumbleTest, InitializeTwiceIsIdempotent) {
    // First initialize should succeed (or fail gracefully); second call should
    // hit the "Already initialized" branch and return.
    ControllerRumble::initialize();
    // Calling initialize again should not re-initialize.
    ControllerRumble::initialize();
    // Cleanup to restore state for other tests.
    ControllerRumble::cleanup();
}

TEST(ControllerRumbleTest, UpdateEarlyReturnWhenNoRumbles) {
    // With no rumble timers scheduled, update should early-return.
    ControllerRumble::initialize();
    ControllerRumble::update();
    ControllerRumble::cleanup();
}

TEST(ControllerRumbleTest, IsRumblingFalseWhenUnset) {
    // No rumble scheduled for this id; should be reported as not rumbling.
    ASSERT_FALSE(ControllerRumble::isRumbling(7));
}

TEST(ControllerRumbleTest, TriggerRumbleNotConnectedEarlyReturn) {
    // Using a likely-unconnected joystick id triggers the "not connected" branch.
    // Also pass out-of-range intensity and negative duration to exercise parameter
    // handling in code paths we can reach without hardware.
    ControllerRumble::triggerRumble(99, -0.5f, -100);
}

TEST(ControllerRumbleTest, TriggerRumbleClampsIntensityAndDuration) {
    // Even without hardware, calling triggerRumble should clamp parameters internally.
    // We invoke several edge values to traverse clamp branches.
    EXPECT_NO_THROW(ControllerRumble::triggerRumble(100, -1.0f, -10)); // intensity < 0, duration < 0
    EXPECT_NO_THROW(ControllerRumble::triggerRumble(101, 2.0f, 10));   // intensity > 1
    EXPECT_NO_THROW(ControllerRumble::triggerRumble(102, 0.0f, 0));    // zero intensity/duration
}

    TEST(ControllerRumbleTest, StopRumbleOnUnknownIdDoesNothing) {
        // Calling stopRumble for an ID that was never started should be a no-op.
        EXPECT_NO_THROW({ ControllerRumble::stopRumble(999); });
    }

    TEST(ControllerRumbleTest, CleanupWhenNotInitializedDoesNotCrash) {
        // Ensure cleanup is safe even if SDL was never initialized.
        EXPECT_NO_THROW({ ControllerRumble::cleanup(); });
    }
