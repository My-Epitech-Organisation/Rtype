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

    /**
     * @brief Destructor - stops context and joins background thread if running
     */
    ~IoContext() { stop(); }

    // Non-copyable
    IoContext(const IoContext&) = delete;
    IoContext& operator=(const IoContext&) = delete;

    // Non-movable (due to work guard complexity)
    IoContext(IoContext&&) = delete;
    IoContext& operator=(IoContext&&) = delete;

    // ========================================================================
    // Access to underlying context
    // ========================================================================

    /**
     * @brief Get reference to the underlying asio::io_context
     *
     * Use this to create sockets and other Asio objects.
     */
    [[nodiscard]] asio::io_context& get() noexcept { return *context_; }

    [[nodiscard]] const asio::io_context& get() const noexcept {
        return *context_;
    }

    /**
     * @brief Implicit conversion to asio::io_context&
     */
    operator asio::io_context&() noexcept { return *context_; }

    // ========================================================================
    // Execution Control
    // ========================================================================

    /**
     * @brief Run the io_context (blocking)
     *
     * Blocks until all work is complete or stop() is called.
     * Use for dedicated network threads.
     *
     * @return Number of handlers executed
     */
    std::size_t run() { return context_->run(); }

    /**
     * @brief Run one handler if available (blocking)
     *
     * Blocks until one handler is executed or stop() is called.
     *
     * @return 1 if handler executed, 0 otherwise
     */
    std::size_t runOne() { return context_->run_one(); }

    /**
     * @brief Poll for ready handlers (non-blocking)
     *
     * Executes all ready handlers without blocking.
     * Ideal for integration into a game loop.
     *
     * @return Number of handlers executed
     */
    std::size_t poll() { return context_->poll(); }

    /**
     * @brief Poll for one ready handler (non-blocking)
     *
     * Executes at most one ready handler without blocking.
     *
     * @return 1 if handler executed, 0 otherwise
     */
    std::size_t pollOne() { return context_->poll_one(); }

    /**
     * @brief Stop the io_context
     *
     * Causes run() to return as soon as possible.
     * All pending handlers are abandoned (not executed).
     */
    void stop() {
        workGuard_.reset();
        context_->stop();

        if (backgroundThread_.joinable()) {
            backgroundThread_.join();
        }
    }

    /**
     * @brief Check if the io_context has been stopped
     */
    [[nodiscard]] bool stopped() const noexcept { return context_->stopped(); }

    /**
     * @brief Restart the io_context after stop
     *
     * Must be called before run/poll if previously stopped.
     */
    void restart() {
        context_->restart();
        workGuard_.emplace(asio::make_work_guard(*context_));
    }

    // ========================================================================
    // Background Thread Mode
    // ========================================================================

    /**
     * @brief Run io_context in a background thread
     *
     * Spawns a dedicated thread that calls run().
     * The thread runs until stop() is called.
     *
     * @note Only call once. Call stop() before calling again.
     */
    void runInBackground() {
        if (backgroundThread_.joinable()) {
            return;  // Already running
        }

        backgroundThread_ = std::thread([this]() { context_->run(); });
    }

    /**
     * @brief Check if running in background mode
     */
    [[nodiscard]] bool isRunningInBackground() const noexcept {
        return backgroundThread_.joinable();
    }

    // ========================================================================
    // Work Guard Management
    // ========================================================================

    /**
     * @brief Release the work guard
     *
     * Allows the io_context to stop when no work remains.
     * Useful for graceful shutdown.
     */
    void releaseWorkGuard() { workGuard_.reset(); }

   private:
    std::unique_ptr<asio::io_context> context_;
    std::optional<WorkGuard> workGuard_;
    std::thread backgroundThread_;
};

}  // namespace rtype::network
