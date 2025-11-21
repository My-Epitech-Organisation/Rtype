/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SparseSet
*/

#ifndef ECS_STORAGE_SPARSE_SET_HPP
    #define ECS_STORAGE_SPARSE_SET_HPP
    #include "ISparseSet.hpp"
    #include <vector>
    #include <algorithm>
    #include <limits>
    #include <stdexcept>
    #include <mutex>

namespace ECS {

    /**
     * @brief Cache-efficient component storage using sparse set data structure.
     *
     * Architecture:
     * - Dense: Contiguous component array (cache-friendly iteration)
     * - Packed: Parallel entity ID array (matches dense indices)
     * - Sparse: Entity index → dense index lookup table
     *
     * Complexity:
     * - Insert: O(1) amortized
     * - Remove: O(1) via swap-and-pop
     * - Lookup: O(1) direct access
     * - Iterate: O(n) linear scan (optimal cache utilization)
     *
     * @tparam T Component type (non-empty)
     */
    template <typename T>
    class SparseSet : public ISparseSet {
    public:
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;

        SparseSet() = default;

        bool contains(Entity entity) const noexcept override {
            auto idx = entity.index();
            return idx < sparse.size() &&
                   sparse[idx] != NullIndex &&
                   sparse[idx] < packed.size() &&
                   packed[sparse[idx]] == entity;
        }

        /**
         * @brief Constructs component in-place for entity.
         * @param entity Target entity
         * @param args Constructor arguments
         * @return Reference to emplaced component
         */
        template <typename... Args>
        reference emplace(Entity entity, Args&&... args) {
            std::lock_guard lock(sparse_set_mutex);

            if (contains(entity))
                return dense[sparse[entity.index()]] = T(std::forward<Args>(args)...);

            auto idx = entity.index();
            if (idx >= sparse.size())
                sparse.resize(idx + 1, NullIndex);

            sparse[idx] = dense.size();
            packed.push_back(entity);
            dense.emplace_back(std::forward<Args>(args)...);

            return dense.back();
        }

        void remove(Entity entity) override {
            std::lock_guard lock(sparse_set_mutex);

            if (!contains(entity)) return;

            auto idx = entity.index();
            size_t dense_idx = sparse[idx];
            size_t last_idx = dense.size() - 1;

            if (dense_idx != last_idx) {
                Entity last_entity = packed[last_idx];
                std::swap(dense[dense_idx], dense[last_idx]);
                std::swap(packed[dense_idx], packed[last_idx]);
                sparse[last_entity.index()] = dense_idx;
            }

            dense.pop_back();
            packed.pop_back();
            sparse[idx] = NullIndex;
        }

        reference get(Entity entity) {
            if (!contains(entity))
                throw std::runtime_error("Entity missing component in SparseSet::get()");
            return dense[sparse[entity.index()]];
        }

        const_reference get(Entity entity) const {
            if (!contains(entity))
                throw std::runtime_error("Entity missing component in SparseSet::get()");
            return dense[sparse[entity.index()]];
        }

        void clear() noexcept override {
            dense.clear();
            packed.clear();
            sparse.clear();
        }

        size_t size() const noexcept override {
            return dense.size();
        }

        iterator begin() noexcept { return dense.begin(); }
        iterator end() noexcept { return dense.end(); }
        const_iterator begin() const noexcept { return dense.begin(); }
        const_iterator end() const noexcept { return dense.end(); }

        const std::vector<Entity>& get_packed() const noexcept { return packed; }
        const std::vector<T>& get_dense() const noexcept { return dense; }

        void reserve(size_t capacity) {
            dense.reserve(capacity);
            packed.reserve(capacity);
            sparse.reserve(capacity);
        }

        /**
         * @brief Releases unused memory.
         * Useful after removing many components to reclaim memory.
         */
        void shrink_to_fit() {
            dense.shrink_to_fit();
            packed.shrink_to_fit();
            sparse.shrink_to_fit();
        }

    private:
        static constexpr size_t NullIndex = std::numeric_limits<size_t>::max();
        std::vector<T> dense;
        std::vector<Entity> packed;
        std::vector<size_t> sparse;
        mutable std::mutex sparse_set_mutex;
    };

} // namespace ECS

#endif // ECS_STORAGE_SPARSE_SET_HPP
