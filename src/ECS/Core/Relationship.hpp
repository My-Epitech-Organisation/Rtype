/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Relationship - Entity parent-child relationships
*/

#ifndef ECS_CORE_RELATIONSHIP_HPP
    #define ECS_CORE_RELATIONSHIP_HPP
    #include "Entity.hpp"
    #include <vector>
    #include <unordered_map>
    #include <unordered_set>
    #include <optional>
    #include <shared_mutex>

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
     *   auto parent = registry.spawn_entity();
     *   auto child = registry.spawn_entity();
     *   mgr.set_parent(child, parent);
     *
     *   for (auto c : mgr.get_children(parent)) {
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
        bool set_parent(Entity child, Entity parent);

        /**
         * @brief Removes parent relationship (orphans the child).
         */
        void remove_parent(Entity child);

        /**
         * @brief Gets parent of entity if it has one.
         * @return Optional parent entity
         */
        std::optional<Entity> get_parent(Entity child) const;

        /**
         * @brief Checks if entity has a parent.
         */
        bool has_parent(Entity child) const;

        /**
         * @brief Gets all direct children of entity.
         * @return Vector of child entities
         */
        std::vector<Entity> get_children(Entity parent) const;

        /**
         * @brief Gets all descendants recursively.
         * @return Vector of all descendant entities (depth-first order)
         */
        std::vector<Entity> get_descendants(Entity parent) const;

        /**
         * @brief Gets all ancestors (parent, grandparent, etc.).
         * @return Vector from immediate parent to root
         */
        std::vector<Entity> get_ancestors(Entity child) const;

        /**
         * @brief Gets the root entity of the hierarchy.
         * @return Root entity (entity with no parent)
         */
        Entity get_root(Entity entity) const;

        /**
         * @brief Checks if an entity is ancestor of another.
         */
        bool is_ancestor(Entity potential_ancestor, Entity entity) const;

        /**
         * @brief Removes all relationships involving an entity.
         * Called automatically when entity is destroyed.
         */
        void remove_entity(Entity entity);

        /**
         * @brief Clears all relationships.
         */
        void clear();

        /**
         * @brief Gets number of children an entity has.
         */
        size_t child_count(Entity parent) const;

        /**
         * @brief Gets depth of entity in hierarchy (0 = root).
         */
        size_t get_depth(Entity entity) const;

    private:
        std::unordered_map<std::uint32_t, Entity> parent_map;
        std::unordered_map<std::uint32_t, std::unordered_map<std::uint32_t, Entity>> children_map;
        mutable std::shared_mutex relationship_mutex;

        bool would_create_cycle(Entity child, Entity parent) const;
        void get_descendants_recursive(Entity parent, std::vector<Entity>* result) const;
    };

} // namespace ECS

#endif // ECS_CORE_RELATIONSHIP_HPP
