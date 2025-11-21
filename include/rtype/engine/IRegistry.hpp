/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IRegistry - Public interface for ECS Registry
*/

#pragma once

#include <cstddef>

namespace rtype::engine {

/**
 * @brief Entity handle for the ECS system
 *
 * Lightweight handle to reference entities in the registry.
 * Use with IRegistry to create, destroy and manage entities.
 */
class EntityHandle {
public:
    using Id = std::size_t;

    EntityHandle() : id_(0), valid_(false) {}
    explicit EntityHandle(Id id) : id_(id), valid_(true) {}
       Id id() const { return id_; }
    bool isValid() const { return valid_; }

    bool operator==(const EntityHandle& other) const {
        return id_ == other.id_ && valid_ == other.valid_;
    }

    bool operator!=(const EntityHandle& other) const {
        return !(*this == other);
    }

private:
    Id id_;
    bool valid_;
};

/**
 * @brief Public interface for the Entity Component System registry
 *
 * Use this interface to create and manage entities in the game.
 * The registry is the core of the ECS architecture and manages
 * entity lifecycles.
 *
 * Example usage:
 * @code
 * IRegistry& registry = getRegistry();
 * EntityHandle player = registry.createEntity();
 * registry.destroyEntity(player);
 * @endcode
 */
class IRegistry {
public:
    virtual ~IRegistry() = default;

    /**
     * @brief Create a new entity
     * @return Handle to the created entity
     */
    virtual EntityHandle createEntity() = 0;

    /**
     * @brief Destroy an entity
     * @param entity The entity to destroy
     */
    virtual void destroyEntity(const EntityHandle& entity) = 0;

    /**
     * @brief Get the number of active entities
     * @return Number of entities currently alive
     */
    virtual std::size_t entityCount() const = 0;

    /**
     * @brief Clear all entities from the registry
     */
    virtual void clear() = 0;
};

} // namespace rtype::engine
