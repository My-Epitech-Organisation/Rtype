/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PacketProcessor - Implementation
*/

#include "PacketProcessor.hpp"

namespace rtype::server {

PacketProcessor::PacketProcessor(std::shared_ptr<ServerMetrics> metrics,
                                 bool verbose)
    : _metrics(std::move(metrics)), _verbose(verbose) {}

std::optional<rtype::network::Packet> PacketProcessor::processRawData(
    const std::string& endpointKey, std::span<const std::uint8_t> rawData) {
    try {
        auto validationResult =
            rtype::network::Serializer::validateAndExtractPacket(rawData,
                                                                 false);

        if (validationResult.isErr()) {
            LOG_DEBUG("[PacketProcessor] Dropped packet from "
                      << endpointKey << " (validation error: "
                      << rtype::network::toString(validationResult.error())
                      << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        auto [header, payload] = validationResult.value();

        auto seqResult =
            _securityContext.validateSequenceId(endpointKey, header.seqId);
        if (seqResult.isErr()) {
            LOG_DEBUG("[PacketProcessor] Dropped packet from "
                      << endpointKey << " (invalid sequence: "
                      << rtype::network::toString(seqResult.error())
                      << ", SeqID=" << header.seqId << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        auto userIdResult =
            _securityContext.validateUserIdMapping(endpointKey, header.userId);
        if (userIdResult.isErr()) {
            LOG_WARNING("[PacketProcessor] Dropped packet from "
                        << endpointKey << " (UserID spoofing: claimed="
                        << header.userId << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        rtype::network::Packet packet(
            static_cast<rtype::network::PacketType>(header.opcode));
        if (header.payloadSize > 0) {
            std::vector<uint8_t> payloadData(payload.begin(), payload.end());
            packet.setData(payloadData);
        }

        if (_verbose) {
            LOG_DEBUG(
                "[PacketProcessor] Accepted packet from "
                << endpointKey << " (OpCode=" << static_cast<int>(header.opcode)
                << ", SeqID=" << header.seqId << ", UserID=" << header.userId
                << ", Payload=" << header.payloadSize << " bytes)");
        }

        return packet;
    } catch (const std::exception& e) {
        LOG_ERROR("[PacketProcessor] Exception processing packet from "
                  << endpointKey << ": " << e.what());
        _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
        return std::nullopt;
    } catch (...) {
        LOG_ERROR("[PacketProcessor] Unknown exception processing packet from "
                  << endpointKey);
        _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
        return std::nullopt;
    }
}

void PacketProcessor::registerConnection(const std::string& endpointKey,
                                         std::uint32_t userId) {
    _securityContext.registerConnection(endpointKey, userId);
    LOG_DEBUG("[PacketProcessor] Registered UserID "
              << userId << " for endpoint " << endpointKey);
}

void PacketProcessor::unregisterConnection(const std::string& endpointKey) {
    _securityContext.removeConnection(endpointKey);
    LOG_DEBUG("[PacketProcessor] Unregistered endpoint " << endpointKey);
}

}  // namespace rtype::server
