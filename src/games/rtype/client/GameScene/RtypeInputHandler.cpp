/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeInputHandler.cpp
*/

#include "RtypeInputHandler.hpp"

#include <algorithm>
#include <memory>

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "GameAction.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "Logger/Macros.hpp"
#include "RtypePauseMenu.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

std::uint8_t RtypeInputHandler::getInputMask(
    std::shared_ptr<KeyboardActions> keybinds) {
    sf::Joystick::update();
    ControllerRumble::update();

    std::uint8_t inputMask = ::rtype::network::InputMask::kNone;

    InputMode mode = keybinds->getInputMode();

    if (mode == InputMode::Keyboard) {
        auto keyMoveUp = keybinds->getKeyBinding(GameAction::MOVE_UP);
        if (keyMoveUp.has_value() && sf::Keyboard::isKeyPressed(*keyMoveUp)) {
            inputMask |= ::rtype::network::InputMask::kUp;
        }

        auto keyMoveDown = keybinds->getKeyBinding(GameAction::MOVE_DOWN);
        if (keyMoveDown.has_value() &&
            sf::Keyboard::isKeyPressed(*keyMoveDown)) {
            inputMask |= ::rtype::network::InputMask::kDown;
        }

        auto keyMoveLeft = keybinds->getKeyBinding(GameAction::MOVE_LEFT);
        if (keyMoveLeft.has_value() &&
            sf::Keyboard::isKeyPressed(*keyMoveLeft)) {
            inputMask |= ::rtype::network::InputMask::kLeft;
        }

        auto keyMoveRight = keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
        if (keyMoveRight.has_value() &&
            sf::Keyboard::isKeyPressed(*keyMoveRight)) {
            inputMask |= ::rtype::network::InputMask::kRight;
        }

        auto keyShoot = keybinds->getKeyBinding(GameAction::SHOOT);
        if (keyShoot.has_value() && sf::Keyboard::isKeyPressed(*keyShoot)) {
            inputMask |= ::rtype::network::InputMask::kShoot;
        }
    } else {
        std::optional<unsigned int> jid;
        for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
            if (sf::Joystick::isConnected(i)) {
                jid = i;
                break;
            }
        }

        if (jid.has_value()) {
            const float deadZone = 30.f;

            float x =
                sf::Joystick::getAxisPosition(*jid, sf::Joystick::Axis::X);
            float y =
                sf::Joystick::getAxisPosition(*jid, sf::Joystick::Axis::Y);

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

            bool shootPressed =
                sf::Joystick::isButtonPressed(*jid, shootBtnNum);

            if (shootPressed) {
                inputMask |= ::rtype::network::InputMask::kShoot;
            }

            static bool lastShootState = false;
            static auto lastRumbleTime = std::chrono::steady_clock::now();
            const auto minRumbleInterval = std::chrono::milliseconds(200);

            auto now = std::chrono::steady_clock::now();
            if (shootPressed && !lastShootState &&
                (now - lastRumbleTime) >= minRumbleInterval) {
                ControllerRumble::shootPulse(*jid);
                lastRumbleTime = now;
            }
            lastShootState = shootPressed;
        }
    }

    return inputMask;
}

bool RtypeInputHandler::handleKeyReleasedEvent(
    const sf::Event& event, std::shared_ptr<KeyboardActions> keybinds,
    std::shared_ptr<ECS::Registry> registry) {
    auto eventKeyRelease = event.getIf<sf::Event::KeyReleased>();
    auto keyPause = keybinds->getKeyBinding(GameAction::PAUSE);

    if (eventKeyRelease && keyPause.has_value() &&
        eventKeyRelease->code == *keyPause) {
        RtypePauseMenu::togglePauseMenu(registry);
        return true;
    }

    if (const auto* joyButton =
            event.getIf<sf::Event::JoystickButtonReleased>()) {
        auto pauseBtn = keybinds->getJoyButtonBinding(GameAction::PAUSE);
        if (pauseBtn.has_value() && joyButton->button == *pauseBtn) {
            RtypePauseMenu::togglePauseMenu(registry);
            return true;
        }
    }

    return false;
}

}  // namespace rtype::games::rtype::client
