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
        free_indices.reserve(capacity / 4); // Estimate 25% recycling
        entity_components.reserve(capacity);
    }

    Entity Registry::spawn_entity() {
        std::unique_lock lock(entity_mutex);
        std::uint32_t idx;

        // Try to recycle a free entity slot
        // We limit attempts to avoid infinite loops if all recycled slots are corrupted
        constexpr int max_recycle_attempts = 5;
        int attempts = 0;

        while (!free_indices.empty() && attempts < max_recycle_attempts) {
            idx = free_indices.back();
            free_indices.pop_back();

            // Validate the recycled slot
            if (idx < generations.size() && generations[idx] < Entity::MaxGeneration) {
                // Valid slot found, reuse it with incremented generation
                return Entity(idx, generations[idx]);
            }

            // Slot is corrupted or reached max generation, mark as permanent tombstone
            tombstones.push_back(idx);
            attempts++;
        }

        // No valid recycled slot found, allocate a new one
        idx = static_cast<std::uint32_t>(generations.size());
        generations.push_back(0); // Start at generation 0
        entity_components.emplace(idx, std::vector<std::type_index>());

        return Entity(idx, 0);
    }

    void Registry::kill_entity(Entity entity) noexcept {
        std::vector<std::type_index> components_to_remove;

        // Phase 1: Mark entity as dead and collect components
        {
            std::unique_lock lock(entity_mutex);

            // Validate entity before killing
            if (entity.index() >= generations.size() ||
                generations[entity.index()] != entity.generation()) {
                return; // Entity already dead or invalid
            }

            // Collect components to remove (done under lock)
            auto it = entity_components.find(entity.index());
            if (it != entity_components.end()) {
                components_to_remove = it->second;
            }

            // Increment generation to invalidate old entity handles
            if (generations[entity.index()] >= Entity::MaxGeneration - 1) {
                // Generation overflow: mark as permanent tombstone
                generations[entity.index()] = Entity::MaxGeneration;
                tombstones.push_back(entity.index());
            } else {
                // Normal case: increment generation and add to free list
                generations[entity.index()]++;
                free_indices.push_back(entity.index());
            }

            // Remove entity's component tracking
            entity_components.erase(entity.index());
        }

        // Phase 2: Remove components from pools (outside entity lock to avoid deadlock)
        for (const auto& type : components_to_remove) {
            try {
                // Dispatch destroy signal
                signal_dispatcher.dispatch_destroy(type, entity);

                // Remove from component pool
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

        // Phase 3: Clean up relationships
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

        // Reset tombstone generations and make them available for recycling
        for (auto tombstone_idx : tombstones) {
            if (tombstone_idx < generations.size()) {
                generations[tombstone_idx] = 0; // Reset to generation 0
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
