/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "../AllComponents.hpp"
#include "AudioLib/AudioLib.hpp"
#include "Components/SoundComponent.hpp"

namespace rtype::games::rtype::client {

EventSystem::EventSystem(std::shared_ptr<sf::RenderWindow> window,
                         std::shared_ptr<AudioLib> audio)
    : ASystem("EventSystem"),
      _window(std::move(window)),
      _audioLib(std::move(audio)) {}

EventSystem::EventSystem(std::shared_ptr<sf::RenderWindow> window,
                         std::shared_ptr<AudioLib> audio,
                         const sf::Event& event)
    : ASystem("EventSystem"),
      _event(event),
      _window(std::move(window)),
      _audioLib(std::move(audio)) {}

void EventSystem::setEvent(const sf::Event& event) { _event = event; }

void EventSystem::clearEvent() { _event.reset(); }

bool EventSystem::_isPointInRect(sf::Vector2i pixelPos,
                                 const Rectangle& rect) const {
    if (!_window) return false;
    sf::Vector2f worldPos = _window->mapPixelToCoords(pixelPos);
    return rect.rectangle.getGlobalBounds().contains(worldPos);
}

void EventSystem::update(ECS::Registry& registry, float /*dt*/) {
    if (!_event.has_value()) return;

    auto buttons = registry.view<Rectangle, UserEvent>();
    std::vector<ECS::Entity> buttonEntities;
    buttons.each([&buttonEntities](auto entity, const auto&, auto&) {
        buttonEntities.push_back(entity);
    });

    if (!buttonEntities.empty()) {
        if (const auto& keyEvent = _event->getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Up ||
                keyEvent->code == sf::Keyboard::Key::Down) {
                _handleMenuNavigation(
                    registry, buttonEntities,
                    keyEvent->code == sf::Keyboard::Key::Down);
            } else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                _handleMenuActivation(registry, buttonEntities);
            }
        } else if (const auto& joyEvent =
                       _event->getIf<sf::Event::JoystickMoved>()) {
            if (joyEvent->axis == sf::Joystick::Axis::Y) {
                if (joyEvent->position > 95.0f) {
                    _handleMenuNavigation(registry, buttonEntities, true);
                } else if (joyEvent->position < -95.0f) {
                    _handleMenuNavigation(registry, buttonEntities, false);
                }
            }
        } else if (const auto& joyBtnEvent =
                       _event->getIf<sf::Event::JoystickButtonPressed>()) {
            if (joyBtnEvent->button == 0) {
                _handleMenuActivation(registry, buttonEntities);
            }
        }
    }

    registry.view<Rectangle, UserEvent>().each(
        [this, &registry](ECS::Entity entity, const Rectangle& rect,
                          UserEvent& actionType) {
            if (registry.hasComponent<HiddenComponent>(entity)) {
                if (registry.getComponent<HiddenComponent>(entity).isHidden) {
                    return;
                }
            }

            bool interaction = false;
            interaction |=
                this->_handleMouseMoved(actionType, rect, registry, entity);
            interaction |=
                this->_handleMousePressed(actionType, rect, registry, entity);
            interaction |= this->_handleMouseReleased(actionType, rect);

            if (interaction || actionType.isHovered || actionType.isPressed) {
                actionType.idle = false;
            }
        });
}

bool EventSystem::_handleMouseMoved(UserEvent& actionType,
                                    const Rectangle& rect, ECS::Registry& reg,
                                    ECS::Entity entt) const {
    const auto* mouseMove = _event->getIf<sf::Event::MouseMoved>();
    if (!mouseMove) return false;

    bool isInside = _isPointInRect(mouseMove->position, rect);
    bool interacted = false;

    if (!actionType.isHovered && isInside) {
        if (reg.hasComponent<ButtonSoundComponent>(entt)) {
            const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
            if (_audioLib && data.hoverSFX) {
                _audioLib->playSFX(*data.hoverSFX);
            }
        }
        interacted = true;
    }

    if (!isInside && actionType.isPressed) {
        actionType.isPressed = false;
        interacted = true;
    }

    actionType.isHovered = isInside;
    return interacted || isInside;
}

bool EventSystem::_handleMousePressed(UserEvent& actionType,
                                      const Rectangle& rect, ECS::Registry& reg,
                                      ECS::Entity entt) const {
    const auto* mousePress = _event->getIf<sf::Event::MouseButtonPressed>();
    if (!mousePress) return false;

    if (mousePress->button == sf::Mouse::Button::Left &&
        _isPointInRect(mousePress->position, rect)) {
        actionType.isPressed = true;

        if (reg.hasComponent<ButtonSoundComponent>(entt)) {
            const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
            if (_audioLib && data.clickSFX) {
                _audioLib->playSFX(*data.clickSFX);
            }
        }
        return true;
    }
    return false;
}

bool EventSystem::_handleMouseReleased(UserEvent& actionType,
                                       const Rectangle& rect) const {
    const auto* mouseRelease = _event->getIf<sf::Event::MouseButtonReleased>();
    if (!mouseRelease) return false;

    if (mouseRelease->button == sf::Mouse::Button::Left) {
        bool wasPressed = actionType.isPressed;
        actionType.isPressed = false;

        if (wasPressed && _isPointInRect(mouseRelease->position, rect)) {
            actionType.isReleased = true;
            return true;
        }
    }
    return false;
}

void EventSystem::_handleMenuNavigation(ECS::Registry& registry,
                                        const std::vector<ECS::Entity>& buttons,
                                        bool moveDown) const {
    if (buttons.empty()) return;

    int currentIndex = -1;
    for (size_t i = 0; i < buttons.size(); ++i) {
        if (registry.hasComponent<UserEvent>(buttons[i])) {
            const auto& event = registry.getComponent<UserEvent>(buttons[i]);
            if (event.isHovered) {
                currentIndex = i;
                break;
            }
        }
    }

    int nextIndex = currentIndex + (moveDown ? 1 : -1);
    if (nextIndex < 0) nextIndex = buttons.size() - 1;
    if (nextIndex >= static_cast<int>(buttons.size())) nextIndex = 0;

    if (currentIndex >= 0 &&
        registry.hasComponent<UserEvent>(buttons[currentIndex])) {
        auto& currentEvent =
            registry.getComponent<UserEvent>(buttons[currentIndex]);
        currentEvent.isHovered = false;
    }

    if (registry.hasComponent<UserEvent>(buttons[nextIndex])) {
        auto& nextEvent = registry.getComponent<UserEvent>(buttons[nextIndex]);
        nextEvent.isHovered = true;

        if (registry.hasComponent<ButtonSoundComponent>(buttons[nextIndex])) {
            const auto& data =
                registry.getComponent<ButtonSoundComponent>(buttons[nextIndex]);
            if (_audioLib && data.hoverSFX) {
                _audioLib->playSFX(*data.hoverSFX);
            }
        }
    }
}

void EventSystem::_handleMenuActivation(
    ECS::Registry& registry, const std::vector<ECS::Entity>& buttons) const {
    for (const auto& button : buttons) {
        if (registry.hasComponent<UserEvent>(button)) {
            const auto& event = registry.getComponent<UserEvent>(button);
            if (event.isHovered) {
                auto& mutableEvent = registry.getComponent<UserEvent>(button);
                mutableEvent.isReleased = true;

                if (registry.hasComponent<ButtonSoundComponent>(button)) {
                    const auto& data =
                        registry.getComponent<ButtonSoundComponent>(button);
                    if (_audioLib && data.clickSFX) {
                        _audioLib->playSFX(*data.clickSFX);
                    }
                }
                break;
            }
        }
    }
}

}  // namespace rtype::games::rtype::client
