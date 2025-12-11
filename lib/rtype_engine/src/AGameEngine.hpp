/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AGameEngine - Abstract base class for game engine implementations
*/

#pragma once

#include "IGameEngine.hpp"

namespace rtype::engine {

/**
 * @class AGameEngine
 * @brief Abstract base class implementing common IGameEngine functionality
 *
 * This class provides default implementations for event management and
 * common engine operations. Concrete game engines should inherit from
 * this class rather than IGameEngine directly.
 *
 * Example usage:
 * @code
 * class RTypeGameEngine : public AGameEngine {
 * public:
 *     bool initialize() override { ... }
 *     void update(float deltaTime) override { ... }
 *     void shutdown() override { ... }
 * };
 * @endcode
 */
class AGameEngine : public IGameEngine {
   public:
    ~AGameEngine() override = default;

    /**
     * @brief Set the callback for game events
     * @param callback Function to call when events occur
     */
    void setEventCallback(EventCallback callback) override {
        _eventCallback = std::move(callback);
    }

    /**
     * @brief Get pending events since last call
     * @return Vector of pending events
     */
    std::vector<GameEvent> getPendingEvents() override {
        return _pendingEvents;
    }

    /**
     * @brief Clear all pending events
     */
    void clearPendingEvents() override { _pendingEvents.clear(); }

    /**
     * @brief Get the current entity count
     * @return Number of active entities
     */
    std::size_t getEntityCount() const override { return _entityCount; }

    /**
     * @brief Check if the engine is running
     * @return true if the engine is initialized and running
     */
    bool isRunning() const override { return _isRunning; }

    // ==================== Default Player Action Implementations ====================

    /**
     * @brief Default implementation - must be overridden by game engines
     * @return 0 (failure) by default
     */
    uint32_t spawnProjectile(uint32_t /*playerNetworkId*/, float /*playerX*/,
                             float /*playerY*/) override {
        return 0;  // Default: not implemented
    }

    /**
     * @brief Default implementation - must be overridden by game engines
     */
    void updatePlayerPositions(
        float /*deltaTime*/,
        std::function<void(uint32_t, float, float, float, float)>
        /*positionCallback*/) override {
        // Default: no-op
    }

    /**
     * @brief Default implementation - must be overridden by game engines
     * @return false by default
     */
    bool setPlayerVelocity(uint32_t /*networkId*/, float /*vx*/,
                           float /*vy*/) override {
        return false;  // Default: not implemented
    }

    /**
     * @brief Default implementation - must be overridden by game engines
     * @return false by default
     */
    bool getPlayerPosition(uint32_t /*networkId*/, float& /*outX*/,
                           float& /*outY*/, float& /*outVx*/,
                           float& /*outVy*/) const override {
        return false;  // Default: not implemented
    }

   protected:
    AGameEngine() = default;

    /**
     * @brief Emit a game event
     *
     * This will call the event callback if set and add the event
     * to the pending events queue.
     *
     * @param event The event to emit
     */
    void emitEvent(const GameEvent& event) {
        _pendingEvents.push_back(event);
        if (_eventCallback) {
            _eventCallback(event);
        }
    }

    /**
     * @brief Set the running state of the engine
     * @param running New running state
     */
    void setRunning(bool running) noexcept { _isRunning = running; }

    /**
     * @brief Set the entity count
     * @param count New entity count
     */
    void setEntityCount(std::size_t count) noexcept { _entityCount = count; }

    EventCallback _eventCallback;           ///< Callback for game events
    std::vector<GameEvent> _pendingEvents;  ///< Queue of pending events
    std::size_t _entityCount = 0;           ///< Number of active entities
    bool _isRunning = false;                ///< Engine running state
};

}  // namespace rtype::engine
