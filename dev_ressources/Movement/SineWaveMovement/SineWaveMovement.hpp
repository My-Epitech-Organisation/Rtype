/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sine Wave Movement PoC
*/

#ifndef SINE_WAVE_MOVEMENT_HPP
    #define SINE_WAVE_MOVEMENT_HPP

#include <cmath>

namespace Movement {

    /**
     * @brief Position component for entities in 2D space
     */
    struct Position {
        float x = 0.0f;
        float y = 0.0f;

        Position() = default;
        Position(float x_, float y_) : x(x_), y(y_) {}
    };

    /**
     * @brief Sine wave parameters for oscillating movement
     */
    struct SineWave {
        float centerY = 0.0f;      // Y center position
        float frequency = 1.0f;    // Oscillation frequency
        float amplitude = 1.0f;    // Oscillation amplitude
        float phase = 0.0f;        // Phase offset (for starting at different points)
        float horizontalSpeed = 0.0f; // Speed of horizontal movement

        SineWave() = default;
        SineWave(float center, float freq, float amp, float hSpeed, float ph = 0.0f)
            : centerY(center), frequency(freq), amplitude(amp), 
              horizontalSpeed(hSpeed), phase(ph) {}
    };

    /**
     * @brief Time accumulator for sine wave calculation
     */
    struct SineTime {
        float elapsed = 0.0f;

        SineTime() = default;
        SineTime(float t) : elapsed(t) {}
    };

    /**
     * @brief Sine wave movement system
     * Formula: y = center + sin(x * freq + phase) * amp
     * 
     * This creates smooth oscillating patterns, suitable for:
     * - Enemy wave patterns (like classic space shooters)
     * - Floating/bobbing objects
     * - Power-ups with visual appeal
     * - Snake-like movement patterns
     */
    class SineWaveMovementSystem {
    public:
        /**
         * @brief Update all entities with sine wave movement
         * @param registry ECS registry
         * @param deltaTime Time elapsed since last frame
         */
        template<typename Registry>
        static void update(Registry& registry, float deltaTime) {
            registry.template view<Position, SineWave, SineTime>().each([deltaTime](
                auto entity, Position& pos, const SineWave& wave, SineTime& time
            ) {
                // Update time
                time.elapsed += deltaTime;

                // Move horizontally
                pos.x += wave.horizontalSpeed * deltaTime;

                // Calculate sine wave position
                // Formula: y = center + sin(time * freq + phase) * amp
                pos.y = wave.centerY + std::sin(time.elapsed * wave.frequency + wave.phase) * wave.amplitude;
            });
        }
    };

} // namespace Movement

#endif // SINE_WAVE_MOVEMENT_HPP
