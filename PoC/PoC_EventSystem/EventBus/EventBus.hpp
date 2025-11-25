/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EventBus for Observer Pattern PoC
*/

#ifndef EVENTBUS_HPP
    #define EVENTBUS_HPP

    #include <functional>
    #include <unordered_map>
    #include <vector>
    #include <typeindex>
    #include <memory>
    #include <mutex>
    #include "Events.hpp"

namespace PoC {

    /**
     * @brief Central event bus implementing Observer Pattern
     * 
     * This class decouples event publishers from subscribers:
     * - Systems publish events without knowing who listens
     * - Systems subscribe to events without knowing who publishes
     * - New systems can be added without modifying existing code
     * 
     * Thread Safety:
     * - Subscribe/unsubscribe operations are thread-safe
     * - Publishing events is thread-safe
     * - Callbacks are executed in the publishing thread
     * 
     * Pros:
     * - Low coupling between systems
     * - Easy to add/remove observers
     * - Systems don't need to know about each other
     * - Flexible and extensible
     * 
     * Cons:
     * - Runtime overhead (function pointer dispatch)
     * - Harder to debug (indirect control flow)
     * - Need to manage subscriber lifetimes
     * - Potential memory overhead for event objects
     */
    class EventBus {
    public:
        using CallbackId = size_t;
        using EventCallback = std::function<void(const Event&)>;

        /**
         * @brief Subscribe to events of a specific type
         * @tparam T Event type to subscribe to
         * @param callback Function to call when event is published
         * @return Callback ID for unsubscribing
         */
        template<typename T>
        CallbackId subscribe(std::function<void(const T&)> callback);

        /**
         * @brief Unsubscribe from events
         * @tparam T Event type
         * @param callbackId ID returned from subscribe()
         */
        template<typename T>
        void unsubscribe(CallbackId callbackId);

        /**
         * @brief Publish an event to all subscribers
         * @tparam T Event type
         * @param event Event instance to publish
         */
        template<typename T>
        void publish(const T& event);

        /**
         * @brief Get number of subscribers for an event type
         * @tparam T Event type
         * @return Number of active subscribers
         */
        template<typename T>
        size_t subscriberCount() const;

        /**
         * @brief Clear all subscribers for an event type
         * @tparam T Event type
         */
        template<typename T>
        void clearSubscribers();

        /**
         * @brief Clear all subscribers for all event types
         */
        void clearAllSubscribers();

    private:
        struct CallbackWrapper {
            CallbackId id;
            EventCallback callback;
        };

        std::unordered_map<std::type_index, std::vector<CallbackWrapper>> _subscribers;
        mutable std::mutex _mutex;
        CallbackId _nextCallbackId = 0;
    };

    // Template implementations
    template<typename T>
    EventBus::CallbackId EventBus::subscribe(std::function<void(const T&)> callback) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto typeIndex = std::type_index(typeid(T));
        CallbackId id = _nextCallbackId++;

        // Wrap the typed callback in a generic EventCallback
        EventCallback wrapper = [callback](const Event& event) {
            callback(static_cast<const T&>(event));
        };

        _subscribers[typeIndex].push_back({id, wrapper});
        return id;
    }

    template<typename T>
    void EventBus::unsubscribe(CallbackId callbackId) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto typeIndex = std::type_index(typeid(T));
        auto it = _subscribers.find(typeIndex);
        
        if (it != _subscribers.end()) {
            auto& callbacks = it->second;
            callbacks.erase(
                std::remove_if(callbacks.begin(), callbacks.end(),
                    [callbackId](const CallbackWrapper& wrapper) {
                        return wrapper.id == callbackId;
                    }),
                callbacks.end()
            );
        }
    }

    template<typename T>
    void EventBus::publish(const T& event) {
        std::vector<CallbackWrapper> callbacks;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto typeIndex = std::type_index(typeid(T));
            auto it = _subscribers.find(typeIndex);
            
            if (it != _subscribers.end()) {
                callbacks = it->second; // Copy to avoid holding lock during callbacks
            }
        }

        // Execute callbacks without holding the lock
        for (const auto& wrapper : callbacks) {
            wrapper.callback(event);
        }
    }

    template<typename T>
    size_t EventBus::subscriberCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto typeIndex = std::type_index(typeid(T));
        auto it = _subscribers.find(typeIndex);
        return (it != _subscribers.end()) ? it->second.size() : 0;
    }

    template<typename T>
    void EventBus::clearSubscribers() {
        std::lock_guard<std::mutex> lock(_mutex);
        auto typeIndex = std::type_index(typeid(T));
        _subscribers.erase(typeIndex);
    }

} // namespace PoC

#endif // EVENTBUS_HPP
