/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkServer - Implementation
*/

#include "NetworkServer.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "Logger/Macros.hpp"
#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Validator.hpp"
#include "server/shared/ServerMetrics.hpp"

namespace rtype::server {

NetworkServer::NetworkServer(const Config& config)
    : config_(config),
      compressor_(config.compressionConfig),
      ioContext_(),
      socket_(network::createAsyncSocket(ioContext_.get())),
      receiveBuffer_(
          std::make_shared<network::Buffer>(network::kMaxPacketSize)),
      receiveSender_(std::make_shared<network::Endpoint>()) {}

NetworkServer::~NetworkServer() { stop(); }

bool NetworkServer::start(std::uint16_t portNum) {
    if (running_) {
        return false;
    }

    auto bindResult = socket_->bind(portNum);
    if (!bindResult) {
        return false;
    }

    running_ = true;
    startReceive();

    return true;
}

void NetworkServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    network::DisconnectPayload payload;
    payload.reason =
        static_cast<std::uint8_t>(network::DisconnectReason::RemoteRequest);
    auto serialized = network::Serializer::serialize(payload);

    for (const auto& [key, client] : clients_) {
        sendToClient(client, network::OpCode::DISCONNECT, serialized);
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.clear();
        userIdToKey_.clear();
    }

    if (socket_) {
        socket_->cancel();
        ioContext_.poll();
        socket_->close();
    }

    if (config_.enablePacketStats) {
        printPacketStatistics();
    }

    ioContext_.stop();
}

bool NetworkServer::isRunning() const noexcept { return running_; }

std::uint16_t NetworkServer::port() const noexcept {
    if (!running_ || !socket_) {
        return 0;
    }
    return socket_->localPort();
}

void NetworkServer::spawnEntity(std::uint32_t id, EntityType type,
                                std::uint8_t subType, float x, float y) {
    network::EntitySpawnPayload payload;
    payload.entityId = id;
    payload.type = static_cast<std::uint8_t>(type);
    payload.subType = subType;
    payload.posX = x;
    payload.posY = y;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    constexpr float SCREEN_WIDTH = 1920.0F;
    constexpr float SCREEN_HEIGHT = 1080.0F;
    constexpr float MARGIN = 100.0F;

    if (x >= -MARGIN && x <= SCREEN_WIDTH + MARGIN && y >= -MARGIN &&
        y <= SCREEN_HEIGHT + MARGIN) {
        broadcastToAll(network::OpCode::S_ENTITY_SPAWN, serialized);
    }
}

void NetworkServer::moveEntity(std::uint32_t id, float x, float y, float vx,
                               float vy) {
    network::EntityMovePayload payload;
    payload.entityId = id;
    payload.serverTick = nextServerTick();
    payload.posX = quantize(x, kPosQuantScale);
    payload.posY = quantize(y, kPosQuantScale);
    payload.velX = quantize(vx, kVelQuantScale);
    payload.velY = quantize(vy, kVelQuantScale);

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_ENTITY_MOVE, serialized);
}

void NetworkServer::moveEntitiesBatch(
    const std::vector<std::tuple<std::uint32_t, float, float, float, float>>&
        entities) {
    if (entities.empty()) {
        return;
    }

    auto count = static_cast<std::uint8_t>(
        std::min(entities.size(), network::kMaxEntitiesPerBatch));

    auto tick = nextServerTick();

    network::Buffer payload;
    payload.reserve(sizeof(network::EntityMoveBatchHeader) +
                    count * sizeof(network::EntityMoveBatchEntry));

    network::EntityMoveBatchHeader header{};
    header.count = count;
    header.serverTick = tick;
    auto headerBytes = network::Serializer::serializeForNetwork(header);
    payload.insert(payload.end(), headerBytes.begin(), headerBytes.end());

    for (std::size_t i = 0; i < count; ++i) {
        const auto& [id, x, y, vx, vy] = entities[i];
        network::EntityMoveBatchEntry entry{};
        entry.entityId = id;
        entry.posX = quantize(x, kPosQuantScale);
        entry.posY = quantize(y, kPosQuantScale);
        entry.velX = quantize(vx, kVelQuantScale);
        entry.velY = quantize(vy, kVelQuantScale);
        auto serialized = network::Serializer::serializeForNetwork(entry);
        payload.insert(payload.end(), serialized.begin(), serialized.end());
    }

    broadcastToAll(network::OpCode::S_ENTITY_MOVE_BATCH, payload);
}

void NetworkServer::destroyEntity(std::uint32_t id) {
    network::EntityDestroyPayload payload;
    payload.entityId = id;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_ENTITY_DESTROY, serialized);
}

void NetworkServer::updateEntityHealth(std::uint32_t id, std::int32_t current,
                                       std::int32_t max) {
    LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                  "[NetworkServer] updateEntityHealth: entityId="
                      << id << " current=" << current << " max=" << max);
    network::EntityHealthPayload payload;
    payload.entityId = id;
    payload.current = current;
    payload.max = max;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_ENTITY_HEALTH, serialized);
}

void NetworkServer::broadcastPowerUp(std::uint32_t playerId,
                                     std::uint8_t powerUpType, float duration) {
    network::PowerUpEventPayload payload{};
    payload.playerId = playerId;
    payload.powerUpType = powerUpType;
    payload.duration = duration;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_POWERUP_EVENT, serialized);
}

void NetworkServer::updateGameState(GameState state) {
    network::UpdateStatePayload payload;
    payload.stateId = static_cast<std::uint8_t>(state);

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_UPDATE_STATE, serialized);
}

void NetworkServer::sendGameOver(std::uint32_t finalScore) {
    LOG_INFO_CAT(
        ::rtype::LogCategory::Network,
        "[NetworkServer] Sending GameOver packet with score=" << finalScore);
    network::GameOverPayload payload;
    payload.finalScore = finalScore;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_GAME_OVER, serialized);
    LOG_INFO_CAT(::rtype::LogCategory::Network,
                 "[NetworkServer] GameOver packet broadcasted to all clients");
}

void NetworkServer::broadcastGameStart(float countdownDuration) {
    network::GameStartPayload payload;
    payload.countdownDuration = countdownDuration;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_GAME_START, serialized);

    LOG_INFO("[NetworkServer] Sent S_GAME_START with countdown="
             << countdownDuration << "s to all clients");
}

void NetworkServer::broadcastPlayerReadyState(std::uint32_t userId,
                                              bool isReady) {
    network::PlayerReadyStatePayload payload;
    payload.userId = userId;
    payload.isReady = isReady ? 1 : 0;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_PLAYER_READY_STATE, serialized);

    LOG_INFO("[NetworkServer] Broadcast player "
             << userId
             << " ready state: " << (isReady ? "READY" : "NOT READY"));
}

void NetworkServer::spawnEntityToClient(std::uint32_t userId, std::uint32_t id,
                                        EntityType type, std::uint8_t subType,
                                        float x, float y) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::EntitySpawnPayload payload;
    payload.entityId = id;
    payload.type = static_cast<std::uint8_t>(type);
    payload.subType = subType;
    payload.posX = x;
    payload.posY = y;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_ENTITY_SPAWN, serialized);
}

void NetworkServer::moveEntityToClient(std::uint32_t userId, std::uint32_t id,
                                       float x, float y, float vx, float vy) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::EntityMovePayload payload;
    payload.entityId = id;
    payload.serverTick = nextServerTick();
    payload.posX = quantize(x, kPosQuantScale);
    payload.posY = quantize(y, kPosQuantScale);
    payload.velX = quantize(vx, kVelQuantScale);
    payload.velY = quantize(vy, kVelQuantScale);

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_ENTITY_MOVE, serialized);
}

void NetworkServer::destroyEntityToClient(std::uint32_t userId,
                                          std::uint32_t id) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::EntityDestroyPayload payload;
    payload.entityId = id;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_ENTITY_DESTROY, serialized);
}

void NetworkServer::updateEntityHealthToClient(std::uint32_t userId,
                                               std::uint32_t id,
                                               std::int32_t current,
                                               std::int32_t max) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::EntityHealthPayload payload;
    payload.entityId = id;
    payload.current = current;
    payload.max = max;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_ENTITY_HEALTH, serialized);
}

void NetworkServer::sendPowerUpToClient(std::uint32_t userId,
                                        std::uint32_t playerId,
                                        std::uint8_t powerUpType,
                                        float duration) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::PowerUpEventPayload payload{};
    payload.playerId = playerId;
    payload.powerUpType = powerUpType;
    payload.duration = duration;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_POWERUP_EVENT, serialized);
}

void NetworkServer::updateGameStateToClient(std::uint32_t userId,
                                            GameState state) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::UpdateStatePayload payload;
    payload.stateId = static_cast<std::uint8_t>(state);

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_UPDATE_STATE, serialized);
}

void NetworkServer::correctPosition(std::uint32_t userId, float x, float y) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::UpdatePosPayload payload;
    payload.posX = x;
    payload.posY = y;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    sendToClient(client, network::OpCode::S_UPDATE_POS, serialized);
}

void NetworkServer::sendUserList(std::uint32_t userId,
                                 const std::vector<std::uint32_t>& userIds) {
    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }

    network::Buffer payload;
    std::uint8_t count = static_cast<std::uint8_t>(std::min(
        userIds.size(), static_cast<size_t>(network::kMaxUsersInResponse)));
    payload.push_back(count);

    for (std::size_t i = 0; i < count; ++i) {
        std::uint32_t netId = network::ByteOrderSpec::toNetwork(userIds[i]);
        auto* bytes = reinterpret_cast<std::uint8_t*>(&netId);
        payload.insert(payload.end(), bytes, bytes + sizeof(netId));
    }

    sendToClient(client, network::OpCode::R_GET_USERS, payload);
}

void NetworkServer::onClientConnected(
    std::function<void(std::uint32_t userId)> callback) {
    onClientConnectedCallback_ = std::move(callback);
}

void NetworkServer::onClientDisconnected(
    std::function<void(std::uint32_t userId, network::DisconnectReason reason)>
        callback) {
    onClientDisconnectedCallback_ = std::move(callback);
}

void NetworkServer::onClientInput(
    std::function<void(std::uint32_t userId, std::uint8_t input)> callback) {
    onClientInputCallback_ = std::move(callback);
}

void NetworkServer::onGetUsersRequest(
    std::function<void(std::uint32_t userId)> callback) {
    onGetUsersRequestCallback_ = std::move(callback);
}
void NetworkServer::onClientReady(
    std::function<void(std::uint32_t, bool)> callback) {
    onClientReadyCallback_ = std::move(callback);
}
void NetworkServer::poll() {
    if (!running_) {
        return;
    }

    ioContext_.poll();

    checkTimeouts();

    std::vector<std::uint32_t> usersToRemove;

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& [key, client] : clients_) {
            auto retransmits = client->reliableChannel.getPacketsToRetransmit();
            for (auto& pkt : retransmits) {
                socket_->asyncSendTo(
                    pkt.data, client->endpoint,
                    [](network::Result<std::size_t> result) { (void)result; });
            }

            auto cleanupResult = client->reliableChannel.cleanup();
            if (!cleanupResult) {
                LOG_WARNING_CAT(
                    ::rtype::LogCategory::Network,
                    "[NetworkServer] Reliable channel retry limit for userId="
                        << client->userId << " pending="
                        << client->reliableChannel.getPendingCount());
                usersToRemove.push_back(client->userId);
            }
        }
    }

    for (std::uint32_t userId : usersToRemove) {
        queueCallback([this, userId]() {
            if (onClientDisconnectedCallback_) {
                onClientDisconnectedCallback_(
                    userId, network::DisconnectReason::MaxRetriesExceeded);
            }
        });
        removeClient(userId);
    }

    dispatchCallbacks();
}

std::vector<std::uint32_t> NetworkServer::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    std::vector<std::uint32_t> result;
    result.reserve(userIdToKey_.size());
    for (const auto& [userId, key] : userIdToKey_) {
        result.push_back(userId);
    }
    return result;
}

std::size_t NetworkServer::clientCount() const noexcept {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    return clients_.size();
}

std::optional<network::Endpoint> NetworkServer::getClientEndpoint(
    std::uint32_t userId) const {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto keyIt = userIdToKey_.find(userId);
    if (keyIt == userIdToKey_.end()) {
        return std::nullopt;
    }
    auto clientIt = clients_.find(keyIt->second);
    if (clientIt == clients_.end()) {
        return std::nullopt;
    }
    return clientIt->second->endpoint;
}

bool NetworkServer::disconnectClient(std::uint32_t userId,
                                     network::DisconnectReason reason) {
    std::shared_ptr<ClientConnection> client;
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        auto keyIt = userIdToKey_.find(userId);
        if (keyIt == userIdToKey_.end()) {
            return false;
        }
        auto clientIt = clients_.find(keyIt->second);
        if (clientIt == clients_.end()) {
            return false;
        }
        client = clientIt->second;
    }

    network::DisconnectPayload payload;
    payload.reason = static_cast<std::uint8_t>(reason);
    auto serialized = network::Serializer::serialize(payload);

    auto disconnectPacket = buildPacket(network::OpCode::DISCONNECT, serialized,
                                        network::kServerUserId, 0, 0, false);

    socket_->asyncSendTo(
        disconnectPacket, client->endpoint,
        [](network::Result<std::size_t> result) { (void)result; });

    queueCallback([this, userId, reason]() {
        if (onClientDisconnectedCallback_) {
            onClientDisconnectedCallback_(userId, reason);
        }
    });

    removeClient(userId);
    return true;
}

void NetworkServer::dispatchCallbacks() {
    std::queue<std::function<void()>> toDispatch;

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        std::swap(toDispatch, callbackQueue_);
    }

    while (!toDispatch.empty()) {
        toDispatch.front()();
        toDispatch.pop();
    }
}

void NetworkServer::queueCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackQueue_.push(std::move(callback));
}

void NetworkServer::startReceive() {
    if (receiveInProgress_.load(std::memory_order_acquire) ||
        !socket_->isOpen()) {
        return;
    }

    receiveInProgress_.store(true, std::memory_order_release);
    receiveBuffer_->resize(network::kMaxPacketSize);

    socket_->asyncReceiveFrom(receiveBuffer_, receiveSender_,
                              [this](network::Result<std::size_t> result) {
                                  handleReceive(std::move(result));
                              });
}

void NetworkServer::handleReceive(network::Result<std::size_t> result) {
    receiveInProgress_.store(false, std::memory_order_release);

    if (result && running_) {
        std::size_t bytesReceived = result.value();
        receiveBuffer_->resize(bytesReceived);

        processIncomingPacket(*receiveBuffer_, *receiveSender_);
    }

    if (running_ && socket_->isOpen()) {
        startReceive();
    }
}

void NetworkServer::processIncomingPacket(const network::Buffer& data,
                                          const network::Endpoint& sender) {
    auto sizeResult = network::Validator::validatePacketSize(data.size());
    if (!sizeResult) {
        return;
    }

    if (_metrics) {
        _metrics->packetsReceived.fetch_add(1, std::memory_order_relaxed);
        _metrics->bytesReceived.fetch_add(data.size(),
                                          std::memory_order_relaxed);
    }

    network::Header header;
    std::memcpy(&header, data.data(), network::kHeaderSize);

    if (!header.hasValidMagic()) {
        return;
    }

    header.payloadSize =
        network::ByteOrderSpec::fromNetwork(header.payloadSize);
    header.userId = network::ByteOrderSpec::fromNetwork(header.userId);
    header.seqId = network::ByteOrderSpec::fromNetwork(header.seqId);
    header.ackId = network::ByteOrderSpec::fromNetwork(header.ackId);

    if (!header.hasValidOpCode()) {
        return;
    }

    recordPacketReceived(header.opcode, data.size());

    std::string connKey = makeConnectionKey(sender);

    auto opcode = static_cast<network::OpCode>(header.opcode);

    if (opcode != network::OpCode::C_CONNECT) {
        auto userResult =
            securityContext_.validateUserIdMapping(connKey, header.userId);
        if (!userResult) {
            return;
        }
    }

    if (header.flags & network::Flags::kIsAck) {
        auto client = findClient(sender);
        if (client) {
            LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                          "[NetworkServer] Processing ACK from userId="
                              << header.userId << " ackId=" << header.ackId
                              << " (seqId=" << header.seqId << ")");
            client->reliableChannel.recordAck(header.ackId);
            client->lastActivity = std::chrono::steady_clock::now();
        }
    }

    auto seqResult = securityContext_.validateSequenceId(connKey, header.seqId);
    if (!seqResult) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                      "[NetworkServer] Sequence validation failed for userId="
                          << header.userId << " seqId=" << header.seqId
                          << " (ACK already processed if present)");
        return;
    }

    if (network::isReliable(opcode)) {
        if (auto client = findClient(sender)) {
            client->reliableChannel.recordReceived(header.seqId);
            client->lastActivity = std::chrono::steady_clock::now();

            network::PongPayload pong{};
            auto serialized = network::Serializer::serializeForNetwork(pong);
            sendToClient(client, network::OpCode::PONG, serialized);
        }
    }

    network::Buffer payload;
    if (header.payloadSize > 0 &&
        data.size() >= network::kHeaderSize + header.payloadSize) {
        network::Buffer rawPayload(data.begin() + network::kHeaderSize,
                                   data.end());

        if (header.flags & network::Flags::kCompressed) {
            auto decompressResult = compressor_.decompress(rawPayload);
            if (!decompressResult) {
                return;
            }
            payload = std::move(decompressResult.value());
        } else {
            payload = std::move(rawPayload);
        }
    }

    switch (opcode) {
        case network::OpCode::C_CONNECT:
            handleConnect(header, payload, sender);
            break;

        case network::OpCode::DISCONNECT:
            handleDisconnect(header, sender);
            break;

        case network::OpCode::C_INPUT:
            handleInput(header, payload, sender);
            break;

        case network::OpCode::C_CHAT:
            handleChat(header, payload, sender);
            break;

        case network::OpCode::C_GET_USERS:
            handleGetUsers(header, sender);
            break;

        case network::OpCode::PING:
            handlePing(header, sender);
            break;

        case network::OpCode::C_JOIN_LOBBY:
            handleJoinLobby(header, payload, sender);
            break;

        case network::OpCode::C_READY:
            handleReady(header, payload, sender);
            break;

        case network::OpCode::C_SET_BANDWIDTH_MODE:
            handleBandwidthMode(header, payload, sender);
            break;

        case network::OpCode::ACK:
            // ACK is handled via kIsAck flag above
            break;

        default:
            break;
    }
}

void NetworkServer::handleConnect(const network::Header& header,
                                  const network::Buffer& payload,
                                  const network::Endpoint& sender) {
    (void)payload;

    std::string connKey = makeConnectionKey(sender);

    auto it = clients_.find(connKey);
    if (it != clients_.end()) {
        it->second->reliableChannel.recordReceived(header.seqId);
        it->second->lastActivity = std::chrono::steady_clock::now();

        network::AcceptPayload acceptPayload;
        acceptPayload.newUserId = it->second->userId;

        auto serialized =
            network::Serializer::serializeForNetwork(acceptPayload);

        sendToClient(it->second, network::OpCode::S_ACCEPT, serialized);
        return;
    }

    if (auto bm = banManager_.lock()) {
        rtype::Endpoint ep{sender.address, sender.port};
        if (bm->isEndpointBanned(ep)) {
            network::DisconnectPayload payload;
            payload.reason =
                static_cast<std::uint8_t>(network::DisconnectReason::Banned);
            auto serialized = network::Serializer::serialize(payload);

            auto packet =
                buildPacket(network::OpCode::DISCONNECT, serialized,
                            network::kServerUserId, 0, header.seqId, false);
            socket_->asyncSendTo(
                packet, sender,
                [](network::Result<std::size_t> result) { (void)result; });
            return;
        }
    }

    std::uint32_t newUserId = nextUserId();

    auto client = std::make_shared<ClientConnection>(sender, newUserId,
                                                     config_.reliabilityConfig);

    client->reliableChannel.recordReceived(header.seqId);
    client->lastActivity = std::chrono::steady_clock::now();

    securityContext_.registerConnection(connKey, newUserId);

    network::AcceptPayload acceptPayload;
    acceptPayload.newUserId = newUserId;

    auto serialized = network::Serializer::serializeForNetwork(acceptPayload);

    sendToClient(client, network::OpCode::S_ACCEPT, serialized);

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        userIdToKey_[newUserId] = connKey;
        clients_[connKey] = client;
    }

    queueCallback([this, newUserId]() {
        if (onClientConnectedCallback_) {
            onClientConnectedCallback_(newUserId);
        }
    });
}

void NetworkServer::handleDisconnect(const network::Header& header,
                                     const network::Endpoint& sender) {
    std::string connKey = makeConnectionKey(sender);

    auto it = clients_.find(connKey);
    if (it == clients_.end()) {
        return;
    }

    std::uint32_t userId = it->second->userId;

    network::DisconnectPayload payload;
    payload.reason =
        static_cast<std::uint8_t>(network::DisconnectReason::RemoteRequest);
    auto serialized = network::Serializer::serialize(payload);

    auto ackPacket =
        buildPacket(network::OpCode::DISCONNECT, serialized,
                    network::kServerUserId, 0, header.seqId, false);

    socket_->asyncSendTo(
        ackPacket, sender,
        [](network::Result<std::size_t> result) { (void)result; });

    removeClient(userId);

    LOG_INFO_CAT(
        ::rtype::LogCategory::Network,
        "[NetworkServer] Client requested disconnect userId=" << userId);

    queueCallback([this, userId]() {
        if (onClientDisconnectedCallback_) {
            onClientDisconnectedCallback_(
                userId, network::DisconnectReason::RemoteRequest);
        }
    });
}

void NetworkServer::handleInput(const network::Header& header,
                                const network::Buffer& payload,
                                const network::Endpoint& sender) {
    (void)sender;

    if (payload.size() < sizeof(network::InputPayload)) {
        return;
    }

    try {
        auto deserialized =
            network::Serializer::deserializeFromNetwork<network::InputPayload>(
                payload);

        std::uint32_t userId = header.userId;
        std::uint8_t inputMask = deserialized.inputMask;

        auto client = findClientByUserId(userId);
        if (!client) {
            return;
        }

        if (!config_.expectedLobbyCode.empty() && !client->joined) {
            return;
        }

        client->lastActivity = std::chrono::steady_clock::now();

        queueCallback([this, userId, inputMask]() {
            if (onClientInputCallback_) {
                onClientInputCallback_(userId, inputMask);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkServer::handleGetUsers(const network::Header& header,
                                   const network::Endpoint& sender) {
    (void)sender;

    std::uint32_t userId = header.userId;

    auto client = findClientByUserId(userId);
    if (!client) {
        return;
    }
    if (!config_.expectedLobbyCode.empty() && !client->joined) {
        return;
    }

    queueCallback([this, userId]() {
        if (onGetUsersRequestCallback_) {
            onGetUsersRequestCallback_(userId);
        }
    });
}

void NetworkServer::handlePing(const network::Header& header,
                               const network::Endpoint& sender) {
    auto client = findClient(sender);
    if (!client) {
        return;
    }

    network::PongPayload payload;
    auto serialized = network::Serializer::serialize(payload);

    auto pongPacket =
        buildPacket(network::OpCode::PONG, serialized, network::kServerUserId,
                    client->nextSeqId++, header.seqId, false);

    socket_->asyncSendTo(
        pongPacket, sender,
        [](network::Result<std::size_t> result) { (void)result; });
}

void NetworkServer::handleReady(const network::Header& header,
                                const network::Buffer& payload,
                                const network::Endpoint& sender) {
    (void)header;

    auto client = findClient(sender);
    if (!client) {
        return;
    }
    if (!config_.expectedLobbyCode.empty() && !client->joined) {
        return;
    }

    if (payload.size() < sizeof(network::LobbyReadyPayload)) {
        return;
    }

    network::LobbyReadyPayload readyPayload;
    std::memcpy(&readyPayload, payload.data(), sizeof(readyPayload));

    bool isReady = (readyPayload.isReady != 0);

    LOG_INFO("[NetworkServer] Client userId="
             << client->userId
             << " ready status: " << (isReady ? "READY" : "NOT READY"));

    if (onClientReadyCallback_) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackQueue_.push([this, userId = client->userId, isReady]() {
            onClientReadyCallback_(userId, isReady);
        });
    }
}

void NetworkServer::handleJoinLobby(const network::Header& header,
                                    const network::Buffer& payload,
                                    const network::Endpoint& sender) {
    (void)header;

    if (payload.size() < sizeof(network::JoinLobbyPayload)) {
        return;
    }

    network::JoinLobbyPayload joinPayload;
    std::memcpy(&joinPayload, payload.data(), sizeof(joinPayload));

    std::string code(joinPayload.code.data(), joinPayload.code.size());

    auto client = findClient(sender);

    network::JoinLobbyResponsePayload resp{};

    if (!client) {
        resp.accepted = 0;
        resp.reason = 1;
        auto ser = network::Serializer::serializeForNetwork(resp);
        auto tempClient = std::make_shared<ClientConnection>(
            sender, 0, config_.reliabilityConfig);
        sendToClient(tempClient, network::OpCode::S_JOIN_LOBBY_RESPONSE, ser);
        return;
    }

    if (config_.expectedLobbyCode.empty() ||
        config_.expectedLobbyCode == code) {
        client->joined = true;
        resp.accepted = 1;
        resp.reason = 0;
        LOG_INFO("[NetworkServer] Client userId="
                 << client->userId << " joined lobby successfully");
    } else {
        resp.accepted = 0;
        resp.reason = 1;
        LOG_INFO("[NetworkServer] Client userId="
                 << client->userId << " provided invalid lobby code: " << code);
    }

    auto ser = network::Serializer::serializeForNetwork(resp);
    sendToClient(client, network::OpCode::S_JOIN_LOBBY_RESPONSE, ser);
}

void NetworkServer::handleBandwidthMode(const network::Header& header,
                                        const network::Buffer& payload,
                                        const network::Endpoint& sender) {
    (void)header;

    auto client = findClient(sender);
    if (!client) {
        return;
    }

    if (payload.size() < sizeof(network::BandwidthModePayload)) {
        return;
    }

    network::BandwidthModePayload modePayload;
    std::memcpy(&modePayload, payload.data(), sizeof(modePayload));

    bool lowBandwidth = (modePayload.mode == static_cast<std::uint8_t>(
                                                 network::BandwidthMode::Low));
    client->lowBandwidthMode = lowBandwidth;

    LOG_INFO("[NetworkServer] Client userId="
             << client->userId
             << " bandwidth mode: " << (lowBandwidth ? "LOW" : "NORMAL"));

    std::uint8_t activeCount = 0;
    for (const auto& [key, c] : clients_) {
        if (c->lowBandwidthMode) {
            activeCount++;
        }
    }

    network::BandwidthModeChangedPayload broadcastPayload;
    broadcastPayload.userId = network::ByteOrderSpec::toNetwork(client->userId);
    broadcastPayload.mode = modePayload.mode;
    broadcastPayload.activeCount = activeCount;

    auto serialized =
        network::Serializer::serializeForNetwork(broadcastPayload);
    broadcastToAll(network::OpCode::S_BANDWIDTH_MODE_CHANGED, serialized);

    if (onBandwidthModeChangedCallback_) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackQueue_.push([this, userId = client->userId, lowBandwidth]() {
            onBandwidthModeChangedCallback_(userId, lowBandwidth);
        });
    }
}

bool NetworkServer::isLowBandwidthMode(std::uint32_t userId) const {
    auto keyIt = userIdToKey_.find(userId);
    if (keyIt == userIdToKey_.end()) {
        return false;
    }
    auto clientIt = clients_.find(keyIt->second);
    if (clientIt == clients_.end()) {
        return false;
    }
    return clientIt->second->lowBandwidthMode;
}

void NetworkServer::setClientBandwidthMode(std::uint32_t userId,
                                           bool lowBandwidth) {
    auto client = findClientByUserId(userId);
    if (client) {
        client->lowBandwidthMode = lowBandwidth;
    }
}

void NetworkServer::onBandwidthModeChanged(BandwidthModeCallback callback) {
    onBandwidthModeChangedCallback_ = std::move(callback);
}

std::string NetworkServer::makeConnectionKey(
    const network::Endpoint& ep) const {
    std::ostringstream oss;
    oss << ep.address << ":" << ep.port;
    return oss.str();
}

std::shared_ptr<NetworkServer::ClientConnection> NetworkServer::findClient(
    const network::Endpoint& ep) {
    std::string key = makeConnectionKey(ep);
    auto it = clients_.find(key);
    if (it != clients_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<NetworkServer::ClientConnection>
NetworkServer::findClientByUserId(std::uint32_t userId) {
    auto keyIt = userIdToKey_.find(userId);
    if (keyIt == userIdToKey_.end()) {
        return nullptr;
    }

    auto clientIt = clients_.find(keyIt->second);
    if (clientIt != clients_.end()) {
        return clientIt->second;
    }
    return nullptr;
}

void NetworkServer::removeClient(std::uint32_t userId) {
    std::lock_guard<std::mutex> lock(clientsMutex_);

    auto keyIt = userIdToKey_.find(userId);
    if (keyIt == userIdToKey_.end()) {
        return;
    }

    std::string key = keyIt->second;
    userIdToKey_.erase(keyIt);
    clients_.erase(key);

    freeUserIds_.push_back(userId);
}

void NetworkServer::checkTimeouts() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::uint32_t> timedOutUsers;

    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& [key, client] : clients_) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - client->lastActivity);

            if (elapsed > config_.clientTimeout) {
                LOG_WARNING_CAT(::rtype::LogCategory::Network,
                                "[NetworkServer] Client timeout userId="
                                    << client->userId << " lastActivityMs="
                                    << std::chrono::duration_cast<
                                           std::chrono::milliseconds>(elapsed)
                                           .count());
                timedOutUsers.push_back(client->userId);
            }
        }
    }

    network::DisconnectPayload payload;
    payload.reason =
        static_cast<std::uint8_t>(network::DisconnectReason::Timeout);
    auto serialized = network::Serializer::serialize(payload);

    for (std::uint32_t userId : timedOutUsers) {
        auto client = findClientByUserId(userId);
        if (client) {
            auto disconnectPacket =
                buildPacket(network::OpCode::DISCONNECT, serialized,
                            network::kServerUserId, 0, 0, false);
            socket_->asyncSendTo(
                disconnectPacket, client->endpoint,
                [](network::Result<std::size_t> result) { (void)result; });
        }

        queueCallback([this, userId]() {
            if (onClientDisconnectedCallback_) {
                onClientDisconnectedCallback_(
                    userId, network::DisconnectReason::Timeout);
            }
        });
        removeClient(userId);
    }
}

network::Buffer NetworkServer::buildPacket(network::OpCode opcode,
                                           const network::Buffer& payload,
                                           std::uint32_t userId,
                                           std::uint16_t seqId,
                                           std::uint16_t ackId, bool reliable) {
    network::Buffer finalPayload = payload;
    bool isCompressed = false;

    if (config_.enableCompression &&
        compressor_.shouldCompress(payload.size())) {
        auto compressionResult = compressor_.compress(payload);
        if (compressionResult.wasCompressed) {
            finalPayload = std::move(compressionResult.data);
            isCompressed = true;
        }
    }

    network::Header header;
    header.magic = network::kMagicByte;
    header.opcode = static_cast<std::uint8_t>(opcode);
    header.payloadSize = network::ByteOrderSpec::toNetwork(
        static_cast<std::uint16_t>(finalPayload.size()));
    header.userId = network::ByteOrderSpec::toNetwork(userId);
    header.seqId = network::ByteOrderSpec::toNetwork(seqId);
    header.ackId = network::ByteOrderSpec::toNetwork(ackId);
    header.flags = network::Flags::kIsAck;
    header.reserved = {0, 0, 0};

    if (reliable) {
        header.flags |= network::Flags::kReliable;
    }

    if (isCompressed) {
        header.flags |= network::Flags::kCompressed;
    }

    network::Buffer packet(network::kHeaderSize + finalPayload.size());
    std::memcpy(packet.data(), &header, network::kHeaderSize);
    if (!finalPayload.empty()) {
        std::memcpy(packet.data() + network::kHeaderSize, finalPayload.data(),
                    finalPayload.size());
    }

    return packet;
}

void NetworkServer::sendToClient(
    const std::shared_ptr<ClientConnection>& client, network::OpCode opcode,
    const network::Buffer& payload) {
    if (!client) {
        return;
    }

    bool reliable = network::isReliable(opcode);
    std::uint16_t seqId = client->nextSeqId++;
    std::uint16_t ackId = client->reliableChannel.getLastReceivedSeqId();

    auto packet = buildPacket(opcode, payload, network::kServerUserId, seqId,
                              ackId, reliable);

    if (reliable) {
        (void)client->reliableChannel.trackOutgoing(seqId, packet);
    }

    if (_metrics) {
        _metrics->packetsSent.fetch_add(1, std::memory_order_relaxed);
        _metrics->bytesSent.fetch_add(packet.size(), std::memory_order_relaxed);
    }

    recordPacketSent(static_cast<std::uint8_t>(opcode), packet.size());

    socket_->asyncSendTo(
        packet, client->endpoint,
        [](network::Result<std::size_t> result) { (void)result; });
}

void NetworkServer::broadcastToAll(network::OpCode opcode,
                                   const network::Buffer& payload) {
    for (const auto& [key, client] : clients_) {
        sendToClient(client, opcode, payload);
    }
}

std::uint32_t NetworkServer::nextUserId() {
    std::lock_guard<std::mutex> lock(clientsMutex_);

    if (!freeUserIds_.empty()) {
        std::uint32_t id = freeUserIds_.back();
        freeUserIds_.pop_back();
        return id;
    }

    std::uint32_t id = nextUserIdCounter_++;

    if (nextUserIdCounter_ >= network::kMaxClientUserId) {
        nextUserIdCounter_ = network::kMinClientUserId;
    }

    return id;
}

void NetworkServer::recordPacketSent(std::uint8_t opcode, std::size_t bytes) {
    if (!config_.enablePacketStats) return;
    std::lock_guard<std::mutex> lock(statsMutex_);
    sentPackets_[opcode].count++;
    sentPackets_[opcode].totalBytes += bytes;
}

void NetworkServer::recordPacketReceived(std::uint8_t opcode,
                                         std::size_t bytes) {
    if (!config_.enablePacketStats) return;
    std::lock_guard<std::mutex> lock(statsMutex_);
    receivedPackets_[opcode].count++;
    receivedPackets_[opcode].totalBytes += bytes;
}

void NetworkServer::printPacketStatistics() const {
    std::lock_guard<std::mutex> lock(statsMutex_);

    LOG_INFO("=== PACKET STATISTICS ===");

    LOG_INFO("--- SENT PACKETS ---");
    std::uint64_t totalSentBytes = 0;
    std::uint64_t totalSentCount = 0;
    for (const auto& [opcode, stats] : sentPackets_) {
        totalSentBytes += stats.totalBytes;
        totalSentCount += stats.count;
        LOG_INFO("  OpCode 0x"
                 << std::hex << static_cast<int>(opcode) << std::dec << ": "
                 << stats.count << " packets, " << stats.totalBytes
                 << " bytes, avg " << stats.getAvgSize() << " bytes/pkt");
    }
    LOG_INFO("  TOTAL SENT: " << totalSentCount << " packets, "
                              << totalSentBytes << " bytes ("
                              << (totalSentBytes / 1024.0) << " KB)");

    LOG_INFO("--- RECEIVED PACKETS ---");
    std::uint64_t totalRecvBytes = 0;
    std::uint64_t totalRecvCount = 0;
    for (const auto& [opcode, stats] : receivedPackets_) {
        totalRecvBytes += stats.totalBytes;
        totalRecvCount += stats.count;
        LOG_INFO("  OpCode 0x"
                 << std::hex << static_cast<int>(opcode) << std::dec << ": "
                 << stats.count << " packets, " << stats.totalBytes
                 << " bytes, avg " << stats.getAvgSize() << " bytes/pkt");
    }
    LOG_INFO("  TOTAL RECEIVED: " << totalRecvCount << " packets, "
                                  << totalRecvBytes << " bytes ("
                                  << (totalRecvBytes / 1024.0) << " KB)");

    LOG_INFO("=== END STATISTICS ===");
}

void NetworkServer::onClientChat(
    std::function<void(std::uint32_t, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    onClientChatCallback_ = std::move(callback);
}

void NetworkServer::handleChat(const network::Header& header,
                               const network::Buffer& payload,
                               const network::Endpoint& sender) {
    if (payload.size() < sizeof(network::ChatPayload)) {
        return;
    }

    auto client = findClient(sender);
    if (!client || !client->joined) {
        return;
    }

    try {
        auto msg =
            network::Serializer::deserializeFromNetwork<network::ChatPayload>(
                payload);

        std::string messageText(msg.message, strnlen(msg.message, 256));

        queueCallback([this, userId = client->userId,
                       message = std::move(messageText)]() {
            if (onClientChatCallback_) {
                onClientChatCallback_(userId, message);
            }
        });
    } catch (...) {
        // Invalid payload
    }
}

void NetworkServer::broadcastChat(std::uint32_t senderId,
                                  const std::string& message) {
    if (message.size() >= 256) {
        LOG_WARNING("Broadcast chat message too long, truncated");
    }

    network::ChatPayload payload;
    payload.userId = network::ByteOrderSpec::toNetwork(senderId);
    std::memset(payload.message, 0, 256);
    std::strncpy(payload.message, message.c_str(), sizeof(payload.message) - 1);
    payload.message[sizeof(payload.message) - 1] = '\0';

    auto serialized = network::Serializer::serializeForNetwork(payload);

    broadcastToAll(network::OpCode::S_CHAT, serialized);
}

}  // namespace rtype::server
