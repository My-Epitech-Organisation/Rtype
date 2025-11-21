/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SignalDispatcher
*/

#include "SignalDispatcher.hpp"
#include <mutex>

namespace ECS {

    void SignalDispatcher::register_construct(std::type_index type, Callback callback) {
        std::unique_lock lock(callbacks_mutex);
        construct_callbacks[type].push_back(std::move(callback));
    }

    void SignalDispatcher::register_destroy(std::type_index type, Callback callback) {
        std::unique_lock lock(callbacks_mutex);
        destroy_callbacks[type].push_back(std::move(callback));
    }

    void SignalDispatcher::dispatch_construct(std::type_index type, Entity entity) {
        std::shared_lock lock(callbacks_mutex);
        auto it = construct_callbacks.find(type);
        if (it != construct_callbacks.end()) {
            std::vector<Callback> callbacks_copy = it->second;
            lock.unlock();

            for (auto& callback : callbacks_copy) {
                callback(entity);
            }
        }
    }

    void SignalDispatcher::dispatch_destroy(std::type_index type, Entity entity) {
        std::shared_lock lock(callbacks_mutex);
        auto it = destroy_callbacks.find(type);
        if (it != destroy_callbacks.end()) {
            std::vector<Callback> callbacks_copy = it->second;
            lock.unlock();

            for (auto& callback : callbacks_copy) {
                callback(entity);
            }
        }
    }

    void SignalDispatcher::clear_callbacks(std::type_index type) {
        std::unique_lock lock(callbacks_mutex);
        construct_callbacks.erase(type);
        destroy_callbacks.erase(type);
    }

    void SignalDispatcher::clear_all_callbacks() {
        std::unique_lock lock(callbacks_mutex);
        construct_callbacks.clear();
        destroy_callbacks.clear();
    }

} // namespace ECS
