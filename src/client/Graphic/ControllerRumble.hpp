/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ControllerRumble.hpp - Controller haptic feedback/rumble support
*/

#ifndef SRC_CLIENT_GRAPHIC_CONTROLLERRUMBLE_HPP_
#define SRC_CLIENT_GRAPHIC_CONTROLLERRUMBLE_HPP_
// clang-format off
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include <chrono>
#include <map>

#include <SFML/Window/Joystick.hpp>
// clang-format on

/**
 * @brief Manages controller rumble/vibration effects
 *
 * Uses SDL2 for Xbox controller rumble support.
 * Note: Only tested with Xbox controllers. Other controllers may not work.
 */
class ControllerRumble {
   private:
    static std::map<unsigned int, std::chrono::steady_clock::time_point>
        _rumbleEndTimes;
    static std::map<unsigned int, SDL_GameController*> _controllers;
    static bool _sdlInitialized;

   public:
    /**
     * @brief Trigger a rumble effect on a controller
     * @param joystickId Joystick index (0-7)
     * @param intensity Strength of rumble (0.0 - 1.0)
     * @param durationMs Duration in milliseconds
     */
    static void triggerRumble(unsigned int joystickId, float intensity,
                              int durationMs);

    /**
     * @brief Stop rumble on a controller
     * @param joystickId Joystick index
     */
    static void stopRumble(unsigned int joystickId);

    /**
     * @brief Check if controller is currently rumbling
     * @param joystickId Joystick index
     * @return true if rumbling
     */
    static bool isRumbling(unsigned int joystickId);

    /**
     * @brief Pump rumble timers and stop finished rumbles
     * Call once per frame to ensure SDL rumble gets cleared.
     */
    static void update();

    /**
     * @brief Initialize SDL2 for controller support
     * Call this once at startup
     */
    static void initialize();

    /**
     * @brief Cleanup SDL2 resources
     * Call this at shutdown
     */
    static void cleanup();

    /**
     * @brief Preset: Light tap (for menu navigation)
     */
    static void lightTap(unsigned int joystickId) {
        triggerRumble(joystickId, 0.3f, 50);
    }

    /**
     * @brief Preset: Medium pulse (for shooting)
     */
    static void shootPulse(unsigned int joystickId) {
        triggerRumble(joystickId, 0.5f, 500);  // 0.5s pulse to signal a shot
    }

    /**
     * @brief Preset: Strong impact (for taking damage)
     */
    static void damageImpact(unsigned int joystickId) {
        triggerRumble(joystickId, 0.8f, 200);
    }

    /**
     * @brief Preset: Intense rumble (for explosions)
     */
    static void explosion(unsigned int joystickId) {
        triggerRumble(joystickId, 1.0f, 300);
    }
};

#endif  // SRC_CLIENT_GRAPHIC_CONTROLLERRUMBLE_HPP_
