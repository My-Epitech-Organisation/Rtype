/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry
*/

#ifndef ECS_CORE_REGISTRY_HPP
    #define ECS_CORE_REGISTRY_HPP
    #include "Entity.hpp"
    #include "../Storage/ISparseSet.hpp"
    #include "../Storage/SparseSet.hpp"
    #include "../Storage/TagSparseSet.hpp"
    #include "../Traits/ComponentTraits.hpp"
    #include "../Signal/SignalDispatcher.hpp"
    #include "../View/View.hpp"
    #include "../View/ParallelView.hpp"
    #include "../View/Group.hpp"
    #include "../View/ExcludeView.hpp"
    #include "Relationship.hpp"
    #include <unordered_map>
    #include <typeindex>
    #include <memory>
    #include <vector>
    #include <any>
    #include <algorithm>
    #include <mutex>
    #include <shared_mutex>
    #include <atomic>

namespace ECS {

    /**
     * @brief Central ECS coordinator managing entities, components, and systems.
     *
     * Responsibilities:
     * - Entity lifecycle (creation, destruction, validation)
     * - Component storage and access
     * - View/query creation for system iteration
     * - Signal/observer pattern support
     * - Global singleton resource management
     *
     * Thread Safety:
     * - parallel_view() is safe for reading/modifying DIFFERENT components
     * - DO NOT add/remove entities or components during parallel iteration
     * - DO NOT modify shared state without synchronization in callbacks
     */
    class Registry {
    public:
        Registry() = default;

        void reserve_entities(size_t capacity);

        template<typename T>
        void reserve_components(size_t capacity) {
            get_sparse_set<T>().reserve(capacity);
        }

        /**
         * @brief Releases unused memory from all component pools.
         * Call this after removing many entities/components to reclaim memory.
         */
        void compact() {
            std::shared_lock lock(component_pool_mutex);
            for (auto& [type, pool] : component_pools) {
                pool->shrink_to_fit();
            }
        }

        /**
         * @brief Releases unused memory from a specific component type.
         */
        template<typename T>
        void compact_component() {
            get_sparse_set<T>().shrink_to_fit();
        }

        Entity spawn_entity();
        void kill_entity(Entity entity) noexcept;
        [[nodiscard]] bool is_alive(Entity entity) const noexcept;

        /**
         * @brief Recycles tombstone entities by resetting their generations.
         * Call this periodically to reclaim entity slots. Thread-safe.
         * @return Number of tombstones recycled
         */
        size_t cleanup_tombstones();

        /**
         * @brief Removes all entities matching a predicate.
         * @param predicate Function returning true for entities to remove
         * @return Number of entities removed
         */
        template<typename Func>
        size_t remove_entities_if(Func&& predicate) {
            std::vector<Entity> to_remove;

            for (size_t i = 0; i < generations.size(); ++i) {
                Entity entity(i, generations[i]);
                if (is_alive(entity) && predicate(entity)) {
                    to_remove.push_back(entity);
                }
            }
            for (auto entity : to_remove) {
                kill_entity(entity);
            }

            return to_remove.size();
        }

        /**
         * @brief Constructs component in-place for entity.
         * Triggers on_construct callbacks for new components only.
         * @return Reference to component
         */
        template <typename T, typename... Args>
        T& emplace_component(Entity entity, Args&&... args) {
            if (!is_alive(entity)) {
                throw std::runtime_error("Cannot add component to dead entity");
            }

            std::type_index type = std::type_index(typeid(T));

            bool is_new_component = false;
            {
                std::unique_lock lock(entity_mutex);

                if (entity.index() >= generations.size() ||
                    generations[entity.index()] != entity.generation()) {
                    throw std::runtime_error("Entity died during component addition");
                }

                auto& components = entity_components[entity.index()];
                auto it = std::find(components.begin(), components.end(), type);
                is_new_component = (it == components.end());

                if (is_new_component) {
                    components.push_back(type);
                }
            }

            auto& result = get_sparse_set<T>().emplace(entity, std::forward<Args>(args)...);

            if (is_new_component) {
                signal_dispatcher.dispatch_construct(type, entity);
            }

            return result;
        }

        /**
         * @brief Gets component if exists, otherwise creates it (lazy initialization).
         * Only triggers on_construct callback if component is newly created.
         * @param entity Target entity
         * @param args Constructor arguments (used only if component doesn't exist)
         * @return Reference to component (existing or newly created)
         */
        template <typename T, typename... Args>
        T& get_or_emplace(Entity entity, Args&&... args) {
            if (has_component<T>(entity)) {
                return get_component<T>(entity);
            }
            return emplace_component<T>(entity, std::forward<Args>(args)...);
        }

        /**
         * @brief Removes component from entity.
         * Triggers on_destroy callbacks.
         */
        template <typename T>
        void remove_component(Entity entity) {
            std::type_index type = std::type_index(typeid(T));
            signal_dispatcher.dispatch_destroy(type, entity);
            get_sparse_set<T>().remove(entity);

            {
                std::unique_lock lock(entity_mutex);
                auto& components = entity_components[entity.index()];
                components.erase(std::remove(components.begin(), components.end(), type), components.end());
            }
        }

        /**
         * @brief Removes all components of a specific type from all entities.
         * Triggers on_destroy callbacks for each component removed.
         */
        template <typename T>
        void clear_components() {
            std::type_index type = std::type_index(typeid(T));
            auto& pool = get_sparse_set<T>();

            std::vector<Entity> entities_to_clear = pool.get_packed();

            for (auto entity : entities_to_clear) {
                signal_dispatcher.dispatch_destroy(type, entity);

                {
                    std::unique_lock lock(entity_mutex);
                    auto& components = entity_components[entity.index()];
                    components.erase(std::remove(components.begin(), components.end(), type), components.end());
                }
            }

            pool.clear();
        }

        template <typename T>
        [[nodiscard]] bool has_component(Entity entity) const noexcept {
            const auto* pool = get_sparse_set_const<T>();
            return pool != nullptr && pool->contains(entity);
        }

        /**
         * @brief Returns the number of entities with a specific component.
         * @return Count of entities having component T
         */
        template <typename T>
        [[nodiscard]] size_t count_components() const noexcept {
            const auto* pool = get_sparse_set_const<T>();
            return pool != nullptr ? pool->size() : 0;
        }

        /**
         * @brief Retrieves component reference.
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        T& get_component(Entity entity) {
            if (!is_alive(entity))
                throw std::runtime_error("Attempted to get component from dead entity");
            if (!has_component<T>(entity))
                throw std::runtime_error("Entity does not have requested component");
            return get_sparse_set<T>().get(entity);
        }

        /**
         * @brief Retrieves const component reference.
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        const T& get_component(Entity entity) const {
            if (!is_alive(entity))
                throw std::runtime_error("Attempted to get component from dead entity");
            if (!has_component<T>(entity))
                throw std::runtime_error("Entity does not have requested component");
            return get_sparse_set_typed_const<T>().get(entity);
        }

        /**
         * @brief Modifies component via callback function.
         * Useful for triggering update events or validation after modification.
         * @param entity Target entity
         * @param func Callback that receives mutable reference to component
         */
        template <typename T, typename Func>
        void patch(Entity entity, Func&& func) {
            if (!is_alive(entity))
                throw std::runtime_error("Attempted to patch component on dead entity");
            if (!has_component<T>(entity))
                throw std::runtime_error("Entity does not have component to patch");

            auto& component = get_sparse_set<T>().get(entity);
            func(component);
        }

        /**
         * @brief Registers callback for component addition events.
         */
        template<typename T>
        void on_construct(std::function<void(Entity)> callback) {
            signal_dispatcher.register_construct(std::type_index(typeid(T)), std::move(callback));
        }

        /**
         * @brief Registers callback for component removal events.
         */
        template<typename T>
        void on_destroy(std::function<void(Entity)> callback) {
            signal_dispatcher.register_destroy(std::type_index(typeid(T)), std::move(callback));
        }

        template<typename... Components>
        View<Components...> view() {
            return View<Components...>(*this);
        }

        template<typename... Components>
        View<Components...> view() const {
            return View<Components...>(const_cast<Registry&>(*this));
        }

        template<typename... Components>
        ParallelView<Components...> parallel_view() {
            return ParallelView<Components...>(*this);
        }

        template<typename... Components>
        Group<Components...> create_group() {
            return Group<Components...>(*this);
        }

        /**
         * @brief Creates or updates global singleton resource.
         * @return Reference to singleton
         */
        template<typename T, typename... Args>
        T& set_singleton(Args&&... args) {
            std::type_index type = std::type_index(typeid(T));
            singletons[type] = std::make_any<T>(std::forward<Args>(args)...);
            return std::any_cast<T&>(singletons[type]);
        }

        template<typename T>
        T& get_singleton() {
            return std::any_cast<T&>(singletons.at(std::type_index(typeid(T))));
        }

        template<typename T>
        [[nodiscard]] bool has_singleton() const noexcept {
            return singletons.find(std::type_index(typeid(T))) != singletons.end();
        }

        template<typename T>
        void remove_singleton() noexcept {
            singletons.erase(std::type_index(typeid(T)));
        }

        /**
         * @brief Gets component types for an entity (for testing/debugging).
         * @param entity Target entity
         * @return Vector of type indices for entity's components
         */
        const std::vector<std::type_index>& get_entity_components(Entity entity) const {
            static const std::vector<std::type_index> empty_vector;
            std::shared_lock lock(entity_mutex);
            auto it = entity_components.find(entity.index());
            return (it != entity_components.end()) ? it->second : empty_vector;
        }

        /**
         * @brief Gets the relationship manager for entity hierarchies.
         * @return Reference to relationship manager
         */
        RelationshipManager& get_relationship_manager() noexcept {
            return relationship_manager;
        }

        /**
         * @brief Gets the relationship manager (const version).
         */
        const RelationshipManager& get_relationship_manager() const noexcept {
            return relationship_manager;
        }

    private:
        std::unordered_map<std::uint32_t, std::vector<std::type_index>> entity_components;
        std::vector<std::uint32_t> generations;
        std::vector<std::uint32_t> free_indices;
        std::vector<std::uint32_t> tombstones;
        std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> component_pools;
        std::unordered_map<std::type_index, std::any> singletons;
        SignalDispatcher signal_dispatcher;
        RelationshipManager relationship_manager;

        mutable std::shared_mutex entity_mutex;
        mutable std::shared_mutex component_pool_mutex;

        template <typename T>
        auto& get_sparse_set() {
            std::type_index type = std::type_index(typeid(T));

            {
                std::shared_lock lock(component_pool_mutex);
                auto it = component_pools.find(type);
                if (it != component_pools.end()) {
                    if constexpr (std::is_empty_v<T>) {
                        return *static_cast<TagSparseSet<T>*>(it->second.get());
                    } else {
                        return *static_cast<SparseSet<T>*>(it->second.get());
                    }
                }
            }
            {
                std::unique_lock lock(component_pool_mutex);
                auto it = component_pools.find(type);
                if (it == component_pools.end()) {
                    if constexpr (std::is_empty_v<T>) {
                        component_pools[type] = std::make_unique<TagSparseSet<T>>();
                    } else {
                        component_pools[type] = std::make_unique<SparseSet<T>>();
                    }
                    it = component_pools.find(type);
                }

                if constexpr (std::is_empty_v<T>) {
                    return *static_cast<TagSparseSet<T>*>(it->second.get());
                } else {
                    return *static_cast<SparseSet<T>*>(it->second.get());
                }
            }
        }

        template <typename T>
        const ISparseSet* get_sparse_set_const() const noexcept {
            // Raw pointer is appropriate here: non-owning, temporary observation only.
            // The Registry owns the actual storage via std::unique_ptr in component_pools.
            // Returning nullptr is a valid sentinel value for "component pool not found".
            std::type_index type = std::type_index(typeid(T));
            std::shared_lock lock(component_pool_mutex);
            auto it = component_pools.find(type);
            if (it == component_pools.end()) {
                return nullptr;
            }
            return it->second.get();
        }

        template <typename T>
        const auto& get_sparse_set_typed_const() const {
            std::type_index type = std::type_index(typeid(T));
            std::shared_lock lock(component_pool_mutex);
            auto it = component_pools.find(type);
            if (it == component_pools.end()) {
                throw std::runtime_error("Component pool does not exist");
            }

            if constexpr (std::is_empty_v<T>) {
                return *static_cast<const TagSparseSet<T>*>(it->second.get());
            } else {
                return *static_cast<const SparseSet<T>*>(it->second.get());
            }
        }

        template<typename...> friend class View;
        template<typename...> friend class ParallelView;
        template<typename...> friend class Group;
    };

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

        auto get_pool_at_index = [this](size_t idx) -> const std::vector<Entity>* {
            const std::vector<Entity>* result = nullptr;
            size_t current = 0;
            ((current++ == idx ? (result = &std::get<Is>(pools)->get_packed(), true) : false) || ...);
            return result;
        };

        const auto* entities_ptr = get_pool_at_index(smallest_pool_index);
        if (!entities_ptr) return;

        for (auto entity : *entities_ptr) {
            if ((std::get<Is>(pools)->contains(entity) && ...)) {
                func(entity, std::get<Is>(pools)->get(entity)...);
            }
        }
    }

    template<typename... Components>
    template<size_t... Is>
    size_t View<Components...>::find_smallest_pool(std::index_sequence<Is...>) {
        std::array<size_t, sizeof...(Components)> sizes = {std::get<Is>(pools)->get_packed().size()...};
        return std::distance(sizes.begin(), std::min_element(sizes.begin(), sizes.end()));
    }

    template<typename... Components>
    template<typename... Excluded>
    auto View<Components...>::exclude() {
        std::vector<ISparseSet*> exclude_pools_vec = {static_cast<ISparseSet*>(&registry.template get_sparse_set<Excluded>())...};
        return ExcludeView<std::tuple<Components...>, std::tuple<Excluded...>>(registry, pools, std::move(exclude_pools_vec), smallest_pool_index);
    }

    template<typename... Includes, typename... Excludes>
    template<typename Func>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::each(Func&& func) {
        each_impl(std::forward<Func>(func), std::index_sequence_for<Includes...>{});
    }

    template<typename... Includes, typename... Excludes>
    template<typename Func, size_t... IncIs>
    void ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::each_impl(Func&& func, std::index_sequence<IncIs...>) {
        if (sizeof...(Includes) == 0) return;

        auto get_pool_at_index = [this](size_t idx) -> const std::vector<Entity>* {
            const std::vector<Entity>* result = nullptr;
            size_t current = 0;
            ((current++ == idx ? (result = &std::get<IncIs>(include_pools)->get_packed(), true) : false) || ...);
            return result;
        };

        const auto* entities_ptr = get_pool_at_index(smallest_pool_index);
        if (!entities_ptr) return;

        for (auto entity : *entities_ptr) {
            if ((std::get<IncIs>(include_pools)->contains(entity) && ...)) {
                if (!is_excluded(entity)) {
                    func(entity, std::get<IncIs>(include_pools)->get(entity)...);
                }
            }
        }
    }

    template<typename... Includes, typename... Excludes>
    bool ExcludeView<std::tuple<Includes...>, std::tuple<Excludes...>>::is_excluded(Entity entity) const {
        for (auto* pool : exclude_pools) {
            if (pool->contains(entity)) {
                return true;
            }
        }
        return false;
    }

    template<typename... Components>
    template<typename Func>
    void ParallelView<Components...>::each(Func&& func) {
        std::tuple<SparseSet<Components>*...> pools =
            std::make_tuple(&registry.template get_sparse_set<Components>()...);

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
                    if ((std::get<SparseSet<Components>*>(pools)->contains(entity) && ...)) {
                        func(entity, std::get<SparseSet<Components>*>(pools)->get(entity)...);
                    }
                }
            });
        }

        for (auto& thread : threads) {
            if (thread.joinable())
                thread.join();
        }
    }

    template<typename... Components>
    Group<Components...>::Group(Registry& reg) : registry(reg) {
        rebuild();
    }

    template<typename... Components>
    void Group<Components...>::rebuild() {
        entities.clear();
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

} // namespace ECS

#endif // ECS_CORE_REGISTRY_HPP
