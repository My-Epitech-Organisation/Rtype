/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <utility>

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

}  // namespace rtype::games::rtype::client
