/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** View
*/

#ifndef ECS_VIEW_VIEW_HPP
    #define ECS_VIEW_VIEW_HPP
    #include "../Core/Entity.hpp"
    #include "../Storage/SparseSet.hpp"
    #include <tuple>
    #include <algorithm>

namespace ECS {

    class Registry;

    template<typename, typename>
    class ExcludeView;

    /**
     * @brief Non-owning view for iterating entities with specific components.
     *
     * Automatically selects the smallest component set for iteration to minimize work.
     * Views are lightweight and designed for single-threaded traversal.
     *
     * Example:
     *   auto view = registry.view<Position, Velocity>();
     *   view.each([](Entity e, Position& p, Velocity& v) {
     *       p.x += v.dx;
     *   });
     */
    template<typename... Components>
    class View {
    public:
        explicit View(Registry* registry);

        /**
         * @brief Applies function to each entity with all required components.
         * @param func Callable with signature (Entity, Components&...)
         */
        template<typename Func>
        void each(Func&& func);

        /**
         * @brief Creates an exclude view that filters out entities with specified components.
         * @return ExcludeView that excludes entities with Excluded components
         */
        template<typename... Excluded>
        auto exclude();

    private:
        // Type alias to transform component types to ISparseSet pointers
        template<typename T>
        using PoolPtr = ISparseSet*;

        Registry* registry;
        // Raw pointers are appropriate here: non-owning, lightweight view for iteration.
        // The Registry owns the component pools via std::unique_ptr. These pointers are
        // temporary references used only during view iteration. Smart pointers would
        // incorrectly imply shared or exclusive ownership.
        // Using ISparseSet* to support both regular components and tags
        std::tuple<PoolPtr<Components>...> pools;
        size_t smallest_pool_index = 0;

        template<size_t... Is>
        void initialize_pools(std::index_sequence<Is...>);

        template<typename Func, size_t... Is>
        void each_impl(Func&& func, std::index_sequence<Is...>);

        template<size_t... Is>
        size_t find_smallest_pool(std::index_sequence<Is...>);

        template<typename T>
        T& get_component_data(Entity entity, ISparseSet* pool);

        template<typename, typename>
        friend class ExcludeView;
    };

} // namespace ECS

#endif // ECS_VIEW_VIEW_HPP
