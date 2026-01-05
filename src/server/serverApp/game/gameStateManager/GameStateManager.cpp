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

        if (_onPlayerReadyStateChangedCallback) {
            _onPlayerReadyStateChangedCallback(userId, true);
        }

        checkAutoStart();
    }
    return inserted;
}

bool GameStateManager::playerNotReady(std::uint32_t userId) {
    if (_state == GameState::Playing || _state == GameState::GameOver) {
        LOG_DEBUG(
            "[GameState] Player "
            << userId
            << " signaled not ready but game is already running or ended");
        return false;
    }

    size_t removed = _readyPlayers.erase(userId);
    if (removed > 0) {
        LOG_INFO("[GameState] Player "
                 << userId << " is no longer ready (" << _readyPlayers.size()
                 << "/" << _minPlayersToStart << " needed to start)");

        if (_onPlayerReadyStateChangedCallback) {
            _onPlayerReadyStateChangedCallback(userId, false);
        }

        if (_countdownActive) {
            LOG_INFO("[GameStateManager] Countdown cancelled due to player "
                     << userId << " becoming not ready");
            _countdownActive = false;
            _countdownRemaining = 0.0f;
            if (_onCountdownCancelledCallback) {
                _onCountdownCancelledCallback();
            }
            if (_state != GameState::WaitingForPlayers) {
                transitionTo(GameState::WaitingForPlayers);
            }
        }
    }
    return removed > 0;
}

void GameStateManager::playerLeft(std::uint32_t userId) {
    _readyPlayers.erase(userId);
    LOG_DEBUG("[GameState] Player " << userId << " left, "
                                    << _readyPlayers.size()
                                    << " players remaining");

    if (_countdownActive) {
        if (_readyPlayers.size() < _minPlayersToStart ||
            (_connectedPlayerCount > 0 &&
             _readyPlayers.size() < _connectedPlayerCount)) {
            LOG_INFO("[GameStateManager] Countdown cancelled due to player "
                     << userId << " leaving");
            _countdownActive = false;
            _countdownRemaining = 0.0f;
            if (_onCountdownCancelledCallback) {
                _onCountdownCancelledCallback();
            }
            if (_state != GameState::WaitingForPlayers) {
                transitionTo(GameState::WaitingForPlayers);
            }
        }
    }

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
    _countdownActive = false;
    _countdownRemaining = 0.0f;
    transitionTo(GameState::WaitingForPlayers);
}

void GameStateManager::checkAutoStart() {
    if (_state != GameState::WaitingForPlayers && _state != GameState::Paused) {
        return;
    }

    if (_readyPlayers.size() < _minPlayersToStart) {
        return;
    }

    if (_connectedPlayerCount > 0 &&
        _readyPlayers.size() < _connectedPlayerCount) {
        return;
    }

    LOG_INFO("[GameStateManager] Auto-start conditions met: ready="
             << _readyPlayers.size() << " connected=" << _connectedPlayerCount
             << " minRequired=" << _minPlayersToStart);

    if (!_countdownActive) {
        _countdownActive = true;
        _countdownRemaining = _defaultCountdown;
        if (_onCountdownStartedCallback) {
            _onCountdownStartedCallback(_countdownRemaining);
        }
    }
}

void GameStateManager::update(float deltaTime) {
    if (!_countdownActive) {
        return;
    }

    _countdownRemaining -= deltaTime;

    if (_countdownRemaining <= 0.0f) {
        _countdownActive = false;
        _countdownRemaining = 0.0f;
        LOG_INFO(
            "[GameStateManager] Countdown finished - transitioning to Playing");
        transitionTo(GameState::Playing);
    }
}

}  // namespace rtype::server
