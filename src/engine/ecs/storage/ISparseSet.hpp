/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISparseSet
*/

#ifndef SRC_ENGINE_ECS_STORAGE_ISPARSESET_HPP_
#define SRC_ENGINE_ECS_STORAGE_ISPARSESET_HPP_

#include <vector>

#include "../core/Entity.hpp"

namespace ECS {

/**
 * @brief Type-erased interface for component storage containers.
 *
 * Enables heterogeneous storage of different component types in a single collection.
 * All concrete SparseSet implementations must provide these operations.
 */
class ISparseSet {
 public:
    ISparseSet() = default;
    virtual ~ISparseSet() = default;

    ISparseSet(const ISparseSet&) = delete;
    auto operator=(const ISparseSet&) -> ISparseSet& = delete;
    ISparseSet(ISparseSet&&) = delete;
    auto operator=(ISparseSet&&) -> ISparseSet& = delete;

    /**
     * @brief Removes component from entity.
     * @param entity Target entity
     */
    virtual void remove(Entity entity) = 0;

    /**
     * @brief Checks if entity has this component.
     * @param entity Target entity
     * @return true if component exists
     */
    [[nodiscard]] virtual auto contains(Entity entity) const noexcept -> bool = 0;

    /**
     * @brief Removes all components from storage.
     */
    virtual void clear() noexcept = 0;

    /**
     * @brief Returns the number of components in storage.
     * @return Count of stored components
     */
    [[nodiscard]] virtual auto size() const noexcept -> size_t = 0;

    /**
     * @brief Releases unused memory.
     */
    virtual void shrinkToFit() = 0;

    /**
     * @brief Returns the packed entity array for iteration.
     * @return Reference to the packed entity vector
     */
    [[nodiscard]] virtual auto getPacked() const noexcept -> const std::vector<Entity>& = 0;
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_STORAGE_ISPARSESET_HPP_
