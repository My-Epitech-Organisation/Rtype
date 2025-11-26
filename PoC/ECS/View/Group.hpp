/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Group
*/

#ifndef ECS_VIEW_GROUP_HPP
    #define ECS_VIEW_GROUP_HPP
    #include "../Core/Entity.hpp"
    #include <vector>
    #include <algorithm>

namespace ECS {

    class Registry;

    /**
     * @brief Cached entity collection for repeated filtered queries.
     *
     * Unlike views which filter on-the-fly, groups maintain a pre-filtered entity list.
     * This provides O(1) iteration at the cost of requiring manual updates after structural changes.
     *
     * Use when:
     * - Same query runs frequently
     * - Entity structure changes infrequently
     * - Iteration speed is critical
     *
     * Example:
     *   auto group = registry.createGroup<Position, Velocity>();
     *   for (auto entity : group) {
     *       // Fast iteration, no filtering
     *   }
     *   registry.emplaceComponent<Velocity>(new_entity);
     *   group.rebuild(); // Update after structural change
     */
    template<typename... Components>
    class Group {
    public:
        explicit Group(std::reference_wrapper<Registry> reg);

        /**
         * @brief Rebuilds entity list by re-filtering all entities.
         * Must be called after adding/removing components that affect membership.
         */
        void rebuild();

        /**
         * @brief Applies function to each entity in the cached group.
         * @param func Callable with signature (Entity, Components&...)
         */
        template<typename Func>
        void each(Func&& func);

        const std::vector<Entity>& getEntities() const noexcept { return _entities; }
        size_t size() const noexcept { return _entities.size(); }
        bool empty() const noexcept { return _entities.empty(); }

        auto begin() const noexcept { return _entities.begin(); }
        auto end() const noexcept { return _entities.end(); }

    private:
        std::reference_wrapper<Registry> _registry;
        std::vector<Entity> _entities;
    };

} // namespace ECS

#endif // ECS_VIEW_GROUP_HPP
