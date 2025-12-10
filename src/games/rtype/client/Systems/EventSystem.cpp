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

namespace rc = ::rtype::games::rtype::client;

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
                                 const rc::Rectangle& rect) const {
    sf::Vector2f worldPos = _window->mapPixelToCoords(pixelPos);
    return rect.rectangle.getGlobalBounds().contains(worldPos);
}

void EventSystem::update(ECS::Registry& registry, float /*dt*/) {
    if (!_event.has_value()) return;

    registry.view<Rectangle, UserEvent>().each(
        [this, &registry](auto entity, const Rectangle& rect,
                          UserEvent& actionType) {
            if (registry.hasComponent<HiddenComponent>(entity)) {
                const auto& hidden =
                    registry.getComponent<HiddenComponent>(entity);
                if (hidden.isHidden) return;
            }
            this->_mouseMoved(actionType, rect, registry, entity);
            this->_mousePressed(actionType, rect, registry, entity);
            this->_mouseReleased(actionType, rect);
        });
}

void EventSystem::_mouseMoved(UserEvent& actionType, const Rectangle& rect,
                              ECS::Registry& reg,
                              const ECS::Entity entt) const {
    if (const auto* mouseMove = _event->getIf<sf::Event::MouseMoved>()) {
        bool isInside = _isPointInRect(mouseMove->position, rect);
        if (!actionType.isHovered && isInside) {
            if (reg.hasComponent<ButtonSoundComponent>(entt)) {
                const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
                this->_audioLib->playSFX(*data.hoverSFX);
            }
        }

        actionType.isHovered = isInside;
        if (!isInside) {
            actionType.isClicked = false;
        }
    }
}

void EventSystem::_mousePressed(UserEvent& actionType, const Rectangle& rect,
                                ECS::Registry& reg,
                                const ECS::Entity entt) const {
    if (const auto* mousePress =
            _event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePress->button == sf::Mouse::Button::Left &&
            _isPointInRect(mousePress->position, rect)) {
            actionType.isClicked = true;
            if (reg.hasComponent<ButtonSoundComponent>(entt)) {
                const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
                this->_audioLib->playSFX(*data.clickSFX);
            }
        }
    }
}

void EventSystem::_mouseReleased(UserEvent& actionType,
                                 const Rectangle& rect) const {
    if (const auto* mouseRelease =
            _event->getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseRelease->button == sf::Mouse::Button::Left) {
            if (_isPointInRect(mouseRelease->position, rect) &&
                actionType.isClicked) {
                actionType.isReleased = true;
            }
            actionType.isClicked = false;
        }
    }
}
}  // namespace rtype::games::rtype::client
