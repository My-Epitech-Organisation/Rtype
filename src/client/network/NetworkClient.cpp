/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkClient - Implementation
*/

#include "NetworkClient.hpp"

#include <cstring>
#include <memory>
#include <queue>
#include <string>
#include <utility>

#include "Logger/Macros.hpp"
#include "Serializer.hpp"
#include "protocol/ByteOrderSpec.hpp"

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
            if (onConnectedCallback_) {
                onConnectedCallback_(userId);
            }
        });
    };

    connCallbacks.onDisconnected = [this](network::DisconnectReason reason) {
        queueCallback([this, reason]() {
            if (onDisconnectedCallback_) {
                onDisconnectedCallback_(reason);
            }
        });
    };

    connCallbacks.onConnectFailed = [this](network::NetworkError error) {
        (void)error;
        queueCallback([this]() {
            if (onDisconnectedCallback_) {
                onDisconnectedCallback_(DisconnectReason::ProtocolError);
            }
        });
    };

    connection_.setCallbacks(connCallbacks);
}

NetworkClient::~NetworkClient() {
    if (isConnected()) {
        disconnect();
    }
    if (socket_) {
        socket_->close();
    }
}

bool NetworkClient::connect(const std::string& host, std::uint16_t port) {
    if (!connection_.isDisconnected()) {
        return false;
    }

    auto bindResult = socket_->bind(0);
    if (!bindResult) {
        return false;
    }

    serverEndpoint_ = network::Endpoint{host, port};

    startReceive();

    auto result = connection_.connect();
    if (!result) {
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
}

bool NetworkClient::isConnected() const noexcept {
    return connection_.isConnected();
}

std::optional<std::uint32_t> NetworkClient::userId() const noexcept {
    return connection_.userId();
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

void NetworkClient::onConnected(
    std::function<void(std::uint32_t myUserId)> callback) {
    onConnectedCallback_ = std::move(callback);
}

void NetworkClient::onDisconnected(
    std::function<void(DisconnectReason)> callback) {
    onDisconnectedCallback_ = std::move(callback);
}

void NetworkClient::onEntitySpawn(
    std::function<void(EntitySpawnEvent)> callback) {
    onEntitySpawnCallback_ = std::move(callback);
}

void NetworkClient::onEntityMove(
    std::function<void(EntityMoveEvent)> callback) {
    onEntityMoveCallback_ = std::move(callback);
}

void NetworkClient::onEntityDestroy(
    std::function<void(std::uint32_t entityId)> callback) {
    onEntityDestroyCallback_ = std::move(callback);
}

void NetworkClient::onPositionCorrection(
    std::function<void(float x, float y)> callback) {
    onPositionCorrectionCallback_ = std::move(callback);
}

void NetworkClient::onGameStateChange(
    std::function<void(GameStateEvent)> callback) {
    onGameStateChangeCallback_ = std::move(callback);
}

void NetworkClient::poll() {
    ioContext_.poll();

    connection_.update();

    flushOutgoing();

    dispatchCallbacks();
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

void NetworkClient::queueCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackQueue_.push(std::move(callback));
}

void NetworkClient::startReceive() {
    if (receiveInProgress_ || !socket_->isOpen()) {
        return;
    }

    receiveInProgress_ = true;
    receiveBuffer_->resize(network::kMaxPacketSize);

    socket_->asyncReceiveFrom(receiveBuffer_, receiveSender_,
                              [this](network::Result<std::size_t> result) {
                                  handleReceive(std::move(result));
                              });
}

void NetworkClient::handleReceive(network::Result<std::size_t> result) {
    receiveInProgress_ = false;

    if (result) {
        std::size_t bytesReceived = result.value();
        receiveBuffer_->resize(bytesReceived);

        processIncomingPacket(*receiveBuffer_, *receiveSender_);
    }

    if (socket_->isOpen()) {
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
        payload.assign(data.begin() + network::kHeaderSize, data.end());
    }

    auto opcode = static_cast<network::OpCode>(header.opcode);

    switch (opcode) {
        case network::OpCode::S_ENTITY_SPAWN:
            handleEntitySpawn(header, payload);
            break;

        case network::OpCode::S_ENTITY_MOVE:
            handleEntityMove(header, payload);
            break;

        case network::OpCode::S_ENTITY_DESTROY:
            handleEntityDestroy(header, payload);
            break;

        case network::OpCode::S_UPDATE_POS:
            handleUpdatePos(header, payload);
            break;

        case network::OpCode::S_UPDATE_STATE:
            handleUpdateState(header, payload);
            break;

        default:
            break;
    }
}

void NetworkClient::handleEntitySpawn(const network::Header& header,
                                      const network::Buffer& payload) {
    (void)header;

    LOG_DEBUG("[NetworkClient] handleEntitySpawn called, payload size=" +
              std::to_string(payload.size()));

    if (payload.size() < sizeof(network::EntitySpawnPayload)) {
        LOG_DEBUG("[NetworkClient] Payload too small for EntitySpawnPayload");
        return;
    }

    try {
        auto deserialized = network::Serializer::deserializeFromNetwork<
            network::EntitySpawnPayload>(payload);

        LOG_DEBUG("[NetworkClient] Deserialized spawn: entityId=" +
                  std::to_string(deserialized.entityId) + " type=" +
                  std::to_string(static_cast<int>(deserialized.type)) +
                  " pos=(" + std::to_string(deserialized.posX) + ", " +
                  std::to_string(deserialized.posY) + ")");

        EntitySpawnEvent event;
        event.entityId = deserialized.entityId;
        event.type = deserialized.getType();
        event.x = deserialized.posX;
        event.y = deserialized.posY;

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
            if (onEntityDestroyCallback_) {
                onEntityDestroyCallback_(entityId);
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

void NetworkClient::flushOutgoing() {
    if (!serverEndpoint_.has_value() || !socket_->isOpen()) {
        return;
    }

    auto packets = connection_.getOutgoingPackets();
    for (auto& pkt : packets) {
        socket_->asyncSendTo(pkt.data, *serverEndpoint_,
                             [](network::Result<std::size_t> result) {
                                 // Fire and forget for now
                                 (void)result;
                             });
    }
}

}  // namespace rtype::client
