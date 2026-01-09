/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkClient - Implementation
*/

#include "NetworkClient.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <queue>
#include <span>
#include <string>
#include <thread>
#include <utility>

#include "Logger/Macros.hpp"
#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"

namespace rtype::client {

NetworkClient::NetworkClient(const Config& config)
    : config_(config),
      ioContext_(),
      socket_(network::createAsyncSocket(ioContext_.get())),
      connection_(config.connectionConfig),
      receiveBuffer_(
          std::make_shared<network::Buffer>(network::kMaxPacketSize)),
      receiveSender_(std::make_shared<network::Endpoint>()) {
    network::ConnectionCallbacks connCallbacks;

    connCallbacks.onConnected = [this](std::uint32_t userId) {
        queueCallback([this, userId]() {
            for (const auto& callback : onConnectedCallbacks_) {
                if (callback) {
                    callback(userId);
                }
            }
        });
    };

    connCallbacks.onDisconnected = [this](network::DisconnectReason reason) {
        queueCallback([this, reason]() {
            connection_.reset();
            serverEndpoint_.reset();
            if (socket_) {
                socket_->cancel();
                ioContext_.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                socket_->close();
                socket_ = network::createAsyncSocket(ioContext_.get());
            }

            for (const auto& cb : onDisconnectedCallbacks_) {
                if (cb) cb(reason);
            }
        });
    };

    connCallbacks.onConnectFailed = [this](network::NetworkError error) {
        (void)error;
        queueCallback([this]() {
            connection_.reset();
            serverEndpoint_.reset();
            if (socket_) {
                socket_->cancel();
                ioContext_.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                socket_->close();
                socket_ = network::createAsyncSocket(ioContext_.get());
            }

            for (const auto& cb : onDisconnectedCallbacks_) {
                if (cb) cb(DisconnectReason::ProtocolError);
            }
        });
    };

    connection_.setCallbacks(connCallbacks);

    networkThreadRunning_.store(true, std::memory_order_release);
    networkThread_ = std::thread([this]() { networkThreadLoop(); });
}

// Testable constructor: allows injecting a mock socket and optionally disabling
// the background network thread to keep tests deterministic.
NetworkClient::NetworkClient(const Config& config,
                             std::unique_ptr<network::IAsyncSocket> socket,
                             bool startNetworkThread)
    : config_(config),
      ioContext_(),
      socket_(std::move(socket)),
      connection_(config.connectionConfig),
      receiveBuffer_(
          std::make_shared<network::Buffer>(network::kMaxPacketSize)),
      receiveSender_(std::make_shared<network::Endpoint>()) {
    network::ConnectionCallbacks connCallbacks;

    connCallbacks.onConnected = [this](std::uint32_t userId) {
        queueCallback([this, userId]() {
            for (const auto& callback : onConnectedCallbacks_) {
                if (callback) {
                    callback(userId);
                }
            }
        });
    };

    connCallbacks.onDisconnected = [this](network::DisconnectReason reason) {
        queueCallback([this, reason]() {
            connection_.reset();
            serverEndpoint_.reset();
            if (socket_) {
                socket_->cancel();
                ioContext_.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                socket_->close();
                socket_ = network::createAsyncSocket(ioContext_.get());
            }

            for (const auto& cb : onDisconnectedCallbacks_) {
                if (cb) cb(reason);
            }
        });
    };

    connCallbacks.onConnectFailed = [this](network::NetworkError error) {
        (void)error;
        queueCallback([this]() {
            connection_.reset();
            serverEndpoint_.reset();
            if (socket_) {
                socket_->cancel();
                ioContext_.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                socket_->close();
                socket_ = network::createAsyncSocket(ioContext_.get());
            }

            for (const auto& cb : onDisconnectedCallbacks_) {
                if (cb) cb(DisconnectReason::ProtocolError);
            }
        });
    };

    connection_.setCallbacks(connCallbacks);

    if (startNetworkThread) {
        networkThreadRunning_.store(true, std::memory_order_release);
        networkThread_ = std::thread([this]() { networkThreadLoop(); });
    } else {
        networkThreadRunning_.store(false, std::memory_order_release);
    }
}

NetworkClient::~NetworkClient() {
    if (isConnected()) {
        disconnect();
    }

    if (socket_) {
        socket_->cancel();
        ioContext_.poll();
    }

    ioContext_.stop();
    networkThreadRunning_.store(false, std::memory_order_release);
    if (networkThread_.joinable()) {
        networkThread_.join();
    }

    if (socket_) {
        socket_->close();
    }
}

bool NetworkClient::connect(const std::string& host, std::uint16_t port) {
    if (!connection_.isDisconnected()) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Cannot connect: not disconnected");
        return false;
    }

    connection_.reset();

    if (socket_) {
        socket_->cancel();
        socket_->close();
        socket_ = network::createAsyncSocket(ioContext_.get());
    }

    auto bindResult = socket_->bind(0);
    if (!bindResult) {
        LOG_ERROR_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Failed to bind socket");
        socket_->close();
        socket_ = network::createAsyncSocket(ioContext_.get());
        return false;
    }

    serverEndpoint_ = network::Endpoint{host, port};

    startReceive();

    auto result = connection_.connect();
    if (!result) {
        LOG_ERROR_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Failed to initiate connection");
        socket_->cancel();
        socket_->close();
        socket_ = network::createAsyncSocket(ioContext_.get());
        serverEndpoint_.reset();
        connection_.reset();
        return false;
    }

    flushOutgoing();

    return true;
}

void NetworkClient::disconnect() {
    if (connection_.isDisconnected()) {
        return;
    }

    auto result = connection_.disconnect();
    if (result) {
        flushOutgoing();
    }

    connection_.reset();
    serverEndpoint_.reset();

    if (socket_) {
        socket_->cancel();
        ioContext_.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        socket_->close();
        socket_ = network::createAsyncSocket(ioContext_.get());
    }
}

bool NetworkClient::isConnected() const noexcept {
    return connection_.isConnected();
}

std::optional<std::uint32_t> NetworkClient::userId() const noexcept {
    return connection_.userId();
}

std::uint32_t NetworkClient::latencyMs() const noexcept {
    return connection_.latencyMs();
}

bool NetworkClient::sendInput(std::uint8_t inputMask) {
    if (!isConnected() || !serverEndpoint_.has_value() || !socket_->isOpen()) {
        return false;
    }

    network::InputPayload payload;
    payload.inputMask = inputMask;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    auto result = connection_.buildPacket(network::OpCode::C_INPUT, serialized);
    if (!result) {
        return false;
    }

    socket_->asyncSendTo(
        result.value().data, *serverEndpoint_,
        [](network::Result<std::size_t> sendResult) { (void)sendResult; });

    return true;
}

bool NetworkClient::ping() {
    if (!isConnected() || !serverEndpoint_.has_value() || !socket_->isOpen()) {
        return false;
    }

    auto result = connection_.buildPacket(network::OpCode::PING, {});
    if (!result) {
        return false;
    }

    socket_->asyncSendTo(
        result.value().data, *serverEndpoint_,
        [](network::Result<std::size_t> sendResult) { (void)sendResult; });

    return true;
}

bool NetworkClient::sendReady(bool isReady) {
    if (!isConnected() || !serverEndpoint_.has_value() || !socket_->isOpen()) {
        return false;
    }

    network::LobbyReadyPayload payload;
    payload.isReady = isReady ? 1 : 0;

    auto serialized = network::Serializer::serializeForNetwork(payload);

    auto result = connection_.buildPacket(network::OpCode::C_READY, serialized);
    if (!result) {
        return false;
    }

    socket_->asyncSendTo(
        result.value().data, *serverEndpoint_,
        [](network::Result<std::size_t> sendResult) { (void)sendResult; });

    return true;
}

bool NetworkClient::requestLobbyList(const std::string& discoveryIp,
                                     std::uint16_t discoveryPort) {
    if (!socket_->isOpen()) {
        if (!socket_->bind(0)) {
            LOG_ERROR_CAT(
                rtype::LogCategory::Network,
                "[NetworkClient] Failed to bind socket for lobby discovery");
            return false;
        }
        LOG_INFO_CAT(rtype::LogCategory::Network,
                     "[NetworkClient] Socket bound for lobby discovery");
    }

    if (!receiveInProgress_.load(std::memory_order_acquire)) {
        LOG_INFO_CAT(
            rtype::LogCategory::Network,
            "[NetworkClient] Starting receive loop for lobby discovery");
        startReceive();
    }

    network::Endpoint discoveryEndpoint{discoveryIp, discoveryPort};

    network::Header header =
        network::Header::create(network::OpCode::C_REQUEST_LOBBIES, 0, 0, 0);

    network::Buffer packet(network::kHeaderSize);
    std::memcpy(packet.data(), &header, network::kHeaderSize);

    socket_->asyncSendTo(
        packet, discoveryEndpoint,
        [discoveryEndpoint](network::Result<std::size_t> sendResult) {
            if (sendResult) {
                LOG_INFO_CAT(rtype::LogCategory::Network,
                             "[NetworkClient] Sent C_REQUEST_LOBBIES to "
                                 << discoveryEndpoint.address << ":"
                                 << discoveryEndpoint.port);
            } else {
                LOG_ERROR_CAT(
                    rtype::LogCategory::Network,
                    "[NetworkClient] Failed to send C_REQUEST_LOBBIES");
            }
        });

    LOG_DEBUG("[NetworkClient] Sent C_REQUEST_LOBBIES to discovery server");
    return true;
}

void NetworkClient::onConnected(
    std::function<void(std::uint32_t myUserId)> callback) {
    onConnectedCallbacks_.push_back(std::move(callback));
}

NetworkClient::CallbackId NetworkClient::addConnectedCallback(
    std::function<void(std::uint32_t myUserId)> callback) {
    onConnectedCallbacks_.push_back(std::move(callback));
    return onConnectedCallbacks_.size() - 1;
}

void NetworkClient::removeConnectedCallback(CallbackId id) {
    if (id < onConnectedCallbacks_.size()) {
        onConnectedCallbacks_[id] = nullptr;
    }
}

void NetworkClient::onDisconnected(
    std::function<void(DisconnectReason)> callback) {
    onDisconnectedCallbacks_.push_back(std::move(callback));
}

NetworkClient::CallbackId NetworkClient::addDisconnectedCallback(
    std::function<void(DisconnectReason)> callback) {
    onDisconnectedCallbacks_.push_back(std::move(callback));
    return onDisconnectedCallbacks_.size() - 1;
}

void NetworkClient::removeDisconnectedCallback(CallbackId id) {
    if (id < onDisconnectedCallbacks_.size()) {
        onDisconnectedCallbacks_[id] = nullptr;
    }
}

void NetworkClient::onEntitySpawn(
    std::function<void(EntitySpawnEvent)> callback) {
    onEntitySpawnCallback_ = std::move(callback);
}

void NetworkClient::onEntityMove(
    std::function<void(EntityMoveEvent)> callback) {
    onEntityMoveCallback_ = std::move(callback);
}

void NetworkClient::onEntityMoveBatch(
    std::function<void(EntityMoveBatchEvent)> callback) {
    onEntityMoveBatchCallback_ = std::move(callback);
}

void NetworkClient::onEntityDestroy(
    std::function<void(std::uint32_t entityId)> callback) {
    onEntityDestroyCallbacks_.push_back(std::move(callback));
}

void NetworkClient::onEntityHealth(
    std::function<void(EntityHealthEvent)> callback) {
    onEntityHealthCallback_ = std::move(callback);
}

void NetworkClient::onPowerUpEvent(std::function<void(PowerUpEvent)> callback) {
    onPowerUpCallback_ = std::move(callback);
}

bool NetworkClient::sendJoinLobby(const std::string& code) {
    if (!isConnected() || !serverEndpoint_.has_value() || !socket_->isOpen()) {
        return false;
    }

    network::JoinLobbyPayload payload{};
    for (size_t i = 0; i < 6 && i < code.size(); ++i) {
        payload.code[i] = code[i];
    }

    auto serialized = network::Serializer::serializeForNetwork(payload);

    auto result =
        connection_.buildPacket(network::OpCode::C_JOIN_LOBBY, serialized);
    if (!result) {
        return false;
    }

    socket_->asyncSendTo(
        result.value().data, *serverEndpoint_,
        [](network::Result<std::size_t> sendResult) { (void)sendResult; });

    return true;
}

void NetworkClient::onJoinLobbyResponse(
    std::function<void(bool, uint8_t)> callback) {
    onJoinLobbyResponseCallback_ = std::move(callback);
}

void NetworkClient::onPositionCorrection(
    std::function<void(float x, float y)> callback) {
    onPositionCorrectionCallback_ = std::move(callback);
}

void NetworkClient::onGameStateChange(
    std::function<void(GameStateEvent)> callback) {
    onGameStateChangeCallback_ = std::move(callback);
}

void NetworkClient::onGameOver(std::function<void(GameOverEvent)> callback) {
    onGameOverCallback_ = std::move(callback);
}

void NetworkClient::onGameStart(std::function<void(float)> callback) {
    onGameStartCallback_ = std::move(callback);
}

void NetworkClient::onPlayerReadyStateChanged(
    std::function<void(std::uint32_t userId, bool isReady)> callback) {
    onPlayerReadyStateChangedCallback_ = std::move(callback);
}

void NetworkClient::onLobbyListReceived(
    std::function<void(LobbyListEvent)> callback) {
    onLobbyListReceivedCallback_ = std::move(callback);
}

void NetworkClient::poll() {
    connection_.update();

    flushOutgoing();

    dispatchCallbacks();
}

void NetworkClient::networkThreadLoop() {
    while (networkThreadRunning_.load(std::memory_order_acquire)) {
        ioContext_.poll();

        std::this_thread::sleep_for(kNetworkThreadSleepDuration);
    }
}

void NetworkClient::dispatchCallbacks() {
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

void NetworkClient::test_dispatchCallbacks() { dispatchCallbacks(); }

void NetworkClient::test_processIncomingPacket(
    const network::Buffer& data, const network::Endpoint& sender) {
    processIncomingPacket(data, sender);
}

void NetworkClient::test_queueCallback(std::function<void()> callback) {
    queueCallback(std::move(callback));
}

void NetworkClient::test_startReceive() { startReceive(); }

void NetworkClient::test_handlePong(const network::Header& header,
                                    const network::Buffer& payload) {
    handlePong(header, payload);
}

void NetworkClient::queueCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackQueue_.push(std::move(callback));
}

void NetworkClient::startReceive() {
    if (receiveInProgress_.load(std::memory_order_acquire) || !socket_ ||
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

void NetworkClient::handleReceive(network::Result<std::size_t> result) {
    receiveInProgress_.store(false, std::memory_order_release);

    if (result) {
        std::size_t bytesReceived = result.value();
        receiveBuffer_->resize(bytesReceived);

        processIncomingPacket(*receiveBuffer_, *receiveSender_);
    }
    if (socket_ && socket_->isOpen()) {
        startReceive();
    }
}

void NetworkClient::processIncomingPacket(const network::Buffer& data,
                                          const network::Endpoint& sender) {
    if (data.size() < network::kHeaderSize) {
        return;
    }

    auto result = connection_.processPacket(data, sender);

    network::Header header;
    std::memcpy(&header, data.data(), network::kHeaderSize);

    header.payloadSize =
        network::ByteOrderSpec::fromNetwork(header.payloadSize);
    header.userId = network::ByteOrderSpec::fromNetwork(header.userId);
    header.seqId = network::ByteOrderSpec::fromNetwork(header.seqId);
    header.ackId = network::ByteOrderSpec::fromNetwork(header.ackId);

    if (header.flags & network::Flags::kIsAck) {
        connection_.recordAck(header.ackId);
    }

    network::Buffer payload;
    if (header.payloadSize > 0 &&
        data.size() >= network::kHeaderSize + header.payloadSize) {
        network::Buffer rawPayload(data.begin() + network::kHeaderSize,
                                   data.end());

        if (header.flags & network::Flags::kCompressed) {
            auto decompressResult = compressor_.decompress(rawPayload);
            if (!decompressResult) {
                LOG_WARNING_CAT(rtype::LogCategory::Network,
                                "[NetworkClient] Failed to decompress payload");
                return;
            }
            payload = std::move(decompressResult.value());
        } else {
            payload = std::move(rawPayload);
        }
    }

    auto opcode = static_cast<network::OpCode>(header.opcode);

    if (network::isReliable(opcode)) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Received reliable packet: opcode="
                          << static_cast<int>(opcode) << " seqId="
                          << header.seqId << " flags=0x" << std::hex
                          << static_cast<int>(header.flags) << std::dec);
        sendAck(header.seqId);
    }

    switch (opcode) {
        case network::OpCode::S_ENTITY_SPAWN:
            handleEntitySpawn(header, payload);
            break;

        case network::OpCode::S_ENTITY_MOVE:
            handleEntityMove(header, payload);
            break;

        case network::OpCode::S_ENTITY_MOVE_BATCH:
            handleEntityMoveBatch(header, payload);
            break;

        case network::OpCode::S_ENTITY_DESTROY:
            handleEntityDestroy(header, payload);
            break;

        case network::OpCode::S_ENTITY_HEALTH:
            handleEntityHealth(header, payload);
            break;

        case network::OpCode::S_POWERUP_EVENT:
            handlePowerUpEvent(header, payload);
            break;

        case network::OpCode::S_UPDATE_POS:
            handleUpdatePos(header, payload);
            break;

        case network::OpCode::S_UPDATE_STATE:
            handleUpdateState(header, payload);
            break;

        case network::OpCode::S_GAME_OVER:
            handleGameOver(header, payload);
            break;

        case network::OpCode::S_GAME_START:
            handleGameStart(header, payload);
            break;

        case network::OpCode::S_PLAYER_READY_STATE:
            handlePlayerReadyState(header, payload);
            break;

        case network::OpCode::S_JOIN_LOBBY_RESPONSE:
            handleJoinLobbyResponse(header, payload);
            break;

        case network::OpCode::S_LOBBY_LIST:
            handleLobbyList(header, payload);
            break;

        case network::OpCode::PONG:
            handlePong(header, payload);
            break;

        case network::OpCode::DISCONNECT: {
            LOG_DEBUG("[NetworkClient] Received DISCONNECT from server");
            network::DisconnectReason reason =
                network::DisconnectReason::RemoteRequest;
            if (payload.size() >= sizeof(network::DisconnectPayload)) {
                try {
                    auto deserialized =
                        network::Serializer::deserializeFromNetwork<
                            network::DisconnectPayload>(payload);
                    reason = static_cast<network::DisconnectReason>(
                        deserialized.reason);
                } catch (...) {
                    // If payload is invalid, default to RemoteRequest
                }
            }

            queueCallback([this, reason]() {
                connection_.reset();
                serverEndpoint_.reset();
                if (socket_) {
                    socket_->cancel();
                    socket_->close();
                    socket_ = network::createAsyncSocket(ioContext_.get());
                }

                for (const auto& cb : onDisconnectedCallbacks_) {
                    if (cb) cb(reason);
                }
            });
            break;
        }

        default:
            break;
    }
}

void NetworkClient::handleEntitySpawn(const network::Header& header,
                                      const network::Buffer& payload) {
    (void)header;

    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[NetworkClient] handleEntitySpawn called, payload size=" +
                      std::to_string(payload.size()));

    if (payload.size() < sizeof(network::EntitySpawnPayload)) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[NetworkClient] Payload too small for EntitySpawnPayload");
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::EntitySpawnPayload>(payload);

        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Deserialized spawn: entityId=" +
                          std::to_string(deserialized.entityId) + " type=" +
                          std::to_string(static_cast<int>(deserialized.type)) +
                          " pos=(" + std::to_string(deserialized.posX) + ", " +
                          std::to_string(deserialized.posY) + ")");

        EntitySpawnEvent event;
        event.entityId = deserialized.entityId;
        event.type = deserialized.getType();
        event.subType = deserialized.subType;
        event.x = deserialized.posX;
        event.y = deserialized.posY;

        if (event.type == network::EntityType::Player) {
            event.userId = deserialized.entityId;
        }

        queueCallback([this, event]() {
            if (onEntitySpawnCallback_) {
                onEntitySpawnCallback_(event);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleEntityMove(const network::Header& header,
                                     const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::EntityMovePayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::EntityMovePayload>(payload);

        EntityMoveEvent event;
        event.entityId = deserialized.entityId;
        event.x = deserialized.posX;
        event.y = deserialized.posY;
        event.vx = deserialized.velX;
        event.vy = deserialized.velY;

        queueCallback([this, event]() {
            if (onEntityMoveCallback_) {
                onEntityMoveCallback_(event);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleEntityMoveBatch(const network::Header& header,
                                          const network::Buffer& payload) {
    (void)header;

    if (payload.size() < 1) {
        return;
    }

    std::uint8_t count = payload[0];
    if (count == 0 || count > network::kMaxEntitiesPerBatch) {
        return;
    }

    constexpr std::size_t entrySize = sizeof(network::EntityMovePayload);
    if (payload.size() < 1 + count * entrySize) {
        return;
    }

    try {
        EntityMoveBatchEvent batchEvent;
        batchEvent.entities.reserve(count);

        for (std::uint8_t i = 0; i < count; ++i) {
            std::size_t offset = 1 + i * entrySize;
            auto entry = network::Serializer::deserializeFromNetwork<
                network::EntityMovePayload>(
                std::span(payload.data() + offset, entrySize));

            EntityMoveEvent event;
            event.entityId = entry.entityId;
            event.x = entry.posX;
            event.y = entry.posY;
            event.vx = entry.velX;
            event.vy = entry.velY;
            batchEvent.entities.push_back(event);
        }

        queueCallback([this, batchEvent]() {
            if (onEntityMoveBatchCallback_) {
                onEntityMoveBatchCallback_(batchEvent);
            } else {
                for (const auto& e : batchEvent.entities) {
                    if (onEntityMoveCallback_) {
                        onEntityMoveCallback_(e);
                    }
                }
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleEntityDestroy(const network::Header& header,
                                        const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::EntityDestroyPayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::EntityDestroyPayload>(payload);

        std::uint32_t entityId = deserialized.entityId;

        queueCallback([this, entityId]() {
            for (const auto& cb : onEntityDestroyCallbacks_) {
                if (cb) cb(entityId);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleEntityHealth(const network::Header& header,
                                       const network::Buffer& payload) {
    (void)header;

    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[NetworkClient] handleEntityHealth called, payload size="
                      << payload.size());

    if (payload.size() < sizeof(network::EntityHealthPayload)) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[NetworkClient] Payload too small for EntityHealthPayload");
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::EntityHealthPayload>(payload);

        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Deserialized health: entityId="
                          << deserialized.entityId
                          << " current=" << deserialized.current
                          << " max=" << deserialized.max);

        EntityHealthEvent event{};
        event.entityId = deserialized.entityId;
        event.current = deserialized.current;
        event.max = deserialized.max;

        queueCallback([this, event]() {
            if (onEntityHealthCallback_) {
                onEntityHealthCallback_(event);
            }
        });
    } catch (...) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] Exception deserializing health payload");
    }
}

void NetworkClient::handlePowerUpEvent(const network::Header& header,
                                       const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::PowerUpEventPayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::PowerUpEventPayload>(payload);

        PowerUpEvent event{};
        event.playerId = deserialized.playerId;
        event.powerUpType = deserialized.powerUpType;
        event.duration = deserialized.duration;

        queueCallback([this, event]() {
            if (onPowerUpCallback_) {
                onPowerUpCallback_(event);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleUpdatePos(const network::Header& header,
                                    const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::UpdatePosPayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::UpdatePosPayload>(payload);

        float x = deserialized.posX;
        float y = deserialized.posY;

        queueCallback([this, x, y]() {
            if (onPositionCorrectionCallback_) {
                onPositionCorrectionCallback_(x, y);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleUpdateState(const network::Header& header,
                                      const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::UpdateStatePayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::UpdateStatePayload>(payload);

        GameStateEvent event;
        event.state = deserialized.getState();

        queueCallback([this, event]() {
            if (onGameStateChangeCallback_) {
                onGameStateChangeCallback_(event);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleGameOver(const network::Header& header,
                                   const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::GameOverPayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::GameOverPayload>(payload);

        GameOverEvent event{deserialized.finalScore};

        queueCallback([this, event]() {
            if (onGameOverCallback_) {
                onGameOverCallback_(event);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleGameStart(const network::Header& header,
                                    const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::GameStartPayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::GameStartPayload>(payload);

        float countdownDuration = deserialized.countdownDuration;

        queueCallback([this, countdownDuration]() {
            if (onGameStartCallback_) {
                onGameStartCallback_(countdownDuration);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handlePlayerReadyState(const network::Header& header,
                                           const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::PlayerReadyStatePayload)) {
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::PlayerReadyStatePayload>(payload);

        uint32_t userId = deserialized.userId;
        bool isReady = (deserialized.isReady != 0);

        queueCallback([this, userId, isReady]() {
            if (onPlayerReadyStateChangedCallback_) {
                onPlayerReadyStateChangedCallback_(userId, isReady);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleJoinLobbyResponse(const network::Header& header,
                                            const network::Buffer& payload) {
    (void)header;

    if (payload.size() < sizeof(network::JoinLobbyResponsePayload)) {
        return;
    }

    try {
        auto resp = network::Serializer::deserializeFromNetwork<
            network::JoinLobbyResponsePayload>(payload);

        queueCallback([this, resp]() {
            if (onJoinLobbyResponseCallback_) {
                onJoinLobbyResponseCallback_(resp.accepted == 1, resp.reason);
            }
        });
    } catch (...) {
        // Invalid payload, ignore
    }
}

void NetworkClient::handleLobbyList(const network::Header& header,
                                    const network::Buffer& payload) {
    (void)header;

    LOG_INFO_CAT(rtype::LogCategory::Network,
                 "[NetworkClient] Received S_LOBBY_LIST with payload size: "
                     << payload.size());

    if (payload.empty()) {
        LOG_DEBUG("[NetworkClient] Received empty lobby list");
        queueCallback([this]() {
            if (onLobbyListReceivedCallback_) {
                onLobbyListReceivedCallback_(LobbyListEvent{{}});
            }
        });
        return;
    }

    try {
        std::size_t offset = 0;

        if (payload.size() < 1) {
            return;
        }
        std::uint8_t lobbyCount = payload[offset++];

        LobbyListEvent event;
        event.lobbies.reserve(lobbyCount);

        constexpr std::size_t kLobbyInfoSize = 11;

        for (std::uint8_t i = 0;
             i < lobbyCount && offset + kLobbyInfoSize <= payload.size(); ++i) {
            LobbyInfo info;

            info.code.assign(
                reinterpret_cast<const char*>(payload.data() + offset), 6);
            offset += 6;

            std::uint16_t portNet;
            std::memcpy(&portNet, payload.data() + offset, sizeof(portNet));
            info.port = network::ByteOrderSpec::fromNetwork(portNet);
            offset += sizeof(portNet);

            info.playerCount = payload[offset++];

            info.maxPlayers = payload[offset++];

            info.isActive = (payload[offset++] != 0);

            event.lobbies.push_back(std::move(info));
        }

        LOG_DEBUG("[NetworkClient] Received lobby list with "
                  << event.lobbies.size() << " lobbies");

        queueCallback([this, event = std::move(event)]() {
            if (onLobbyListReceivedCallback_) {
                onLobbyListReceivedCallback_(event);
            }
        });
    } catch (...) {
        LOG_ERROR("[NetworkClient] Failed to parse lobby list");
    }
}

void NetworkClient::handlePong(const network::Header& header,
                               const network::Buffer& payload) {
    (void)header;
    (void)payload;

    LOG_DEBUG("[NetworkClient] Received PONG from server - connection alive");
}

void NetworkClient::flushOutgoing() {
    if (!serverEndpoint_.has_value() || !socket_->isOpen()) {
        return;
    }

    auto packets = connection_.getOutgoingPackets();
    for (auto& pkt : packets) {
        if (pkt.data.size() >= network::kHeaderSize) {
            network::Header hdr;
            std::memcpy(&hdr, pkt.data.data(), network::kHeaderSize);
            auto opcode = static_cast<network::OpCode>(hdr.opcode);
            if (opcode == network::OpCode::PING && connection_.isConnected()) {
                auto seq = network::ByteOrderSpec::fromNetwork(hdr.seqId);
                auto ack = network::ByteOrderSpec::fromNetwork(hdr.ackId);
                LOG_DEBUG("[NetworkClient] Sending PING keepalive seqId="
                          << seq << " ack=" << ack
                          << " missedPongs=" << connection_.missedPingCount());
            }
        }
        socket_->asyncSendTo(pkt.data, *serverEndpoint_,
                             [](network::Result<std::size_t> result) {
                                 // Fire and forget for now
                                 (void)result;
                             });
    }
}

void NetworkClient::sendAck(std::uint16_t ackSeqId) {
    if (!serverEndpoint_.has_value() || !socket_->isOpen()) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[NetworkClient] sendAck: no endpoint or socket not open");
        return;
    }

    auto ackPacket = connection_.buildAckPacket(ackSeqId);
    if (ackPacket.has_value()) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[NetworkClient] sendAck: sending ACK for seqId="
                          << ackSeqId
                          << " packet size=" << ackPacket.value().size());
        socket_->asyncSendTo(
            ackPacket.value(), *serverEndpoint_,
            [ackSeqId](network::Result<std::size_t> result) {
                if (result) {
                    LOG_DEBUG_CAT(
                        rtype::LogCategory::Network,
                        "[NetworkClient] ACK sent successfully for seqId="
                            << ackSeqId << " bytes=" << result.value());
                } else {
                    LOG_WARNING_CAT(rtype::LogCategory::Network,
                                    "[NetworkClient] ACK send failed for seqId="
                                        << ackSeqId);
                }
            });
        connection_.recordPacketSent();
    } else {
        LOG_WARNING_CAT(
            rtype::LogCategory::Network,
            "[NetworkClient] sendAck: buildAckPacket returned nullopt");
    }
}

}  // namespace rtype::client
