/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PacketProcessor - Network packet processing and validation
*/

#ifndef SRC_SERVER_SERVERAPP_PACKETPROCESSOR_PACKETPROCESSOR_HPP_
#define SRC_SERVER_SERVERAPP_PACKETPROCESSOR_PACKETPROCESSOR_HPP_

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <rtype/common.hpp>
#include <rtype/network.hpp>

#include "ServerMetrics.hpp"

namespace rtype::server {

/**
 * @brief Processes and validates incoming network packets
 *
 * Handles:
 * - RTGP packet validation
 * - Sequence ID tracking
 * - UserID spoofing prevention
 * - Metrics tracking for dropped packets
 */
class PacketProcessor {
   public:
    /**
     * @brief Construct a PacketProcessor
     * @param metrics Shared metrics for tracking dropped packets
     * @param verbose Enable verbose logging
     */
    explicit PacketProcessor(std::shared_ptr<ServerMetrics> metrics,
                             bool verbose = false);

    ~PacketProcessor() = default;
    PacketProcessor(const PacketProcessor&) = delete;
    PacketProcessor& operator=(const PacketProcessor&) = delete;
    PacketProcessor(PacketProcessor&&) = default;
    PacketProcessor& operator=(PacketProcessor&&) = default;

    /**
     * @brief Process raw data and extract a valid packet
     * @param endpointKey Unique endpoint identifier string
     * @param rawData Raw packet bytes
     * @return Validated packet if successful, nullopt if validation failed
     */
    [[nodiscard]] std::optional<rtype::network::Packet> processRawData(
        const std::string& endpointKey, std::span<const std::uint8_t> rawData);

    /**
     * @brief Register a connection for UserID validation
     * @param endpointKey Unique endpoint identifier
     * @param userId Assigned user ID
     */
    void registerConnection(const std::string& endpointKey,
                            std::uint32_t userId);

    /**
     * @brief Unregister a connection
     * @param endpointKey Unique endpoint identifier
     */
    void unregisterConnection(const std::string& endpointKey);

    /**
     * @brief Get security context for external use
     */
    [[nodiscard]] rtype::network::SecurityContext&
    getSecurityContext() noexcept {
        return _securityContext;
    }

   private:
    std::shared_ptr<ServerMetrics> _metrics;
    rtype::network::SecurityContext _securityContext;
    bool _verbose;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_PACKETPROCESSOR_PACKETPROCESSOR_HPP_
