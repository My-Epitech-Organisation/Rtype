/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SecurityContext - Anti-replay and security state tracking
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "../core/Error.hpp"
#include "Header.hpp"

namespace rtype::network {

inline constexpr size_t kAntiReplayWindowSize = 1000;

/**
 * @brief Security context for tracking packet validation state
 *
 * Maintains per-connection state for anti-replay protection and sequence
 * tracking. Each endpoint/UserID combination has its own context.
 */
class SecurityContext {
   public:
    /**
     * @brief Information about a tracked connection
     */
    struct ConnectionInfo {
        uint32_t userId;
        uint16_t lastValidSeqId;
        std::set<uint16_t> receivedSeqs;
        std::chrono::steady_clock::time_point lastActivity;
        bool initialized;

        ConnectionInfo()
            : userId(kUnassignedUserId),
              lastValidSeqId(0),
              lastActivity(std::chrono::steady_clock::now()),
              initialized(false) {}
    };

    /**
     * @brief Validate sequence ID for anti-replay protection
     *
     * As per RFC Section 6, packets with stale sequence IDs are discarded.
     * Uses a sliding window to track received packets.
     *
     * @param connectionKey Unique identifier for the connection
     * @param seqId Sequence ID to validate
     * @return Success if valid, InvalidSequence or DuplicatePacket error
     */
    [[nodiscard]] Result<void> validateSequenceId(
        const std::string& connectionKey, uint16_t seqId) noexcept {
        auto& info = connections_[connectionKey];

        if (!info.initialized) {
            info.lastValidSeqId = seqId;
            info.receivedSeqs.insert(seqId);
            info.initialized = true;
            info.lastActivity = std::chrono::steady_clock::now();
            return Result<void>::ok();
        }

        if (info.receivedSeqs.count(seqId) > 0) {
            return Result<void>::err(NetworkError::DuplicatePacket);
        }

        int32_t distance = static_cast<int32_t>(seqId) -
                           static_cast<int32_t>(info.lastValidSeqId);

        if (distance < -32768) {
            distance += 65536;
        } else if (distance > 32768) {
            distance -= 65536;
        }

        if (distance < -static_cast<int32_t>(kAntiReplayWindowSize)) {
            return Result<void>::err(NetworkError::InvalidSequence);
        }

        info.receivedSeqs.insert(seqId);
        if (distance > 0) {
            info.lastValidSeqId = seqId;
        }

        while (info.receivedSeqs.size() > kAntiReplayWindowSize) {
            info.receivedSeqs.erase(info.receivedSeqs.begin());
        }

        info.lastActivity = std::chrono::steady_clock::now();
        return Result<void>::ok();
    }

    /**
     * @brief Associate a UserID with a connection
     *
     * Used by the server to bind a UserID to an IP:Port combination.
     * Prevents UserID spoofing.
     *
     * @param connectionKey Unique identifier for the connection (e.g.,
     * "IP:Port")
     * @param userId The user ID to associate
     */
    void registerConnection(const std::string& connectionKey,
                            uint32_t userId) noexcept {
        auto& info = connections_[connectionKey];
        info.userId = userId;
        info.lastActivity = std::chrono::steady_clock::now();
    }

    /**
     * @brief Validate that the UserID matches the connection
     *
     * Prevents UserID spoofing by verifying the claimed UserID matches
     * the registered UserID for this endpoint.
     *
     * @param connectionKey Unique identifier for the connection
     * @param claimedUserId The UserID from the packet header
     * @return Success if valid, InvalidUserId if mismatch
     */
    [[nodiscard]] Result<void> validateUserIdMapping(
        const std::string& connectionKey, uint32_t claimedUserId) noexcept {
        auto it = connections_.find(connectionKey);
        if (it == connections_.end()) {
            if (claimedUserId == kUnassignedUserId) {
                return Result<void>::ok();
            }
            return Result<void>::err(NetworkError::InvalidUserId);
        }

        const auto& info = it->second;

        if (info.userId == kUnassignedUserId &&
            claimedUserId == kUnassignedUserId) {
            return Result<void>::ok();
        }

        if (info.userId != claimedUserId) {
            return Result<void>::err(NetworkError::InvalidUserId);
        }

        return Result<void>::ok();
    }

    /**
     * @brief Remove a connection from tracking
     */
    void removeConnection(const std::string& connectionKey) noexcept {
        connections_.erase(connectionKey);
    }

    /**
     * @brief Get connection info for a given key
     *
     * @param connectionKey Unique identifier for the connection
     * @return Const reference to connection info if found
     * @throws std::out_of_range if connection not found
     */
    [[nodiscard]] const ConnectionInfo& getConnectionInfo(
        const std::string& connectionKey) const {
        return connections_.at(connectionKey);
    }

    /**
     * @brief Clean up stale connections
     *
     * Removes connections that haven't sent packets in the specified timeout.
     *
     * @param timeoutSeconds Timeout in seconds
     * @return Number of connections removed
     */
    std::size_t cleanupStaleConnections(uint32_t timeoutSeconds) noexcept {
        const auto now = std::chrono::steady_clock::now();
        const auto timeout = std::chrono::seconds(timeoutSeconds);
        std::size_t removed = 0;

        for (auto it = connections_.begin(); it != connections_.end();) {
            if (now - it->second.lastActivity > timeout) {
                it = connections_.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }

        return removed;
    }

    /**
     * @brief Get total number of tracked connections
     */
    [[nodiscard]] std::size_t getConnectionCount() const noexcept {
        return connections_.size();
    }

    /**
     * @brief Clear all connection state
     */
    void clear() noexcept { connections_.clear(); }

   private:
    std::map<std::string, ConnectionInfo> connections_;
};

}  // namespace rtype::network
