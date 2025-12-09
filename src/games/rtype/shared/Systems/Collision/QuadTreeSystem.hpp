/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** QuadTreeSystem - Spatial partitioning system for optimized collision
** detection
*/

#pragma once

#include <rtype/engine.hpp>
#include <vector>

#include "../../Components/BoundingBoxComponent.hpp"
#include "../../Components/TransformComponent.hpp"
#include "QuadTree.hpp"
#include "Rect.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct CollisionPair
 * @brief Represents a pair of entities that are potentially colliding
 */
struct CollisionPair {
    ECS::Entity entityA;
    ECS::Entity entityB;

    CollisionPair(ECS::Entity a, ECS::Entity b) : entityA(a), entityB(b) {}
};

/**
 * @class QuadTreeSystem
 * @brief System that uses QuadTree spatial partitioning for optimized collision
 * detection
 *
 * This system builds a QuadTree each frame with all collidable entities and
 * provides efficient collision queries. Instead of O(n²) brute-force collision
 * checks, it reduces complexity to O(n log n) average case.
 *
 * The system rebuilds the QuadTree each frame to handle moving entities.
 * For static entities, consider using a separate static QuadTree that doesn't
 * rebuild.
 *
 * Usage:
 * 1. Call update() each frame to rebuild the QuadTree
 * 2. Use queryCollisions() to get potential collision pairs
 * 3. Use queryNearby() to get entities near a specific point/area
 */
class QuadTreeSystem : public ::rtype::engine::ASystem {
   public:
    /**
     * @brief Constructs a QuadTreeSystem with specified world bounds
     *
     * @param worldBounds The bounds of the game world (default: 1920x1080)
     * @param maxObjects Maximum objects per QuadTree node before subdivision
     * @param maxDepth Maximum depth of the QuadTree
     */
    explicit QuadTreeSystem(
        const collision::Rect& worldBounds = collision::Rect(0, 0, 1920, 1080),
        size_t maxObjects = 10, size_t maxDepth = 5);

    /**
     * @brief Updates the QuadTree by rebuilding it with current entity
     * positions
     *
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update (unused but required by
     * interface)
     */
    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Queries all potential collision pairs in the current frame
     *
     * This method returns pairs of entities whose bounding boxes may overlap.
     * Fine-grained collision detection (AABB overlap) should still be performed
     * on these pairs.
     *
     * @param registry ECS registry for component access
     * @return Vector of collision pairs to check
     */
    [[nodiscard]] std::vector<CollisionPair> queryCollisionPairs(
        ECS::Registry& registry) const;

    /**
     * @brief Queries entities near a specific area
     *
     * @param area The rectangular area to query
     * @return Vector of entity IDs within or intersecting the area
     */
    [[nodiscard]] std::vector<ECS::Entity> queryNearby(
        const collision::Rect& area) const;

    /**
     * @brief Queries entities near a specific point with a radius
     *
     * @param x X coordinate of the center point
     * @param y Y coordinate of the center point
     * @param radius Search radius around the point
     * @return Vector of entity IDs within the radius
     */
    [[nodiscard]] std::vector<ECS::Entity> queryNearby(float x, float y,
                                                       float radius) const;

    /**
     * @brief Gets the current world bounds
     * @return The world bounds rectangle
     */
    [[nodiscard]] const collision::Rect& getWorldBounds() const noexcept {
        return _worldBounds;
    }

    /**
     * @brief Sets new world bounds (will take effect on next update)
     * @param bounds New world bounds
     */
    void setWorldBounds(const collision::Rect& bounds) noexcept {
        _worldBounds = bounds;
    }

    /**
     * @brief Gets statistics about the current QuadTree
     * @return Number of nodes in the tree
     */
    [[nodiscard]] size_t getNodeCount() const noexcept {
        return _quadTree ? _quadTree->getNodeCount() : 0;
    }

    /**
     * @brief Gets the total number of entities in the QuadTree
     * @return Number of entities
     */
    [[nodiscard]] size_t getEntityCount() const noexcept {
        return _quadTree ? _quadTree->totalSize() : 0;
    }

   private:
    /**
     * @brief Creates a Rect from transform and bounding box components
     *
     * @param transform The entity's transform component
     * @param bbox The entity's bounding box component
     * @return Rect representing the entity's bounds
     */
    [[nodiscard]] static collision::Rect createRectFromComponents(
        const TransformComponent& transform, const BoundingBoxComponent& bbox);

    collision::Rect _worldBounds;
    size_t _maxObjects;
    size_t _maxDepth;
    std::unique_ptr<collision::QuadTree<uint32_t>> _quadTree;
};

}  // namespace rtype::games::rtype::shared

