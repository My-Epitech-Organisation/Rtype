/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TagSparseSet
*/

#ifndef ECS_STORAGE_TAG_SPARSE_SET_HPP
    #define ECS_STORAGE_TAG_SPARSE_SET_HPP
    #include "ISparseSet.hpp"
    #include <vector>
    #include <limits>
    #include <stdexcept>
    #include <mutex>
    #include <utility>

namespace ECS {

    /**
     * @brief Memory-efficient storage for empty components (tags).
     *
     * Tags are marker components without data (e.g., "Player", "Enemy").
     * This specialized container stores only entity IDs, eliminating wasted memory.
     *
     * @tparam T Must be an empty type (sizeof(T) == 1, no members)
     */
    template <typename T>
    class TagSparseSet : public ISparseSet {
        static_assert(std::is_empty_v<T>, "TagSparseSet requires empty type");

    public:
        bool contains(Entity entity) const noexcept override {
            std::lock_guard lock(_sparseSetMutex);

            auto idx = entity.index();
            return idx < _sparse.size() &&
                   _sparse[idx] != NullIndex &&
                   _sparse[idx] < _packed.size() &&
                   _packed[_sparse[idx]] == entity;
        }

        /**
         * @brief Adds tag to entity (idempotent).
         * @return Dummy reference (tags have no data)
         */
        template <typename... Args>
        T& emplace(Entity entity, Args&&...) {
            std::lock_guard lock(_sparseSetMutex);
            if (containsUnsafe(entity))
                return _dummyInstance;

            auto idx = entity.index();
            if (idx >= _sparse.size())
                _sparse.resize(idx + 1, NullIndex);

            _sparse[idx] = _packed.size();
            _packed.push_back(entity);
            return _dummyInstance;
        }

        void remove(Entity entity) override {
            std::lock_guard lock(_sparseSetMutex);
            if (!containsUnsafe(entity)) return;

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

        T& get(Entity entity) {
            std::lock_guard lock(_sparseSetMutex);

            if (!containsUnsafe(entity))
                throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
            return _dummyInstance;
        }

        const T& get(Entity entity) const {
            std::lock_guard lock(_sparseSetMutex);

            if (!containsUnsafe(entity))
                throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
            return _dummyInstance;
        }

        void clear() noexcept override {
            std::lock_guard lock(_sparseSetMutex);

            _packed.clear();
            _sparse.clear();
        }

        size_t size() const noexcept override {
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
        const std::vector<Entity>& getPacked() const noexcept override {
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
        static inline T _dummyInstance{};

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

#endif // ECS_STORAGE_TAG_SPARSE_SET_HPP
