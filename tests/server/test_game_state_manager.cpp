/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_game_state_manager - Unit tests for GameStateManager
*/

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "../../src/server/serverApp/game/gameStateManager/GameStateManager.hpp"

using namespace rtype::server;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class GameStateManagerTest : public ::testing::Test {
   protected:
    void SetUp() override {}

    void TearDown() override {}
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(GameStateManagerTest, Constructor_DefaultMinPlayers) {
    GameStateManager manager;

    EXPECT_EQ(manager.getState(), GameState::WaitingForPlayers);
    EXPECT_TRUE(manager.isWaiting());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_FALSE(manager.isPaused());
}

TEST_F(GameStateManagerTest, Constructor_CustomMinPlayers) {
    GameStateManager manager(3);

    EXPECT_EQ(manager.getState(), GameState::WaitingForPlayers);
}

TEST_F(GameStateManagerTest, Constructor_ZeroMinPlayers) {
    GameStateManager manager(0);

    EXPECT_EQ(manager.getState(), GameState::WaitingForPlayers);
}

// ============================================================================
// PLAYER READY TESTS
// ============================================================================

TEST_F(GameStateManagerTest, PlayerReady_FirstPlayer) {
    GameStateManager manager(2);

    bool result = manager.playerReady(1);

    EXPECT_TRUE(result);
    EXPECT_EQ(manager.getReadyPlayerCount(), 1u);
    EXPECT_TRUE(manager.isPlayerReady(1));
}

TEST_F(GameStateManagerTest, PlayerReady_DuplicatePlayer) {
    GameStateManager manager(2);

    manager.playerReady(1);
    bool result = manager.playerReady(1);

    EXPECT_FALSE(result);
    EXPECT_EQ(manager.getReadyPlayerCount(), 1u);
}

TEST_F(GameStateManagerTest, PlayerReady_WhenAlreadyPlaying) {
    GameStateManager manager(1);

    manager.playerReady(1);
    // Now countdown should be active instead of immediate playing
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isCountdownActive());

    bool result = manager.playerReady(2);

    // Adding another player should succeed and increase ready count
    EXPECT_TRUE(result);
    EXPECT_EQ(manager.getReadyPlayerCount(), 2u);
}

TEST_F(GameStateManagerTest, PlayerReady_WithZeroMinPlayers) {
    GameStateManager manager(0);

    // With 0 min players, should start countdown (but not immediately play)
    manager.playerReady(1);
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isCountdownActive());
}

TEST_F(GameStateManagerTest, PlayerLeft_LastPlayer_PausesGame) {
    GameStateManager manager(1);

    manager.playerReady(1);
    EXPECT_TRUE(manager.isCountdownActive());

    manager.playerLeft(1);
    // Countdown cancelled and no game started
    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_EQ(manager.getReadyPlayerCount(), 0u);
}

TEST_F(GameStateManagerTest, PlayerLeft_NotLastPlayer_ContinuesGame) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_TRUE(manager.isCountdownActive());

    // Leave player 1, player 2 still in ready set
    manager.playerLeft(1);
    
    // Countdown should be cancelled because ready < min required
    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_EQ(manager.getReadyPlayerCount(), 1u);
}

TEST_F(GameStateManagerTest, PlayerLeft_WithZeroConnected_CancelsCountdown) {
    GameStateManager manager(1);

    // Simulate uninitialized/zero connected players
    manager.setConnectedPlayerCount(0);

    manager.playerReady(1);
    EXPECT_TRUE(manager.isCountdownActive());

    manager.playerLeft(1);

    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isWaiting());
    EXPECT_EQ(manager.getReadyPlayerCount(), 0u);
}

TEST_F(GameStateManagerTest, PlayerReady_TriggersAutoStart) {
    GameStateManager manager(2);

    manager.playerReady(1);
    EXPECT_FALSE(manager.isPlaying());

    manager.playerReady(2);
    // Now countdown should be active but game should not yet be playing
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isCountdownActive());
}

TEST_F(GameStateManagerTest, Countdown_Finishes_StartsGame) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_TRUE(manager.isCountdownActive());

    // Fast-forward beyond default countdown
    manager.update(5.0f);
    EXPECT_TRUE(manager.isPlaying());
}

TEST_F(GameStateManagerTest, Countdown_Cancelled_ByUnready) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_TRUE(manager.isCountdownActive());

    // One player becomes not ready
    manager.playerNotReady(2);
    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isWaiting());
}

TEST_F(GameStateManagerTest, Countdown_Cancelled_FromPaused_ByUnready) {
    GameStateManager manager(2);

    // Start from Paused and then satisfy auto-start conditions
    manager.transitionTo(GameState::Paused);
    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_TRUE(manager.isCountdownActive());

    // One player becomes not ready - countdown should cancel and state should be WaitingForPlayers
    manager.playerNotReady(2);
    EXPECT_FALSE(manager.isCountdownActive());
    EXPECT_FALSE(manager.isPlaying());
    EXPECT_TRUE(manager.isWaiting());
}

// ============================================================================
// PLAYER LEFT TESTS
// ============================================================================

TEST_F(GameStateManagerTest, PlayerLeft_DecreasesCount) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_EQ(manager.getReadyPlayerCount(), 2u);

    manager.playerLeft(1);
    EXPECT_EQ(manager.getReadyPlayerCount(), 1u);
}

TEST_F(GameStateManagerTest, PlayerLeft_NonExistentPlayer) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerLeft(999);  // Non-existent

    EXPECT_EQ(manager.getReadyPlayerCount(), 1u);
}



// ============================================================================
// STATE TRANSITION TESTS
// ============================================================================

TEST_F(GameStateManagerTest, TransitionTo_SameState) {
    GameStateManager manager;

    manager.transitionTo(GameState::WaitingForPlayers);

    EXPECT_TRUE(manager.isWaiting());
}

TEST_F(GameStateManagerTest, TransitionTo_Playing) {
    GameStateManager manager;

    manager.transitionTo(GameState::Playing);

    EXPECT_TRUE(manager.isPlaying());
    EXPECT_FALSE(manager.isWaiting());
    EXPECT_FALSE(manager.isPaused());
}

TEST_F(GameStateManagerTest, TransitionTo_Paused) {
    GameStateManager manager;

    manager.transitionTo(GameState::Paused);

    EXPECT_TRUE(manager.isPaused());
    EXPECT_FALSE(manager.isWaiting());
    EXPECT_FALSE(manager.isPlaying());
}

TEST_F(GameStateManagerTest, TransitionTo_WithCallback) {
    GameStateManager manager;

    GameState oldState = GameState::Playing;
    GameState newState = GameState::WaitingForPlayers;

    manager.setStateChangeCallback([&](GameState old_, GameState new_) {
        oldState = old_;
        newState = new_;
    });

    manager.transitionTo(GameState::Playing);

    EXPECT_EQ(oldState, GameState::WaitingForPlayers);
    EXPECT_EQ(newState, GameState::Playing);
}

TEST_F(GameStateManagerTest, TransitionTo_SameState_NoCallback) {
    GameStateManager manager;

    bool callbackCalled = false;
    manager.setStateChangeCallback([&](GameState, GameState) {
        callbackCalled = true;
    });

    manager.transitionTo(GameState::WaitingForPlayers);  // Same state

    EXPECT_FALSE(callbackCalled);
}

// ============================================================================
// FORCE START TESTS
// ============================================================================

TEST_F(GameStateManagerTest, ForceStart_FromWaiting) {
    GameStateManager manager;

    manager.forceStart();

    EXPECT_TRUE(manager.isPlaying());
}

TEST_F(GameStateManagerTest, ForceStart_FromPaused) {
    GameStateManager manager;

    manager.transitionTo(GameState::Paused);
    manager.forceStart();

    EXPECT_TRUE(manager.isPlaying());
}

TEST_F(GameStateManagerTest, ForceStart_AlreadyPlaying) {
    GameStateManager manager;

    manager.forceStart();
    manager.forceStart();  // Second call

    EXPECT_TRUE(manager.isPlaying());
}

// ============================================================================
// PAUSE TESTS
// ============================================================================

TEST_F(GameStateManagerTest, Pause_FromPlaying) {
    GameStateManager manager;

    manager.forceStart();
    manager.pause();

    EXPECT_TRUE(manager.isPaused());
}

TEST_F(GameStateManagerTest, Pause_FromWaiting) {
    GameStateManager manager;

    manager.pause();

    EXPECT_TRUE(manager.isPaused());
}

TEST_F(GameStateManagerTest, Pause_AlreadyPaused) {
    GameStateManager manager;

    manager.pause();
    manager.pause();  // Second call

    EXPECT_TRUE(manager.isPaused());
}

// ============================================================================
// RESET TESTS
// ============================================================================

TEST_F(GameStateManagerTest, Reset_ClearsReadyPlayers) {
    GameStateManager manager(2);

    manager.playerReady(1);
    manager.playerReady(2);
    EXPECT_EQ(manager.getReadyPlayerCount(), 2u);

    manager.reset();

    EXPECT_EQ(manager.getReadyPlayerCount(), 0u);
}

TEST_F(GameStateManagerTest, Reset_TransitionsToWaiting) {
    GameStateManager manager;

    manager.forceStart();
    EXPECT_TRUE(manager.isPlaying());

    manager.reset();

    EXPECT_TRUE(manager.isWaiting());
}

TEST_F(GameStateManagerTest, Reset_FromPaused) {
    GameStateManager manager;

    manager.pause();
    manager.reset();

    EXPECT_TRUE(manager.isWaiting());
}

// ============================================================================
// GET READY PLAYERS TESTS
// ============================================================================

TEST_F(GameStateManagerTest, GetReadyPlayers_Empty) {
    GameStateManager manager;

    const auto& players = manager.getReadyPlayers();

    EXPECT_TRUE(players.empty());
}

TEST_F(GameStateManagerTest, GetReadyPlayers_WithPlayers) {
    GameStateManager manager(5);

    manager.playerReady(1);
    manager.playerReady(3);
    manager.playerReady(5);

    const auto& players = manager.getReadyPlayers();

    EXPECT_EQ(players.size(), 3u);
    EXPECT_EQ(players.count(1), 1u);
    EXPECT_EQ(players.count(3), 1u);
    EXPECT_EQ(players.count(5), 1u);
}

// ============================================================================
// IS PLAYER READY TESTS
// ============================================================================

TEST_F(GameStateManagerTest, IsPlayerReady_True) {
    GameStateManager manager(2);

    manager.playerReady(42);

    EXPECT_TRUE(manager.isPlayerReady(42));
}

TEST_F(GameStateManagerTest, IsPlayerReady_False) {
    GameStateManager manager(2);

    EXPECT_FALSE(manager.isPlayerReady(42));
}

TEST_F(GameStateManagerTest, IsPlayerReady_AfterLeft) {
    GameStateManager manager(2);

    manager.playerReady(42);
    manager.playerLeft(42);

    EXPECT_FALSE(manager.isPlayerReady(42));
}

// ============================================================================
// AUTO START FROM PAUSED TESTS
// ============================================================================

TEST_F(GameStateManagerTest, AutoStart_FromPaused) {
    GameStateManager manager(1);

    manager.transitionTo(GameState::Paused);

    manager.playerReady(1);

    // Paused -> ready should start countdown, not immediately play
    EXPECT_TRUE(manager.isCountdownActive());
}

TEST_F(GameStateManagerTest, CheckAutoStart_FromPlaying_NoTransition) {
    GameStateManager manager(1);

    manager.forceStart();
    EXPECT_TRUE(manager.isPlaying());

    // Adding player when already playing shouldn't change state
    // (playerReady returns early when Playing)
    manager.playerReady(2);
    EXPECT_TRUE(manager.isPlaying());
}

// ============================================================================
// TO STRING TESTS
// ============================================================================

TEST_F(GameStateManagerTest, ToString_WaitingForPlayers) {
    EXPECT_EQ(toString(GameState::WaitingForPlayers), "WaitingForPlayers");
}

TEST_F(GameStateManagerTest, ToString_Playing) {
    EXPECT_EQ(toString(GameState::Playing), "Playing");
}

TEST_F(GameStateManagerTest, ToString_Paused) {
    EXPECT_EQ(toString(GameState::Paused), "Paused");
}
