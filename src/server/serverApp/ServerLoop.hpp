/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerLoop - Game loop timing management
*/

#ifndef SRC_SERVER_SERVERAPP_SERVERLOOP_HPP_
#define SRC_SERVER_SERVERAPP_SERVERLOOP_HPP_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

namespace rtype::server {

/**
 * @brief Configuration for the server loop timing
 */
struct LoopTiming {
    std::chrono::nanoseconds fixedDeltaNs;
    std::chrono::nanoseconds maxFrameTime;
    uint32_t maxUpdatesPerFrame;
};

/**
 * @brief State for the main loop
 */
struct LoopState {
    std::chrono::steady_clock::time_point previousTime;
    std::chrono::nanoseconds accumulator{0};
};

/**
 * @brief Manages the game loop timing for the server
 *
 * This class encapsulates the fixed timestep game loop logic,
 * handling frame timing, accumulator management, and sleep timing.
 *
 * Features:
 * - Fixed timestep updates with accumulator pattern
 * - Frame time clamping to prevent spiral of death
 * - Configurable tick rate
 * - Tick overrun detection
 *
 * Usage:
 * @code
 * ServerLoop loop(60, shutdownFlag);
 * loop.run([&](float deltaTime) {
 *     // Game update logic
 * });
 * @endcode
 */
class ServerLoop {
   public:
    /**
     * @brief Maximum physics/logic updates per frame to prevent spiral of death
     */
    static constexpr uint32_t MAX_UPDATES_PER_FRAME = 5;

    /**
     * @brief Maximum frame time in milliseconds before clamping
     */
    static constexpr uint32_t MAX_FRAME_TIME_MS = 250;

    /**
     * @brief Percentage of calculated sleep time to actually sleep
     */
    static constexpr uint32_t SLEEP_TIME_SAFETY_PERCENT = 95;

    /**
     * @brief Minimum sleep threshold in microseconds
     */
    static constexpr uint32_t MIN_SLEEP_THRESHOLD_US = 100;

    /**
     * @brief Callback type for frame updates
     *
     * Called once per frame before fixed updates.
     */
    using FrameCallback = std::function<void()>;

    /**
     * @brief Callback type for fixed timestep updates
     *
     * Called at fixed intervals (tickRate times per second).
     *
     * @param deltaTime Fixed delta time in seconds (1.0 / tickRate)
     */
    using UpdateCallback = std::function<void(float deltaTime)>;

    /**
     * @brief Callback type for post-update operations
     *
     * Called once per frame after all fixed updates.
     */
    using PostUpdateCallback = std::function<void()>;

    /**
     * @brief Construct a ServerLoop
     *
     * @param tickRate Server tick rate in Hz (must be > 0)
     * @param shutdownFlag Shared pointer to external shutdown flag
     * @throws std::invalid_argument if tickRate is 0
     */
    ServerLoop(uint32_t tickRate,
               std::shared_ptr<std::atomic<bool>> shutdownFlag);

    ~ServerLoop() = default;

    ServerLoop(const ServerLoop&) = delete;
    ServerLoop& operator=(const ServerLoop&) = delete;
    ServerLoop(ServerLoop&&) = delete;
    ServerLoop& operator=(ServerLoop&&) = delete;

    /**
     * @brief Run the game loop
     *
     * This is a blocking call that runs until the shutdown flag is set.
     *
     * @param frameCallback Called once per frame (for input/network processing)
     * @param updateCallback Called at fixed timestep intervals
     * @param postUpdateCallback Called once per frame after updates
     */
    void run(FrameCallback frameCallback, UpdateCallback updateCallback,
             PostUpdateCallback postUpdateCallback);

    /**
     * @brief Get the tick rate
     *
     * @return Tick rate in Hz
     */
    [[nodiscard]] uint32_t getTickRate() const noexcept { return _tickRate; }

    /**
     * @brief Get the fixed delta time
     *
     * @return Delta time in seconds
     */
    [[nodiscard]] float getDeltaTime() const noexcept {
        return 1.0F / static_cast<float>(_tickRate);
    }

    /**
     * @brief Get tick overrun count
     *
     * @return Number of frames where time was clamped
     */
    [[nodiscard]] uint64_t getTickOverruns() const noexcept {
        return _tickOverruns.load(std::memory_order_relaxed);
    }

    /**
     * @brief Get loop timing configuration
     *
     * @return Loop timing configuration
     */
    [[nodiscard]] LoopTiming getLoopTiming() const noexcept;

   private:
    /**
     * @brief Calculate frame time and clamp if necessary
     */
    [[nodiscard]] std::chrono::nanoseconds calculateFrameTime(
        LoopState& state, const LoopTiming& timing) noexcept;

    /**
     * @brief Sleep to maintain target frame rate
     */
    void sleepUntilNextFrame(
        std::chrono::steady_clock::time_point frameStartTime,
        const LoopTiming& timing) noexcept;

    uint32_t _tickRate;
    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
    std::atomic<uint64_t> _tickOverruns{0};
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_SERVERLOOP_HPP_
