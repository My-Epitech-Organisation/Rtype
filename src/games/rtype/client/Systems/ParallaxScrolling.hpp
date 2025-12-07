/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_

#include <memory>

#include <SFML/Graphics/View.hpp>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System for parallax scrolling background layers.
 *
 * Uses cached view dimensions to avoid per-frame recalculation.
 * Call invalidateCache() if view size changes.
 */
class ParallaxScrolling : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<sf::View> _view;

    /// @brief Cached half-width of the view
    float _cachedHalfWidth = 0.0f;

    /// @brief Cached half-height of the view
    float _cachedHalfHeight = 0.0f;

    /// @brief Whether cache needs refresh
    bool _cacheValid = false;

    /// @brief Update cached dimensions from view
    void _updateCache();

   public:
    explicit ParallaxScrolling(std::shared_ptr<sf::View> view);

    /// @brief Invalidate cached dimensions (call when view resizes)
    void invalidateCache();

    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_PARALLAXSCROLLING_HPP_
