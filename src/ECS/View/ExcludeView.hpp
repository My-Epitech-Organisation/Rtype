/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ExcludeView
*/

#ifndef ECS_VIEW_EXCLUDE_VIEW_HPP
    #define ECS_VIEW_EXCLUDE_VIEW_HPP
    #include "../Core/Entity.hpp"
    #include "../Storage/SparseSet.hpp"
    #include <tuple>
    #include <algorithm>

namespace ECS {

    class Registry;

    /**
     * @brief View for iterating entities with specific components while excluding others.
     *
     * Provides efficient filtering by checking exclusion criteria only for matching entities.
     *
     * Example:
     *   auto view = registry.view<Position, Velocity>().exclude<Dead, Frozen>();
     *   view.each([](Entity e, Position& p, Velocity& v) {
     *       // Only entities with Position & Velocity, but NOT Dead or Frozen
     *       p.x += v.dx;
     *   });
     *
     * Uses type packs to separate includes and excludes
     */
    template<typename IncludePack, typename ExcludePack>
    class ExcludeView;

    template<typename... Includes, typename... Excludes>
    class ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>> {
    public:
        template<typename ExcludeTuple>
        ExcludeView(Registry& reg, std::tuple<SparseSet<Includes>*...> inc_pools,
                    ExcludeTuple&& exc_pools, size_t smallest_idx)
            : registry(reg), include_pools(inc_pools), exclude_pools(std::forward<ExcludeTuple>(exc_pools)),
              smallest_pool_index(smallest_idx) {}

        /**
         * @brief Applies function to each entity matching include/exclude criteria.
         * @param func Callable with signature (Entity, Includes&...)
         */
        template<typename Func>
        void each(Func&& func);

    private:
        Registry& registry;
        // Raw pointers are appropriate here: non-owning, lightweight views for iteration.
        // The Registry owns the component pools. These pointers are temporary and used
        // only during view construction and iteration. Using smart pointers would imply
        // ownership semantics which is incorrect for this use case.
        std::tuple<SparseSet<Includes>*...> include_pools;
        std::vector<ISparseSet*> exclude_pools;
        size_t smallest_pool_index;

        template<typename Func, size_t... IncIs>
        void each_impl(Func&& func, std::index_sequence<IncIs...>);

        bool is_excluded(Entity entity) const;
    };

} // namespace ECS

#endif // ECS_VIEW_EXCLUDE_VIEW_HPP
