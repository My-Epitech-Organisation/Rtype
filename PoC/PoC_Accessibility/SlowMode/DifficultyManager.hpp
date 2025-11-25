#pragma once

#include <string>

/**
 * @brief Manages difficulty presets based on time scaling
 *
 * Provides accessibility-focused difficulty modes that scale game speed
 * to accommodate different reaction time capabilities.
 */
class DifficultyManager {
public:
    enum class Preset {
        Slow,       // 50% speed - accessibility mode
        Normal,     // 100% speed - standard gameplay
        Fast,       // 150% speed - challenge mode
        Custom      // User-defined speed
    };

    DifficultyManager();

    /**
     * @brief Set difficulty preset
     * @param preset Predefined difficulty level
     * @return Time scale value for the preset
     */
    float setPreset(Preset preset);

    /**
     * @brief Set custom time scale
     * @param scale Custom time scale value (0.3 to 2.0)
     * @return Clamped time scale value
     */
    float setCustomScale(float scale);

    /**
     * @brief Get current preset
     */
    Preset getCurrentPreset() const { return m_currentPreset; }

    /**
     * @brief Get current time scale
     */
    float getCurrentTimeScale() const { return m_currentTimeScale; }

    /**
     * @brief Get preset name as string
     */
    static std::string getPresetName(Preset preset);

    /**
     * @brief Get accessibility description for preset
     */
    static std::string getPresetDescription(Preset preset);

private:
    Preset m_currentPreset;
    float m_currentTimeScale;

    static constexpr float MIN_TIME_SCALE = 0.3f;
    static constexpr float MAX_TIME_SCALE = 2.0f;
};
