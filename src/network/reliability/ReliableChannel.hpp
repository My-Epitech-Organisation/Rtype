/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ReliableChannel - Implements Reliable UDP (RUDP) layer per RFC RTGP v1.1.0
** Section 4.3
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/Error.hpp"
#include "core/Types.hpp"
#include "protocol/Header.hpp"

namespace rtype::network {

/**
 * @brief Pending reliable packet awaiting acknowledgement
 *
 * Tracks a packet that was sent with RELIABLE flag and monitors
 * for ACK or timeout (triggering retransmission).
 */
struct PendingPacket {
    std::vector<std::uint8_t> data;
    std::uint16_t seqId;
    std::chrono::steady_clock::time_point sentTime;
    int retryCount;
    bool isAcked;
};

/**
 * @brief Reliable UDP Channel for critical packets
 *
 * Implements selective reliability (RFC RTGP v1.1.0 Section 4.3):
 * - Tracks outgoing RELIABLE packets
 * - Implements ACK piggybacking (don't send dedicated ACKs)
 * - Handles retransmission on timeout
 * - Detects and drops duplicate packets
 * - Manages sequence number wraparound (uint16)
 *
 * Key properties:
 * - Default timeout: 200ms per retransmission
 * - Default max retries: 5 (retransmission attempts, excluding initial send)
 * - Only RELIABLE packets (0x01 flag) tracked
 * - ACKs piggybacked on any outgoing packet with IS_ACK flag (0x02)
 *
 * Thread-safety: NOT thread-safe. Use externally with synchronization.
 *
 * @see RFC RTGP v1.1.0 Section 4.3
 */
class ReliableChannel {
   public:
    using Clock = std::chrono::steady_clock;

    /**
     * @brief Configuration for RUDP behavior
     */
    struct Config {
        std::chrono::milliseconds retransmitTimeout;
        int maxRetries;

        Config() noexcept : retransmitTimeout(200), maxRetries(5) {}

        Config(std::chrono::milliseconds timeout, int maxRetries) noexcept
            : retransmitTimeout(timeout), maxRetries(maxRetries) {}
    };

    /**
     * @brief Construct a reliable channel with config
     * @param config RUDP configuration (or defaults)
     */
    explicit ReliableChannel(const Config& config = Config{}) noexcept;

    ~ReliableChannel() = default;

    ReliableChannel(const ReliableChannel&) = delete;
    ReliableChannel& operator=(const ReliableChannel&) = delete;
    ReliableChannel(ReliableChannel&&) = delete;
    ReliableChannel& operator=(ReliableChannel&&) = delete;

    /**
     * @brief Track an outgoing RELIABLE packet
     *
     * Called when sending a packet with RELIABLE flag set.
     * Stores packet data for potential retransmission.
     *
     * @param seqId Sequence ID from packet header
     * @param data Serialized packet (header + payload)
     * @return Ok on success, Err if seqId already tracked
     */
    [[nodiscard]] Result<void> trackOutgoing(
        std::uint16_t seqId, const std::vector<std::uint8_t>& data);

    /**
     * @brief Record receipt of ACK for a sequence ID
     *
     * Called when receiving a packet with IS_ACK flag.
     * Marks the corresponding packet as acknowledged.
     *
     * @param ackId Sequence ID being acknowledged
     */
    void recordAck(std::uint16_t ackId) noexcept;

    /**
     * @brief Structure for packets that need retransmission
     *
     * Contains metadata and owned data for packets that are scheduled
     * for retransmission due to timeout or lack of acknowledgement.
     */
    struct RetransmitPacket {
        std::uint16_t seqId;
        std::vector<std::uint8_t> data;
        int retryCount;
    };

    /**
     * @brief Get packets ready for retransmission
     *
     * Returns all packets whose timeout has expired and haven't
     * reached max retries. Caller is responsible for re-sending.
     *
     * @return Vector of packets to retransmit (copied data, safe ownership)
     */
    [[nodiscard]] std::vector<RetransmitPacket> getPacketsToRetransmit();

    /**
     * @brief Check if sequence ID is a duplicate
     *
     * Detects packets that were already received successfully.
     * Caller should drop duplicates before processing.
     *
     * @param seqId Sequence ID to check
     * @return true if this seqId was already received
     */
    [[nodiscard]] bool isDuplicate(std::uint16_t seqId) const noexcept;

    /**
     * @brief Record successful receipt of packet sequence
     *
     * Marks seqId as received to detect future duplicates.
     * Should be called BEFORE processing payload.
     *
     * @param seqId Sequence ID of received packet
     */
    void recordReceived(std::uint16_t seqId) noexcept;

    /**
     * @brief Get the latest ACK ID to piggyback
     *
     * Returns the sequence ID of the most recent packet received
     * from the remote peer. Caller should include this in outgoing
     * packet header with IS_ACK flag set.
     *
     * @return Most recent received sequence ID, or 0 if none
     */
    [[nodiscard]] std::uint16_t getLastReceivedSeqId() const noexcept;

    /**
     * @brief Clean up acknowledged packets and expired retries
     *
     * Removes ACKed packets and disconnects if max retries exceeded.
     * Should be called periodically.
     *
     * @return Ok if channel healthy, Err if too many retries
     */
    [[nodiscard]] Result<void> cleanup();

    /**
     * @brief Get count of pending reliable packets
     *
     * Useful for debugging and monitoring.
     *
     * @return Number of unacknowledged reliable packets
     */
    [[nodiscard]] std::size_t getPendingCount() const noexcept;

    /**
     * @brief Get count of received sequence IDs being tracked
     *
     * Useful for debugging duplicate detection.
     *
     * @return Number of sequence IDs in received set
     */
    [[nodiscard]] std::size_t getReceivedCount() const noexcept;

    /**
     * @brief Clear all state (for disconnections)
     *
     * Resets all pending packets and received sequence IDs.
     */
    void clear() noexcept;

   private:
    static constexpr std::uint16_t kReceivedSeqIdWindow = 1000;

    /**
     * @brief Prune old received sequence IDs to prevent unbounded memory growth
     *
     * Removes sequence IDs that are outside the sliding window behind the highest
     * received sequence ID to maintain bounded memory usage.
     */
    void pruneOldReceivedSeqIds() noexcept;

    Config config_;
    std::unordered_map<std::uint16_t, PendingPacket> pendingPackets_;
    std::unordered_set<std::uint16_t> receivedSeqIds_;
    std::uint16_t lastReceivedSeqId_{0};
    bool hasReceivedAny_{false};
};

}  // namespace rtype::network
