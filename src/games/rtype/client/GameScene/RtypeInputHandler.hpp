/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeInputHandler.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEINPUTHANDLER_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEINPUTHANDLER_HPP_

#include <cstdint>
#include <memory>
#include <unordered_set>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ECS.hpp"
#include "Graphic/KeyboardActions.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Handles R-Type specific input processing
 */
class RtypeInputHandler {
   public:
    /**
     * @brief Get the current input mask based on pressed keys
     *
     * @param keybinds Keyboard bindings
     * @return Input mask representing current inputs
     */
    static std::uint8_t getInputMask(std::shared_ptr<KeyboardActions> keybinds);

    /**
     * @brief Handle key released events
     *
     * @param event The SFML event
     * @param keybinds Keyboard bindings
     * @param registry ECS registry (for pause menu toggle)
     * @return true if event was handled
     */
    static bool handleKeyReleasedEvent(
        const sf::Event& event, std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<ECS::Registry> registry);

    /**
     * @brief Handle key pressed events to track keyboard state
     *
     * @param event The SFML event
     */
    static void handleKeyPressedEvent(const sf::Event& event);

    /**
     * @brief Clear all pressed keys state
     */
    static void clearKeyStates();

   private:
    // Track currently pressed keys (only for focused window)
    static std::unordered_set<sf::Keyboard::Key> pressedKeys_;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEINPUTHANDLER_HPP_
