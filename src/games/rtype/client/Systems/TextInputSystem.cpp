/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextInputSystem.cpp
*/

#include "TextInputSystem.hpp"

#include <limits>

#include "Components/RectangleComponent.hpp"
#include "Components/UserEventComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

TextInputSystem::TextInputSystem(std::shared_ptr<sf::RenderWindow> window)
    : _window(std::move(window)) {}

bool TextInputSystem::handleEvent(ECS::Registry& registry,
                                  const sf::Event& event) {
    if (auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2f worldPos =
                _window->mapPixelToCoords(mousePressed->position);
            handleClick(registry, worldPos.x, worldPos.y);
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

    ECS::Entity topInput;
    int highestZIndex = std::numeric_limits<int>::min();
    bool foundInput = false;

    view.each([this, mouseX, mouseY, &topInput, &highestZIndex, &foundInput,
               &registry](ECS::Entity entity, TextInput& input,
                          shared::Position& pos, auto) {
        sf::FloatRect bounds = input.background.getGlobalBounds();
        bounds.position = {pos.x, pos.y};

        if (bounds.contains({mouseX, mouseY})) {
            int zIndex = 0;
            if (registry.hasComponent<ZIndex>(entity)) {
                zIndex = registry.getComponent<ZIndex>(entity).depth;
            }
            if (!foundInput || zIndex > highestZIndex) {
                topInput = entity;
                highestZIndex = zIndex;
                foundInput = true;
            }
        }
    });

    if (foundInput) {
        auto interactiveView =
            registry.view<Rectangle, shared::Position, UserEvent>();
        bool blockedByOther = false;

        interactiveView.each([mouseX, mouseY, highestZIndex, &blockedByOther,
                              &registry](ECS::Entity entity, Rectangle& rect,
                                         shared::Position& pos, auto) {
            if (registry.hasComponent<TextInputTag>(entity)) {
                return;
            }
            sf::FloatRect bounds = rect.rectangle.getGlobalBounds();
            bounds.position = {pos.x, pos.y};
            if (bounds.contains({mouseX, mouseY})) {
                int otherZIndex = 0;
                if (registry.hasComponent<ZIndex>(entity)) {
                    otherZIndex = registry.getComponent<ZIndex>(entity).depth;
                }
                if (otherZIndex >= highestZIndex) {
                    blockedByOther = true;
                }
            }
        });
        if (blockedByOther) {
            foundInput = false;
        }
    }

    if (foundInput && registry.isAlive(topInput)) {
        auto& input = registry.getComponent<TextInput>(topInput);
        input.setFocus(true);
        _focusedInput = topInput;
    }
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
