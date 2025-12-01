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

void Registry::reserveEntities(size_t capacity) {
    std::unique_lock lock(_entityMutex);
    _generations.reserve(capacity);
    _freeIndices.reserve(capacity / 4);
    _entityComponents.reserve(capacity);
}

auto Registry::spawnEntity() -> Entity {
    std::unique_lock lock(_entityMutex);
    std::uint32_t idx = 0;
    constexpr int max_recycle_attempts = 5;
    int attempts = 0;

    while (!_freeIndices.empty() && attempts < max_recycle_attempts) {
        idx = _freeIndices.back();
        _freeIndices.pop_back();

        if (idx < _generations.size() &&
            _generations[idx] < Entity::_MaxGeneration) {
            return {idx, _generations[idx]};
        }

        _tombstones.push_back(idx);
        attempts++;
    }

    idx = static_cast<std::uint32_t>(_generations.size());
    _generations.push_back(0);
    _entityComponents.emplace(idx, std::vector<std::type_index>());

    return {idx, 0};
}

void Registry::killEntity(Entity entity) noexcept {
    std::vector<std::type_index> components_to_remove;

    {
        std::unique_lock lock(_entityMutex);

        if (entity.index() >= _generations.size() ||
            _generations[entity.index()] != entity.generation()) {
            return;
        }

        auto iter = _entityComponents.find(entity.index());
        if (iter != _entityComponents.end()) {
            components_to_remove = iter->second;
        }

        if (_generations[entity.index()] >= Entity::_MaxGeneration - 1) {
            _generations[entity.index()] = Entity::_MaxGeneration;
            _tombstones.push_back(entity.index());
        } else {
            _generations[entity.index()]++;
            _freeIndices.push_back(entity.index());
        }

        _entityComponents.erase(entity.index());
    }

    for (const auto& type : components_to_remove) {
        try {
            _signalDispatcher.dispatchDestroy(type, entity);

            std::shared_lock pool_lock(_componentPoolMutex);
            auto iter = _componentPools.find(type);
            if (iter != _componentPools.end()) {
                iter->second->remove(entity);
            }
        } catch (...) {
        }
    }

    _relationshipManager.removeEntity(entity);
}

auto Registry::isAlive(Entity entity) const noexcept -> bool {
    std::shared_lock lock(_entityMutex);

    return entity.index() < _generations.size() &&
           _generations[entity.index()] == entity.generation();
}

// ========================================================================
// ENTITY MAINTENANCE
// ========================================================================

auto Registry::cleanupTombstones() -> size_t {
    std::unique_lock lock(_entityMutex);

    if (_tombstones.empty()) {
        return 0;
    }

    size_t cleaned = 0;

    for (auto tombstone_idx : _tombstones) {
        if (tombstone_idx < _generations.size()) {
            _generations[tombstone_idx] = 0;
            _freeIndices.push_back(tombstone_idx);
            cleaned++;
        }
    }

    _tombstones.clear();
    return cleaned;
}

// ========================================================================
// DEBUGGING/INTROSPECTION
// ========================================================================

auto Registry::getEntityComponents(Entity entity) const
    -> const std::vector<std::type_index>& {
    static const std::vector<std::type_index> empty_vector;
    std::shared_lock lock(_entityMutex);

    auto iter = _entityComponents.find(entity.index());
    return (iter != _entityComponents.end()) ? iter->second : empty_vector;
}

}  // namespace ECS
