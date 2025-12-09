/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MapScrollingSystem - Updates map element positions based on level scroll
*/

#pragma once

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "Components.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class MapScrollingSystem
 * @brief Updates map element screen positions based on level scroll state
 *
 * This system takes the current scroll offset and updates the X position
 * of all map elements to their correct screen positions. The Y position
 * remains unchanged.
 *
 * The scrolling is timer-based (using deltaTime) and NOT tied to CPU speed,
 * ensuring consistent gameplay across different hardware.
 *
 * Usage:
 * @code
 *   MapScrollingSystem scrollSystem;
 *   scrollSystem.setScrollState(&levelLoader.getScrollState());
 *
 *   // In game loop:
 *   scrollSystem.update(registry, deltaTime);
 * @endcode
 */
class MapScrollingSystem : public engine::ASystem {
   public:
    MapScrollingSystem();
    ~MapScrollingSystem() override = default;

    /**
     * @brief Set the scroll state to use for position calculations
     * @param scrollState Pointer to the level scroll state
     */
    void setScrollState(const LevelScrollState* scrollState) noexcept {
        m_scrollState = scrollState;
    }

    /**
     * @brief Update map element positions based on current scroll
     * @param registry ECS registry
     * @param deltaTime Time since last update (unused here, scroll state handles time)
     */
    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    const LevelScrollState* m_scrollState = nullptr;
};

}  // namespace rtype::games::rtype::shared
