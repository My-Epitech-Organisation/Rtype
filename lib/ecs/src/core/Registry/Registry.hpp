/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Main interface
*/

#ifndef SRC_ENGINE_ECS_CORE_REGISTRY_REGISTRY_HPP_
#define SRC_ENGINE_ECS_CORE_REGISTRY_REGISTRY_HPP_

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "../../signal/SignalDispatcher.hpp"
#include "../../storage/ISparseSet.hpp"
#include "../../storage/SparseSet.hpp"
#include "../../traits/ComponentTraits.hpp"
#include "../../view/ExcludeView.hpp"
#include "../../view/Group.hpp"
#include "../../view/ParallelView.hpp"
#include "../../view/View.hpp"
#include "../Entity.hpp"
#include "../Relationship.hpp"

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
    auto spawnEntity() -> Entity;

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
    [[nodiscard]] auto isAlive(Entity entity) const noexcept -> bool;

    /**
     * @brief Removes all entities, components, and singletons.
     * Use this when shutting down or switching major states.
     */
    void clear();

    /**
     * @brief Recycles tombstone entities by resetting their generations.
     * Call this periodically to reclaim entity slots. Thread-safe.
     * @return Number of tombstones recycled
     */
    auto cleanupTombstones() -> size_t;

    /**
     * @brief Removes all entities matching a predicate.
     * @param predicate Function returning true for entities to remove
     * @return Number of entities removed
     */
    template <typename Func>
    auto removeEntitiesIf(Func&& predicate) -> size_t;

    // ========================================================================
    // COMPONENT MANAGEMENT
    // ========================================================================

    /**
     * @brief Pre-allocates memory for components of a specific type.
     * @tparam T Component type
     * @param capacity Expected number of components
     */
    template <typename T>
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
    template <typename T>
    void compactComponent();

    /**
     * @brief Constructs component in-place for entity.
     * Triggers onConstruct callbacks for new components only.
     * @tparam T Component type
     * @param entity Target entity
     * @param args Constructor arguments
     * @return Reference to component (const for tags, mutable for data
     * components)
     */
    template <typename T, typename... Args>
    auto emplaceComponent(Entity entity, Args&&... args) -> decltype(auto);

    /**
     * @brief Gets component if exists, otherwise creates it (lazy
     * initialization). Only triggers onConstruct callback if component is newly
     * created.
     * @tparam T Component type
     * @param entity Target entity
     * @param args Constructor arguments (used only if component doesn't exist)
     * @return Reference to component (const for tags, mutable for data
     * components)
     */
    template <typename T, typename... Args>
    auto getOrEmplace(Entity entity, Args&&... args) -> decltype(auto);

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
    [[nodiscard]] auto hasComponent(Entity entity) const noexcept -> bool;

    /**
     * @brief Returns the number of entities with a specific component.
     * @tparam T Component type
     * @return Count of entities having component T
     */
    template <typename T>
    [[nodiscard]] auto countComponents() const noexcept -> size_t;

    /**
     * @brief Retrieves component reference.
     * @tparam T Component type
     * @param entity Target entity
     * @return Reference to component (const for tags, mutable for data
     * components)
     * @throws std::runtime_error if entity dead or component missing
     */
    template <typename T>
    auto getComponent(Entity entity) -> decltype(auto);

    /**
     * @brief Retrieves const component reference.
     * @tparam T Component type
     * @param entity Target entity
     * @return Const reference to component
     * @throws std::runtime_error if entity dead or component missing
     */
    template <typename T>
    auto getComponent(Entity entity) const -> const T&;

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
    template <typename T>
    void onConstruct(std::function<void(Entity)> callback);

    /**
     * @brief Registers callback for component removal events.
     * @tparam T Component type to observe
     * @param callback Function called when component is removed
     */
    template <typename T>
    void onDestroy(std::function<void(Entity)> callback);

    // ========================================================================
    // VIEW/QUERY SYSTEM
    // ========================================================================

    /**
     * @brief Creates a view for iterating entities with specific components.
     * @tparam Components Component types to query
     * @return View object for iteration
     */
    template <typename... Components>
    auto view() -> View<Components...>;

    /**
     * @brief Creates a const view for read-only iteration.
     * @tparam Components Component types to query
     * @return Const view object
     */
    template <typename... Components>
    auto view() const -> View<Components...>;

    /**
     * @brief Creates a parallel view for multi-threaded iteration.
     * @tparam Components Component types to query
     * @return ParallelView object for parallel processing
     */
    template <typename... Components>
    auto parallelView() -> ParallelView<Components...>;

    /**
     * @brief Creates a group for cached entity sets.
     * @tparam Components Component types to group
     * @return Group object with pre-filtered entities
     */
    template <typename... Components>
    auto createGroup() -> Group<Components...>;

    // ========================================================================
    // SINGLETON RESOURCES
    // ========================================================================

    /**
     * @brief Creates or updates global singleton resource.
     * @tparam T Singleton type
     * @param args Constructor arguments
     * @return Reference to singleton
     */
    template <typename T, typename... Args>
    auto setSingleton(Args&&... args) -> T&;

    /**
     * @brief Retrieves singleton resource.
     * @tparam T Singleton type
     * @return Reference to singleton
     * @throws std::out_of_range if singleton doesn't exist
     */
    template <typename T>
    auto getSingleton() -> T&;

    /**
     * @brief Checks if singleton exists.
     * @tparam T Singleton type
     * @return true if singleton is registered
     */
    template <typename T>
    [[nodiscard]] auto hasSingleton() const noexcept -> bool;

    /**
     * @brief Removes singleton resource.
     * @tparam T Singleton type
     */
    template <typename T>
    void removeSingleton() noexcept;

    // ========================================================================
    // RELATIONSHIPS
    // ========================================================================

    /**
     * @brief Gets the relationship manager for entity hierarchies.
     * @return Reference to relationship manager
     */
    auto getRelationshipManager() noexcept -> RelationshipManager&;

    /**
     * @brief Gets the relationship manager (const version).
     * @return Const reference to relationship manager
     */
    auto getRelationshipManager() const noexcept -> const RelationshipManager&;

    // ========================================================================
    // DEBUGGING/INTROSPECTION
    // ========================================================================

    /**
     * @brief Gets component types for an entity (for testing/debugging).
     * @param entity Target entity
     * @return Vector of type indices for entity's components
     */
    [[nodiscard]] auto getEntityComponents(Entity entity) const
        -> const std::vector<std::type_index>&;

   private:
    // ========================================================================
    // INTERNAL DATA STRUCTURES
    // ========================================================================

    // Entity management
    std::unordered_map<std::uint32_t, std::vector<std::type_index>>
        _entityComponents;
    std::vector<std::uint32_t> _generations;
    std::vector<std::uint32_t> _freeIndices;
    std::vector<std::uint32_t> _tombstones;

    // Component storage
    std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>>
        _componentPools;

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
    auto getSparseSet() -> auto&;

    /**
     * @brief Gets const _sparse set reference (returns std::nullopt if not
     * found).
     * @tparam T Component type
     * @return Optional reference to _sparse set or std::nullopt
     */
    template <typename T>
    auto getSparseSetConst() const noexcept
        -> std::optional<std::reference_wrapper<const ISparseSet>>;

    /**
     * @brief Gets typed const _sparse set (throws if not found).
     * @tparam T Component type
     * @return Reference wrapper to typed _sparse set
     */
    template <typename T>
    auto getSparseSetTypedConst() const;

    // Friend declarations for view access
    template <typename...>
    friend class View;
    template <typename...>
    friend class ParallelView;
    template <typename...>
    friend class Group;
};

// Include template implementations (must be inside namespace)
#include "RegistryComponent.inl"
#include "RegistrySingleton.inl"
#include "RegistryView.inl"

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_CORE_REGISTRY_REGISTRY_HPP_
