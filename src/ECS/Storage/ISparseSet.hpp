/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISparseSet
*/

#ifndef ECS_STORAGE_ISPARSE_SET_HPP
    #define ECS_STORAGE_ISPARSE_SET_HPP
    #include "../Core/Entity.hpp"

namespace ECS {

    /**
     * @brief Type-erased interface for component storage containers.
     *
     * Enables heterogeneous storage of different component types in a single collection.
     * All concrete SparseSet implementations must provide these operations.
     */
    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;

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
        virtual bool contains(Entity entity) const noexcept = 0;

        /**
         * @brief Removes all components from storage.
         */
        virtual void clear() noexcept = 0;

        /**
         * @brief Returns the number of components in storage.
         * @return Count of stored components
         */
        virtual size_t size() const noexcept = 0;

        /**
         * @brief Releases unused memory.
         */
        virtual void shrink_to_fit() = 0;
    };

} // namespace ECS

#endif // ECS_STORAGE_ISPARSE_SET_HPP
