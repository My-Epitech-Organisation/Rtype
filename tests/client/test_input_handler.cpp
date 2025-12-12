/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RtypeInputHandler pause handling and branches
*/

#include <gtest/gtest.h>

#include <memory>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "games/rtype/client/GameScene/RtypeInputHandler.hpp"
#include "client/Graphic/KeyboardActions.hpp"
#include "games/rtype/client/GameScene/RtypePauseMenu.hpp"
#include "protocol/Payloads.hpp"
#include "ECS.hpp"

namespace client = rtype::games::rtype::client;

// Helper to create a registry expected by pause menu
static std::shared_ptr<ECS::Registry> makeRegistry() {
    return std::make_shared<ECS::Registry>();
}

TEST(RtypeInputHandlerTest, HandleKeyReleasedActivatesPauseMenu) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setKeyBinding(GameAction::PAUSE, sf::Keyboard::Key::Escape);
    auto registry = makeRegistry();

    sf::Event::KeyReleased kr;
    kr.code = sf::Keyboard::Key::Escape;
    sf::Event ev(kr);

    EXPECT_TRUE(client::RtypeInputHandler::handleKeyReleasedEvent(ev, keybinds, registry));
}

TEST(RtypeInputHandlerTest, HandleKeyReleasedWrongKeyDoesNothing) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setKeyBinding(GameAction::PAUSE, sf::Keyboard::Key::Escape);
    auto registry = makeRegistry();

    sf::Event::KeyReleased kr;
    kr.code = sf::Keyboard::Key::Space; // not the pause key
    sf::Event ev(kr);

    EXPECT_FALSE(client::RtypeInputHandler::handleKeyReleasedEvent(ev, keybinds, registry));
}

TEST(RtypeInputHandlerTest, HandleJoystickButtonReleasedActivatesPauseMenu) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setJoyButtonBinding(GameAction::PAUSE, 7u);
    auto registry = makeRegistry();

    sf::Event::JoystickButtonReleased jbr;
    jbr.joystickId = 0;
    jbr.button = 7u;
    sf::Event ev(jbr);

    EXPECT_TRUE(client::RtypeInputHandler::handleKeyReleasedEvent(ev, keybinds, registry));
}

TEST(RtypeInputHandlerTest, HandleJoystickButtonReleasedWrongButtonDoesNothing) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setJoyButtonBinding(GameAction::PAUSE, 7u);
    auto registry = makeRegistry();

    sf::Event::JoystickButtonReleased jbr;
    jbr.joystickId = 0;
    jbr.button = 3u; // not the pause button
    sf::Event ev(jbr);

    EXPECT_FALSE(client::RtypeInputHandler::handleKeyReleasedEvent(ev, keybinds, registry));
}

TEST(RtypeInputHandlerTest, GetInputMaskKeyboardNoKeysPressed) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setInputMode(InputMode::Keyboard);

    keybinds->setKeyBinding(GameAction::MOVE_UP, sf::Keyboard::Key::Up);
    keybinds->setKeyBinding(GameAction::MOVE_DOWN, sf::Keyboard::Key::Down);
    keybinds->setKeyBinding(GameAction::MOVE_LEFT, sf::Keyboard::Key::Left);
    keybinds->setKeyBinding(GameAction::MOVE_RIGHT, sf::Keyboard::Key::Right);
    keybinds->setKeyBinding(GameAction::SHOOT, sf::Keyboard::Key::Space);

    auto mask = client::RtypeInputHandler::getInputMask(keybinds);

    EXPECT_EQ(mask, ::rtype::network::InputMask::kNone);
}

TEST(RtypeInputHandlerTest, GetInputMaskControllerNoJoystickConnected) {
    auto keybinds = std::make_shared<KeyboardActions>();
    keybinds->setInputMode(InputMode::Controller);

    auto mask = client::RtypeInputHandler::getInputMask(keybinds);

    EXPECT_EQ(mask, ::rtype::network::InputMask::kNone);
}
