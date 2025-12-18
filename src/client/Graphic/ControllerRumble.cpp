/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ControllerRumble.cpp - Controller haptic feedback implementation
*/

#include "ControllerRumble.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <SFML/Window/Joystick.hpp>

#include "Logger/Macros.hpp"

std::map<unsigned int, std::chrono::steady_clock::time_point>
    ControllerRumble::_rumbleEndTimes;
std::map<unsigned int, SDL_GameController*> ControllerRumble::_controllers;
bool ControllerRumble::_sdlInitialized = false;

void ControllerRumble::triggerRumble(unsigned int joystickId, float intensity,
                                     int durationMs) {
    if (!_sdlInitialized) {
        LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] First rumble call - initializing SDL2...");
        initialize();
    }

    if (!sf::Joystick::isConnected(joystickId)) {
        LOG_WARNING_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Joystick " +
                    std::to_string(joystickId) + " not connected!");
        return;
    }

    SDL_GameController* controller = nullptr;
    if (_controllers.find(joystickId) != _controllers.end()) {
        controller = _controllers[joystickId];
    }

    if (!controller) {
        LOG_WARNING_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Controller " +
                    std::to_string(joystickId) +
                    " is not an Xbox controller or doesn't support rumble (not "
                    "in SDL game controller DB)");
        return;
    }

    auto endTime = std::chrono::steady_clock::now() +
                   std::chrono::milliseconds(std::max(0, durationMs));
    _rumbleEndTimes[joystickId] = endTime;

    float clampedIntensity = std::clamp(intensity, 0.0f, 1.0f);
    Uint16 rumbleStrength = static_cast<Uint16>(clampedIntensity * 65535);
    LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Calling SDL_GameControllerRumble(id=" +
             std::to_string(joystickId) +
             ", strength=" + std::to_string(rumbleStrength) +
             ", duration=" + std::to_string(std::max(0, durationMs)) + "ms)");

    int result = SDL_GameControllerRumble(
        controller, rumbleStrength, rumbleStrength, std::max(0, durationMs));
    if (result == 0) {
        LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] ✓ Rumble triggered successfully!");
    } else {
        LOG_WARNING_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Failed to trigger rumble: " +
                    std::string(SDL_GetError()));
    }
}

void ControllerRumble::stopRumble(unsigned int joystickId) {
    if (_rumbleEndTimes.find(joystickId) != _rumbleEndTimes.end()) {
        _rumbleEndTimes.erase(joystickId);

        if (_controllers.find(joystickId) != _controllers.end()) {
            SDL_GameController* controller = _controllers[joystickId];
            if (controller) {
                SDL_GameControllerRumble(controller, 0, 0, 0);
            }
        }

        LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Stopped rumble on controller " +
                  std::to_string(joystickId));
    }
}

bool ControllerRumble::isRumbling(unsigned int joystickId) {
    if (_rumbleEndTimes.find(joystickId) == _rumbleEndTimes.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    if (now >= _rumbleEndTimes[joystickId]) {
        _rumbleEndTimes.erase(joystickId);
        return false;
    }

    return true;
}

void ControllerRumble::update() {
    if (!_sdlInitialized || _rumbleEndTimes.empty()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    std::vector<unsigned int> toStop;
    toStop.reserve(_rumbleEndTimes.size());

    for (const auto& entry : _rumbleEndTimes) {
        if (now >= entry.second) {
            toStop.push_back(entry.first);
        }
    }

    for (unsigned int id : toStop) {
        auto itCtrl = _controllers.find(id);
        if (itCtrl != _controllers.end() && itCtrl->second) {
            SDL_GameControllerRumble(itCtrl->second, 0, 0, 0);
        }
        _rumbleEndTimes.erase(id);
        LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Auto-stopped rumble on controller " +
                  std::to_string(id));
    }
}

void ControllerRumble::initialize() {
    if (_sdlInitialized) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Already initialized");
        return;
    }

    LOG_INFO_CAT(::rtype::LogCategory::Input,
        "[ControllerRumble] Initializing SDL2 GameController subsystem...");

    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        LOG_ERROR_CAT(::rtype::LogCategory::Input,
            "[ControllerRumble] Failed to initialize SDL GameController: " +
            std::string(SDL_GetError()));
        return;
    }

    _sdlInitialized = true;
    LOG_INFO_CAT(::rtype::LogCategory::Input,
        "[ControllerRumble] SDL2 initialized successfully for Xbox controller "
        "rumble!");

    int numJoysticks = SDL_NumJoysticks();
    LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Found " + std::to_string(numJoysticks) +
             " joystick(s)");

    for (int i = 0; i < numJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* ctrl = SDL_GameControllerOpen(i);
            if (ctrl) {
                const char* name = SDL_GameControllerName(ctrl);
                LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Opened controller " +
                         std::to_string(i) + ": " +
                         std::string(name ? name : "Unknown"));
                _controllers[i] = ctrl;
            } else {
                LOG_WARNING_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Failed to open controller " +
                            std::to_string(i) + ": " +
                            std::string(SDL_GetError()));
            }
        } else {
            LOG_WARNING_CAT(::rtype::LogCategory::Input, "[ControllerRumble] Joystick " + std::to_string(i) +
                        " is not a game controller");
        }
    }
}

void ControllerRumble::cleanup() {
    for (auto& pair : _controllers) {
        if (pair.second) {
            SDL_GameControllerRumble(pair.second, 0, 0, 0);
            SDL_GameControllerClose(pair.second);
        }
    }
    _controllers.clear();
    _rumbleEndTimes.clear();

    if (_sdlInitialized) {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        _sdlInitialized = false;
        LOG_INFO_CAT(::rtype::LogCategory::Input, "[ControllerRumble] SDL2 cleaned up");
    }
}
