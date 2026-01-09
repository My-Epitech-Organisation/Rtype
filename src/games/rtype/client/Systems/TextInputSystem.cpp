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

TextInputSystem::TextInputSystem(
    std::shared_ptr<::rtype::display::IDisplay> display)
    : _display(std::move(display)) {}

bool TextInputSystem::handleEvent(ECS::Registry& registry,
                                  const ::rtype::display::Event& event) {
    if (event.type == ::rtype::display::EventType::MouseButtonPressed) {
        if (event.mouseButton.button == ::rtype::display::MouseButton::Left) {
            auto worldPos = _display->mapPixelToCoords(
                {event.mouseButton.x, event.mouseButton.y});
            handleClick(registry, worldPos.x, worldPos.y);
            return true;
        }
    }
    if (event.type == ::rtype::display::EventType::TextEntered) {
        return handleTextEntered(registry, event.text.unicode);
    }
    if (event.type == ::rtype::display::EventType::KeyPressed) {
        return handleKeyPressed(registry, event.key.code);
    }

    return false;
}

void TextInputSystem::update(ECS::Registry& /*registry*/, float /*deltaTime*/) {
}

std::optional<ECS::Entity> TextInputSystem::getFocusedInput() const {
    return _focusedInput;
}

void TextInputSystem::handleClick(ECS::Registry& registry, float mouseX,
                                  float mouseY) {
    auto view =
        registry.view<TextInput, shared::TransformComponent, TextInputTag>();
    view.each(
        [](auto, TextInput& input, auto, auto) { input.setFocus(false); });
    _focusedInput = std::nullopt;

    ECS::Entity topInput;
    int highestZIndex = std::numeric_limits<int>::min();
    bool foundInput = false;

    view.each([this, mouseX, mouseY, &topInput, &highestZIndex, &foundInput,
               &registry](ECS::Entity entity, TextInput& input,
                          shared::TransformComponent& pos, auto) {
        if (mouseX >= pos.x && mouseX <= pos.x + input.size.x &&
            mouseY >= pos.y && mouseY <= pos.y + input.size.y) {
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
            registry.view<Rectangle, shared::TransformComponent, UserEvent>();
        bool blockedByOther = false;

        interactiveView.each([mouseX, mouseY, highestZIndex, &blockedByOther,
                              &registry](ECS::Entity entity, Rectangle& rect,
                                         shared::TransformComponent& pos,
                                         auto) {
            if (registry.hasComponent<TextInputTag>(entity)) {
                return;
            }
            if (mouseX >= pos.x && mouseX <= pos.x + rect.size.first &&
                mouseY >= pos.y && mouseY <= pos.y + rect.size.second) {
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
                                       ::rtype::display::Key key) {
    if (!_focusedInput.has_value()) return false;

    auto& input = registry.getComponent<TextInput>(*_focusedInput);

    if (key == ::rtype::display::Key::BackSpace) {
        input.handleBackspace();
        return true;
    }

    if (key == ::rtype::display::Key::Return) {
        if (input.onSubmit) {
            input.onSubmit(input.content);
        }
        return true;
    }

    if (key == ::rtype::display::Key::Tab) {
        return true;
    }

    if (key == ::rtype::display::Key::Escape) {
        input.setFocus(false);
        _focusedInput = std::nullopt;
        return true;
    }

    return false;
}

}  // namespace rtype::games::rtype::client
