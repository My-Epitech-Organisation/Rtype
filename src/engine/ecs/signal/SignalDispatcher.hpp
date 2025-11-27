/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SignalDispatcher
*/

#ifndef SRC_ENGINE_ECS_SIGNAL_SIGNALDISPATCHER_HPP_
#define SRC_ENGINE_ECS_SIGNAL_SIGNALDISPATCHER_HPP_

#include <functional>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "../core/Entity.hpp"

namespace ECS {

/**
 * @brief Event system for component lifecycle notifications.
 *
 * Enables reactive programming patterns in ECS:
 * - onConstruct: Triggered when component is added
 * - onDestroy: Triggered when component is removed
 *
 * Thread Safety:
 * - All operations are thread-safe
 * - Multiple threads can register callbacks concurrently
 * - Dispatching events is safe from multiple threads
 * - Callbacks are executed WITHOUT holding locks to prevent deadlocks
 *
 * Deadlock Prevention:
 * - Callbacks are copied before execution
 * - Reentrant dispatch is supported (callbacks can trigger other callbacks)
 * - No locks held during callback execution
 *
 * Use cases: physics initialization, resource cleanup, debugging, logging.
 */
class SignalDispatcher {
 public:
    using Callback = std::function<void(Entity)>;

    void registerConstruct(std::type_index type, Callback callback);
    void registerDestroy(std::type_index type, Callback callback);
    void dispatchConstruct(std::type_index type, Entity entity);
    void dispatchDestroy(std::type_index type, Entity entity);

    /**
     * @brief Clears all callbacks for a specific component type.
     * Useful for cleanup or testing.
     */
    void clearCallbacks(std::type_index type);

    /**
     * @brief Clears all registered callbacks.
     */
    void clearAllCallbacks();

 private:
    std::unordered_map<std::type_index, std::vector<Callback>> _constructCallbacks;
    std::unordered_map<std::type_index, std::vector<Callback>> _destroyCallbacks;
    mutable std::shared_mutex callbacks_mutex;
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_SIGNAL_SIGNALDISPATCHER_HPP_
