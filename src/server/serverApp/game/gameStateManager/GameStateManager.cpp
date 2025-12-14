/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameStateManager - Implementation
*/

#include "GameStateManager.hpp"

#include <rtype/common.hpp>

namespace rtype::server {

GameStateManager::GameStateManager(size_t minPlayersToStart)
    : _minPlayersToStart(minPlayersToStart) {}

bool GameStateManager::playerReady(std::uint32_t userId) {
    if (_state == GameState::Playing || _state == GameState::GameOver) {
        LOG_DEBUG("[GameState] Player "
                  << userId
                  << " signaled ready but game is already running or ended");
        return false;
    }

    auto [_, inserted] = _readyPlayers.insert(userId);
    if (inserted) {
        LOG_INFO("[GameState] Player "
                 << userId << " is ready (" << _readyPlayers.size() << "/"
                 << _minPlayersToStart << " needed to start)");
        checkAutoStart();
    }
    return inserted;
}

void GameStateManager::playerLeft(std::uint32_t userId) {
    _readyPlayers.erase(userId);
    LOG_DEBUG("[GameState] Player " << userId << " left, "
                                    << _readyPlayers.size()
                                    << " players remaining");

    if (_state == GameState::Playing && _readyPlayers.empty()) {
        LOG_INFO("[GameState] All players left during game. Ending game...");
        transitionTo(GameState::GameOver);
    }
}

void GameStateManager::transitionTo(GameState newState) {
    if (_state == newState) {
        return;
    }

    LOG_INFO("[GameState] State transition: " << toString(_state) << " -> "
                                              << toString(newState));

    GameState oldState = _state;
    _state = newState;

    if (_stateChangeCallback) {
        _stateChangeCallback(oldState, newState);
    }
}

void GameStateManager::forceStart() { transitionTo(GameState::Playing); }

void GameStateManager::pause() { transitionTo(GameState::Paused); }

void GameStateManager::reset() {
    _readyPlayers.clear();
    transitionTo(GameState::WaitingForPlayers);
}

void GameStateManager::checkAutoStart() {
    if (_state != GameState::WaitingForPlayers && _state != GameState::Paused) {
        return;
    }

    if (_readyPlayers.size() >= _minPlayersToStart) {
        transitionTo(GameState::Playing);
    }
}

}  // namespace rtype::server
