/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
#include <memory>
#include <optional>
#include <vector>

#include "rtype/display/IDisplay.hpp"
#include "rtype/display/DisplayTypes.hpp"

#include "../Components/RectangleComponent.hpp"
#include "../Components/UserEventComponent.hpp"
#include "ASystem.hpp"
#include "AudioLib/AudioLib.hpp"
#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System responsible for processing input events.
 *
 * Handles mouse interactions with UI elements:
 * - Mouse hover detection
 * - Mouse click detection
 * - Mouse release detection
 *
 * The system can be reused across multiple events by calling setEvent()
 * before update(), avoiding per-event allocation.
 */
class EventSystem : public ::rtype::engine::ASystem {
   private:
    std::optional<::rtype::display::Event> _event;
    std::shared_ptr<::rtype::display::IDisplay> _display;
    std::shared_ptr<AudioLib> _audioLib;

    /// @brief Check if world position is within rectangle bounds
    [[nodiscard]] bool _isPointInRect(
        ::rtype::display::Vector2i pixelPos,
        const ::rtype::games::rtype::client::Rectangle& rect,
        ::rtype::display::Vector2f position) const;

    bool _handleMouseMoved(UserEvent& actionType, const Rectangle& rect,
                           ECS::Registry& reg, ECS::Entity entt,
                           ::rtype::display::Vector2f position) const;

    bool _handleMousePressed(UserEvent& actionType, const Rectangle& rect,
                             ECS::Registry& reg, ECS::Entity entt,
                             ::rtype::display::Vector2f position) const;

    bool _handleMouseReleased(UserEvent& actionType, const Rectangle& rect,
                              ::rtype::display::Vector2f position) const;

    void _handleMenuNavigation(ECS::Registry& registry,
                               const std::vector<ECS::Entity>& buttons,
                               bool moveDown) const;

    void _handleMenuActivation(ECS::Registry& registry,
                               const std::vector<ECS::Entity>& buttons) const;

   public:
    /**
     * @brief Construct a new EventSystem (reusable version).
     * @param display Shared pointer to the display interface
     * @param audioLib Shared pointer to the AudioLib
     */
    explicit EventSystem(std::shared_ptr<::rtype::display::IDisplay> display,
                         std::shared_ptr<AudioLib> audioLib);

    /**
     * @brief Set the current event to process.
     * @param event The event to process
     */
    void setEvent(const ::rtype::display::Event& event);

    /**
     * @brief Clear the current event.
     */
    void clearEvent();

    /**
     * @brief Process the current event for all UI entities.
     * @param registry The ECS registry
     * @param dt Delta time (unused)
     */
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client


#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
