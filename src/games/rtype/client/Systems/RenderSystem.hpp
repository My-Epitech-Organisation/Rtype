/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp - System for rendering all visual entities
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_

#include <memory>
#include <vector>

#include "../../../../../include/rtype/display/IDisplay.hpp"
#include "../Components/HiddenComponent.hpp"
#include "ASystem.hpp"
#include "ECS.hpp"

constexpr int kTextInputHorizontalPadding = 20;

namespace rtype::games::rtype::client {

/**
 * @brief System responsible for rendering all visual entities.
 *
 * Handles rendering of:
 * - Images (sprites with textures)
 * - Rectangles (UI elements, backgrounds)
 * - Buttons (Rectangle + Text combinations)
 * - Static text
 *
 * Entities are sorted by ZIndex before rendering to ensure proper layering.
 * Hidden entities (with HiddenComponent.isHidden = true) are skipped.
 *
 * @note Uses cached entity vector to avoid per-frame allocations.
 */
class RenderSystem : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<::rtype::display::IDisplay> _display;

    /// @brief Cached vector for drawable entities (avoids per-frame allocation)
    mutable std::vector<ECS::Entity> _cachedDrawableEntities;

    /// @brief Last known count of drawable entities (for dirty check)
    mutable std::size_t _lastDrawableCount = 0;

    /// @brief Force re-sort on next frame
    mutable bool _needsResort = true;

    /**
     * @brief Render all image/sprite entities sorted by ZIndex.
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderImages(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Render all non-button rectangle entities.
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderRectangles(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Render all button entities (Rectangle + Text).
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderButtons(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Render all static text entities.
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderStaticText(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Render text input entities.
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderTextInputs(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Render screen-space rectangles (UI elements not affected by
     * camera).
     * @param registry The ECS registry
     * @param entity The entity to try to render
     */
    void _renderHudRectangles(ECS::Registry& registry, ECS::Entity entity);

   public:
    /**
     * @brief Check if an entity should be hidden from rendering.
     * @param registry The ECS registry
     * @param entity The entity to check
     * @return true if entity has HiddenComponent with isHidden = true
     */
    static bool isEntityHidden(ECS::Registry& registry, ECS::Entity entity);

    /**
     * @brief Construct a new RenderSystem.
     * @param display Shared pointer to the display interface
     */
    explicit RenderSystem(std::shared_ptr<::rtype::display::IDisplay> display);

    /**
     * @brief Render all visible entities to the window.
     *
     * Rendering order:
     * 1. Images sorted by ZIndex (lowest first)
     * 2. Rectangles (non-button)
     * 3. Buttons (Rectangle + Text)
     * 4. Static text
     *
     * @param registry The ECS registry containing entities
     * @param dt Delta time (unused in rendering)
     */
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
