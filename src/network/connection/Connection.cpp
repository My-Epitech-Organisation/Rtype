/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Connection - Implementation
*/

#include "Connection.hpp"

#include <cstring>

#include "protocol/ByteOrderSpec.hpp"

namespace rtype::network {

Connection::Connection(const Config& config)
    : config_(config),
      stateMachine_(config.stateConfig),
      reliableChannel_(config.reliabilityConfig) {}

Result<void> Connection::connect() {
    auto result = stateMachine_.initiateConnect();
    if (!result) {
        return result;
    }

    Buffer packet = buildConnectPacket();
    queuePacket(std::move(packet), true);
    return Ok();
}

Result<void> Connection::disconnect() {
    auto result = stateMachine_.initiateDisconnect();
    if (!result) {
        return result;
    }

    Buffer packet = buildDisconnectPacket();
    queuePacket(std::move(packet), true);
    return Ok();
}

Result<void> Connection::processPacket(const Buffer& data,
                                        const Endpoint& sender) {
    if (data.size() < kHeaderSize) {
        return Err<void>(NetworkError::PacketTooSmall);
    }

    Header header;
    std::memcpy(&header, data.data(), kHeaderSize);

    if (header.magic != kMagicByte) {
        return Err<void>(NetworkError::InvalidMagic);
    }

    header.payloadSize = ByteOrderSpec::fromNetwork(header.payloadSize);
    header.userId = ByteOrderSpec::fromNetwork(header.userId);
    header.seqId = ByteOrderSpec::fromNetwork(header.seqId);
    header.ackId = ByteOrderSpec::fromNetwork(header.ackId);

    stateMachine_.recordActivity();
    processReliabilityAck(header);

    if (reliableChannel_.isDuplicate(header.seqId)) {
        return Err<void>(NetworkError::DuplicatePacket);
    }

    if (header.flags & Flags::kReliable) {
        reliableChannel_.recordReceived(header.seqId);
    }

    Buffer payload;
    if (data.size() > kHeaderSize) {
        payload.assign(data.begin() + kHeaderSize, data.end());
    }

    auto opcode = static_cast<OpCode>(header.opcode);

    switch (opcode) {
        case OpCode::S_ACCEPT:
            return handleConnectAccept(header, payload);

        case OpCode::DISCONNECT:
            return handleDisconnect(header);

        default:
            break;
    }

    return Ok();
}

void Connection::update() {
    auto updateResult = stateMachine_.update();

    switch (updateResult) {
        case ConnectionStateMachine::UpdateResult::ShouldRetryConnect: {
            Buffer packet = buildConnectPacket();
            queuePacket(std::move(packet), true);
            break;
        }

        case ConnectionStateMachine::UpdateResult::ConnectionTimedOut:
        case ConnectionStateMachine::UpdateResult::DisconnectComplete:
            reliableChannel_.clear();
            break;

        default:
            break;
    }

    auto retransmits = reliableChannel_.getPacketsToRetransmit();
    for (auto& pkt : retransmits) {
        OutgoingPacket outgoing;
        outgoing.data = std::move(pkt.data);
        outgoing.isReliable = true;
        outgoingQueue_.push(std::move(outgoing));
    }

    auto cleanupResult = reliableChannel_.cleanup();
    if (!cleanupResult) {
        stateMachine_.forceDisconnect(DisconnectReason::MaxRetriesExceeded);
    }
}

std::vector<Connection::OutgoingPacket> Connection::getOutgoingPackets() {
    std::vector<OutgoingPacket> packets;
    while (!outgoingQueue_.empty()) {
        packets.push_back(std::move(outgoingQueue_.front()));
        outgoingQueue_.pop();
    }
    return packets;
}

Result<Connection::OutgoingPacket> Connection::buildPacket(
    OpCode opcode, const Buffer& payload) {
    if (!canSendData(stateMachine_.state())) {
        return Err<OutgoingPacket>(NetworkError::NotConnected);
    }

    auto uid = stateMachine_.userId();
    if (!uid) {
        return Err<OutgoingPacket>(NetworkError::NotConnected);
    }

    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(opcode);
    header.payloadSize = ByteOrderSpec::toNetwork(
        static_cast<std::uint16_t>(payload.size()));
    header.userId = ByteOrderSpec::toNetwork(*uid);
    header.seqId = ByteOrderSpec::toNetwork(nextSequenceId());
    header.ackId = ByteOrderSpec::toNetwork(
        reliableChannel_.getLastReceivedSeqId());
    header.flags = Flags::kIsAck;
    header.reserved = {0, 0, 0};

    bool reliable = isReliable(opcode);
    if (reliable) {
        header.flags |= Flags::kReliable;
    }

    Buffer packet(kHeaderSize + payload.size());
    std::memcpy(packet.data(), &header, kHeaderSize);
    if (!payload.empty()) {
        std::memcpy(packet.data() + kHeaderSize, payload.data(), payload.size());
    }

    if (reliable) {
        std::uint16_t seqId = ByteOrderSpec::fromNetwork(header.seqId);
        (void)reliableChannel_.trackOutgoing(seqId, packet);
    }

    OutgoingPacket outgoing;
    outgoing.data = std::move(packet);
    outgoing.isReliable = reliable;
    return Ok(std::move(outgoing));
}

void Connection::recordAck(std::uint16_t ackId) noexcept {
    reliableChannel_.recordAck(ackId);
}

ConnectionState Connection::state() const noexcept {
    return stateMachine_.state();
}

bool Connection::isConnected() const noexcept {
    return stateMachine_.isConnected();
}

bool Connection::isDisconnected() const noexcept {
    return stateMachine_.isDisconnected();
}

std::optional<std::uint32_t> Connection::userId() const noexcept {
    return stateMachine_.userId();
}

std::optional<DisconnectReason> Connection::lastDisconnectReason()
    const noexcept {
    return stateMachine_.lastDisconnectReason();
}

void Connection::setCallbacks(ConnectionCallbacks callbacks) noexcept {
    stateMachine_.setCallbacks(std::move(callbacks));
}

void Connection::reset() noexcept {
    stateMachine_.reset();
    reliableChannel_.clear();
    while (!outgoingQueue_.empty()) {
        outgoingQueue_.pop();
    }
    sequenceId_ = 0;
}

Buffer Connection::buildConnectPacket() {
    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(OpCode::C_CONNECT);
    header.payloadSize = 0;
    header.userId = ByteOrderSpec::toNetwork(kUnassignedUserId);
    header.seqId = ByteOrderSpec::toNetwork(nextSequenceId());
    header.ackId = ByteOrderSpec::toNetwork(
        reliableChannel_.getLastReceivedSeqId());
    header.flags = Flags::kReliable | Flags::kIsAck;
    header.reserved = {0, 0, 0};

    Buffer packet(kHeaderSize);
    std::memcpy(packet.data(), &header, kHeaderSize);

    std::uint16_t seqId = ByteOrderSpec::fromNetwork(header.seqId);
    (void)reliableChannel_.trackOutgoing(seqId, packet);

    return packet;
}

Buffer Connection::buildDisconnectPacket() {
    auto uid = stateMachine_.userId().value_or(kUnassignedUserId);

    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(OpCode::DISCONNECT);
    header.payloadSize = 0;
    header.userId = ByteOrderSpec::toNetwork(uid);
    header.seqId = ByteOrderSpec::toNetwork(nextSequenceId());
    header.ackId = ByteOrderSpec::toNetwork(
        reliableChannel_.getLastReceivedSeqId());
    header.flags = Flags::kReliable | Flags::kIsAck;
    header.reserved = {0, 0, 0};

    Buffer packet(kHeaderSize);
    std::memcpy(packet.data(), &header, kHeaderSize);

    std::uint16_t seqId = ByteOrderSpec::fromNetwork(header.seqId);
    (void)reliableChannel_.trackOutgoing(seqId, packet);

    return packet;
}

Result<void> Connection::handleConnectAccept(const Header& header,
                                              const Buffer& payload) {
    if (payload.size() < sizeof(AcceptPayload)) {
        return Err<void>(NetworkError::MalformedPacket);
    }

    AcceptPayload acceptPayload;
    std::memcpy(&acceptPayload, payload.data(), sizeof(AcceptPayload));
    std::uint32_t newUserId =
        ByteOrderSpec::fromNetwork(acceptPayload.newUserId);

    return stateMachine_.handleAccept(newUserId);
}

Result<void> Connection::handleDisconnect(const Header& header) {
    return stateMachine_.handleRemoteDisconnect();
}

void Connection::processReliabilityAck(const Header& header) {
    if (header.flags & Flags::kIsAck) {
        reliableChannel_.recordAck(header.ackId);
    }
}

void Connection::queuePacket(Buffer data, bool reliable) {
    OutgoingPacket pkt;
    pkt.data = std::move(data);
    pkt.isReliable = reliable;
    outgoingQueue_.push(std::move(pkt));
}

std::uint16_t Connection::nextSequenceId() noexcept {
    return sequenceId_++;
}

}  // namespace rtype::network
