/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TagSparseSet
*/

#ifndef SRC_ENGINE_ECS_STORAGE_TAGSPARSESET_HPP_
#define SRC_ENGINE_ECS_STORAGE_TAGSPARSESET_HPP_

#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ISparseSet.hpp"
#include "../traits/ComponentTraits.hpp"

namespace ECS {

/**
 * @brief Memory-efficient storage for empty components (tags).
 *
 * Tags are marker components without data (e.g., "Player", "Enemy").
 * This specialized container stores only entity IDs, eliminating wasted memory.
 *
 * Thread Safety:
 * - Mutating operations (emplace, remove, clear, reserve, shrinkToFit) are thread-safe.
 * - Read operations (contains, get, size) are thread-safe.
 * - Direct access (getPacked) is NOT thread-safe and requires external synchronization.
 * - Typically safe when ECS systems run sequentially in the game loop.
 *
 * Implementation Note:
 * - _dummyInstance is shared across all TagSparseSet<T> instances (static inline const).
 *   This is safe because empty types have no mutable state.
 *
 * @tparam T Must be an empty type (sizeof(T) == 1, no members) and satisfy Component concept
 */
template <Component T>
class TagSparseSet : public ISparseSet {
    static_assert(std::is_empty_v<T>, "TagSparseSet requires empty type");

 public:
    auto contains(Entity entity) const noexcept -> bool override {
        std::lock_guard lock(_sparseSetMutex);

        auto idx = entity.index();
        return idx < _sparse.size() &&
               _sparse[idx] != NullIndex &&
               _sparse[idx] < _packed.size() &&
               _packed[_sparse[idx]] == entity;
    }

    /**
     * @brief Adds tag to entity (idempotent).
     * @return Const reference to dummy instance (tags have no data)
     * @note Args are intentionally ignored - tags have no data to construct
     */
    template <typename... Args>
    auto emplace(Entity entity, [[maybe_unused]] Args... /*unused*/) -> const T& {
        std::lock_guard lock(_sparseSetMutex);
        if (containsUnsafe(entity)) {
            return _dummyInstance;
        }

        auto idx = entity.index();
        if (idx >= _sparse.size()) {
            _sparse.resize(idx + 1, NullIndex);
        }

        _sparse[idx] = _packed.size();
        _packed.push_back(entity);
        return _dummyInstance;
    }

    void remove(Entity entity) override {
        std::lock_guard lock(_sparseSetMutex);
        if (!containsUnsafe(entity)) {
            return;
        }

        auto idx = entity.index();
        size_t dense_idx = _sparse[idx];
        size_t last_idx = _packed.size() - 1;

        if (dense_idx != last_idx) {
            Entity last_entity = _packed[last_idx];
            std::swap(_packed[dense_idx], _packed[last_idx]);
            _sparse[last_entity.index()] = dense_idx;
        }

        _packed.pop_back();
        _sparse[idx] = NullIndex;
    }

    auto get(Entity entity) -> const T& {
        std::lock_guard lock(_sparseSetMutex);

        if (!containsUnsafe(entity)) {
            throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
        }
        return _dummyInstance;
    }

    auto get(Entity entity) const -> const T& {
        std::lock_guard lock(_sparseSetMutex);

        if (!containsUnsafe(entity)) {
            throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
        }
        return _dummyInstance;
    }

    void clear() noexcept override {
        std::lock_guard lock(_sparseSetMutex);

        _packed.clear();
        _sparse.clear();
    }

    auto size() const noexcept -> size_t override {
        std::lock_guard lock(_sparseSetMutex);

        return _packed.size();
    }

    /**
     * @brief Direct access to internal entity array.
     * @warning NOT THREAD-SAFE: Returns reference to internal data.
     *          The lock is released after returning, so the reference
     *          can be invalidated by concurrent modifications.
     *          External synchronization is required during access.
     *          Typically safe when systems run sequentially in the game loop.
     */
    auto getPacked() const noexcept -> const std::vector<Entity>& override {
        return _packed;
    }

    /**
     * @brief Pre-allocates memory for expected number of entities.
     * @param capacity Number of entities to reserve space for
     * @note Thread-safe. Not part of ISparseSet interface (type-specific optimization).
     */
    void reserve(size_t capacity) {
        std::lock_guard lock(_sparseSetMutex);

        _packed.reserve(capacity);
        _sparse.reserve(capacity);
    }

    /**
     * @brief Releases unused memory.
     */
    void shrinkToFit() override {
        std::lock_guard lock(_sparseSetMutex);

        _packed.shrink_to_fit();
        _sparse.shrink_to_fit();
    }

 private:
    static constexpr size_t NullIndex = std::numeric_limits<size_t>::max();
    std::vector<Entity> _packed;
    std::vector<size_t> _sparse;
    mutable std::mutex _sparseSetMutex;
    static inline const T _dummyInstance{};

    /**
     * @brief Internal contains check without locking (caller must hold lock).
     */
    auto containsUnsafe(Entity entity) const noexcept -> bool {
        auto idx = entity.index();
        return idx < _sparse.size() &&
               _sparse[idx] != NullIndex &&
               _sparse[idx] < _packed.size() &&
               _packed[_sparse[idx]] == entity;
    }
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_STORAGE_TAGSPARSESET_HPP_
