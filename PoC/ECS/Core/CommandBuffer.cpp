/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CommandBuffer
*/

#include "CommandBuffer.hpp"
#include "Registry.hpp"

namespace ECS {

    Entity CommandBuffer::spawnEntityDeferred() {
        std::lock_guard lock(_commandsMutex);

        std::uint32_t placeholder_id = _nextPlaceholderId++;
        Entity placeholder(placeholder_id, 0);

        _commands.push_back([this, placeholder_id]() {
            Entity real_entity = _registry.get().spawnEntity();
            _placeholdertoReal[placeholder_id] = real_entity;
        });

        return placeholder;
    }

    void CommandBuffer::destroyEntityDeferred(Entity entity) {
        std::lock_guard lock(_commandsMutex);
        _commands.push_back([this, entity]() {
            Entity target_entity = entity;
            auto it = _placeholdertoReal.find(entity.id);
            if (it != _placeholdertoReal.end()) {
                target_entity = it->second;
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

    size_t CommandBuffer::pendingCount() const {
        std::lock_guard lock(_commandsMutex);
        return _commands.size();
    }

    void CommandBuffer::clear() {
        std::lock_guard lock(_commandsMutex);
        _commands.clear();
        _placeholdertoReal.clear();
        _nextPlaceholderId = 0;
    }

} // namespace ECS
