/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CommandBuffer
*/

#include "CommandBuffer.hpp"

#include "Registry/Registry.hpp"

namespace ECS {

auto CommandBuffer::spawnEntityDeferred() -> Entity {
    std::lock_guard lock(_commandsMutex);

    std::uint32_t placeholder_id = _nextPlaceholderId++;
    Entity placeholder(placeholder_id, 0);

    _commands.emplace_back([this, placeholder_id]() {
        Entity real_entity = _registry.get().spawnEntity();
        _placeholdertoReal[placeholder_id] = real_entity;
    });

    return placeholder;
}

void CommandBuffer::destroyEntityDeferred(Entity entity) {
    std::lock_guard lock(_commandsMutex);
    _commands.emplace_back([this, entity]() {
        Entity target_entity = entity;
        auto iter = _placeholdertoReal.find(entity.id);
        if (iter != _placeholdertoReal.end()) {
            target_entity = iter->second;
        }

        _registry.get().killEntity(target_entity);
    });
}

void CommandBuffer::flush() {
    std::lock_guard lock(_commandsMutex);

    _placeholdertoReal.clear();

    for (auto& command : _commands) {
        command();
    }

    _commands.clear();
    _nextPlaceholderId = 0;
}

auto CommandBuffer::pendingCount() const -> size_t {
    std::lock_guard lock(_commandsMutex);
    return _commands.size();
}

void CommandBuffer::clear() {
    std::lock_guard lock(_commandsMutex);
    _commands.clear();
    _placeholdertoReal.clear();
    _nextPlaceholderId = 0;
}

}  // namespace ECS
