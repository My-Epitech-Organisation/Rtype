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

        // ========================================================================
        // ENTITY MANAGEMENT
        // ========================================================================

        /**
         * @brief Pre-allocates memory for entities to reduce allocations.
         * @param capacity Expected number of entities
         */
        void reserve_entities(size_t capacity);

        /**
         * @brief Creates a new entity with a unique ID.
         * @return New entity handle
         */
        Entity spawn_entity();

        /**
         * @brief Destroys an entity and all its components.
         * @param entity Entity to destroy (safe to call on dead entities)
         */
        void kill_entity(Entity entity) noexcept;

        /**
         * @brief Checks if an entity is still valid.
         * @param entity Entity to check
         * @return true if entity exists and generation matches
         */
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
        size_t remove_entities_if(Func&& predicate);

        // ========================================================================
        // COMPONENT MANAGEMENT
        // ========================================================================

        /**
         * @brief Pre-allocates memory for components of a specific type.
         * @tparam T Component type
         * @param capacity Expected number of components
         */
        template<typename T>
        void reserve_components(size_t capacity);

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
        void compact_component();

        /**
         * @brief Constructs component in-place for entity.
         * Triggers on_construct callbacks for new components only.
         * @tparam T Component type
         * @param entity Target entity
         * @param args Constructor arguments
         * @return Reference to component
         */
        template <typename T, typename... Args>
        T& emplace_component(Entity entity, Args&&... args);

        /**
         * @brief Gets component if exists, otherwise creates it (lazy initialization).
         * Only triggers on_construct callback if component is newly created.
         * @tparam T Component type
         * @param entity Target entity
         * @param args Constructor arguments (used only if component doesn't exist)
         * @return Reference to component (existing or newly created)
         */
        template <typename T, typename... Args>
        T& get_or_emplace(Entity entity, Args&&... args);

        /**
         * @brief Removes component from entity.
         * Triggers on_destroy callbacks.
         * @tparam T Component type
         * @param entity Target entity
         */
        template <typename T>
        void remove_component(Entity entity);

        /**
         * @brief Removes all components of a specific type from all entities.
         * Triggers on_destroy callbacks for each component removed.
         * @tparam T Component type
         */
        template <typename T>
        void clear_components();

        /**
         * @brief Checks if entity has a component.
         * @tparam T Component type
         * @param entity Target entity
         * @return true if entity has component
         */
        template <typename T>
        [[nodiscard]] bool has_component(Entity entity) const noexcept;

        /**
         * @brief Returns the number of entities with a specific component.
         * @tparam T Component type
         * @return Count of entities having component T
         */
        template <typename T>
        [[nodiscard]] size_t count_components() const noexcept;

        /**
         * @brief Retrieves component reference.
         * @tparam T Component type
         * @param entity Target entity
         * @return Mutable reference to component
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        T& get_component(Entity entity);

        /**
         * @brief Retrieves const component reference.
         * @tparam T Component type
         * @param entity Target entity
         * @return Const reference to component
         * @throws std::runtime_error if entity dead or component missing
         */
        template <typename T>
        const T& get_component(Entity entity) const;

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
        void on_construct(std::function<void(Entity)> callback);

        /**
         * @brief Registers callback for component removal events.
         * @tparam T Component type to observe
         * @param callback Function called when component is removed
         */
        template<typename T>
        void on_destroy(std::function<void(Entity)> callback);

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
        ParallelView<Components...> parallel_view();

        /**
         * @brief Creates a group for cached entity sets.
         * @tparam Components Component types to group
         * @return Group object with pre-filtered entities
         */
        template<typename... Components>
        Group<Components...> create_group();

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
        T& set_singleton(Args&&... args);

        /**
         * @brief Retrieves singleton resource.
         * @tparam T Singleton type
         * @return Reference to singleton
         * @throws std::out_of_range if singleton doesn't exist
         */
        template<typename T>
        T& get_singleton();

        /**
         * @brief Checks if singleton exists.
         * @tparam T Singleton type
         * @return true if singleton is registered
         */
        template<typename T>
        [[nodiscard]] bool has_singleton() const noexcept;

        /**
         * @brief Removes singleton resource.
         * @tparam T Singleton type
         */
        template<typename T>
        void remove_singleton() noexcept;

        // ========================================================================
        // RELATIONSHIPS
        // ========================================================================

        /**
         * @brief Gets the relationship manager for entity hierarchies.
         * @return Reference to relationship manager
         */
        RelationshipManager& get_relationship_manager() noexcept;

        /**
         * @brief Gets the relationship manager (const version).
         * @return Const reference to relationship manager
         */
        const RelationshipManager& get_relationship_manager() const noexcept;

        // ========================================================================
        // DEBUGGING/INTROSPECTION
        // ========================================================================

        /**
         * @brief Gets component types for an entity (for testing/debugging).
         * @param entity Target entity
         * @return Vector of type indices for entity's components
         */
        const std::vector<std::type_index>& get_entity_components(Entity entity) const;

    private:
        // ========================================================================
        // INTERNAL DATA STRUCTURES
        // ========================================================================

        // Entity management
        std::unordered_map<std::uint32_t, std::vector<std::type_index>> entity_components;
        std::vector<std::uint32_t> generations;
        std::vector<std::uint32_t> free_indices;
        std::vector<std::uint32_t> tombstones;

        // Component storage
        std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> component_pools;

        // Global resources
        std::unordered_map<std::type_index, std::any> singletons;

        // Systems
        SignalDispatcher signal_dispatcher;
        RelationshipManager relationship_manager;

        // Thread safety
        mutable std::shared_mutex entity_mutex;
        mutable std::shared_mutex component_pool_mutex;

        // ========================================================================
        // INTERNAL HELPERS
        // ========================================================================

        /**
         * @brief Gets or creates sparse set for component type.
         * @tparam T Component type
         * @return Reference to sparse set
         */
        template <typename T>
        auto& get_sparse_set();

        /**
         * @brief Gets const sparse set pointer (returns nullptr if not found).
         * @tparam T Component type
         * @return Pointer to sparse set or nullptr
         */
        template <typename T>
        const ISparseSet* get_sparse_set_const() const noexcept;

        /**
         * @brief Gets typed const sparse set (throws if not found).
         * @tparam T Component type
         * @return Const reference to typed sparse set
         */
        template <typename T>
        const auto& get_sparse_set_typed_const() const;

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
