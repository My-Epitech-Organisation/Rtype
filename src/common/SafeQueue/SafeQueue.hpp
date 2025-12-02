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

template <typename T>
class SafeQueue {
   public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
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

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

   private:
    std::queue<T> queue_;
    std::mutex mutex_;
};
