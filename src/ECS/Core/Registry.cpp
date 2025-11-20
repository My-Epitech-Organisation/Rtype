/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry
*/

#include "Registry.hpp"

namespace ECS {

    void Registry::reserve_entities(size_t capacity) {
        std::unique_lock lock(entity_mutex);
        generations.reserve(capacity);
        free_indices.reserve(capacity / 4);
        entity_components.reserve(capacity);
    }

    Entity Registry::spawn_entity() {
        std::unique_lock lock(entity_mutex);
        std::uint32_t idx;
        
        // Fast path: Try to reuse a free index quickly (max 5 attempts to avoid long locks)
        constexpr int max_attempts = 5;
        int attempts = 0;
        while (!free_indices.empty() && attempts < max_attempts) {
            idx = free_indices.back();
            free_indices.pop_back();
            
            // Check if this slot is valid (not a tombstone)
            if (idx < generations.size() && generations[idx] < Entity::MaxGeneration) {
                return Entity(idx, generations[idx]);
            }
            // Tombstone found, mark for later cleanup
            tombstones.push_back(idx);
            attempts++;
        }
        
        // Allocate new slot (fast path, no cleanup here)
        idx = static_cast<std::uint32_t>(generations.size());
        generations.push_back(0);
        entity_components.emplace(idx, std::vector<std::type_index>());
        return Entity(idx, 0);
    }

    void Registry::kill_entity(Entity entity) noexcept {
        std::vector<std::type_index> components_to_remove;
        
        // Critical section: check if alive and get components atomically
        {
            std::unique_lock lock(entity_mutex);
            
            // Validate entity is alive
            if (entity.index() >= generations.size() ||
                generations[entity.index()] != entity.generation()) {
                return; // Already dead or invalid
            }
            
            // Get components to remove
            auto it = entity_components.find(entity.index());
            if (it != entity_components.end()) {
                components_to_remove = it->second;
            }
            
            // Increment generation or mark as tombstone
            if (generations[entity.index()] >= Entity::MaxGeneration - 1) {
                generations[entity.index()] = Entity::MaxGeneration;
                tombstones.push_back(entity.index());
            } else {
                generations[entity.index()]++;
                free_indices.push_back(entity.index());
            }
            
            // Remove from entity_components
            entity_components.erase(entity.index());
        }

        // Remove components outside the entity lock
        for (const auto& type : components_to_remove) {
            try {
                signal_dispatcher.dispatch_destroy(type, entity);
                std::shared_lock pool_lock(component_pool_mutex);
                auto it = component_pools.find(type);
                if (it != component_pools.end()) {
                    it->second->remove(entity);
                }
            } catch (...) {
                // Swallow exceptions to ensure noexcept guarantee
            }
        }

        // Clean up relationships
        relationship_manager.remove_entity(entity);
    }

    bool Registry::is_alive(Entity entity) const noexcept {
        std::shared_lock lock(entity_mutex);
        return entity.index() < generations.size() &&
               generations[entity.index()] == entity.generation();
    }

    size_t Registry::cleanup_tombstones() {
        std::unique_lock lock(entity_mutex);
        
        if (tombstones.empty()) {
            return 0;
        }
        
        size_t cleaned = 0;
        for (auto tombstone_idx : tombstones) {
            if (tombstone_idx < generations.size()) {
                // Reset generation to 0 to make it reusable
                generations[tombstone_idx] = 0;
                free_indices.push_back(tombstone_idx);
                cleaned++;
            }
        }
        tombstones.clear();
        
        return cleaned;
    }

} // namespace ECS
