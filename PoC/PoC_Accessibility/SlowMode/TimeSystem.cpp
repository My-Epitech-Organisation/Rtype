#include "TimeSystem.hpp"
#include <algorithm>

TimeSystem::TimeSystem()
    : m_lastFrameTime(Clock::now())
    , m_rawDeltaTime(0.0f)
    , m_scaledDeltaTime(0.0f)
    , m_globalTimeScale(1.0f)
    , m_totalScaledTime(0.0f)
{
}

void TimeSystem::update() {
    TimePoint currentTime = Clock::now();
    Duration elapsed = currentTime - m_lastFrameTime;

    m_rawDeltaTime = elapsed.count();
    m_scaledDeltaTime = m_rawDeltaTime * m_globalTimeScale;
    m_totalScaledTime += m_scaledDeltaTime;

    m_lastFrameTime = currentTime;
}

void TimeSystem::setGlobalTimeScale(float scale) {
    // Clamp to reasonable values (0.1 to 3.0)
    m_globalTimeScale = std::clamp(scale, 0.1f, 3.0f);
}
