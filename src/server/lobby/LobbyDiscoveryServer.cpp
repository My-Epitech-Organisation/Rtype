/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** LobbyDiscoveryServer - Implementation
*/

#include "LobbyDiscoveryServer.hpp"

#include <cstring>
#include <format>

#include "LobbyManager.hpp"
#include "Logger/Logger.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "transport/AsioUdpSocket.hpp"

namespace rtype::server {

using rtype::network::Buffer;
using rtype::network::Endpoint;
using rtype::network::Header;
using rtype::network::OpCode;
using rtype::network::Result;
namespace Flags = rtype::network::Flags;
using rtype::network::kHeaderSize;
using rtype::network::kMagicByte;
using rtype::network::kMaxLobbiesInResponse;
using rtype::network::kMaxPacketSize;
using rtype::network::kServerUserId;
using rtype::network::LobbyInfo;
namespace ByteOrderSpec = rtype::network::ByteOrderSpec;

LobbyDiscoveryServer::LobbyDiscoveryServer(std::uint16_t port,
                                           LobbyManager& lobbyManager)
    : port_(port), lobbyManager_(lobbyManager), ioContext_() {}

LobbyDiscoveryServer::~LobbyDiscoveryServer() { stop(); }

bool LobbyDiscoveryServer::start() {
    if (running_) {
        rtype::Logger::instance().warning(
            std::format("Discovery server already running"));
        return false;
    }

    try {
        socket_ = createAsyncSocket(ioContext_);
        receiveBuffer_ = std::make_shared<Buffer>(kMaxPacketSize);
        receiveSender_ = std::make_shared<Endpoint>();

        if (!socket_->bind(port_)) {
            rtype::Logger::instance().error(std::format(
                "Failed to bind discovery server to port {}", port_));
            return false;
        }

        running_ = true;
        rtype::Logger::instance().info(
            std::format("Discovery server started on port {}", port_));

        startReceive();

        return true;
    } catch (const std::exception& e) {
        rtype::Logger::instance().error(
            std::format("Failed to start discovery server: {}", e.what()));
        return false;
    }
}

void LobbyDiscoveryServer::stop() {
    if (!running_) {
        return;
    }

    rtype::Logger::instance().info(std::format("Stopping discovery server..."));
    running_ = false;

    if (socket_) {
        socket_->cancel();
        ioContext_.poll();
        socket_->close();
    }

    socket_.reset();
    receiveBuffer_.reset();
    receiveSender_.reset();

    rtype::Logger::instance().info(std::format("Discovery server stopped"));
}

void LobbyDiscoveryServer::poll() {
    if (!running_) {
        return;
    }

    ioContext_.poll();
}

void LobbyDiscoveryServer::startReceive() {
    if (!running_ || !socket_) {
        return;
    }

    receiveBuffer_->resize(kMaxPacketSize);

    socket_->asyncReceiveFrom(
        receiveBuffer_, receiveSender_, [this](Result<std::size_t> result) {
            if (result && running_) {
                receiveBuffer_->resize(result.value());
                handlePacket(*receiveBuffer_, *receiveSender_);
            }

            if (running_) {
                startReceive();
            }
        });
}

void LobbyDiscoveryServer::handlePacket(const Buffer& data,
                                        const Endpoint& sender) {
    if (data.size() < kHeaderSize) {
        return;
    }

    Header header;
    std::memcpy(&header, data.data(), kHeaderSize);

    if (header.magic != kMagicByte) {
        return;
    }

    OpCode opcode = static_cast<OpCode>(header.opcode);

    if (opcode == OpCode::C_REQUEST_LOBBIES) {
        rtype::Logger::instance().info(std::format(
            "Discovery server received C_REQUEST_LOBBIES from {}:{}",
            sender.address, sender.port));
        handleLobbyListRequest(sender);
    }
}

void LobbyDiscoveryServer::handleLobbyListRequest(const Endpoint& sender) {
    Buffer responsePacket = buildLobbyListPacket();

    rtype::Logger::instance().info(
        std::format("Discovery server sending S_LOBBY_LIST ({} bytes) to {}:{}",
                    responsePacket.size(), sender.address, sender.port));

    auto responseBuffer = std::make_shared<Buffer>(std::move(responsePacket));
    socket_->asyncSendTo(
        *responseBuffer, sender,
        [this, responseBuffer, sender](Result<std::size_t> result) {
            if (result) {
                rtype::Logger::instance().info(
                    std::format("Discovery server sent {} bytes to {}:{}",
                                result.value(), sender.address, sender.port));
            } else {
                rtype::Logger::instance().error(
                    std::format("Discovery server failed to send to {}:{}",
                                sender.address, sender.port));
            }
            if (!result) {
                // Error sending, log if verbose
            }
        });
}

Buffer LobbyDiscoveryServer::buildLobbyListPacket() {
    auto lobbies = lobbyManager_.getActiveLobbyList();

    Header header{};
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(OpCode::S_LOBBY_LIST);
    header.userId = ByteOrderSpec::toNetwork(kServerUserId);
    header.flags = Flags::kReliable;

    Buffer payload;

    std::uint8_t count = static_cast<std::uint8_t>(
        std::min(lobbies.size(), static_cast<size_t>(kMaxLobbiesInResponse)));
    payload.push_back(count);

    for (size_t i = 0; i < count; ++i) {
        const auto& lobby = lobbies[i];

        LobbyInfo info{};

        std::memcpy(info.code.data(), lobby.code.c_str(),
                    std::min(lobby.code.size(), static_cast<size_t>(6)));

        info.port = ByteOrderSpec::toNetwork(lobby.port);

        info.playerCount = static_cast<std::uint8_t>(lobby.playerCount);
        info.maxPlayers = static_cast<std::uint8_t>(lobby.maxPlayers);

        info.isActive = lobby.isActive ? 1 : 0;

        const std::uint8_t* infoBytes =
            reinterpret_cast<const std::uint8_t*>(&info);
        payload.insert(payload.end(), infoBytes, infoBytes + sizeof(LobbyInfo));
    }

    header.payloadSize =
        ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));

    Buffer packet;
    packet.reserve(kHeaderSize + payload.size());

    const std::uint8_t* headerBytes =
        reinterpret_cast<const std::uint8_t*>(&header);
    packet.insert(packet.end(), headerBytes, headerBytes + kHeaderSize);

    packet.insert(packet.end(), payload.begin(), payload.end());

    return packet;
}

}  // namespace rtype::server
