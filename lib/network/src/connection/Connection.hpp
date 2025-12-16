/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Connection - High-level connection abstraction
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <queue>
#include <vector>

#include "ConnectionEvents.hpp"
#include "ConnectionState.hpp"
#include "ConnectionStateMachine.hpp"
#include "core/Error.hpp"
#include "core/Types.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "reliability/ReliableChannel.hpp"

namespace rtype::network {

/**
 * @brief High-level connection abstraction for client-side networking
 *
 * Orchestrates the connection lifecycle by combining:
 * - ConnectionStateMachine for state management
 * - ReliableChannel for RUDP functionality
 * - Packet building and parsing
 *
 * This class handles all connection-related protocol details and provides
 * a simple interface for the game code.
 *
 * Thread-safety: NOT thread-safe. All methods must be called from the same
 * thread (typically the network/game thread).
 *
 * @note This class does not own the socket. It generates packets to send
 *       and processes received packets, but actual I/O is handled externally.
 */
class Connection {
   public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::milliseconds;

    struct Config {
        ConnectionStateMachine::Config stateConfig;
        ReliableChannel::Config reliabilityConfig;

        Config() = default;
    };

    /**
     * @brief Packet ready to be sent over the network
     */
    struct OutgoingPacket {
        Buffer data;
        bool isReliable;
    };

    explicit Connection(const Config& config = Config{});

    ~Connection() = default;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    /**
     * @brief Initiate connection to server
     * @return Ok if connection attempt started, Err if already
     * connecting/connected
     */
    [[nodiscard]] Result<void> connect();

    /**
     * @brief Initiate graceful disconnection
     * @return Ok if disconnect started, Err if not connected
     */
    [[nodiscard]] Result<void> disconnect();

    /**
     * @brief Process a received packet
     * @param data Raw packet data including header
     * @param sender Source endpoint (for validation)
     * @return Ok if processed, Err if invalid packet
     */
    [[nodiscard]] Result<void> processPacket(const Buffer& data,
                                             const Endpoint& sender);

    /**
     * @brief Update connection state and handle timeouts
     *
     * Must be called regularly (e.g., each frame or tick).
     * Checks for timeouts, processes retransmissions, and updates state.
     */
    void update();

    /**
     * @brief Get packets that need to be sent
     * @return Queue of packets to transmit
     */
    [[nodiscard]] std::vector<OutgoingPacket> getOutgoingPackets();

    /**
     * @brief Build a game packet for sending (only when connected)
     * @param opcode Operation code
     * @param payload Serialized payload data
     * @return Ok with packet data, Err if not connected or invalid opcode
     */
    [[nodiscard]] Result<OutgoingPacket> buildPacket(OpCode opcode,
                                                     const Buffer& payload);

    /**
     * @brief Record that an ACK was received for a sequence ID
     * @param ackId Acknowledged sequence ID
     */
    void recordAck(std::uint16_t ackId) noexcept;


    [[nodiscard]] ConnectionState state() const noexcept;

    [[nodiscard]] bool isConnected() const noexcept;

    [[nodiscard]] bool isDisconnected() const noexcept;

    [[nodiscard]] std::optional<std::uint32_t> userId() const noexcept;

    [[nodiscard]] std::optional<DisconnectReason> lastDisconnectReason()
        const noexcept;

    [[nodiscard]] const ReliableChannel& reliableChannel() const noexcept {
        return reliableChannel_;
    }

    /**
     * @brief Build an ACK packet for a specific sequence ID
     * @param ackSeqId The specific sequence ID to acknowledge
     * @return ACK packet buffer, or nullopt if not connected
     */
    [[nodiscard]] std::optional<Buffer> buildAckPacket(std::uint16_t ackSeqId);

    void setCallbacks(ConnectionCallbacks callbacks) noexcept;

    void reset() noexcept;

   private:
    [[nodiscard]] Buffer buildConnectPacket();
    [[nodiscard]] Buffer buildDisconnectPacket();
    [[nodiscard]] Buffer buildAckPacketInternal(std::uint32_t userId);
    [[nodiscard]] Buffer buildAckPacketInternal(std::uint32_t userId,
                                                std::uint16_t ackSeqId);
    [[nodiscard]] Buffer buildPingPacket();
    [[nodiscard]] Result<void> handleConnectAccept(const Header& header,
                                                   const Buffer& payload,
                                                   const Endpoint& sender);
    [[nodiscard]] Result<void> handleDisconnect(const Header& header);
    void processReliabilityAck(const Header& header);
    void queuePacket(Buffer data, bool reliable);
    [[nodiscard]] std::uint16_t nextSequenceId() noexcept;
    void recordPacketSent() noexcept;
    [[nodiscard]] bool shouldSendKeepalive() const noexcept;

    static constexpr Duration kKeepaliveInterval{3000};  // 3 seconds

    Config config_;
    ConnectionStateMachine stateMachine_;
    ReliableChannel reliableChannel_;
    std::queue<OutgoingPacket> outgoingQueue_;
    std::uint16_t sequenceId_{0};
    std::optional<Endpoint> serverEndpoint_;
    Clock::time_point lastPacketSentTime_{Clock::now()};
};

}  // namespace rtype::network
