/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - View System Template Implementations
*/

#ifndef ECS_CORE_REGISTRY_VIEW_INL
    #define ECS_CORE_REGISTRY_VIEW_INL

// This file is included inside the ECS namespace in Registry.hpp
// Do not add namespace ECS here!

// ========================================================================
// VIEW CREATION
// ========================================================================

    template<typename... Components>
    View<Components...> Registry::view() {
        return View<Components...>(*this);
    }

    template<typename... Components>
    View<Components...> Registry::view() const {
        return View<Components...>(const_cast<Registry&>(*this));
    }

    template<typename... Components>
    ParallelView<Components...> Registry::parallel_view() {
        return ParallelView<Components...>(*this);
    }

    template<typename... Components>
    Group<Components...> Registry::create_group() {
        return Group<Components...>(*this);
    }

    // ========================================================================
    // RELATIONSHIP ACCESSORS
    // ========================================================================

    inline RelationshipManager& Registry::get_relationship_manager() noexcept {
        return relationship_manager;
    }

    inline const RelationshipManager& Registry::get_relationship_manager() const noexcept {
        return relationship_manager;
    }

    // ========================================================================
    // VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    View<Components...>::View(Registry& registry) : registry(registry) {
        initialize_pools(std::index_sequence_for<Components...>{});
        smallest_pool_index = find_smallest_pool(std::index_sequence_for<Components...>{});
    }

    template<typename... Components>
    template<typename Func>
    void View<Components...>::each(Func&& func) {
        each_impl(std::forward<Func>(func), std::index_sequence_for<Components...>{});
    }

    template<typename... Components>
    template<size_t... Is>
    void View<Components...>::initialize_pools(std::index_sequence<Is...>) {
        pools = std::make_tuple(&registry.template get_sparse_set<Components>()...);
    }

    template<typename... Components>
    template<typename Func, size_t... Is>
    void View<Components...>::each_impl(Func&& func, std::index_sequence<Is...>) {
        if (sizeof...(Components) == 0) return;

        // Lambda to get pool at runtime index
        auto get_pool_at_index = [this](size_t idx) -> const std::vector<Entity>* {
            const std::vector<Entity>* result = nullptr;
            size_t current = 0;
            ((current++ == idx ? (result = &std::get<Is>(pools)->get_packed(), true) : false) || ...);
            return result;
        };

        // Iterate over smallest pool for efficiency
        const auto* entities_ptr = get_pool_at_index(smallest_pool_index);
        if (!entities_ptr) return;

        for (auto entity : *entities_ptr) {
            // Check all pools contain the entity
            if ((std::get<Is>(pools)->contains(entity) && ...)) {
                func(entity, std::get<Is>(pools)->get(entity)...);
            }
        }
    }

    template<typename... Components>
    template<size_t... Is>
    size_t View<Components...>::find_smallest_pool(std::index_sequence<Is...>) {
        std::array<size_t, sizeof...(Components)> sizes = {
            std::get<Is>(pools)->get_packed().size()...
        };
        return std::distance(sizes.begin(), std::min_element(sizes.begin(), sizes.end()));
    }

    template<typename... Components>
    template<typename... Excluded>
    auto View<Components...>::exclude() {
        std::vector<ISparseSet*> exclude_pools_vec = {
            static_cast<ISparseSet*>(&registry.template get_sparse_set<Excluded>())...
        };
        return ExcludeView<std::tuple<Components...>, std::tuple<Excluded...>>(
            registry,
            pools,
            std::move(exclude_pools_vec),
            smallest_pool_index
        );
    }

    // ========================================================================
    // EXCLUDE VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Includes, typename... Excludes>
    template<typename Func>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::each(Func&& func) {
        each_impl(std::forward<Func>(func), std::index_sequence_for<Includes...>{});
    }

    template<typename... Includes, typename... Excludes>
    template<typename Func, size_t... IncIs>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::each_impl(
        Func&& func,
        std::index_sequence<IncIs...>
    ) {
        if (sizeof...(Includes) == 0) return;

        // Lambda to get pool at runtime index
        auto get_pool_at_index = [this](size_t idx) -> const std::vector<Entity>* {
            const std::vector<Entity>* result = nullptr;
            size_t current = 0;
            ((current++ == idx ? (result = &std::get<IncIs>(include_pools)->get_packed(), true) : false) || ...);
            return result;
        };

        const auto* entities_ptr = get_pool_at_index(smallest_pool_index);
        if (!entities_ptr) return;

        for (auto entity : *entities_ptr) {
            // Check entity has all included components
            if ((std::get<IncIs>(include_pools)->contains(entity) && ...)) {
                // Check entity doesn't have any excluded components
                if (!is_excluded(entity)) {
                    func(entity, std::get<IncIs>(include_pools)->get(entity)...);
                }
            }
        }
    }

    template<typename... Includes, typename... Excludes>
    bool ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::is_excluded(
        Entity entity
    ) const {
        for (auto* pool : exclude_pools) {
            if (pool->contains(entity)) {
                return true;
            }
        }
        return false;
    }

    // ========================================================================
    // PARALLEL VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    template<typename Func>
    void ParallelView<Components...>::each(Func&& func) {
        // Get all component pools
        std::tuple<SparseSet<Components>*...> pools =
            std::make_tuple(&registry.template get_sparse_set<Components>()...);

        // Find smallest pool for optimal iteration
        size_t min_size = std::numeric_limits<size_t>::max();
        const std::vector<Entity>* smallest_entities = nullptr;

        auto check_pool_size = [&]<size_t I>() {
            const auto& packed = std::get<I>(pools)->get_packed();
            if (packed.size() < min_size) {
                min_size = packed.size();
                smallest_entities = &packed;
            }
        };

        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (check_pool_size.template operator()<Is>(), ...);
        }(std::index_sequence_for<Components...>{});

        if (!smallest_entities || smallest_entities->empty()) return;

        const auto& entities = *smallest_entities;

        // Calculate threading parameters
        const size_t num_threads = std::thread::hardware_concurrency();
        const size_t chunk_size = std::max(size_t(1), entities.size() / num_threads);

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        // Spawn worker threads
        for (size_t t = 0; t < num_threads; ++t) {
            size_t start = t * chunk_size;
            size_t end = (t == num_threads - 1) ? entities.size() : start + chunk_size;

            if (start >= entities.size()) break;

            threads.emplace_back([&, start, end, pools]() {
                for (size_t i = start; i < end; ++i) {
                    Entity entity = entities[i];
                    // Check all pools contain the entity
                    if ((std::get<SparseSet<Components>*>(pools)->contains(entity) && ...)) {
                        func(entity, std::get<SparseSet<Components>*>(pools)->get(entity)...);
                    }
                }
            });
        }

        // Join all threads
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    // ========================================================================
    // GROUP IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    Group<Components...>::Group(Registry& reg) : registry(reg) {
        rebuild();
    }

    template<typename... Components>
    void Group<Components...>::rebuild() {
        entities.clear();

        // Find smallest pool
        size_t min_size = std::numeric_limits<size_t>::max();
        const std::vector<Entity>* smallest_entities = nullptr;

        auto check_pool_size = [&]<typename T>() {
            auto& pool = registry.template get_sparse_set<T>();
            const auto& packed = pool.get_packed();
            if (packed.size() < min_size) {
                min_size = packed.size();
                smallest_entities = &packed;
            }
        };

        (check_pool_size.template operator()<Components>(), ...);

        if (!smallest_entities) return;

        // Filter entities that have all components
        for (auto entity : *smallest_entities) {
            if ((registry.template has_component<Components>(entity) && ...)) {
                entities.push_back(entity);
            }
        }
    }

    template<typename... Components>
    template<typename Func>
    void Group<Components...>::each(Func&& func) {
        for (auto entity : entities) {
            func(entity, registry.template get_component<Components>(entity)...);
        }
    }

#endif // ECS_CORE_REGISTRY_VIEW_INL
