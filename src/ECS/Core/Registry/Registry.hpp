/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Main interface
*/

#ifndef ECS_CORE_REGISTRY_REGISTRY_HPP
    #define ECS_CORE_REGISTRY_REGISTRY_HPP
    #include "../Entity.hpp"
    #include "../../Storage/ISparseSet.hpp"
    #include "../../Storage/SparseSet.hpp"
    #include "../../Storage/TagSparseSet.hpp"
    #include "../../Traits/ComponentTraits.hpp"
    #include "../../Signal/SignalDispatcher.hpp"
    #include "../../View/View.hpp"
    #include "../../View/ParallelView.hpp"
    #include "../../View/Group.hpp"
    #include "../../View/ExcludeView.hpp"
    #include "../Relationship.hpp"
    #include <unordered_map>
    #include <typeindex>
    #include <memory>
    #include <vector>
    #include <any>
    #include <optional>
    #include <functional>
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
     * - parallelView() is safe for reading/modifying DIFFERENT components
     * - DO NOT add/remove entities or components during parallel iteration
     * - DO NOT modify shared state without synchronization in callbacks
     */
    class Registry {
    public:
        Registry() = default;

        // ========================================================================
        // ENTITY MANAGEMENT
        // ========================================================================

        /**
         * @brief Pre-allocates memory for entities to reduce allocations.
         * @param capacity Expected number of entities
         */
        void reserveEntities(size_t capacity);

        /**
         * @brief Creates a new entity with a unique ID.
         * @return New entity handle
         */
        Entity spawnEntity();

        /**
         * @brief Destroys an entity and all its components.
         * @param entity Entity to destroy (safe to call on dead entities)
         */
        void killEntity(Entity entity) noexcept;

        /**
         * @brief Checks if an entity is still valid.
         * @param entity Entity to check
         * @return true if entity exists and generation matches
         */
        [[nodiscard]] bool isAlive(Entity entity) const noexcept;

        /**
         * @brief Recycles tombstone entities by resetting their generations.
         * Call this periodically to reclaim entity slots. Thread-safe.
         * @return Number of tombstones recycled
         */
        size_t cleanupTombstones();

        /**
         * @brief Removes all entities matching a predicate.
         * @param predicate Function returning true for entities to remove
         * @return Number of entities removed
         */
        template<typename Func>
        size_t removeEntitiesIf(Func&& predicate);

        // ========================================================================
        // COMPONENT MANAGEMENT
        // ========================================================================

        /**
         * @brief Pre-allocates memory for components of a specific type.
         * @tparam T Component type
         * @param capacity Expected number of components
         */
        template<typename T>
        void reserveComponents(size_t capacity);

        /**
         * @brief Releases unused memory from all component pools.
         * Call this after removing many entities/components to reclaim memory.
         */
        void compact();

        /**
         * @brief Releases unused memory from a specific component type.
         * @tparam T Component type to compact
         */
        template<typename T>
        void compactComponent();

        /**
         * @brief Constructs component in-place for entity.
         * Triggers onConstruct callbacks for new components only.
         * @tparam T Component type
         * @param entity Target entity
         * @param args Constructor arguments
         * @return Reference to component
         */
        template <typename T, typename... Args>
        T& emplaceComponent(Entity entity, Args&&... args);

        /**
         * @brief Gets component if exists, otherwise creates it (lazy initialization).
         * Only triggers onConstruct callback if component is newly created.
         * @tparam T Component type
         * @param entity Target entity
         * @param args Constructor arguments (used only if component doesn't exist)
         * @return Reference to component (existing or newly created)
         */
        template <typename T, typename... Args>
        T& getOrEmplace(Entity entity, Args&&... args);

        /**
         * @brief Removes component from entity.
         * Triggers onDestroy callbacks.
         * @tparam T Component type
         * @param entity Target entity
         */
        template <typename T>
        void removeComponent(Entity entity);

        /**
         * @brief Removes all components of a specific type from all entities.
         * Triggers onDestroy callbacks for each component removed.
         * @tparam T Component type
         */
        template <typename T>
        void clearComponents();

        /**
         * @brief Checks if entity has a component.
         * @tparam T Component type
         * @param entity Target entity
         * @return true if entity has component
         */
        template <typename T>
        [[nodiscard]] bool hasComponent(Entity entity) const noexcept;

        /**
         * @brief Returns the number of entities with a specific component.
         * @tparam T Component type
         * @return Count of entities having component T
         */
        template <typename T>
        [[nodiscard]] size_t countComponents() const noexcept;

        /**
         * @brief Retrieves component reference.
         * @tparam T Component type
         * @param entity Target entity
         * @return Mutable reference to component
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        T& getComponent(Entity entity);

        /**
         * @brief Retrieves const component reference.
         * @tparam T Component type
         * @param entity Target entity
         * @return Const reference to component
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        const T& getComponent(Entity entity) const;

        /**
         * @brief Modifies component via callback function.
         * Useful for triggering update events or validation after modification.
         * @tparam T Component type
         * @param entity Target entity
         * @param func Callback that receives mutable reference to component
         */
        template <typename T, typename Func>
        void patch(Entity entity, Func&& func);

        // ========================================================================
        // SIGNAL/OBSERVER PATTERN
        // ========================================================================

        /**
         * @brief Registers callback for component addition events.
         * @tparam T Component type to observe
         * @param callback Function called when component is added
         */
        template<typename T>
        void onConstruct(std::function<void(Entity)> callback);

        /**
         * @brief Registers callback for component removal events.
         * @tparam T Component type to observe
         * @param callback Function called when component is removed
         */
        template<typename T>
        void onDestroy(std::function<void(Entity)> callback);

        // ========================================================================
        // VIEW/QUERY SYSTEM
        // ========================================================================

        /**
         * @brief Creates a view for iterating entities with specific components.
         * @tparam Components Component types to query
         * @return View object for iteration
         */
        template<typename... Components>
        View<Components...> view();

        /**
         * @brief Creates a const view for read-only iteration.
         * @tparam Components Component types to query
         * @return Const view object
         */
        template<typename... Components>
        View<Components...> view() const;

        /**
         * @brief Creates a parallel view for multi-threaded iteration.
         * @tparam Components Component types to query
         * @return ParallelView object for parallel processing
         */
        template<typename... Components>
        ParallelView<Components...> parallelView();

        /**
         * @brief Creates a group for cached entity sets.
         * @tparam Components Component types to group
         * @return Group object with pre-filtered entities
         */
        template<typename... Components>
        Group<Components...> createGroup();

        // ========================================================================
        // SINGLETON RESOURCES
        // ========================================================================

        /**
         * @brief Creates or updates global singleton resource.
         * @tparam T Singleton type
         * @param args Constructor arguments
         * @return Reference to singleton
         */
        template<typename T, typename... Args>
        T& setSingleton(Args&&... args);

        /**
         * @brief Retrieves singleton resource.
         * @tparam T Singleton type
         * @return Reference to singleton
         * @throws std::out_of_range if singleton doesn't exist
         */
        template<typename T>
        T& getSingleton();

        /**
         * @brief Checks if singleton exists.
         * @tparam T Singleton type
         * @return true if singleton is registered
         */
        template<typename T>
        [[nodiscard]] bool hasSingleton() const noexcept;

        /**
         * @brief Removes singleton resource.
         * @tparam T Singleton type
         */
        template<typename T>
        void removeSingleton() noexcept;

        // ========================================================================
        // RELATIONSHIPS
        // ========================================================================

        /**
         * @brief Gets the relationship manager for entity hierarchies.
         * @return Reference to relationship manager
         */
        RelationshipManager& getRelationshipManager() noexcept;

        /**
         * @brief Gets the relationship manager (const version).
         * @return Const reference to relationship manager
         */
        const RelationshipManager& getRelationshipManager() const noexcept;

        // ========================================================================
        // DEBUGGING/INTROSPECTION
        // ========================================================================

        /**
         * @brief Gets component types for an entity (for testing/debugging).
         * @param entity Target entity
         * @return Vector of type indices for entity's components
         */
        const std::vector<std::type_index>& getEntityComponents(Entity entity) const;

    private:
        // ========================================================================
        // INTERNAL DATA STRUCTURES
        // ========================================================================

        // Entity management
        std::unordered_map<std::uint32_t, std::vector<std::type_index>> _entityComponents;
        std::vector<std::uint32_t> _generations;
        std::vector<std::uint32_t> _freeIndices;
        std::vector<std::uint32_t> _tombstones;

        // Component storage
        std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> _componentPools;

        // Global resources
        std::unordered_map<std::type_index, std::any> _singletons;

        // Systems
        SignalDispatcher _signalDispatcher;
        RelationshipManager _relationshipManager;

        // Thread safety
        mutable std::shared_mutex _entityMutex;
        mutable std::shared_mutex _componentPoolMutex;

        // ========================================================================
        // INTERNAL HELPERS
        // ========================================================================

        /**
         * @brief Gets or creates _sparse set for component type.
         * @tparam T Component type
         * @return Reference to _sparse set
         */
        template <typename T>
        auto& getSparseSet();

        /**
         * @brief Gets const _sparse set reference (returns std::nullopt if not found).
         * @tparam T Component type
         * @return Optional reference to _sparse set or std::nullopt
         */
        template <typename T>
        std::optional<std::reference_wrapper<const ISparseSet>> getSparseSetConst() const noexcept;

        /**
         * @brief Gets typed const _sparse set (throws if not found).
         * @tparam T Component type
         * @return Reference wrapper to typed _sparse set
         */
        template <typename T>
        auto getSparseSetTypedConst() const;

        // Friend declarations for view access
        template<typename...> friend class View;
        template<typename...> friend class ParallelView;
        template<typename...> friend class Group;
    };

    // Include template implementations (must be inside namespace)
    #include "RegistryComponent.inl"
    #include "RegistrySingleton.inl"
    #include "RegistryView.inl"

} // namespace ECS

#endif // ECS_CORE_REGISTRY_REGISTRY_HPP
