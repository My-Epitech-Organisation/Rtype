/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ParallelView
*/

#ifndef ECS_VIEW_parallelView_HPP
    #define ECS_VIEW_parallelView_HPP
    #include "../Core/Entity.hpp"
    #include "../Storage/SparseSet.hpp"
    #include <thread>
    #include <vector>
    #include <algorithm>

namespace ECS {

    class Registry;

    /**
     * @brief Thread-safe view for parallel component iteration.
     *
     * Distributes work across multiple threads for performance on large datasets.
     *
     * Thread Safety Guarantees:
     * - Safe: Concurrent reads of same component
     * - Safe: Concurrent writes to different components of same entity
     * - Unsafe: Adding/removing entities during iteration
     * - Unsafe: Adding/removing components during iteration
     * - Unsafe: Shared mutable state in callback without synchronization
     *
     * Example:
     *   registry.parallelView<Position, Velocity>().each([](Entity e, Position& p, Velocity& v) {
     *       p.x += v.dx; // Each thread processes different entities
     *   });
     */
    template<typename... Components>
    class ParallelView {
    public:
        explicit ParallelView(std::reference_wrapper<Registry> reg) : _registry(reg) {}

        /**
         * @brief Applies function to entities across multiple threads.
         * @param func Thread-safe callable with signature (Entity, Components&...)
         */
        template<typename Func>
        void each(Func&& func);

    private:
        std::reference_wrapper<Registry> _registry;
    };

} // namespace ECS

#endif // ECS_VIEW_parallelView_HPP
