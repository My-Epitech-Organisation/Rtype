/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ConnectionStateMachine - FSM for connection lifecycle management
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "ConnectionEvents.hpp"
#include "ConnectionState.hpp"
#include "core/Error.hpp"

namespace rtype::network {

/**
 * @brief Finite State Machine managing connection lifecycle
 *
 * Handles state transitions, timeouts, and retry logic for network connections.
 * This class is pure logic with no I/O - it must be driven by an external
 * update loop that provides timing information.
 *
 * Thread-safety: NOT thread-safe. Caller must synchronize access.
 */
class ConnectionStateMachine {
   public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::milliseconds;

    struct Config {
        Duration connectTimeout = Duration{2000};
        Duration disconnectTimeout = Duration{1000};
        Duration heartbeatInterval = Duration{1000};
        Duration heartbeatTimeout = Duration{10000};
        int maxConnectRetries = 3;
    };

    /**
     * @brief Result of an update tick
     */
    enum class UpdateResult : std::uint8_t {
        NoAction,
        ShouldSendConnect,
        ShouldSendDisconnect,
        ShouldRetryConnect,
        ConnectionTimedOut,
        DisconnectComplete
    };

    ConnectionStateMachine() noexcept;
    explicit ConnectionStateMachine(const Config& config) noexcept;

    ~ConnectionStateMachine() = default;

    ConnectionStateMachine(const ConnectionStateMachine&) = delete;
    ConnectionStateMachine& operator=(const ConnectionStateMachine&) = delete;
    ConnectionStateMachine(ConnectionStateMachine&&) = default;
    ConnectionStateMachine& operator=(ConnectionStateMachine&&) = default;

    /**
     * @brief Initiate connection attempt (DISCONNECTED → CONNECTING)
     * @return Ok if transition valid, Err(InvalidStateTransition) otherwise
     */
    [[nodiscard]] Result<void> initiateConnect();

    /**
     * @brief Handle S_ACCEPT received (CONNECTING → CONNECTED)
     * @param userId Assigned user ID from server
     * @return Ok if transition valid, Err otherwise
     */
    [[nodiscard]] Result<void> handleAccept(std::uint32_t userId);

    /**
     * @brief Initiate graceful disconnect (CONNECTED/CONNECTING → DISCONNECTING)
     * @return Ok if transition valid, Err otherwise
     */
    [[nodiscard]] Result<void> initiateDisconnect();

    /**
     * @brief Handle DISCONNECT acknowledgement or completion
     * @return Ok if transition valid, Err otherwise
     */
    [[nodiscard]] Result<void> handleDisconnectAck();

    /**
     * @brief Handle remote DISCONNECT packet received
     * @return Ok if processed, Err if invalid state
     */
    [[nodiscard]] Result<void> handleRemoteDisconnect();

    /**
     * @brief Force immediate transition to DISCONNECTED
     * @param reason Reason for forced disconnect
     */
    void forceDisconnect(DisconnectReason reason) noexcept;

    /**
     * @brief Update FSM and check for timeouts
     * @return Action to take based on current state and timing
     */
    [[nodiscard]] UpdateResult update();

    /**
     * @brief Record that data was received (resets heartbeat timer)
     */
    void recordActivity() noexcept;

    [[nodiscard]] ConnectionState state() const noexcept { return state_; }

    [[nodiscard]] bool isConnected() const noexcept {
        return state_ == ConnectionState::Connected;
    }

    [[nodiscard]] bool isDisconnected() const noexcept {
        return state_ == ConnectionState::Disconnected;
    }

    [[nodiscard]] std::optional<std::uint32_t> userId() const noexcept {
        return userId_;
    }

    [[nodiscard]] int retryCount() const noexcept { return retryCount_; }

    [[nodiscard]] Duration timeInCurrentState() const noexcept;

    [[nodiscard]] std::optional<DisconnectReason> lastDisconnectReason()
        const noexcept {
        return lastDisconnectReason_;
    }

    void setCallbacks(ConnectionCallbacks callbacks) noexcept;

    void reset() noexcept;

   private:
    void transitionTo(ConnectionState newState) noexcept;
    [[nodiscard]] bool isTimedOut() const noexcept;
    [[nodiscard]] bool isHeartbeatTimedOut() const noexcept;

    Config config_;
    ConnectionState state_{ConnectionState::Disconnected};
    std::optional<std::uint32_t> userId_;
    int retryCount_{0};
    TimePoint stateEnteredAt_{Clock::now()};
    TimePoint lastActivityAt_{Clock::now()};
    std::optional<DisconnectReason> lastDisconnectReason_;
    ConnectionCallbacks callbacks_;
};

}  // namespace rtype::network
