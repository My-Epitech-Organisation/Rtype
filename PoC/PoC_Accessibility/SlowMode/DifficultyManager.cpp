#include "DifficultyManager.hpp"
#include <algorithm>

DifficultyManager::DifficultyManager()
    : m_currentPreset(Preset::Normal)
    , m_currentTimeScale(1.0f)
{
}

float DifficultyManager::setPreset(Preset preset) {
    m_currentPreset = preset;

    switch (preset) {
        case Preset::Slow:
            m_currentTimeScale = 0.5f;
            break;
        case Preset::Normal:
            m_currentTimeScale = 1.0f;
            break;
        case Preset::Fast:
            m_currentTimeScale = 1.5f;
            break;
        case Preset::Custom:
            // Keep current custom scale
            break;
    }

    return m_currentTimeScale;
}

float DifficultyManager::setCustomScale(float scale) {
    m_currentPreset = Preset::Custom;
    m_currentTimeScale = std::clamp(scale, MIN_TIME_SCALE, MAX_TIME_SCALE);
    return m_currentTimeScale;
}

std::string DifficultyManager::getPresetName(Preset preset) {
    switch (preset) {
        case Preset::Slow:   return "Slow Mode";
        case Preset::Normal: return "Normal";
        case Preset::Fast:   return "Fast";
        case Preset::Custom: return "Custom";
        default:             return "Unknown";
    }
}

std::string DifficultyManager::getPresetDescription(Preset preset) {
    switch (preset) {
        case Preset::Slow:
            return "50% speed - Beginner/Accessibility mode for players with slower reaction times";
        case Preset::Normal:
            return "100% speed - Standard R-Type gameplay experience";
        case Preset::Fast:
            return "150% speed - Challenge mode for experienced players";
        case Preset::Custom:
            return "User-defined speed (30% to 200%)";
        default:
            return "";
    }
}
