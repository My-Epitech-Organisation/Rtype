#include <gtest/gtest.h>

#include "ControllerRumble.hpp"

using namespace ::testing;

TEST(ControllerRumbleTest, StopWithoutRumbleNoCrash) {
    // Should not throw or crash even if no rumble is registered
    EXPECT_NO_THROW(ControllerRumble::stopRumble(12345));
}

TEST(ControllerRumbleTest, IsRumblingFalseWhenEmpty) {
    EXPECT_FALSE(ControllerRumble::isRumbling(0));
}

TEST(ControllerRumbleTest, InitializeUpdateCleanupNoCrash) {
    // These should be safe to call even if SDL is not available / no controllers
    EXPECT_NO_THROW(ControllerRumble::initialize());
    EXPECT_NO_THROW(ControllerRumble::update());
    EXPECT_NO_THROW(ControllerRumble::cleanup());
}
