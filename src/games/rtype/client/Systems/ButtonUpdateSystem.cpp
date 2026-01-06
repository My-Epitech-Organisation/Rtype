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

ButtonUpdateSystem::ButtonUpdateSystem(std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("ButtonUpdateSystem"),
      _display(std::move(display)) {}

void ButtonUpdateSystem::update(ECS::Registry& registry, float /*dt*/) {
    std::vector<std::function<void()>> callbacksToRun;

    registry.view<Button<>, UserEvent>().each(
        [&callbacksToRun](ECS::Entity /*entity*/, auto& buttonAct, auto& actionType) {
            if (!actionType.idle && actionType.isReleased &&
                actionType.isHovered) {
                LOG_DEBUG(
                    "[ButtonUpdateSystem] Button click detected, queueing callback");
                callbacksToRun.push_back(buttonAct.callback);
            }
        });

    for (auto& callback : callbacksToRun) {
        LOG_DEBUG("[ButtonUpdateSystem] Executing button callback");
        try {
            callback();
        } catch (const SceneNotFound& e) {
            ::rtype::Logger::instance().error(
                std::string("Error executing button callback: ") +
                std::string(e.what()));
        } catch (const std::exception& e) {
            ::rtype::Logger::instance().error(
                std::string("Exception in button callback: ") +
                std::string(e.what()));
        } catch (...) {
            ::rtype::Logger::instance().error(
                "Unknown error in button callback");
        }
    }

    registry.view<Rectangle, UserEvent, ButtonTag>().each(
        [](auto /*entity*/, auto& rect, auto& actionType, auto /*tag*/) {
            rect.currentColor =
                actionType.isHovered ? rect.hoveredColor : rect.mainColor;
        });
}

}  // namespace rtype::games::rtype::client
