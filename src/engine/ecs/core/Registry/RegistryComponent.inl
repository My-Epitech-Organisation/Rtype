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
    auto Registry::emplaceComponent(Entity entity, Args&&... args) -> decltype(auto) {
        if (!isAlive(entity)) {
            throw std::runtime_error("Cannot add component to dead entity");
        }

        auto type = std::type_index(typeid(T));
        bool is_new_component = false;

        {
            std::unique_lock lock(_entityMutex);

            if (entity.index() >= _generations.size() ||
                _generations[entity.index()] != entity.generation()) {
                throw std::runtime_error("Entity died during component addition");
            }

            auto& components = _entityComponents[entity.index()];
            auto iter = std::find(components.begin(), components.end(), type);
            is_new_component = (iter == components.end());

            if (is_new_component) {
                components.push_back(type);
            }
        }

        if constexpr (std::is_empty_v<T>) {
            const T& result = getSparseSet<T>().emplace(entity, std::forward<Args>(args)...);
            if (is_new_component) {
                _signalDispatcher.dispatchConstruct(type, entity);
            }
            return result;
        } else {
            T& result = getSparseSet<T>().emplace(entity, std::forward<Args>(args)...);
            if (is_new_component) {
                _signalDispatcher.dispatchConstruct(type, entity);
            }
            return result;
        }
    }

    template <typename T, typename... Args>
    auto Registry::getOrEmplace(Entity entity, Args&&... args) -> decltype(auto) {
        if (hasComponent<T>(entity)) {
            return getComponent<T>(entity);
        }
        return emplaceComponent<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void Registry::removeComponent(Entity entity) {
        auto type = std::type_index(typeid(T));

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
        auto type = std::type_index(typeid(T));
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
    auto Registry::hasComponent(Entity entity) const noexcept -> bool {
        auto pool = getSparseSetConst<T>();
        return pool.has_value() && pool->get().contains(entity);
    }

    template <typename T>
    auto Registry::countComponents() const noexcept -> size_t {
        auto pool = getSparseSetConst<T>();
        return pool.has_value() ? pool->get().size() : 0;
    }

    template <typename T>
    auto Registry::getComponent(Entity entity) -> decltype(auto) {
        if (!isAlive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        if constexpr (std::is_empty_v<T>) {
            return getSparseSet<T>().get(entity);  // Returns const T&
        } else {
            return getSparseSet<T>().get(entity);  // Returns T&
        }
    }

    template <typename T>
    auto Registry::getComponent(Entity entity) const -> const T& {
        if (!isAlive(entity)) {
            throw std::runtime_error("Attempted to get component from dead entity");
        }
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have requested component");
        }
        return getSparseSetTypedConst<T>().get().get(entity);
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
        std::forward<Func>(func)(component);
    }

    // ========================================================================
    // ENTITY BULK OPERATIONS
    // ========================================================================

    template<typename Func>
    auto Registry::removeEntitiesIf(Func&& predicate) -> size_t {
        std::vector<Entity> to_remove;

        for (size_t i = 0; i < _generations.size(); ++i) {
            Entity entity(static_cast<std::uint32_t>(i), _generations[i]);
            if (isAlive(entity) && std::forward<Func>(predicate)(entity)) {
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
    // INTERNAL Sparse SET ACCESS
    // ========================================================================

    template <typename T>
    auto Registry::getSparseSet() -> auto& {
        auto type = std::type_index(typeid(T));

        {
            std::shared_lock lock(_componentPoolMutex);
            auto iter = _componentPools.find(type);
            if (iter != _componentPools.end()) {
                if constexpr (std::is_empty_v<T>) {
                    return static_cast<TagSparseSet<T>&>(*iter->second);
                } else {
                    return static_cast<SparseSet<T>&>(*iter->second);
                }
            }
        }

        {
            std::unique_lock lock(_componentPoolMutex);

            auto iter = _componentPools.find(type);
            if (iter == _componentPools.end()) {
                if constexpr (std::is_empty_v<T>) {
                    _componentPools[type] = std::make_unique<TagSparseSet<T>>();
                } else {
                    _componentPools[type] = std::make_unique<SparseSet<T>>();
                }
                iter = _componentPools.find(type);
            }

            if constexpr (std::is_empty_v<T>) {
                return static_cast<TagSparseSet<T>&>(*iter->second);
            } else {
                return static_cast<SparseSet<T>&>(*iter->second);
            }
        }
    }

    template <typename T>
    auto Registry::getSparseSetConst() const noexcept
        -> std::optional<std::reference_wrapper<const ISparseSet>> {
        auto type = std::type_index(typeid(T));
        std::shared_lock lock(_componentPoolMutex);

        auto iter = _componentPools.find(type);
        if (iter == _componentPools.end()) {
            return std::nullopt;
        }

        return std::cref(*iter->second);
    }

    template <typename T>
    auto Registry::getSparseSetTypedConst() const {
        auto type = std::type_index(typeid(T));
        std::shared_lock lock(_componentPoolMutex);

        auto iter = _componentPools.find(type);
        if (iter == _componentPools.end()) {
            throw std::runtime_error("Component pool does not exist");
        }

        if constexpr (std::is_empty_v<T>) {
            return std::cref(static_cast<const TagSparseSet<T>&>(*iter->second));
        } else {
            return std::cref(static_cast<const SparseSet<T>&>(*iter->second));
        }
    }

#endif // ECS_CORE_REGISTRY_COMPONENT_INL
