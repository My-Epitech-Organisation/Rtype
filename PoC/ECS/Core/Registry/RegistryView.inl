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
    ParallelView<Components...> Registry::parallelView() {
        return ParallelView<Components...>(*this);
    }

    template<typename... Components>
    Group<Components...> Registry::createGroup() {
        return Group<Components...>(*this);
    }

    // ========================================================================
    // RELATIONSHIP ACCESSORS
    // ========================================================================

    inline RelationshipManager& Registry::getRelationshipManager() noexcept {
        return _relationshipManager;
    }

    inline const RelationshipManager& Registry::getRelationshipManager() const noexcept {
        return _relationshipManager;
    }

    // ========================================================================
    // VIEW IMPLEMENTATION
    // ========================================================================

    template<typename... Components>
    View<Components...>::View(std::reference_wrapper<Registry> registry)
        : registry(registry),
          pools(std::ref(static_cast<ISparseSet&>(registry.get().template getSparseSet<Components>()))...) {
        _smallestPoolIndex = findSmallestPool(std::index_sequence_for<Components...>{});
    }

    template<typename... Components>
    template<typename Func>
    void View<Components...>::each(Func&& func) {
        eachImpl(std::forward<Func>(func), std::index_sequence_for<Components...>{});
    }

    template<typename... Components>
    template<typename Func, size_t... Is>
    void View<Components...>::eachImpl(Func&& func, std::index_sequence<Is...>) {
        if (sizeof...(Components) == 0) return;

        auto get_pool_at_index = [this](size_t idx) -> std::optional<std::reference_wrapper<const std::vector<Entity>>> {
            std::optional<std::reference_wrapper<const std::vector<Entity>>> result;
            size_t current = 0;
            ((current++ == idx ? (result = std::cref(std::get<Is>(pools).get().getPacked()), true) : false) || ...);
            return result;
        };

        auto entities_opt = get_pool_at_index(_smallestPoolIndex);
        if (!entities_opt.has_value()) return;

        for (auto entity : entities_opt->get()) {
            if ((std::get<Is>(pools).get().contains(entity) && ...)) {
                func(entity, getComponentData<Components>(entity, std::get<Is>(pools).get())...);
            }
        }
    }

    template<typename... Components>
    template<size_t... Is>
    size_t View<Components...>::findSmallestPool(std::index_sequence<Is...>) {
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
        std::index_sequence<IncIs...>
    ) {
        if (sizeof...(Includes) == 0) return;

        auto get_pool_at_index = [this](size_t idx) -> std::optional<std::reference_wrapper<const std::vector<Entity>>> {
            std::optional<std::reference_wrapper<const std::vector<Entity>>> result;
            size_t current = 0;
            ((current++ == idx ? (result = std::cref(std::get<IncIs>(_includePools).get().getPacked()), true) : false) || ...);
            return result;
        };

        auto entities_opt = get_pool_at_index(_smallestPoolIndex);
        if (!entities_opt.has_value()) return;

        for (auto entity : entities_opt->get()) {
            if ((std::get<IncIs>(_includePools).get().contains(entity) && ...)) {
                if (!is_excluded(entity)) {
                    func(entity, getComponentData<Includes>(entity, std::get<IncIs>(_includePools).get())...);
                }
            }
        }
    }

    template<typename... Includes, typename... Excludes>
    bool ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::is_excluded(
        Entity entity
    ) const {
        for (auto& pool : _excludePools) {
            if (pool.get().contains(entity)) {
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

        if (!smallest_entities.has_value() || smallest_entities->get().empty()) return;

        const auto& entities = smallest_entities->get();

        const size_t num_threads = std::thread::hardware_concurrency();
        const size_t chunk_size = std::max(size_t(1), entities.size() / num_threads);

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (size_t t = 0; t < num_threads; ++t) {
            size_t start = t * chunk_size;
            size_t end = (t == num_threads - 1) ? entities.size() : start + chunk_size;

            if (start >= entities.size()) break;

            threads.emplace_back([&, start, end, pools]() {
                for (size_t i = start; i < end; ++i) {
                    Entity entity = entities[i];
                    if ((std::get<std::reference_wrapper<SparseSet<Components>>>(pools).get().contains(entity) && ...)) {
                        func(entity, std::get<std::reference_wrapper<SparseSet<Components>>>(pools).get().get(entity)...);
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

        if (!smallest_entities.has_value()) return;

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
            func(entity, _registry.get().template getComponent<Components>(entity)...);
        }
    }

    // ========================================================================
    // VIEW COMPONENT ACCESS HELPER
    // ========================================================================

    template<typename... Components>
    template<typename T>
    T& View<Components...>::getComponentData(Entity entity, const ISparseSet& pool) {
        if constexpr (std::is_empty_v<T>) {
            return const_cast<TagSparseSet<T>&>(static_cast<const TagSparseSet<T>&>(pool)).get(entity);
        } else {
            return const_cast<SparseSet<T>&>(static_cast<const SparseSet<T>&>(pool)).get(entity);
        }
    }

    template<typename... Includes, typename... Excludes>
    template<typename T>
    T& ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::getComponentData(
        Entity entity, const ISparseSet& pool
    ) {
        if constexpr (std::is_empty_v<T>) {
            return const_cast<TagSparseSet<T>&>(static_cast<const TagSparseSet<T>&>(pool)).get(entity);
        } else {
            return const_cast<SparseSet<T>&>(static_cast<const SparseSet<T>&>(pool)).get(entity);
        }
    }

#endif // ECS_CORE_REGISTRY_VIEW_INL
