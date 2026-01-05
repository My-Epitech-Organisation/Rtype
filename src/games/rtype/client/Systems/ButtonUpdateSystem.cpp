/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonUpdateSystem.cpp
*/

#include "ButtonUpdateSystem.hpp"

#include <exception>
#include <string>
#include <utility>

#include "../AllComponents.hpp"
#include "Logger/Logger.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

namespace rtype::games::rtype::client {

ButtonUpdateSystem::ButtonUpdateSystem(std::shared_ptr<sf::RenderWindow> window)
    : ::rtype::engine::ASystem("ButtonUpdateSystem"),
      _window(std::move(window)) {}

void ButtonUpdateSystem::update(ECS::Registry& registry, float /*dt*/) {
    registry.view<Button<>, UserEvent>().each(
        [](ECS::Entity /*entity*/, auto& buttonAct, auto& actionType) {
            if (!actionType.idle && actionType.isReleased &&
                actionType.isHovered) {
                LOG_DEBUG_CAT(
                    ::rtype::LogCategory::UI,
                    "[ButtonUpdateSystem] Button clicked, executing callback");
                try {
                    buttonAct.callback();
                } catch (SceneNotFound& e) {
                    ::rtype::Logger::instance().error(
                        std::string("Error executing button callback: ") +
                        std::string(e.what()));
                }
            }
        });

    registry.view<Rectangle, UserEvent, ButtonTag>().each(
        [](auto /*entity*/, auto& rect, auto& actionType, auto /*tag*/) {
            rect.currentColor =
                actionType.isHovered ? rect.hoveredColor : rect.mainColor;
        });
}

}  // namespace rtype::games::rtype::client
