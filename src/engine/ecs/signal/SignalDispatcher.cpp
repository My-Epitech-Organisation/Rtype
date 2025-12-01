/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SignalDispatcher
*/

#include "SignalDispatcher.hpp"

#include <mutex>
#include <utility>

namespace ECS {

void SignalDispatcher::registerConstruct(std::type_index type,
                                         Callback callback) {
    std::unique_lock lock(callbacks_mutex);
    _constructCallbacks[type].push_back(std::move(callback));
}

void SignalDispatcher::registerDestroy(std::type_index type,
                                       Callback callback) {
    std::unique_lock lock(callbacks_mutex);
    _destroyCallbacks[type].push_back(std::move(callback));
}

void SignalDispatcher::dispatchConstruct(std::type_index type, Entity entity) {
    std::shared_lock lock(callbacks_mutex);
    auto iter = _constructCallbacks.find(type);
    if (iter != _constructCallbacks.end()) {
        std::vector<Callback> callbacks_copy = iter->second;
        lock.unlock();

        for (auto& callback : callbacks_copy) {
            callback(entity);
        }
    }
}

void SignalDispatcher::dispatchDestroy(std::type_index type, Entity entity) {
    std::shared_lock lock(callbacks_mutex);
    auto iter = _destroyCallbacks.find(type);
    if (iter != _destroyCallbacks.end()) {
        std::vector<Callback> callbacks_copy = iter->second;
        lock.unlock();

        for (auto& callback : callbacks_copy) {
            callback(entity);
        }
    }
}

void SignalDispatcher::clearCallbacks(std::type_index type) {
    std::unique_lock lock(callbacks_mutex);
    _constructCallbacks.erase(type);
    _destroyCallbacks.erase(type);
}

void SignalDispatcher::clearAllCallbacks() {
    std::unique_lock lock(callbacks_mutex);
    _constructCallbacks.clear();
    _destroyCallbacks.clear();
}

}  // namespace ECS
