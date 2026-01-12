/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "../AllComponents.hpp"
#include "AudioLib/AudioLib.hpp"
#include "Components/SoundComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::client {

EventSystem::EventSystem(std::shared_ptr<::rtype::display::IDisplay> display,
                         std::shared_ptr<AudioLib> audio)
    : ASystem("EventSystem"),
      _display(std::move(display)),
      _audioLib(std::move(audio)) {}

void EventSystem::setEvent(const ::rtype::display::Event& event) {
    _event = event;
}

void EventSystem::clearEvent() { _event.reset(); }

bool EventSystem::_isPointInRect(::rtype::display::Vector2i pixelPos,
                                 const Rectangle& rect,
                                 ::rtype::display::Vector2f position) const {
    if (!_display) return false;
    return (pixelPos.x >= position.x &&
            pixelPos.x <= position.x + rect.size.first &&
            pixelPos.y >= position.y &&
            pixelPos.y <= position.y + rect.size.second);
}

void EventSystem::update(ECS::Registry& registry, float /*dt*/) {
    if (!_event.has_value()) return;

    auto buttons = registry.view<Rectangle, UserEvent>();
    std::vector<ECS::Entity> buttonEntities;
    buttons.each([&buttonEntities](auto entity, const auto&, auto&) {
        buttonEntities.push_back(entity);
    });

    if (!buttonEntities.empty()) {
        if (_event->type == ::rtype::display::EventType::KeyPressed) {
            if (_event->key.code == ::rtype::display::Key::Up ||
                _event->key.code == ::rtype::display::Key::Down) {
                _handleMenuNavigation(
                    registry, buttonEntities,
                    _event->key.code == ::rtype::display::Key::Down);
            } else if (_event->key.code == ::rtype::display::Key::Return) {
                _handleMenuActivation(registry, buttonEntities);
            }
        } else if (_event->type == ::rtype::display::EventType::JoystickMoved) {
            if (_event->joystickMove.axis ==
                ::rtype::display::JoystickAxis::Y) {
                static std::map<unsigned int, bool> lastUpPressed;
                static std::map<unsigned int, bool> lastDownPressed;

                if (_event->joystickMove.position > 95.0f) {
                    if (!lastUpPressed[_event->joystickMove.joystickId]) {
                        _handleMenuNavigation(registry, buttonEntities, true);
                        lastUpPressed[_event->joystickMove.joystickId] = true;
                    }
                } else if (_event->joystickMove.position < -95.0f) {
                    if (!lastDownPressed[_event->joystickMove.joystickId]) {
                        _handleMenuNavigation(registry, buttonEntities, false);
                        lastDownPressed[_event->joystickMove.joystickId] = true;
                    }
                } else {
                    lastUpPressed[_event->joystickMove.joystickId] = false;
                    lastDownPressed[_event->joystickMove.joystickId] = false;
                }
            }
        } else if (_event->type ==
                   ::rtype::display::EventType::JoystickButtonPressed) {
            if (_event->joystickButton.button == 0) {
                _handleMenuActivation(registry, buttonEntities);
            }
        }
    }

    registry
        .view<Rectangle, UserEvent,
              ::rtype::games::rtype::shared::TransformComponent>()
        .each([this, &registry](
                  ECS::Entity entity, const Rectangle& rect,
                  UserEvent& actionType,
                  const ::rtype::games::rtype::shared::TransformComponent&
                      transform) {
            if (registry.hasComponent<HiddenComponent>(entity)) {
                if (registry.getComponent<HiddenComponent>(entity).isHidden) {
                    return;
                }
            }

            ::rtype::display::Vector2f position{transform.x, transform.y};

            if (registry.hasComponent<CenteredBtnTag>(entity)) {
                position.x -= rect.size.first / 2.0f;
                position.y -= rect.size.second / 2.0f;
            }

            bool interaction = false;
            interaction |= this->_handleMouseMoved(actionType, rect, registry,
                                                   entity, position);
            interaction |= this->_handleMousePressed(actionType, rect, registry,
                                                     entity, position);
            interaction |=
                this->_handleMouseReleased(actionType, rect, position);

            if (interaction || actionType.isHovered || actionType.isPressed) {
                actionType.idle = false;
            }
        });
}

bool EventSystem::_handleMouseMoved(UserEvent& actionType,
                                    const Rectangle& rect, ECS::Registry& reg,
                                    ECS::Entity entt,
                                    ::rtype::display::Vector2f position) const {
    if (_event->type != ::rtype::display::EventType::MouseMoved) return false;

    bool isInside = _isPointInRect({_event->mouseMove.x, _event->mouseMove.y},
                                   rect, position);
    bool interacted = false;

    if (!actionType.isHovered && isInside) {
        if (reg.hasComponent<ButtonSoundComponent>(entt)) {
            const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
            if (_audioLib && data.hoverSFX) {
                _audioLib->playSFX(data.hoverSFX);
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

bool EventSystem::_handleMousePressed(
    UserEvent& actionType, const Rectangle& rect, ECS::Registry& reg,
    ECS::Entity entt, ::rtype::display::Vector2f position) const {
    if (_event->type != ::rtype::display::EventType::MouseButtonPressed)
        return false;

    if (_event->mouseButton.button == ::rtype::display::MouseButton::Left &&
        _isPointInRect({_event->mouseButton.x, _event->mouseButton.y}, rect,
                       position)) {
        actionType.isPressed = true;

        if (reg.hasComponent<ButtonSoundComponent>(entt)) {
            const auto& data = reg.getComponent<ButtonSoundComponent>(entt);
            if (_audioLib && data.clickSFX) {
                _audioLib->playSFX(data.clickSFX);
            }
        }
        return true;
    }
    return false;
}

bool EventSystem::_handleMouseReleased(
    UserEvent& actionType, const Rectangle& rect,
    ::rtype::display::Vector2f position) const {
    if (_event->type != ::rtype::display::EventType::MouseButtonReleased)
        return false;

    if (_event->mouseButton.button == ::rtype::display::MouseButton::Left) {
        bool wasPressed = actionType.isPressed;
        actionType.isPressed = false;

        if (wasPressed &&
            _isPointInRect({_event->mouseButton.x, _event->mouseButton.y}, rect,
                           position)) {
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
                _audioLib->playSFX(data.hoverSFX);
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
                        _audioLib->playSFX(data.clickSFX);
                    }
                }
                break;
            }
        }
    }
}

}  // namespace rtype::games::rtype::client
