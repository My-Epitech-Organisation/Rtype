/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IoContext - RAII wrapper for Asio io_context
*/

#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include <thread>

#include <asio.hpp>

namespace rtype::network {

/**
 * @brief RAII wrapper for asio::io_context
 *
 * Manages Asio I/O context lifecycle. Use poll() for game loop integration
 * or runInBackground() for dedicated network thread.
 */
class IoContext {
   public:
    using WorkGuard =
        asio::executor_work_guard<asio::io_context::executor_type>;

    /**
     * @brief Construct a new IoContext
     */
    IoContext()
        : context_(std::make_unique<asio::io_context>()),
          workGuard_(asio::make_work_guard(*context_)) {}

    ~IoContext() { stop(); }

    // Non-copyable
    IoContext(const IoContext&) = delete;
    IoContext& operator=(const IoContext&) = delete;

    // Non-movable (due to work guard complexity)
    IoContext(IoContext&&) = delete;
    IoContext& operator=(IoContext&&) = delete;

    [[nodiscard]] asio::io_context& get() noexcept { return *context_; }
    [[nodiscard]] const asio::io_context& get() const noexcept { return *context_; }
    operator asio::io_context&() noexcept { return *context_; }

    std::size_t run() { return context_->run(); }
    std::size_t runOne() { return context_->run_one(); }
    std::size_t poll() { return context_->poll(); }
    std::size_t pollOne() { return context_->poll_one(); }

    void stop() {
        workGuard_.reset();
        context_->stop();
        if (backgroundThread_.joinable()) {
            backgroundThread_.join();
        }
    }

    [[nodiscard]] bool stopped() const noexcept { return context_->stopped(); }

    void restart() {
        context_->restart();
        workGuard_.emplace(asio::make_work_guard(*context_));
    }

    void runInBackground() {
        if (backgroundThread_.joinable()) {
            return;
        }
        backgroundThread_ = std::thread([this]() { context_->run(); });
    }

    [[nodiscard]] bool isRunningInBackground() const noexcept {
        return backgroundThread_.joinable();
    }

    void releaseWorkGuard() { workGuard_.reset(); }

   private:
    std::unique_ptr<asio::io_context> context_;
    std::optional<WorkGuard> workGuard_;
    std::thread backgroundThread_;
};

}  // namespace rtype::network
