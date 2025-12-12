/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerMetrics - Server performance and activity metrics
*/

#ifndef SRC_SERVER_SHARED_SERVERMETRICS_HPP_
#define SRC_SERVER_SHARED_SERVERMETRICS_HPP_

#include <atomic>
#include <cstdint>

namespace rtype::server {

/**
 * @brief Server metrics for monitoring
 *
 * Thread-safe metrics structure using atomics for lock-free access.
 * All counters can be safely read and updated from multiple threads.
 */
struct ServerMetrics {
    std::atomic<uint64_t> packetsReceived{0};  ///< Total packets received
    std::atomic<uint64_t> packetsSent{0};      ///< Total packets sent
    std::atomic<uint64_t> packetsDropped{
        0};  ///< Packets dropped (validation failed)
    std::atomic<uint64_t> bytesReceived{0};  ///< Total bytes received
    std::atomic<uint64_t> bytesSent{0};      ///< Total bytes sent
    std::atomic<uint64_t> tickOverruns{
        0};  ///< Number of ticks that took too long
    std::atomic<uint64_t> connectionsRejected{
        0};  ///< Connections rejected (server full/rate limit)
    std::atomic<uint64_t> totalConnections{
        0};  ///< Total connections since start
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_SERVERMETRICS_HPP_
