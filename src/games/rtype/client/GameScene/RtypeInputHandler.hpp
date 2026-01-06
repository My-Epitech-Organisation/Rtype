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
#include <unordered_map>
#include <unordered_set>

#include "../../../../../include/rtype/display/IDisplay.hpp"
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
     * @param event The display event
     * @param keybinds Keyboard bindings
     * @param registry ECS registry (for pause menu toggle)
     * @return true if event was handled
     */
    static bool handleKeyReleasedEvent(
        const ::rtype::display::Event& event,
        std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<ECS::Registry> registry);

    /**
     * @brief Update keyboard state based on press/release events
     *
     * Tracks which keys are currently pressed by listening to both
     * KeyPressed and KeyReleased events. This ensures input is only
     * captured from the focused window.
     *
     * @param event The display event (KeyPressed or KeyReleased)
     */
    static void updateKeyState(const ::rtype::display::Event& event);

    /**
     * @brief Clear all pressed keys state
     */
    static void clearKeyStates();

   private:
    /**
     * @brief Track currently pressed keys (only for focused window)
     *
     * @note Thread-safety: This is accessed only from the main game loop
     * thread. No mutex needed as all input processing is single-threaded.
     * If multi-threaded input handling is added in the future, synchronization
     * will be required.
     */
    static std::unordered_set<::rtype::display::Key> pressedKeys_;
    static std::unordered_map<
        unsigned int, std::unordered_map<::rtype::display::JoystickAxis, float>>
        joystickAxes_;
    static std::unordered_map<unsigned int, std::unordered_set<unsigned int>>
        joystickButtons_;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEINPUTHANDLER_HPP_
