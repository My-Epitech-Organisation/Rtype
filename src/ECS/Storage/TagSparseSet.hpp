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
            auto idx = entity.index();
            return idx < sparse.size() &&
                   sparse[idx] != NullIndex &&
                   sparse[idx] < packed.size() &&
                   packed[sparse[idx]] == entity;
        }

        /**
         * @brief Adds tag to entity (idempotent).
         * @return Dummy reference (tags have no data)
         */
        template <typename... Args>
        T& emplace(Entity entity, Args&&...) {
            std::lock_guard lock(sparse_set_mutex);
            if (contains(entity))
                return dummy_instance;

            auto idx = entity.index();
            if (idx >= sparse.size())
                sparse.resize(idx + 1, NullIndex);

            sparse[idx] = packed.size();
            packed.push_back(entity);
            return dummy_instance;
        }

        void remove(Entity entity) override {
            std::lock_guard lock(sparse_set_mutex);
            if (!contains(entity)) return;

            auto idx = entity.index();
            size_t dense_idx = sparse[idx];
            size_t last_idx = packed.size() - 1;

            if (dense_idx != last_idx) {
                Entity last_entity = packed[last_idx];
                std::swap(packed[dense_idx], packed[last_idx]);
                sparse[last_entity.index()] = dense_idx;
            }

            packed.pop_back();
            sparse[idx] = NullIndex;
        }

        T& get(Entity entity) {
            if (!contains(entity))
                throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
            return dummy_instance;
        }

        const T& get(Entity entity) const {
            if (!contains(entity))
                throw std::runtime_error("Entity missing tag component in TagSparseSet::get()");
            return dummy_instance;
        }

        void clear() noexcept override {
            packed.clear();
            sparse.clear();
        }

        size_t size() const noexcept override {
            return packed.size();
        }

        const std::vector<Entity>& get_packed() const noexcept { return packed; }

        void reserve(size_t capacity) {
            packed.reserve(capacity);
            sparse.reserve(capacity);
        }

        /**
         * @brief Releases unused memory.
         */
        void shrink_to_fit() {
            packed.shrink_to_fit();
            sparse.shrink_to_fit();
        }

    private:
        static constexpr size_t NullIndex = std::numeric_limits<size_t>::max();
        std::vector<Entity> packed;
        std::vector<size_t> sparse;
        mutable std::mutex sparse_set_mutex;
        static inline T dummy_instance{};
    };

} // namespace ECS

#endif // ECS_STORAGE_TAG_SPARSE_SET_HPP
