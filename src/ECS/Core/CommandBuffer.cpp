/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CommandBuffer
*/

#include "CommandBuffer.hpp"
#include "Registry.hpp"

namespace ECS {

    Entity CommandBuffer::spawn_entity_deferred() {
        std::lock_guard lock(commands_mutex);

        std::uint32_t placeholder_id = next_placeholder_id++;
        Entity placeholder(placeholder_id, 0);

        commands.push_back([this, placeholder_id]() {
            Entity real_entity = registry.spawn_entity();
            placeholder_to_real[placeholder_id] = real_entity;
        });

        return placeholder;
    }

    void CommandBuffer::destroy_entity_deferred(Entity entity) {
        std::lock_guard lock(commands_mutex);
        commands.push_back([this, entity]() {
            Entity target_entity = entity;
            auto it = placeholder_to_real.find(entity.id);
            if (it != placeholder_to_real.end()) {
                target_entity = it->second;
            }

            registry.kill_entity(target_entity);
        });
    }

    void CommandBuffer::flush() {
        std::lock_guard lock(commands_mutex);

        placeholder_to_real.clear();

        for (auto& command : commands) {
            command();
        }

        commands.clear();
        next_placeholder_id = 0;
    }

    size_t CommandBuffer::pending_count() const {
        std::lock_guard lock(commands_mutex);
        return commands.size();
    }

    void CommandBuffer::clear() {
        std::lock_guard lock(commands_mutex);
        commands.clear();
        placeholder_to_real.clear();
        next_placeholder_id = 0;
    }

} // namespace ECS
