/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextInputSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_

#include <memory>
#include <optional>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "Components/PositionComponent.hpp"
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
    explicit TextInputSystem(std::shared_ptr<sf::RenderWindow> window)
        : _window(std::move(window)) {}

    /**
     * @brief Handle a keyboard event for text inputs
     * @param registry ECS registry
     * @param event The SFML event
     * @return true if the event was consumed by a text input
     */
    bool handleEvent(ECS::Registry& registry, const sf::Event& event) {
        if (auto* mousePressed =
                event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                handleClick(registry,
                            static_cast<float>(mousePressed->position.x),
                            static_cast<float>(mousePressed->position.y));
                return true;
            }
        }
        if (auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            return handleTextEntered(registry, textEntered->unicode);
        }
        if (auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            return handleKeyPressed(registry, keyPressed->code);
        }

        return false;
    }

    /**
     * @brief Update and render text inputs
     */
    void update(ECS::Registry& registry, float /*deltaTime*/) {
        auto view =
            registry.view<TextInput, shared::Position, TextInputTag>();

        view.each([this](auto /*entity*/, TextInput& input,
                         shared::Position& pos, auto) {
            input.background.setPosition({pos.x, pos.y});
            input.text.setPosition({pos.x + 10.f, pos.y + 5.f});
            _window->draw(input.background);
            _window->draw(input.text);
        });
    }

    /**
     * @brief Get the currently focused input entity
     */
    [[nodiscard]] std::optional<ECS::Entity> getFocusedInput() const {
        return _focusedInput;
    }

   private:
    void handleClick(ECS::Registry& registry, float mouseX, float mouseY) {
        auto view =
            registry.view<TextInput, shared::Position, TextInputTag>();
        view.each([](auto, TextInput& input, auto, auto) {
            input.setFocus(false);
        });
        _focusedInput = std::nullopt;
        view.each([this, mouseX, mouseY](ECS::Entity entity, TextInput& input,
                                         shared::Position& pos, auto) {
            sf::FloatRect bounds = input.background.getGlobalBounds();
            bounds.position = {pos.x, pos.y};

            if (bounds.contains({mouseX, mouseY})) {
                input.setFocus(true);
                _focusedInput = entity;
            }
        });
    }

    bool handleTextEntered(ECS::Registry& registry, std::uint32_t unicode) {
        if (!_focusedInput.has_value()) return false;

        auto& input = registry.getComponent<TextInput>(*_focusedInput);
        if (unicode >= 32 && unicode < 127) {
            return input.handleTextInput(static_cast<char>(unicode));
        }
        return false;
    }

    bool handleKeyPressed(ECS::Registry& registry, sf::Keyboard::Key key) {
        if (!_focusedInput.has_value()) return false;

        auto& input = registry.getComponent<TextInput>(*_focusedInput);

        if (key == sf::Keyboard::Key::Backspace) {
            input.handleBackspace();
            return true;
        }

        if (key == sf::Keyboard::Key::Enter) {
            if (input.onSubmit) {
                input.onSubmit(input.content);
            }
            return true;
        }

        if (key == sf::Keyboard::Key::Tab) {
            return true;
        }

        if (key == sf::Keyboard::Key::Escape) {
            input.setFocus(false);
            _focusedInput = std::nullopt;
            return true;
        }

        return false;
    }

    std::shared_ptr<sf::RenderWindow> _window;
    std::optional<ECS::Entity> _focusedInput;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_TEXTINPUTSYSTEM_HPP_
