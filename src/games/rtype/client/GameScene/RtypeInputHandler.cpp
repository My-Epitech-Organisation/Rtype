/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeInputHandler.cpp
*/

#include "RtypeInputHandler.hpp"

#include <SFML/Window/Keyboard.hpp>

#include "GameAction.hpp"
#include "RtypePauseMenu.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

std::uint8_t RtypeInputHandler::getInputMask(
    std::shared_ptr<KeyboardActions> keybinds) {
    std::uint8_t inputMask = ::rtype::network::InputMask::kNone;

    auto keyMoveUp = keybinds->getKeyBinding(GameAction::MOVE_UP);
    if (keyMoveUp.has_value() && sf::Keyboard::isKeyPressed(*keyMoveUp)) {
        inputMask |= ::rtype::network::InputMask::kUp;
    }

    auto keyMoveDown = keybinds->getKeyBinding(GameAction::MOVE_DOWN);
    if (keyMoveDown.has_value() && sf::Keyboard::isKeyPressed(*keyMoveDown)) {
        inputMask |= ::rtype::network::InputMask::kDown;
    }

    auto keyMoveLeft = keybinds->getKeyBinding(GameAction::MOVE_LEFT);
    if (keyMoveLeft.has_value() && sf::Keyboard::isKeyPressed(*keyMoveLeft)) {
        inputMask |= ::rtype::network::InputMask::kLeft;
    }

    auto keyMoveRight = keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
    if (keyMoveRight.has_value() && sf::Keyboard::isKeyPressed(*keyMoveRight)) {
        inputMask |= ::rtype::network::InputMask::kRight;
    }

    auto keyShoot = keybinds->getKeyBinding(GameAction::SHOOT);
    if (keyShoot.has_value() && sf::Keyboard::isKeyPressed(*keyShoot)) {
        inputMask |= ::rtype::network::InputMask::kShoot;
    }

    return inputMask;
}

bool RtypeInputHandler::handleKeyReleasedEvent(
    const sf::Event& event, std::shared_ptr<KeyboardActions> keybinds,
    std::shared_ptr<ECS::Registry> registry) {
    auto eventKeyRelease = event.getIf<sf::Event::KeyReleased>();
    auto keyPause = keybinds->getKeyBinding(GameAction::PAUSE);

    if (!eventKeyRelease || !keyPause.has_value()) {
        return false;
    }

    if (eventKeyRelease->code == *keyPause) {
        RtypePauseMenu::togglePauseMenu(registry);
        return true;
    }

    return false;
}

}  // namespace rtype::games::rtype::client
