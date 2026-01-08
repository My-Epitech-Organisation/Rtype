/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerMetrics - Server performance and activity metrics
*/

#ifndef SRC_SERVER_SHARED_SERVERMETRICS_HPP_
#define SRC_SERVER_SHARED_SERVERMETRICS_HPP_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <shared_mutex>

namespace rtype::server {

/**
 * @brief Snapshot of metrics at a point in time
 */
struct MetricsSnapshot {
    std::chrono::system_clock::time_point timestamp;
    uint32_t playerCount{0};
    uint64_t packetsReceived{0};
    uint64_t packetsSent{0};
    uint64_t bytesReceived{0};
    uint64_t bytesSent{0};
    double packetLossPercent{0.0};
    uint64_t tickOverruns{0};
};

/**
 * @brief Server metrics for monitoring
 *
 * Thread-safe metrics structure using atomics for lock-free access.
 * All counters can be safely read and updated from multiple threads.
 * Maintains a circular buffer of historical snapshots (max 60 entries).
 */
struct ServerMetrics {
    std::atomic<uint64_t> packetsReceived{0};
    std::atomic<uint64_t> packetsSent{0};
    std::atomic<uint64_t> packetsDropped{0};
    std::atomic<uint64_t> bytesReceived{0};
    std::atomic<uint64_t> bytesSent{0};
    std::atomic<uint64_t> tickOverruns{0};
    std::atomic<uint64_t> connectionsRejected{0};
    std::atomic<uint64_t> totalConnections{0};
    std::chrono::steady_clock::time_point serverStartTime{
        std::chrono::steady_clock::now()};

    mutable std::shared_mutex historyMutex;
    std::deque<MetricsSnapshot> history;
    static constexpr size_t MAX_HISTORY_SIZE = 60;  // in seconds

    /**
     * @brief Add a snapshot to the history
     * @param snapshot Metrics snapshot to add
     */
    void addSnapshot(const MetricsSnapshot& snapshot) {
        std::unique_lock lock(historyMutex);
        if (history.size() >= MAX_HISTORY_SIZE) {
            history.pop_front();
        }
        history.push_back(snapshot);
    }

    /**
     * @brief Get all historical snapshots
     * @return Copy of history vector
     */
    [[nodiscard]] std::deque<MetricsSnapshot> getHistory() const {
        std::shared_lock lock(historyMutex);
        return history;
    }

    /**
     * @brief Clear all historical data
     */
    void clearHistory() {
        std::unique_lock lock(historyMutex);
        history.clear();
    }

    /**
     * @brief Get uptime in seconds
     * @return Uptime duration
     */
    [[nodiscard]] uint64_t getUptimeSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now -
                                                                serverStartTime)
            .count();
    }
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_SERVERMETRICS_HPP_
