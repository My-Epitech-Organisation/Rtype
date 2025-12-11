/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerLoop - Implementation
*/

#include "ServerLoop.hpp"

#include <stdexcept>
#include <thread>

namespace rtype::server {

ServerLoop::ServerLoop(uint32_t tickRate,
                       std::shared_ptr<std::atomic<bool>> shutdownFlag)
    : _tickRate(tickRate), _shutdownFlag(std::move(shutdownFlag)) {
    if (tickRate == 0) {
        throw std::invalid_argument("tickRate cannot be zero");
    }
}

LoopTiming ServerLoop::getLoopTiming() const noexcept {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;

    const auto fixedDeltaTime =
        duration<double>(1.0 / static_cast<double>(_tickRate));

    return {.fixedDeltaNs = duration_cast<nanoseconds>(fixedDeltaTime),
            .maxFrameTime =
                duration_cast<nanoseconds>(milliseconds(MAX_FRAME_TIME_MS)),
            .maxUpdatesPerFrame = MAX_UPDATES_PER_FRAME};
}

std::chrono::nanoseconds ServerLoop::calculateFrameTime(
    LoopState& state, const LoopTiming& timing) noexcept {
    using std::chrono::duration_cast;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;

    const auto currentTime = steady_clock::now();
    auto frameTime =
        duration_cast<nanoseconds>(currentTime - state.previousTime);
    state.previousTime = currentTime;

    if (frameTime > timing.maxFrameTime) {
        _tickOverruns.fetch_add(1, std::memory_order_relaxed);
        frameTime = timing.maxFrameTime;
    }

    return frameTime;
}

void ServerLoop::sleepUntilNextFrame(
    std::chrono::steady_clock::time_point frameStartTime,
    const LoopTiming& timing) noexcept {
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;

    const auto elapsed = steady_clock::now() - frameStartTime;
    const auto sleepTime = timing.fixedDeltaNs - elapsed;

    if (sleepTime <= nanoseconds{0}) {
        return;
    }

    const auto safeSleepTime =
        duration_cast<nanoseconds>(sleepTime * SLEEP_TIME_SAFETY_PERCENT / 100);

    if (safeSleepTime > microseconds(MIN_SLEEP_THRESHOLD_US)) {
        std::this_thread::sleep_for(safeSleepTime);
    }

    const auto targetTime = frameStartTime + timing.fixedDeltaNs;
    while (steady_clock::now() < targetTime) {
        std::this_thread::yield();
    }
}

void ServerLoop::run(FrameCallback frameCallback, UpdateCallback updateCallback,
                     PostUpdateCallback postUpdateCallback) {
    const auto timing = getLoopTiming();
    LoopState state;
    state.previousTime = std::chrono::steady_clock::now();

    const float deltaTime = getDeltaTime();

    while (!_shutdownFlag->load(std::memory_order_acquire)) {
        const auto frameStartTime = std::chrono::steady_clock::now();

        const auto frameTime = calculateFrameTime(state, timing);
        state.accumulator += frameTime;

        if (frameCallback) {
            frameCallback();
        }

        uint32_t updateCount = 0;
        while (state.accumulator >= timing.fixedDeltaNs &&
               updateCount < timing.maxUpdatesPerFrame) {
            if (updateCallback) {
                updateCallback(deltaTime);
            }
            state.accumulator -= timing.fixedDeltaNs;
            ++updateCount;
        }

        if (updateCount >= timing.maxUpdatesPerFrame &&
            state.accumulator >= timing.fixedDeltaNs) {
            state.accumulator = state.accumulator % timing.fixedDeltaNs;
        }

        if (postUpdateCallback) {
            postUpdateCallback();
        }

        sleepUntilNextFrame(frameStartTime, timing);
    }
}

}  // namespace rtype::server
