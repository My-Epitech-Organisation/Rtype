/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SignalDispatcher
*/

#ifndef ECS_SIGNAL_SIGNAL_DISPATCHER_HPP
    #define ECS_SIGNAL_SIGNAL_DISPATCHER_HPP
    #include "../Core/Entity.hpp"
    #include <functional>
    #include <vector>
    #include <typeindex>
    #include <unordered_map>
    #include <shared_mutex>

namespace ECS {

    /**
     * @brief Event system for component lifecycle notifications.
     *
     * Enables reactive programming patterns in ECS:
     * - on_construct: Triggered when component is added
     * - on_destroy: Triggered when component is removed
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

        void register_construct(std::type_index type, Callback callback);
        void register_destroy(std::type_index type, Callback callback);
        void dispatch_construct(std::type_index type, Entity entity);
        void dispatch_destroy(std::type_index type, Entity entity);
        
        /**
         * @brief Clears all callbacks for a specific component type.
         * Useful for cleanup or testing.
         */
        void clear_callbacks(std::type_index type);
        
        /**
         * @brief Clears all registered callbacks.
         */
        void clear_all_callbacks();

    private:
        std::unordered_map<std::type_index, std::vector<Callback>> construct_callbacks;
        std::unordered_map<std::type_index, std::vector<Callback>> destroy_callbacks;
        mutable std::shared_mutex callbacks_mutex;
    };

} // namespace ECS

#endif // ECS_SIGNAL_SIGNAL_DISPATCHER_HPP
