/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextInputSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_

#include <cctype>
#include <memory>
#include <optional>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "Components/TextInputComponent.hpp"
#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System for handling text input fields.
 *
 * Manages focus, keyboard input, and rendering of TextInput components.
 */
class TextInputSystem {
   public:
    explicit TextInputSystem(std::shared_ptr<sf::RenderWindow> window);

    /**
     * @brief Handle a keyboard event for text inputs
     * @param registry ECS registry
     * @param event The SFML event
     * @return true if the event was consumed by a text input
     */
    bool handleEvent(ECS::Registry& registry, const sf::Event& event);

    /**
     * @brief Update and render text inputs
     */
    void update(ECS::Registry& registry, float deltaTime);

    /**
     * @brief Get the currently focused input entity
     */
    [[nodiscard]] std::optional<ECS::Entity> getFocusedInput() const;

   private:
    void handleClick(ECS::Registry& registry, float mouseX, float mouseY);
    bool handleTextEntered(ECS::Registry& registry, std::uint32_t unicode);
    bool handleKeyPressed(ECS::Registry& registry, sf::Keyboard::Key key);

    std::shared_ptr<sf::RenderWindow> _window;
    std::optional<ECS::Entity> _focusedInput;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_
