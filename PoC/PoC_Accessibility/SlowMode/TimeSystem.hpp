#pragma once

#include <chrono>

/**
 * @brief Central Time System managing global time scale for accessibility
 *
 * This system allows slowing down or speeding up the entire game by
 * scaling delta time, which is crucial for players with slower reaction times.
 */
class TimeSystem {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<float>;

    TimeSystem();

    /**
     * @brief Update the time system (call once per frame)
     */
    void update();

    /**
     * @brief Get the raw delta time (unscaled)
     * @return Raw time elapsed since last frame in seconds
     */
    float getRawDeltaTime() const { return m_rawDeltaTime; }

    /**
     * @brief Get the scaled delta time (affected by global time scale)
     * @return Scaled time for gameplay systems in seconds
     */
    float getScaledDeltaTime() const { return m_scaledDeltaTime; }

    /**
     * @brief Get the current global time scale
     * @return Current time scale multiplier
     */
    float getGlobalTimeScale() const { return m_globalTimeScale; }

    /**
     * @brief Set the global time scale
     * @param scale Time scale multiplier (0.5 = 50% speed, 1.0 = normal, 2.0 = 200% speed)
     */
    void setGlobalTimeScale(float scale);

    /**
     * @brief Get total scaled time elapsed since start
     * @return Total scaled time in seconds
     */
    float getTotalScaledTime() const { return m_totalScaledTime; }

private:
    TimePoint m_lastFrameTime;
    float m_rawDeltaTime;
    float m_scaledDeltaTime;
    float m_globalTimeScale;
    float m_totalScaledTime;
};
