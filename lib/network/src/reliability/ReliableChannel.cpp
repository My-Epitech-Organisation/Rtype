/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ReliableChannel - Implementation of RUDP layer per RFC RTGP v1.1.0
** Section 4.3
*/

#include "ReliableChannel.hpp"

#include <vector>

namespace rtype::network {

ReliableChannel::ReliableChannel(const Config& config) noexcept
    : config_(config) {}

Result<void> ReliableChannel::trackOutgoing(
    std::uint16_t seqId, const std::vector<std::uint8_t>& data) {
    if (pendingPackets_.find(seqId) != pendingPackets_.end()) {
        return Err(NetworkError::DuplicatePacket);
    }

    PendingPacket packet{
        .data = data,
        .seqId = seqId,
        .sentTime = Clock::now(),
        .retryCount = 0,
        .isAcked = false,
    };
    pendingPackets_[seqId] = packet;
    return Ok();
}

void ReliableChannel::recordAck(std::uint16_t ackId) noexcept {
    auto it = pendingPackets_.find(ackId);
    if (it != pendingPackets_.end()) {
        it->second.isAcked = true;
    }
}

std::vector<ReliableChannel::RetransmitPacket>
ReliableChannel::getPacketsToRetransmit() {
    std::vector<RetransmitPacket> toRetransmit;
    auto now = Clock::now();

    for (auto& [seqId, packet] : pendingPackets_) {
        if (packet.isAcked) {
            continue;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - packet.sentTime);
        if (elapsed >= config_.retransmitTimeout) {
            if (packet.retryCount < config_.maxRetries) {
                packet.retryCount++;
                toRetransmit.push_back({seqId, packet.data, packet.retryCount});
                packet.sentTime = now;
            }
        }
    }

    return toRetransmit;
}

bool ReliableChannel::isDuplicate(std::uint16_t seqId) const noexcept {
    return receivedSeqIds_.find(seqId) != receivedSeqIds_.end();
}

void ReliableChannel::recordReceived(std::uint16_t seqId) noexcept {
    receivedSeqIds_.insert(seqId);
    auto isNewer = [](std::uint16_t a, std::uint16_t b) {
        return static_cast<int16_t>(a - b) > 0;
    };
    if (!hasReceivedAny_ || isNewer(seqId, lastReceivedSeqId_)) {
        lastReceivedSeqId_ = seqId;
        hasReceivedAny_ = true;
        pruneOldReceivedSeqIds();
    }
}

void ReliableChannel::pruneOldReceivedSeqIds() noexcept {
    if (receivedSeqIds_.size() <= kReceivedSeqIdWindow) {
        return;
    }

    for (auto it = receivedSeqIds_.begin(); it != receivedSeqIds_.end();) {
        std::uint16_t seqId = *it;
        bool isTooOld = static_cast<int16_t>(lastReceivedSeqId_ - seqId) >
                        static_cast<int16_t>(kReceivedSeqIdWindow);
        if (isTooOld) {
            it = receivedSeqIds_.erase(it);
        } else {
            ++it;
        }
    }
}

std::uint16_t ReliableChannel::getLastReceivedSeqId() const noexcept {
    return lastReceivedSeqId_;
}

Result<void> ReliableChannel::cleanup() {
    std::vector<std::uint16_t> toRemove;

    for (auto& [seqId, packet] : pendingPackets_) {
        if (packet.isAcked) {
            toRemove.push_back(seqId);
        } else if (packet.retryCount >= config_.maxRetries) {
            return Err(NetworkError::RetryLimitExceeded);
        }
    }

    for (std::uint16_t seqId : toRemove) {
        pendingPackets_.erase(seqId);
    }

    return Ok();
}

std::size_t ReliableChannel::getPendingCount() const noexcept {
    return pendingPackets_.size();
}

std::size_t ReliableChannel::getReceivedCount() const noexcept {
    return receivedSeqIds_.size();
}

void ReliableChannel::clear() noexcept {
    pendingPackets_.clear();
    receivedSeqIds_.clear();
    lastReceivedSeqId_ = 0;
    hasReceivedAny_ = false;
}

}  // namespace rtype::network
