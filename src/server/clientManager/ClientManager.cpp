/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientManager - Client connection management implementation
*/

#include "ClientManager.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace rtype::server {

ClientManager::ClientManager(size_t maxPlayers,
                             std::shared_ptr<ServerMetrics> metrics,
                             bool verbose)
    : _maxPlayers(maxPlayers),
      _metrics(std::move(metrics)),
      _verbose(verbose),
      _rateLimitResetTimeMs(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now().time_since_epoch())
              .count()) {
    _timeoutBuffer.reserve(maxPlayers);
}

ClientId ClientManager::handleNewConnection(const Endpoint& endpoint) {
    std::unique_lock lock(_clientsMutex);
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now().time_since_epoch())
                           .count();

    updateRateLimitWindow(nowMs);
    if (isRateLimitExceeded(endpoint)) {
        return INVALID_CLIENT_ID;
    }
    if (const auto existingId = findClientByEndpointInternal(endpoint);
        existingId != INVALID_CLIENT_ID) {
        LOG_WARNING_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Connection attempt from already connected endpoint: "
                << endpoint.toString());
        return existingId;
    }
    if (isServerFull()) {
        return INVALID_CLIENT_ID;
    }
    const ClientId assignedId = generateNextClientId();
    if (assignedId == INVALID_CLIENT_ID) {
        return INVALID_CLIENT_ID;
    }
    registerClient(assignedId, endpoint);
    return assignedId;
}

void ClientManager::updateRateLimitWindow(int64_t nowMs) noexcept {
    auto resetTimeMs = _rateLimitResetTimeMs.load(std::memory_order_relaxed);
    if (nowMs >= resetTimeMs) {
        int64_t newResetTime = nowMs + RATE_LIMIT_WINDOW_MS;
        if (_rateLimitResetTimeMs.compare_exchange_strong(
                resetTimeMs, newResetTime, std::memory_order_relaxed)) {
            _connectionsThisSecond.store(0, std::memory_order_relaxed);
        }
    }
}

bool ClientManager::isRateLimitExceeded(const Endpoint& endpoint) noexcept {
    const auto connectionsThisSecond =
        _connectionsThisSecond.load(std::memory_order_relaxed);

    if (connectionsThisSecond >= MAX_CONNECTIONS_PER_SECOND) {
        LOG_WARNING_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Rate limit exceeded, rejecting connection from "
                << endpoint.toString());
        if (auto metrics = _metrics.lock()) {
            metrics->connectionsRejected.fetch_add(1,
                                                   std::memory_order_relaxed);
        }
        return true;
    }
    return false;
}

bool ClientManager::isServerFull() const noexcept {
    if (_clients.size() >= _maxPlayers) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Connection rejected: server full ("
                         << _maxPlayers << "/" << _maxPlayers << " players)");
        if (auto metrics = _metrics.lock()) {
            metrics->connectionsRejected.fetch_add(1,
                                                   std::memory_order_relaxed);
        }
        // TODO(Clem): Send rejection packet to client
        return true;
    }
    return false;
}

ClientId ClientManager::generateNextClientId() noexcept {
    ClientId newId;
    ClientId currentId = _nextClientId.load(std::memory_order_relaxed);
    do {
        if (currentId == std::numeric_limits<ClientId>::max()) {
            LOG_ERROR_CAT(
                ::rtype::LogCategory::GameEngine,
                "[Server] Client ID overflow! Cannot accept new connections.");
            if (auto metrics = _metrics.lock()) {
                metrics->connectionsRejected.fetch_add(
                    1, std::memory_order_relaxed);
            }
            return INVALID_CLIENT_ID;
        }
        newId = currentId + 1;
    } while (!_nextClientId.compare_exchange_weak(currentId, newId,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed));
    return currentId;
}

void ClientManager::registerClient(ClientId clientId,
                                   const Endpoint& endpoint) noexcept {
    _connectionsThisSecond.fetch_add(1, std::memory_order_relaxed);

    Client newClient{.id = clientId,
                     .endpoint = endpoint,
                     .lastActivityTime = std::chrono::steady_clock::now(),
                     .state = ClientState::Connected};

    _clients.emplace(clientId, newClient);
    _endpointToClient.emplace(endpoint, clientId);
    if (auto metrics = _metrics.lock()) {
        metrics->totalConnections.fetch_add(1, std::memory_order_relaxed);
    }

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] New client connected: ID=" << clientId << " from "
                                                      << endpoint.toString());
    printConnectedClients();
    notifyClientConnected(clientId);
}

void ClientManager::handleClientDisconnect(ClientId clientId,
                                           DisconnectReason reason) noexcept {
    std::unique_lock lock(_clientsMutex);
    handleClientDisconnectInternal(clientId, reason);
}

void ClientManager::handleClientDisconnectInternal(
    ClientId clientId, DisconnectReason reason) noexcept {
    const auto it = _clients.find(clientId);
    if (it == _clients.end()) {
        return;
    }
    const Endpoint endpoint = it->second.endpoint;
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Client " << clientId << " disconnected ("
                                    << toString(reason) << ")");
    removeClientFromMaps(clientId, endpoint);
    notifyClientDisconnected(clientId, reason);
    printConnectedClients();
}

void ClientManager::removeClientFromMaps(ClientId clientId,
                                         const Endpoint& endpoint) noexcept {
    _clients.erase(clientId);
    _endpointToClient.erase(endpoint);
}

void ClientManager::updateClientActivity(ClientId clientId) noexcept {
    std::unique_lock lock(_clientsMutex);

    if (const auto it = _clients.find(clientId); it != _clients.end()) {
        it->second.lastActivityTime = std::chrono::steady_clock::now();
    }
}

ClientId ClientManager::findClientByEndpoint(
    const Endpoint& endpoint) const noexcept {
    std::shared_lock lock(_clientsMutex);
    return findClientByEndpointInternal(endpoint);
}

ClientId ClientManager::findClientByEndpointInternal(
    const Endpoint& endpoint) const noexcept {
    if (const auto it = _endpointToClient.find(endpoint);
        it != _endpointToClient.end()) {
        return it->second;
    }
    return INVALID_CLIENT_ID;
}

size_t ClientManager::getConnectedClientCount() const noexcept {
    std::shared_lock lock(_clientsMutex);
    return _clients.size();
}

std::vector<ClientId> ClientManager::getConnectedClientIds() const {
    std::shared_lock lock(_clientsMutex);
    std::vector<ClientId> ids;
    ids.reserve(_clients.size());

    for (const auto& [id, client] : _clients) {
        ids.push_back(id);
    }
    return ids;
}

std::optional<Client> ClientManager::getClientInfo(ClientId clientId) const {
    std::shared_lock lock(_clientsMutex);

    if (const auto it = _clients.find(clientId); it != _clients.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ClientManager::checkClientTimeouts(uint32_t timeoutSeconds) noexcept {
    std::unique_lock lock(_clientsMutex);
    const auto now = std::chrono::steady_clock::now();

    _timeoutBuffer.clear();
    for (const auto& [id, client] : _clients) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - client.lastActivityTime);
        if (elapsed.count() >= static_cast<int64_t>(timeoutSeconds)) {
            _timeoutBuffer.emplace_back(id, client.endpoint);
        }
    }
    for (const auto& [id, endpoint] : _timeoutBuffer) {
        handleClientDisconnectInternal(id, DisconnectReason::Timeout);
    }
}

void ClientManager::clearAllClients() noexcept {
    std::unique_lock lock(_clientsMutex);
    for (const auto& [id, client] : _clients) {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Disconnecting client " << id);
    }
    _clients.clear();
    _endpointToClient.clear();
}

void ClientManager::notifyClientConnected(ClientId newClientId) noexcept {
    // TODO(Clem): Send notification packet to all other clients
    // Pseudocode:
    // Packet packet(PacketType::PlayerJoin);
    // packet.setData(serialize(newClientId));
    // broadcastToAllExcept(packet, newClientId);

    LOG_DEBUG_CAT(
        ::rtype::LogCategory::GameEngine,
        "[Server] Notifying other clients about new player " << newClientId);
}

void ClientManager::notifyClientDisconnected(ClientId clientId,
                                             DisconnectReason reason) noexcept {
    // TODO(Clem): Send notification packet to all remaining clients
    // This is critical for game state consistency when a client crashes
    //
    // Pseudocode:
    // Packet packet(PacketType::PlayerLeave);
    // packet.setData(serialize(clientId, reason));
    // broadcastToAll(packet);

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Notifying other clients about player "
                      << clientId << " leaving (" << toString(reason) << ")");
}

void ClientManager::printConnectedClients() const noexcept {
    if (!_verbose) {
        return;
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] === Connected Clients ===");
    if (_clients.empty()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server]   (no clients connected)");
    } else {
        const auto now = std::chrono::steady_clock::now();
        for (const auto& [id, client] : _clients) {
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - client.lastActivityTime);
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[Server]   Client "
                              << id << " - " << client.endpoint.toString()
                              << " [" << toString(client.state) << "]"
                              << " (last seen: " << elapsed.count()
                              << "s ago)");
        }
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] ==============================");
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Total: " << _clients.size() << "/" << _maxPlayers
                                     << " players");
}

}  // namespace rtype::server
