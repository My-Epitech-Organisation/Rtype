/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ConnectionStateMachine - Implementation
*/

#include "ConnectionStateMachine.hpp"

namespace rtype::network {

ConnectionStateMachine::ConnectionStateMachine() noexcept : config_() {}

ConnectionStateMachine::ConnectionStateMachine(const Config& config) noexcept
    : config_(config) {}

Result<void> ConnectionStateMachine::initiateConnect() {
    if (!canInitiateConnect(state_)) {
        return Err<void>(NetworkError::InvalidStateTransition);
    }
    retryCount_ = 0;
    userId_ = std::nullopt;
    lastDisconnectReason_ = std::nullopt;
    transitionTo(ConnectionState::Connecting);
    return Ok();
}

Result<void> ConnectionStateMachine::handleAccept(std::uint32_t userId) {
    if (!canReceiveAccept(state_)) {
        return Err<void>(NetworkError::InvalidStateTransition);
    }
    userId_ = userId;
    transitionTo(ConnectionState::Connected);
    if (callbacks_.onConnected) {
        callbacks_.onConnected(userId);
    }
    return Ok();
}

Result<void> ConnectionStateMachine::initiateDisconnect() {
    if (!canInitiateDisconnect(state_)) {
        return Err<void>(NetworkError::InvalidStateTransition);
    }
    transitionTo(ConnectionState::Disconnecting);
    return Ok();
}

Result<void> ConnectionStateMachine::handleDisconnectAck() {
    if (!canFinalizeDisconnect(state_)) {
        return Err<void>(NetworkError::InvalidStateTransition);
    }
    lastDisconnectReason_ = DisconnectReason::LocalRequest;
    transitionTo(ConnectionState::Disconnected);
    if (callbacks_.onDisconnected) {
        callbacks_.onDisconnected(DisconnectReason::LocalRequest);
    }
    return Ok();
}

Result<void> ConnectionStateMachine::handleRemoteDisconnect() {
    if (state_ == ConnectionState::Disconnected) {
        return Err<void>(NetworkError::InvalidStateTransition);
    }
    lastDisconnectReason_ = DisconnectReason::RemoteRequest;
    transitionTo(ConnectionState::Disconnected);
    if (callbacks_.onDisconnected) {
        callbacks_.onDisconnected(DisconnectReason::RemoteRequest);
    }
    return Ok();
}

void ConnectionStateMachine::forceDisconnect(DisconnectReason reason) noexcept {
    if (state_ == ConnectionState::Disconnected) {
        return;
    }
    lastDisconnectReason_ = reason;
    transitionTo(ConnectionState::Disconnected);
    if (callbacks_.onDisconnected) {
        callbacks_.onDisconnected(reason);
    }
}

ConnectionStateMachine::UpdateResult ConnectionStateMachine::update() {
    switch (state_) {
        case ConnectionState::Disconnected:
            return UpdateResult::NoAction;

        case ConnectionState::Connecting:
            if (isTimedOut()) {
                ++retryCount_;
                if (retryCount_ > config_.maxConnectRetries) {
                    lastDisconnectReason_ =
                        DisconnectReason::MaxRetriesExceeded;
                    transitionTo(ConnectionState::Disconnected);
                    if (callbacks_.onConnectFailed) {
                        callbacks_.onConnectFailed(
                            NetworkError::RetryLimitExceeded);
                    }
                    if (callbacks_.onDisconnected) {
                        callbacks_.onDisconnected(
                            DisconnectReason::MaxRetriesExceeded);
                    }
                    return UpdateResult::ConnectionTimedOut;
                }
                stateEnteredAt_ = Clock::now();
                return UpdateResult::ShouldRetryConnect;
            }
            return UpdateResult::NoAction;

        case ConnectionState::Connected:
            if (isHeartbeatTimedOut()) {
                lastDisconnectReason_ = DisconnectReason::Timeout;
                transitionTo(ConnectionState::Disconnected);
                if (callbacks_.onDisconnected) {
                    callbacks_.onDisconnected(DisconnectReason::Timeout);
                }
                return UpdateResult::ConnectionTimedOut;
            }
            return UpdateResult::NoAction;

        case ConnectionState::Disconnecting:
            if (isTimedOut()) {
                lastDisconnectReason_ = DisconnectReason::LocalRequest;
                transitionTo(ConnectionState::Disconnected);
                if (callbacks_.onDisconnected) {
                    callbacks_.onDisconnected(DisconnectReason::LocalRequest);
                }
                return UpdateResult::DisconnectComplete;
            }
            return UpdateResult::NoAction;
    }
    return UpdateResult::NoAction;
}

void ConnectionStateMachine::recordActivity() noexcept {
    lastActivityAt_ = Clock::now();
}

ConnectionStateMachine::Duration ConnectionStateMachine::timeInCurrentState()
    const noexcept {
    return std::chrono::duration_cast<Duration>(Clock::now() - stateEnteredAt_);
}

void ConnectionStateMachine::setCallbacks(
    ConnectionCallbacks callbacks) noexcept {
    callbacks_ = std::move(callbacks);
}

void ConnectionStateMachine::reset() noexcept {
    state_ = ConnectionState::Disconnected;
    userId_ = std::nullopt;
    retryCount_ = 0;
    stateEnteredAt_ = Clock::now();
    lastActivityAt_ = Clock::now();
    lastDisconnectReason_ = std::nullopt;
}

void ConnectionStateMachine::transitionTo(ConnectionState newState) noexcept {
    if (state_ == newState) {
        return;
    }
    ConnectionState oldState = state_;
    state_ = newState;
    stateEnteredAt_ = Clock::now();
    lastActivityAt_ = Clock::now();

    if (newState == ConnectionState::Disconnected) {
        userId_ = std::nullopt;
    }

    if (callbacks_.onStateChange) {
        callbacks_.onStateChange(oldState, newState);
    }
}

bool ConnectionStateMachine::isTimedOut() const noexcept {
    Duration timeout;
    switch (state_) {
        case ConnectionState::Connecting:
            timeout = config_.connectTimeout;
            break;
        case ConnectionState::Disconnecting:
            timeout = config_.disconnectTimeout;
            break;
        default:
            return false;
    }
    return timeInCurrentState() >= timeout;
}

bool ConnectionStateMachine::isHeartbeatTimedOut() const noexcept {
    if (state_ != ConnectionState::Connected) {
        return false;
    }
    auto timeSinceActivity =
        std::chrono::duration_cast<Duration>(Clock::now() - lastActivityAt_);
    return timeSinceActivity >= config_.heartbeatTimeout;
}

}  // namespace rtype::network
