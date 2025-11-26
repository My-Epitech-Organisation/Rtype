/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** View
*/

#ifndef ECS_VIEW_VIEW_HPP
    #define ECS_VIEW_VIEW_HPP
    #include "../core/Entity.hpp"
    #include "../storage/SparseSet.hpp"
    #include <tuple>
    #include <algorithm>
    #include <functional>

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
        explicit View(std::reference_wrapper<Registry> registry);

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
        template<typename T>
        using PoolPtr = std::reference_wrapper<ISparseSet>;

        std::reference_wrapper<Registry> registry;
        std::tuple<PoolPtr<Components>...> pools;
        size_t _smallestPoolIndex = 0;

        template<typename Func, size_t... Is>
        void eachImpl(Func&& func, std::index_sequence<Is...>);

        template<size_t... Is>
        size_t findSmallestPool(std::index_sequence<Is...>);

        template<typename T>
        decltype(auto) getComponentData(Entity entity, const ISparseSet& pool);

        template<typename, typename>
        friend class ExcludeView;
    };

} // namespace ECS

#endif // ECS_VIEW_VIEW_HPP
