/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextInputSystem.cpp
*/

#include "TextInputSystem.hpp"

namespace rtype::games::rtype::client {

TextInputSystem::TextInputSystem(std::shared_ptr<sf::RenderWindow> window)
    : _window(std::move(window)) {}

bool TextInputSystem::handleEvent(ECS::Registry& registry,
                                  const sf::Event& event) {
    if (auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            handleClick(registry, static_cast<float>(mousePressed->position.x),
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

void TextInputSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    auto view = registry.view<TextInput, shared::Position, TextInputTag>();

    view.each(
        [this](auto /*entity*/, TextInput& input, shared::Position& pos, auto) {
            input.background.setPosition({pos.x, pos.y});
            input.text.setPosition({pos.x + 10.f, pos.y + 5.f});
            _window->draw(input.background);
            _window->draw(input.text);
        });
}

std::optional<ECS::Entity> TextInputSystem::getFocusedInput() const {
    return _focusedInput;
}

void TextInputSystem::handleClick(ECS::Registry& registry, float mouseX,
                                  float mouseY) {
    auto view = registry.view<TextInput, shared::Position, TextInputTag>();
    view.each(
        [](auto, TextInput& input, auto, auto) { input.setFocus(false); });
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

bool TextInputSystem::handleTextEntered(ECS::Registry& registry,
                                        std::uint32_t unicode) {
    if (!_focusedInput.has_value()) return false;

    auto& input = registry.getComponent<TextInput>(*_focusedInput);
    if (unicode >= 32 && unicode < 127) {
        return input.handleTextInput(static_cast<char>(unicode));
    }
    return false;
}

bool TextInputSystem::handleKeyPressed(ECS::Registry& registry,
                                       sf::Keyboard::Key key) {
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

}  // namespace rtype::games::rtype::client
