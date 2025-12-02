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
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }

    void push(T&& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

   private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
};
