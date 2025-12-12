/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEventProcessor - Implementation
*/

#include "GameEventProcessor.hpp"

#include <rtype/common.hpp>

#include "network/ServerNetworkSystem.hpp"

namespace rtype::server {

GameEventProcessor::GameEventProcessor(
    std::shared_ptr<engine::IGameEngine> gameEngine,
        std::shared_ptr<ServerNetworkSystem> networkSystem, bool verbose,
        std::function<void(const engine::GameEvent&)> eventObserver)
    : _gameEngine(std::move(gameEngine)),
      _networkSystem(std::move(networkSystem)),
            _verbose(verbose),
            _eventObserver(std::move(eventObserver)) {}

void GameEventProcessor::processEvents() {
    if (!_gameEngine || !_networkSystem) {
        return;
    }

    auto events = _gameEngine->getPendingEvents();

    for (const auto& event : events) {
        auto processed = _gameEngine->processEvent(event);

        if (!processed.valid) {
            if (_verbose) {
                LOG_DEBUG("[EventProcessor] Event not processed: type="
                          << static_cast<int>(event.type)
                          << " networkId=" << event.entityNetworkId);
            }
            continue;
        }

        if (_eventObserver) {
            _eventObserver(event);
        }

        switch (processed.type) {
            case engine::GameEventType::EntitySpawned: {
                network::EntityType networkType =
                    static_cast<network::EntityType>(
                        processed.networkEntityType);

                _networkSystem->broadcastEntitySpawn(
                    processed.networkId, networkType, processed.x, processed.y);

                if (_verbose) {
                    LOG_DEBUG("[EventProcessor] Entity spawned & broadcast: "
                              << "networkId=" << processed.networkId << " type="
                              << static_cast<int>(processed.networkEntityType)
                              << " pos=(" << processed.x << ", " << processed.y
                              << ")");
                }
                break;
            }
            case engine::GameEventType::EntityDestroyed: {
                _networkSystem->unregisterNetworkedEntityById(
                    processed.networkId);
                if (_verbose) {
                    LOG_DEBUG("[EventProcessor] Entity destroyed: networkId="
                              << processed.networkId);
                }
                break;
            }
            case engine::GameEventType::EntityUpdated: {
                _networkSystem->updateEntityPosition(
                    processed.networkId, processed.x, processed.y, processed.vx,
                    processed.vy);
                break;
            }
            case engine::GameEventType::EntityHealthChanged: {
                _networkSystem->updateEntityHealth(event.entityNetworkId,
                                                   event.healthCurrent,
                                                   event.healthMax);
                if (_verbose) {
                    LOG_DEBUG("[EventProcessor] Entity health changed: "
                              << "networkId=" << event.entityNetworkId
                              << " health=" << event.healthCurrent << "/"
                              << event.healthMax);
                }
                break;
            }
        }
    }

    _gameEngine->clearPendingEvents();
}

void GameEventProcessor::syncEntityPositions() {
    if (!_gameEngine || !_networkSystem) {
        return;
    }

    _gameEngine->syncEntityPositions(
        [this](uint32_t networkId, float x, float y, float vx, float vy) {
            _networkSystem->updateEntityPosition(networkId, x, y, vx, vy);
        });

    _networkSystem->broadcastEntityUpdates();
}

}  // namespace rtype::server
