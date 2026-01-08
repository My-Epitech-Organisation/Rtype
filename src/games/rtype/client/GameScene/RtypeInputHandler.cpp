/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeInputHandler.cpp
*/

#include "RtypeInputHandler.hpp"

#include <algorithm>
#include <map>
#include <memory>

#include "GameAction.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "Logger/Macros.hpp"
#include "RtypePauseMenu.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

std::unordered_set<::rtype::display::Key> RtypeInputHandler::pressedKeys_;
std::unordered_map<unsigned int,
                   std::unordered_map<::rtype::display::JoystickAxis, float>>
    RtypeInputHandler::joystickAxes_;
std::unordered_map<unsigned int, std::unordered_set<unsigned int>>
    RtypeInputHandler::joystickButtons_;

std::uint8_t RtypeInputHandler::getInputMask(
    std::shared_ptr<KeyboardActions> keybinds) {
    ControllerRumble::update();

    std::uint8_t inputMask = ::rtype::network::InputMask::kNone;

    InputMode mode = keybinds->getInputMode();

    if (mode == InputMode::Keyboard) {
        auto keyMoveUp = keybinds->getKeyBinding(GameAction::MOVE_UP);
        if (keyMoveUp.has_value() && pressedKeys_.contains(*keyMoveUp)) {
            inputMask |= ::rtype::network::InputMask::kUp;
        }

        auto keyMoveDown = keybinds->getKeyBinding(GameAction::MOVE_DOWN);
        if (keyMoveDown.has_value() && pressedKeys_.contains(*keyMoveDown)) {
            inputMask |= ::rtype::network::InputMask::kDown;
        }

        auto keyMoveLeft = keybinds->getKeyBinding(GameAction::MOVE_LEFT);
        if (keyMoveLeft.has_value() && pressedKeys_.contains(*keyMoveLeft)) {
            inputMask |= ::rtype::network::InputMask::kLeft;
        }

        auto keyMoveRight = keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
        if (keyMoveRight.has_value() && pressedKeys_.contains(*keyMoveRight)) {
            inputMask |= ::rtype::network::InputMask::kRight;
        }

        auto keyShoot = keybinds->getKeyBinding(GameAction::SHOOT);
        if (keyShoot.has_value() && pressedKeys_.contains(*keyShoot)) {
            inputMask |= ::rtype::network::InputMask::kShoot;
        }
    } else {
        std::optional<unsigned int> jid;
        for (unsigned int i = 0; i < 8; ++i) {
            if (joystickAxes_.contains(i) || joystickButtons_.contains(i)) {
                jid = i;
                break;
            }
        }

        if (jid.has_value()) {
            const float deadZone = 30.f;

            float x = joystickAxes_[*jid][::rtype::display::JoystickAxis::X];
            float y = joystickAxes_[*jid][::rtype::display::JoystickAxis::Y];

            if (keybinds->isJoyAxisInverted(GameAction::MOVE_UP)) {
                y = -y;
            }
            if (keybinds->isJoyAxisInverted(GameAction::MOVE_LEFT)) {
                x = -x;
            }

            if (y < -deadZone) inputMask |= ::rtype::network::InputMask::kUp;
            if (y > deadZone) inputMask |= ::rtype::network::InputMask::kDown;
            if (x < -deadZone) inputMask |= ::rtype::network::InputMask::kLeft;
            if (x > deadZone) inputMask |= ::rtype::network::InputMask::kRight;

            auto shootBtn = keybinds->getJoyButtonBinding(GameAction::SHOOT);
            unsigned int shootBtnNum = shootBtn.has_value() ? *shootBtn : 0;

            bool shootPressed = joystickButtons_[*jid].contains(shootBtnNum);

            if (shootPressed) {
                inputMask |= ::rtype::network::InputMask::kShoot;
            }

            static std::map<unsigned int, bool> lastShootStates;
            static std::map<unsigned int, std::chrono::steady_clock::time_point>
                lastRumbleTimes;

            if (lastShootStates.find(*jid) == lastShootStates.end()) {
                lastShootStates[*jid] = false;
                lastRumbleTimes[*jid] = std::chrono::steady_clock::now();
            }

            const auto minRumbleInterval = std::chrono::milliseconds(200);

            auto now = std::chrono::steady_clock::now();
            if (shootPressed && !lastShootStates[*jid] &&
                (now - lastRumbleTimes[*jid]) >= minRumbleInterval) {
                ControllerRumble::shootPulse(*jid);
                lastRumbleTimes[*jid] = now;
            }
            lastShootStates[*jid] = shootPressed;
        }
    }

    return inputMask;
}

bool RtypeInputHandler::handleKeyReleasedEvent(
    const ::rtype::display::Event& event,
    std::shared_ptr<KeyboardActions> keybinds,
    std::shared_ptr<ECS::Registry> registry) {
    auto keyPause = keybinds->getKeyBinding(GameAction::PAUSE);

    if (event.type == ::rtype::display::EventType::KeyReleased &&
        keyPause.has_value() && event.key.code == *keyPause) {
        RtypePauseMenu::togglePauseMenu(registry);
        return true;
    }

    if (event.type == ::rtype::display::EventType::JoystickButtonReleased) {
        auto pauseBtn = keybinds->getJoyButtonBinding(GameAction::PAUSE);
        if (pauseBtn.has_value() && event.joystickButton.button == *pauseBtn) {
            RtypePauseMenu::togglePauseMenu(registry);
            return true;
        }
    }

    return false;
}

void RtypeInputHandler::updateKeyState(const ::rtype::display::Event& event) {
    if (event.type == ::rtype::display::EventType::KeyPressed) {
        pressedKeys_.insert(event.key.code);
    } else if (event.type == ::rtype::display::EventType::KeyReleased) {
        pressedKeys_.erase(event.key.code);
    } else if (event.type ==
               ::rtype::display::EventType::JoystickButtonPressed) {
        joystickButtons_[event.joystickButton.joystickId].insert(
            event.joystickButton.button);
    } else if (event.type ==
               ::rtype::display::EventType::JoystickButtonReleased) {
        joystickButtons_[event.joystickButton.joystickId].erase(
            event.joystickButton.button);
    } else if (event.type == ::rtype::display::EventType::JoystickMoved) {
        joystickAxes_[event.joystickMove.joystickId][event.joystickMove.axis] =
            event.joystickMove.position;
    }
}

void RtypeInputHandler::clearKeyStates() {
    pressedKeys_.clear();
    joystickAxes_.clear();
    joystickButtons_.clear();
}

}  // namespace rtype::games::rtype::client
