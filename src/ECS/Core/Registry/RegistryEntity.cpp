/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Entity Management Implementation
*/

#include "Registry.hpp"

namespace ECS {

    // ========================================================================
    // ENTITY LIFECYCLE
    // ========================================================================

    void Registry::reserve_entities(size_t capacity) {
        std::unique_lock lock(entity_mutex);
        generations.reserve(capacity);
        free_indices.reserve(capacity / 4);
        entity_components.reserve(capacity);
    }

    Entity Registry::spawn_entity() {
        std::unique_lock lock(entity_mutex);
        std::uint32_t idx;
        constexpr int max_recycle_attempts = 5;
        int attempts = 0;

        while (!free_indices.empty() && attempts < max_recycle_attempts) {
            idx = free_indices.back();
            free_indices.pop_back();

            if (idx < generations.size() && generations[idx] < Entity::MaxGeneration) {
                return Entity(idx, generations[idx]);
            }

            tombstones.push_back(idx);
            attempts++;
        }

        idx = static_cast<std::uint32_t>(generations.size());
        generations.push_back(0);
        entity_components.emplace(idx, std::vector<std::type_index>());

        return Entity(idx, 0);
    }

    void Registry::kill_entity(Entity entity) noexcept {
        std::vector<std::type_index> components_to_remove;

        {
            std::unique_lock lock(entity_mutex);

            if (entity.index() >= generations.size() ||
                generations[entity.index()] != entity.generation()) {
                return;
            }

            auto it = entity_components.find(entity.index());
            if (it != entity_components.end()) {
                components_to_remove = it->second;
            }

            if (generations[entity.index()] >= Entity::MaxGeneration - 1) {
                generations[entity.index()] = Entity::MaxGeneration;
                tombstones.push_back(entity.index());
            } else {
                generations[entity.index()]++;
                free_indices.push_back(entity.index());
            }

            entity_components.erase(entity.index());
        }

        for (const auto& type : components_to_remove) {
            try {
                signal_dispatcher.dispatch_destroy(type, entity);

                std::shared_lock pool_lock(component_pool_mutex);
                auto it = component_pools.find(type);
                if (it != component_pools.end()) {
                    it->second->remove(entity);
                }
            } catch (...) {
                // Swallow exceptions to maintain noexcept guarantee
                // In production, you might want to log this error
            }
        }

        relationship_manager.remove_entity(entity);
    }

    bool Registry::is_alive(Entity entity) const noexcept {
        std::shared_lock lock(entity_mutex);

        // Entity is alive if:
        // 1. Its index is within bounds
        // 2. Its generation matches the current generation
        return entity.index() < generations.size() &&
               generations[entity.index()] == entity.generation();
    }

    // ========================================================================
    // ENTITY MAINTENANCE
    // ========================================================================

    size_t Registry::cleanup_tombstones() {
        std::unique_lock lock(entity_mutex);

        if (tombstones.empty()) {
            return 0;
        }

        size_t cleaned = 0;

        for (auto tombstone_idx : tombstones) {
            if (tombstone_idx < generations.size()) {
                generations[tombstone_idx] = 0;
                free_indices.push_back(tombstone_idx);
                cleaned++;
            }
        }

        tombstones.clear();
        return cleaned;
    }

    // ========================================================================
    // DEBUGGING/INTROSPECTION
    // ========================================================================

    const std::vector<std::type_index>& Registry::get_entity_components(Entity entity) const {
        static const std::vector<std::type_index> empty_vector;
        std::shared_lock lock(entity_mutex);

        auto it = entity_components.find(entity.index());
        return (it != entity_components.end()) ? it->second : empty_vector;
    }

} // namespace ECS
