/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EventBus implementation
*/

#include "EventBus.hpp"

namespace PoC {

    void EventBus::clearAllSubscribers() {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers.clear();
    }

} // namespace PoC
