/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SparseSet
*/

#ifndef ECS_STORAGE_SPARSE_SET_HPP
    #define ECS_STORAGE_SPARSE_SET_HPP
    #include "ISparseSet.hpp"
    #include "../traits/ComponentTraits.hpp"
    #include <vector>
    #include <algorithm>
    #include <limits>
    #include <memory>
    #include <stdexcept>
    #include <mutex>
    #include <utility>

namespace ECS {

    /**
     * @brief Cache-efficient component storage using sparse set data structure.
     *
     * Architecture:
     * - Dense: Contiguous component array (cache-friendly iteration)
     * - _packed: Parallel entity ID array (matches dense indices)
     * - _sparse: Entity index → dense index lookup table
     *
     * Complexity:
     * - Insert: O(1) amortized
     * - Remove: O(1) via swap-and-pop
     * - Lookup: O(1) direct access
     * - Iterate: O(n) linear scan (optimal cache utilization)
     *
     * Thread Safety:
     * - Mutating operations (emplace, remove, clear, reserve, shrinkToFit) are thread-safe.
     * - Read operations (contains, get, size) are thread-safe.
     * - Iteration (begin/end) and direct access (getPacked/getDense) are NOT thread-safe.
     *   These require external synchronization if concurrent modifications may occur.
     * - Typically safe when ECS systems run sequentially in the game loop.
     *
     * @tparam T Component type (must satisfy Component concept: move-constructible)
     */
    template <Component T>
    class SparseSet : public ISparseSet {
    public:

        SparseSet() = default;

        bool contains(Entity entity) const noexcept override {
            std::lock_guard lock(_sparseSetMutex);

            auto idx = entity.index();
            return idx < _sparse.size() &&
                   _sparse[idx] != NullIndex &&
                   _sparse[idx] < _packed.size() &&
                   _packed[_sparse[idx]] == entity;
        }

        /**
         * @brief Constructs component in-place for entity.
         * @param entity Target entity
         * @param args Constructor arguments
         * @return Reference to emplaced component
         */
        template <typename... Args>
        T& emplace(Entity entity, Args&&... args) {
            std::lock_guard lock(_sparseSetMutex);

            if (containsUnsafe(entity)) {
                T& ref = _dense[_sparse[entity.index()]];
                std::destroy_at(std::addressof(ref));
                std::construct_at(std::addressof(ref), std::forward<Args>(args)...);
                return ref;
            }

            auto idx = entity.index();
            if (idx >= _sparse.size())
                _sparse.resize(idx + 1, NullIndex);

            _sparse[idx] = _dense.size();
            _packed.push_back(entity);
            _dense.emplace_back(std::forward<Args>(args)...);

            return _dense.back();
        }

        void remove(Entity entity) override {
            std::lock_guard lock(_sparseSetMutex);

            if (!containsUnsafe(entity)) return;

            auto idx = entity.index();
            size_t dense_idx = _sparse[idx];
            size_t last_idx = _dense.size() - 1;

            if (dense_idx != last_idx) {
                Entity last_entity = _packed[last_idx];
                std::swap(_dense[dense_idx], _dense[last_idx]);
                std::swap(_packed[dense_idx], _packed[last_idx]);
                _sparse[last_entity.index()] = dense_idx;
            }

            _dense.pop_back();
            _packed.pop_back();
            _sparse[idx] = NullIndex;
        }

        T& get(Entity entity) {
            std::lock_guard lock(_sparseSetMutex);

            if (!containsUnsafe(entity))
                throw std::runtime_error("Entity missing component in SparseSet::get()");
            return _dense[_sparse[entity.index()]];
        }

        const T& get(Entity entity) const {
            std::lock_guard lock(_sparseSetMutex);

            if (!containsUnsafe(entity))
                throw std::runtime_error("Entity missing component in SparseSet::get()");
            return _dense[_sparse[entity.index()]];
        }

        void clear() noexcept override {
            std::lock_guard lock(_sparseSetMutex);

            _dense.clear();
            _packed.clear();
            _sparse.clear();
        }

        size_t size() const noexcept override {
            std::lock_guard lock(_sparseSetMutex);

            return _dense.size();
        }

        /**
         * @brief Iterator access for range-based loops and STL algorithms.
         * @warning NOT THREAD-SAFE: These methods do not acquire locks.
         *          External synchronization is required if iterating while
         *          another thread may modify the container (emplace/remove/clear).
         *          Typically safe when systems run sequentially in the game loop.
         */
        std::vector<T>::iterator begin() noexcept { return _dense.begin(); }
        std::vector<T>::iterator end() noexcept { return _dense.end(); }
        std::vector<T>::const_iterator begin() const noexcept { return _dense.begin(); }
        std::vector<T>::const_iterator end() const noexcept { return _dense.end(); }

        /**
         * @brief Direct access to internal arrays.
         * @warning NOT THREAD-SAFE: Returns reference to internal data.
         *          The lock is released after returning, so the reference
         *          can be invalidated by concurrent modifications.
         *          External synchronization is required during access.
         *          Typically safe when systems run sequentially in the game loop.
         */
        const std::vector<Entity>& getPacked() const noexcept override {
            return _packed;
        }

        const std::vector<T>& getDense() const noexcept {
            return _dense;
        }

        /**
         * @brief Pre-allocates memory for expected number of entities.
         * @param capacity Number of entities to reserve space for
         * @note Thread-safe. Not part of ISparseSet interface (type-specific optimization).
         */
        void reserve(size_t capacity) {
            std::lock_guard lock(_sparseSetMutex);

            _dense.reserve(capacity);
            _packed.reserve(capacity);
            _sparse.reserve(capacity);
        }

        /**
         * @brief Releases unused memory.
         * Useful after removing many components to reclaim memory.
         */
        void shrinkToFit() override {
            std::lock_guard lock(_sparseSetMutex);

            _dense.shrink_to_fit();
            _packed.shrink_to_fit();
            _sparse.shrink_to_fit();
        }

    private:
        static constexpr size_t NullIndex = std::numeric_limits<size_t>::max();
        std::vector<T> _dense;
        std::vector<Entity> _packed;
        std::vector<size_t> _sparse;
        mutable std::mutex _sparseSetMutex;

        /**
         * @brief Internal contains check without locking (caller must hold lock).
         */
        bool containsUnsafe(Entity entity) const noexcept {
            auto idx = entity.index();
            return idx < _sparse.size() &&
                   _sparse[idx] != NullIndex &&
                   _sparse[idx] < _packed.size() &&
                   _packed[_sparse[idx]] == entity;
        }
    };

} // namespace ECS

#endif // ECS_STORAGE_SPARSE_SET_HPP
