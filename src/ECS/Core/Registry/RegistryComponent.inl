/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Component Management Template Implementations
*/

#ifndef ECS_CORE_REGISTRY_COMPONENT_INL
    #define ECS_CORE_REGISTRY_COMPONENT_INL

// This file is included inside the ECS namespace in Registry.hpp
// Do not add namespace ECS here!

// ========================================================================
// MEMORY MANAGEMENT
// ========================================================================

    template<typename T>
    void Registry::reserve_components(size_t capacity) {
        get_sparse_set<T>().reserve(capacity);
    }

    inline void Registry::compact() {
        std::shared_lock lock(component_pool_mutex);
        for (auto& [type, pool] : component_pools) {
            pool->shrink_to_fit();
        }
    }

    template<typename T>
    void Registry::compact_component() {
        get_sparse_set<T>().shrink_to_fit();
    }

    // ========================================================================
    // COMPONENT OPERATIONS
    // ========================================================================

    template <typename T, typename... Args>
    T& Registry::emplace_component(Entity entity, Args&&... args) {
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

    template <typename T, typename... Args>
    T& Registry::get_or_emplace(Entity entity, Args&&... args) {
        if (has_component<T>(entity)) {
            return get_component<T>(entity);
        }
        return emplace_component<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void Registry::remove_component(Entity entity) {
        std::type_index type = std::type_index(typeid(T));

        signal_dispatcher.dispatch_destroy(type, entity);

        get_sparse_set<T>().remove(entity);

        {
            std::unique_lock lock(entity_mutex);
            auto& components = entity_components[entity.index()];
            components.erase(
                std::remove(components.begin(), components.end(), type),
                components.end()
            );
        }
    }

    template <typename T>
    void Registry::clear_components() {
        std::type_index type = std::type_index(typeid(T));
        auto& pool = get_sparse_set<T>();

        std::vector<Entity> entities_to_clear = pool.get_packed();

        for (auto entity : entities_to_clear) {
            signal_dispatcher.dispatch_destroy(type, entity);

            {
                std::unique_lock lock(entity_mutex);
                auto& components = entity_components[entity.index()];
                components.erase(
                    std::remove(components.begin(), components.end(), type),
                    components.end()
                );
            }
        }

        pool.clear();
    }

    // ========================================================================
    // COMPONENT QUERIES
    // ========================================================================

    template <typename T>
    bool Registry::has_component(Entity entity) const noexcept {
        const auto* pool = get_sparse_set_const<T>();
        return pool != nullptr && pool->contains(entity);
    }

    template <typename T>
    size_t Registry::count_components() const noexcept {
        const auto* pool = get_sparse_set_const<T>();
        return pool != nullptr ? pool->size() : 0;
    }

    template <typename T>
    T& Registry::get_component(Entity entity) {
        if (!is_alive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!has_component<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        return get_sparse_set<T>().get(entity);
    }

    template <typename T>
    const T& Registry::get_component(Entity entity) const {
        if (!is_alive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!has_component<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        return get_sparse_set_typed_const<T>().get(entity);
    }

    template <typename T, typename Func>
    void Registry::patch(Entity entity, Func&& func) {
        if (!is_alive(entity)) {
            throw std::runtime_error("Attempted to patch component on dead entity");
        }
        if (!has_component<T>(entity)) {
            throw std::runtime_error("Entity does not have component to patch");
        }

        auto& component = get_sparse_set<T>().get(entity);
        func(component);
    }

    // ========================================================================
    // ENTITY BULK OPERATIONS
    // ========================================================================

    template<typename Func>
    size_t Registry::remove_entities_if(Func&& predicate) {
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

    // ========================================================================
    // SIGNAL/OBSERVER REGISTRATION
    // ========================================================================

    template<typename T>
    void Registry::on_construct(std::function<void(Entity)> callback) {
        signal_dispatcher.register_construct(
            std::type_index(typeid(T)),
            std::move(callback)
        );
    }

    template<typename T>
    void Registry::on_destroy(std::function<void(Entity)> callback) {
        signal_dispatcher.register_destroy(
            std::type_index(typeid(T)),
            std::move(callback)
        );
    }

    // ========================================================================
    // INTERNAL SPARSE SET ACCESS
    // ========================================================================

    template <typename T>
    auto& Registry::get_sparse_set() {
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
    const ISparseSet* Registry::get_sparse_set_const() const noexcept {
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
    const auto& Registry::get_sparse_set_typed_const() const {
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

#endif // ECS_CORE_REGISTRY_COMPONENT_INL
