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

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

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
    std::optional<sf::Event> _event;
    std::shared_ptr<sf::RenderWindow> _window;
    std::shared_ptr<AudioLib> _audioLib;

    /// @brief Check if world position is within rectangle bounds
    [[nodiscard]] bool _isPointInRect(
        sf::Vector2i pixelPos,
        const ::rtype::games::rtype::client::Rectangle& rect) const;

    bool _handleMouseMoved(UserEvent& actionType, const Rectangle& rect,
                           ECS::Registry& reg, ECS::Entity entt) const;

    bool _handleMousePressed(UserEvent& actionType, const Rectangle& rect,
                             ECS::Registry& reg, ECS::Entity entt) const;

    bool _handleMouseReleased(UserEvent& actionType,
                              const Rectangle& rect) const;

   public:
    /**
     * @brief Construct a new EventSystem (reusable version).
     * @param window Shared pointer to the SFML render window
     * @param audioLib Shared pointer to the AudioLib
     */
    explicit EventSystem(std::shared_ptr<sf::RenderWindow> window,
                         std::shared_ptr<AudioLib> audioLib);

    /**
     * @brief Legacy constructor for backward compatibility.
     * @param window Shared pointer to the SFML render window
     * @param audioLib Shared pointer to the AudioLib
     * @param event The event to process
     * @deprecated Use the single-argument constructor and setEvent() instead
     */
    EventSystem(std::shared_ptr<sf::RenderWindow> window,
                std::shared_ptr<AudioLib> audioLib, const sf::Event& event);

    /**
     * @brief Set the current event to process.
     * @param event The event to process
     */
    void setEvent(const sf::Event& event);

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
