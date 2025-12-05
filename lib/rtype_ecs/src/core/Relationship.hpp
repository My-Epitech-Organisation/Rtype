/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Relationship - Entity parent-child relationships
*/

#ifndef SRC_ENGINE_ECS_CORE_RELATIONSHIP_HPP_
#define SRC_ENGINE_ECS_CORE_RELATIONSHIP_HPP_

#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Entity.hpp"

namespace ECS {

/**
 * @brief Manages hierarchical relationships between entities.
 *
 * Features:
 * - Parent-child relationships (tree structure)
 * - Automatic cleanup on entity destruction
 * - Thread-safe operations
 * - Efficient child iteration
 *
 * Use cases:
 * - Scene graphs (transform hierarchies)
 * - UI widget trees
 * - Entity prefab instances
 * - Networked object ownership
 *
 * Example:
 *   RelationshipManager mgr;
 *   auto parent = registry.spawnEntity();
 *   auto child = registry.spawnEntity();
 *   mgr.setParent(child, parent);
 *
 *   for (auto c : mgr.getChildren(parent)) {
 *       // Process children
 *   }
 */
class RelationshipManager {
   public:
    RelationshipManager() = default;

    /**
     * @brief Sets parent-child relationship.
     * Removes previous parent if child already has one.
     * @param child Child entity
     * @param parent Parent entity
     * @return true if successful, false if would create cycle
     */
    auto setParent(Entity child, Entity parent) -> bool;

    /**
     * @brief Removes parent relationship (orphans the child).
     */
    void removeParent(Entity child);

    /**
     * @brief Gets parent of entity if it has one.
     * @return Optional parent entity
     */
    auto getParent(Entity child) const -> std::optional<Entity>;

    /**
     * @brief Checks if entity has a parent.
     */
    auto hasParent(Entity child) const -> bool;

    /**
     * @brief Gets all direct children of entity.
     * @return Vector of child entities
     */
    auto getChildren(Entity parent) const -> std::vector<Entity>;

    /**
     * @brief Gets all descendants recursively.
     * @return Vector of all descendant entities (depth-first order)
     */
    auto getDescendants(Entity parent) const -> std::vector<Entity>;

    /**
     * @brief Gets all ancestors (parent, grandparent, etc.).
     * @return Vector from immediate parent to root
     */
    auto getAncestors(Entity child) const -> std::vector<Entity>;

    /**
     * @brief Gets the root entity of the hierarchy.
     * @return Root entity (entity with no parent)
     */
    auto getRoot(Entity entity) const -> Entity;

    /**
     * @brief Checks if an entity is ancestor of another.
     */
    auto isAncestor(Entity potential_ancestor, Entity entity) const -> bool;

    /**
     * @brief Removes all relationships involving an entity.
     * Called automatically when entity is destroyed.
     */
    void removeEntity(Entity entity);

    /**
     * @brief Clears all relationships.
     */
    void clear();

    /**
     * @brief Gets number of children an entity has.
     */
    auto childCount(Entity parent) const -> size_t;

    /**
     * @brief Gets depth of entity in hierarchy (0 = root).
     */
    auto getDepth(Entity entity) const -> size_t;

   private:
    std::unordered_map<std::uint32_t, Entity> _parentMap;
    std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Entity>>
        _childrenMap;
    mutable std::shared_mutex _relationshipMutex;

    auto wouldCreateCycle(Entity child, Entity parent) const -> bool;
    void getDescendantsRecursive(Entity parent,
                                 std::vector<Entity>* result) const;
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_CORE_RELATIONSHIP_HPP_
