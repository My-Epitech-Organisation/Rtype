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
    void Registry::reserveComponents(size_t capacity) {
        getSparseSet<T>().reserve(capacity);
    }

    inline void Registry::compact() {
        std::shared_lock lock(_componentPoolMutex);
        for (auto& [type, pool] : _componentPools) {
            pool->shrinkToFit();
        }
    }

    template<typename T>
    void Registry::compactComponent() {
        getSparseSet<T>().shrinkToFit();
    }

    // ========================================================================
    // COMPONENT OPERATIONS
    // ========================================================================

    template <typename T, typename... Args>
    T& Registry::emplaceComponent(Entity entity, Args&&... args) {
        if (!isAlive(entity)) {
            throw std::runtime_error("Cannot add component to dead entity");
        }

        std::type_index type = std::type_index(typeid(T));
        bool is_new_component = false;

        {
            std::unique_lock lock(_entityMutex);

            if (entity.index() >= _generations.size() ||
                _generations[entity.index()] != entity.generation()) {
                throw std::runtime_error("Entity died during component addition");
            }

            auto& components = _entityComponents[entity.index()];
            auto it = std::find(components.begin(), components.end(), type);
            is_new_component = (it == components.end());

            if (is_new_component) {
                components.push_back(type);
            }
        }

        auto& result = getSparseSet<T>().emplace(entity, std::forward<Args>(args)...);

        if (is_new_component) {
            _signalDispatcher.dispatchConstruct(type, entity);
        }

        return result;
    }

    template <typename T, typename... Args>
    T& Registry::getOrEmplace(Entity entity, Args&&... args) {
        if (hasComponent<T>(entity)) {
            return getComponent<T>(entity);
        }
        return emplaceComponent<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void Registry::removeComponent(Entity entity) {
        std::type_index type = std::type_index(typeid(T));

        _signalDispatcher.dispatchDestroy(type, entity);

        getSparseSet<T>().remove(entity);

        {
            std::unique_lock lock(_entityMutex);
            auto& components = _entityComponents[entity.index()];
            components.erase(
                std::remove(components.begin(), components.end(), type),
                components.end()
            );
        }
    }

    template <typename T>
    void Registry::clearComponents() {
        std::type_index type = std::type_index(typeid(T));
        auto& pool = getSparseSet<T>();

        std::vector<Entity> entities_to_clear = pool.getPacked();

        for (auto entity : entities_to_clear) {
            _signalDispatcher.dispatchDestroy(type, entity);

            {
                std::unique_lock lock(_entityMutex);
                auto& components = _entityComponents[entity.index()];
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
    bool Registry::hasComponent(Entity entity) const noexcept {
        const auto* pool = getSparseSetConst<T>();
        return pool != nullptr && pool->contains(entity);
    }

    template <typename T>
    size_t Registry::countComponents() const noexcept {
        const auto* pool = getSparseSetConst<T>();
        return pool != nullptr ? pool->size() : 0;
    }

    template <typename T>
    T& Registry::getComponent(Entity entity) {
        if (!isAlive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        return getSparseSet<T>().get(entity);
    }

    template <typename T>
    const T& Registry::getComponent(Entity entity) const {
        if (!isAlive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        return getSparseSetTypedConst<T>().get(entity);
    }

    template <typename T, typename Func>
    void Registry::patch(Entity entity, Func&& func) {
        if (!isAlive(entity)) {
            throw std::runtime_error("Attempted to patch component on dead entity");
        }
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have component to patch");
        }

        auto& component = getSparseSet<T>().get(entity);
        func(component);
    }

    // ========================================================================
    // ENTITY BULK OPERATIONS
    // ========================================================================

    template<typename Func>
    size_t Registry::removeEntitiesIf(Func&& predicate) {
        std::vector<Entity> to_remove;

        for (size_t i = 0; i < _generations.size(); ++i) {
            Entity entity(i, _generations[i]);
            if (isAlive(entity) && predicate(entity)) {
                to_remove.push_back(entity);
            }
        }

        for (auto entity : to_remove) {
            killEntity(entity);
        }

        return to_remove.size();
    }

    // ========================================================================
    // SIGNAL/OBSERVER REGISTRATION
    // ========================================================================

    template<typename T>
    void Registry::onConstruct(std::function<void(Entity)> callback) {
        _signalDispatcher.registerConstruct(
            std::type_index(typeid(T)),
            std::move(callback)
        );
    }

    template<typename T>
    void Registry::onDestroy(std::function<void(Entity)> callback) {
        _signalDispatcher.registerDestroy(
            std::type_index(typeid(T)),
            std::move(callback)
        );
    }

    // ========================================================================
    // INTERNAL _sparse SET ACCESS
    // ========================================================================

    template <typename T>
    auto& Registry::getSparseSet() {
        std::type_index type = std::type_index(typeid(T));

        {
            std::shared_lock lock(_componentPoolMutex);
            auto it = _componentPools.find(type);
            if (it != _componentPools.end()) {
                if constexpr (std::is_empty_v<T>) {
                    return *static_cast<TagSparseSet<T>*>(it->second.get());
                } else {
                    return *static_cast<SparseSet<T>*>(it->second.get());
                }
            }
        }

        {
            std::unique_lock lock(_componentPoolMutex);

            auto it = _componentPools.find(type);
            if (it == _componentPools.end()) {
                if constexpr (std::is_empty_v<T>) {
                    _componentPools[type] = std::make_unique<TagSparseSet<T>>();
                } else {
                    _componentPools[type] = std::make_unique<SparseSet<T>>();
                }
                it = _componentPools.find(type);
            }

            if constexpr (std::is_empty_v<T>) {
                return *static_cast<TagSparseSet<T>*>(it->second.get());
            } else {
                return *static_cast<SparseSet<T>*>(it->second.get());
            }
        }
    }

    template <typename T>
    const ISparseSet* Registry::getSparseSetConst() const noexcept {
        // Raw pointer is appropriate here: non-owning, temporary observation only.
        // The Registry owns the actual storage via std::unique_ptr in _componentPools.
        // Returning nullptr is a valid sentinel value for "component pool not found".
        std::type_index type = std::type_index(typeid(T));
        std::shared_lock lock(_componentPoolMutex);

        auto it = _componentPools.find(type);
        if (it == _componentPools.end()) {
            return nullptr;
        }

        return it->second.get();
    }

    template <typename T>
    const auto& Registry::getSparseSetTypedConst() const {
        std::type_index type = std::type_index(typeid(T));
        std::shared_lock lock(_componentPoolMutex);

        auto it = _componentPools.find(type);
        if (it == _componentPools.end()) {
            throw std::runtime_error("Component pool does not exist");
        }

        if constexpr (std::is_empty_v<T>) {
            return *static_cast<const TagSparseSet<T>*>(it->second.get());
        } else {
            return *static_cast<const SparseSet<T>*>(it->second.get());
        }
    }

#endif // ECS_CORE_REGISTRY_COMPONENT_INL
