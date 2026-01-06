#include <gtest/gtest.h>

#include "../../src/server/serverApp/game/gameStateManager/GameStateManager.hpp"

using namespace rtype::server;

TEST(GameStateManagerCountdownTest, CountdownStartsWhenDefaultNonZero) {
    GameStateManager manager(1);

    manager.setDefaultCountdown(2.5f);

    bool countdownStarted = false;
    float startValue = 0.0f;
    manager.setOnCountdownStarted([&](float v) {
        countdownStarted = true;
        startValue = v;
    });

    manager.playerReady(1);

    EXPECT_TRUE(countdownStarted);
    EXPECT_TRUE(manager.isCountdownActive());
    EXPECT_FLOAT_EQ(manager.getCountdownRemaining(), startValue);
    EXPECT_FLOAT_EQ(startValue, 2.5f);
}

TEST(GameStateManagerCountdownTest, CountdownFinishesAndTransitionsToPlaying) {
    GameStateManager manager(1);
    manager.setDefaultCountdown(0.5f);

    bool countdownStarted = false;
    manager.setOnCountdownStarted([&](float) { countdownStarted = true; });

    GameState oldState = GameState::WaitingForPlayers;
    GameState newState = GameState::WaitingForPlayers;
    manager.setStateChangeCallback([&](GameState o, GameState n) {
        oldState = o;
        newState = n;
    });

    manager.playerReady(1);
    EXPECT_TRUE(countdownStarted);
    EXPECT_TRUE(manager.isCountdownActive());

    manager.update(0.6f);

    EXPECT_TRUE(manager.isPlaying());
    EXPECT_EQ(oldState, GameState::WaitingForPlayers);
    EXPECT_EQ(newState, GameState::Playing);
    EXPECT_FALSE(manager.isCountdownActive());
}

TEST(GameStateManagerCountdownTest, CountdownCancelledOnPlayerNotReady) {
    GameStateManager manager(1);
    manager.setDefaultCountdown(1.0f);

    bool cancelled = false;
    manager.setOnCountdownCancelled([&]() { cancelled = true; });

    manager.playerReady(1);
    EXPECT_TRUE(manager.isCountdownActive());

    bool result = manager.playerNotReady(1);
    EXPECT_TRUE(result);
    EXPECT_TRUE(cancelled);
    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_TRUE(manager.isWaiting());
}

TEST(GameStateManagerCountdownTest, DoesNotStartWhenConnectedCountGreater) {
    GameStateManager manager(1);
    manager.setDefaultCountdown(1.0f);

    manager.setConnectedPlayerCount(2);
    manager.playerReady(1);

    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_TRUE(manager.isWaiting());
}

TEST(GameStateManagerCountdownTest, PlayerLeftCancelsCountdownAndTransitionsWhenNeeded) {
    GameStateManager manager(2);
    manager.setDefaultCountdown(1.0f);

    bool cancelled = false;
    manager.setOnCountdownCancelled([&]() { cancelled = true; });

    manager.setConnectedPlayerCount(2);
    manager.playerReady(1);
    manager.playerReady(2);

    EXPECT_TRUE(manager.isCountdownActive());

    // Force state to Playing to exercise the branch that triggers a transition
    manager.transitionTo(GameState::Playing);
    EXPECT_TRUE(manager.isPlaying());

    manager.playerLeft(1);

    EXPECT_TRUE(cancelled);
    // After cancellation, the code transitions back to Waiting when state != Waiting
    EXPECT_TRUE(manager.isWaiting());
}
