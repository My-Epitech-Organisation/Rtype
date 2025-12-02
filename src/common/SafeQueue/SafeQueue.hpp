/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SafeQueue
*/

#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <utility>

template <typename T>
class SafeQueue {
   public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(item);
    }

    void push(T&& item) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(item));
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return std::nullopt;
        }
        T item = _queue.front();
        _queue.pop();
        return item;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

   private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
};
