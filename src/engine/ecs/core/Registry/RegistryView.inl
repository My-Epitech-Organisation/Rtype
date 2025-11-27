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
    auto Registry::view() -> View<Components...> {
        return View<Components...>(std::ref(*this));
    }

    template<typename... Components>
    auto Registry::view() const -> View<Components...> {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return View<Components...>(std::ref(const_cast<Registry&>(*this)));
    }

    template<typename... Components>
    auto Registry::parallelView() -> ParallelView<Components...> {
        return ParallelView<Components...>(std::ref(*this));
    }

    template<typename... Components>
    auto Registry::createGroup() -> Group<Components...> {
        return Group<Components...>(std::ref(*this));
    }

    // ========================================================================
    // RELATIONSHIP ACCESSORS
    // ========================================================================

    inline auto Registry::getRelationshipManager() noexcept -> RelationshipManager& {
        return _relationshipManager;
    }

    inline auto Registry::getRelationshipManager() const noexcept -> const RelationshipManager& {
        return _relationshipManager;
    }

    // ========================================================================
    // VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    View<Components...>::View(std::reference_wrapper<Registry> registry)
        : registry(registry),
          pools(std::ref(static_cast<ISparseSet&>(registry.get().template getSparseSet<Components>()))...),
          _smallestPoolIndex(findSmallestPool(std::index_sequence_for<Components...>{})) {
    }

    template<typename... Components>
    template<typename Func>
    void View<Components...>::each(Func&& func) {
        eachImpl(std::forward<Func>(func), std::index_sequence_for<Components...>{});
    }

    template<typename... Components>
    template<typename Func, size_t... Is>
    void View<Components...>::eachImpl(Func&& func, std::index_sequence<Is...> /*unused*/) {
        if (sizeof...(Components) == 0) {
            return;
        }

        std::array<std::reference_wrapper<const std::vector<Entity>>, sizeof...(Components)> allPools = {
            std::cref(std::get<Is>(pools).get().getPacked())...
        };

        const auto& entities = allPools[_smallestPoolIndex].get();

        for (auto entity : entities) {
            if ((std::get<Is>(pools).get().contains(entity) && ...)) {
                std::forward<Func>(func)(entity, getComponentData<Components>(entity, std::get<Is>(pools).get())...);
            }
        }
    }

    template<typename... Components>
    template<size_t... Is>
    auto View<Components...>::findSmallestPool(std::index_sequence<Is...> /*unused*/) -> size_t {
        std::array<size_t, sizeof...(Components)> sizes = {
            std::get<Is>(pools).get().getPacked().size()...
        };
        return std::distance(sizes.begin(), std::min_element(sizes.begin(), sizes.end()));
    }

    template<typename... Components>
    template<typename... Excluded>
    auto View<Components...>::exclude() {
        std::vector<std::reference_wrapper<ISparseSet>> _excludePools_vec = {
            std::ref(static_cast<ISparseSet&>(registry.get().template getSparseSet<Excluded>()))...
        };
        return ExcludeView<std::tuple<Components...>, std::tuple<Excluded...>>(
            registry.get(),
            pools,
            std::move(_excludePools_vec),
            _smallestPoolIndex
        );
    }

    // ========================================================================
    // EXCLUDE VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Includes, typename... Excludes>
    template<typename Func>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::each(Func&& func) {
        eachImpl(std::forward<Func>(func), std::index_sequence_for<Includes...>{});
    }

    template<typename... Includes, typename... Excludes>
    template<typename Func, size_t... IncIs>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::eachImpl(
        Func&& func,
        std::index_sequence<IncIs...> /*unused*/
    ) {
        if (sizeof...(Includes) == 0) {
            return;
        }

        std::array<std::reference_wrapper<const std::vector<Entity>>, sizeof...(Includes)> allPools = {
            std::cref(std::get<IncIs>(_includePools).get().getPacked())...
        };

        const auto& entities = allPools[_smallestPoolIndex].get();

        for (auto entity : entities) {
            if ((std::get<IncIs>(_includePools).get().contains(entity) && ...)) {
                if (!is_excluded(entity)) {
                    std::forward<Func>(func)(entity, getComponentData<Includes>(entity, std::get<IncIs>(_includePools).get())...);
                }
            }
        }
    }

    template<typename... Includes, typename... Excludes>
    auto ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::is_excluded(
        Entity entity
    ) const -> bool {
        return std::ranges::any_of(_excludePools, [&entity](const auto& pool) {
            return pool.get().contains(entity);
        });
    }

    // ========================================================================
    // PARALLEL VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    template<typename Func>
    void ParallelView<Components...>::each(Func&& func) {
        std::tuple<std::reference_wrapper<SparseSet<Components>>...> pools =
            std::make_tuple(std::ref(_registry.get().template getSparseSet<Components>())...);

        size_t min_size = std::numeric_limits<size_t>::max();
        std::optional<std::reference_wrapper<const std::vector<Entity>>> smallest_entities;

        auto check_pool_size = [&]<size_t I>() {
            const auto& _packed = std::get<I>(pools).get().getPacked();
            if (_packed.size() < min_size) {
                min_size = _packed.size();
                smallest_entities = std::cref(_packed);
            }
        };

        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (check_pool_size.template operator()<Is>(), ...);
        }(std::index_sequence_for<Components...>{});

        if (!smallest_entities.has_value() || smallest_entities->get().empty()) {
            return;
        }

        const auto& entities = smallest_entities->get();

        const size_t num_threads = std::thread::hardware_concurrency();
        const size_t chunk_size = std::max(size_t(1), entities.size() / num_threads);

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (size_t thread_idx = 0; thread_idx < num_threads; ++thread_idx) {
            size_t start = thread_idx * chunk_size;
            size_t end = (thread_idx == num_threads - 1) ? entities.size() : start + chunk_size;

            if (start >= entities.size()) {
                break;
            }

            threads.emplace_back([&, start, end, pools]() {
                for (size_t i = start; i < end; ++i) {
                    Entity entity = entities[i];
                    if ((std::get<std::reference_wrapper<SparseSet<Components>>>(pools).get().contains(entity) && ...)) {
                        std::forward<Func>(func)(entity, std::get<std::reference_wrapper<SparseSet<Components>>>(pools).get().get(entity)...);
                    }
                }
            });
        }

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
    Group<Components...>::Group(std::reference_wrapper<Registry> reg) : _registry(reg) {
        rebuild();
    }

    template<typename... Components>
    void Group<Components...>::rebuild() {
        _entities.clear();

        size_t min_size = std::numeric_limits<size_t>::max();
        std::optional<std::reference_wrapper<const std::vector<Entity>>> smallest_entities;

        auto check_pool_size = [&]<typename T>() {
            auto& pool = _registry.get().template getSparseSet<T>();
            const auto& _packed = pool.getPacked();
            if (_packed.size() < min_size) {
                min_size = _packed.size();
                smallest_entities = std::cref(_packed);
            }
        };

        (check_pool_size.template operator()<Components>(), ...);

        if (!smallest_entities.has_value()) {
            return;
        }

        for (auto entity : smallest_entities->get()) {
            if ((_registry.get().template hasComponent<Components>(entity) && ...)) {
                _entities.push_back(entity);
            }
        }
    }

    template<typename... Components>
    template<typename Func>
    void Group<Components...>::each(Func&& func) {
        for (auto entity : _entities) {
            std::forward<Func>(func)(entity, _registry.get().template getComponent<Components>(entity)...);
        }
    }

    // ========================================================================
    // VIEW COMPONENT ACCESS HELPER
    // ========================================================================

    template<typename... Components>
    template<typename T>
    auto View<Components...>::getComponentData(Entity entity, const ISparseSet& pool) -> decltype(auto) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return const_cast<SparseSet<T>&>(static_cast<const SparseSet<T>&>(pool)).get(entity);
    }

    template<typename... Includes, typename... Excludes>
    template<typename T>
    auto ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::getComponentData(
        Entity entity, const ISparseSet& pool
    ) -> decltype(auto) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return const_cast<SparseSet<T>&>(static_cast<const SparseSet<T>&>(pool)).get(entity);
    }

#endif // ECS_CORE_REGISTRY_VIEW_INL
