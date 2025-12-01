/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CommandBuffer
*/

#ifndef SRC_ENGINE_ECS_CORE_COMMANDBUFFER_HPP_
#define SRC_ENGINE_ECS_CORE_COMMANDBUFFER_HPP_

#include <any>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Entity.hpp"
#include "Registry/Registry.hpp"

namespace ECS {

/**
 * @brief Thread-safe command buffer for deferred ECS operations.
 *
 * Useful for:
 * - Recording operations during parallel iteration
 * - Batching entity/component changes for performance
 * - Avoiding structural changes during view iteration
 *
 * Example:
 *   CommandBuffer cmd(registry);
 *   registry.parallelView<Position>().each([&](Entity e, Position& p) {
 *       if (p.x > 100) {
 *           cmd.destroyEntityDeferred(e);  // Safe during parallel iteration
 *       }
 *   });
 *   cmd.flush();  // Apply all changes at once
 */
class CommandBuffer {
   public:
    explicit CommandBuffer(std::reference_wrapper<Registry> reg)
        : _registry(reg) {}

    /**
     * @brief Records entity creation for later execution.
     * @return Placeholder entity (real entity created on flush)
     */
    auto spawnEntityDeferred() -> Entity;

    /**
     * @brief Records entity destruction for later execution.
     */
    void destroyEntityDeferred(Entity entity);

    /**
     * @brief Records component addition for later execution.
     */
    template <typename T, typename... Args>
    void emplaceComponentDeferred(Entity entity, Args&&... args);

    /**
     * @brief Records component removal for later execution.
     */
    template <typename T>
    void removeComponentDeferred(Entity entity);

    /**
     * @brief Applies all recorded commands and clears the buffer.
     * NOT thread-safe: Call from main thread only.
     */
    void flush();

    /**
     * @brief Returns number of pending commands.
     */
    auto pendingCount() const -> size_t;

    /**
     * @brief Clears all pending commands without executing them.
     */
    void clear();

   private:
    std::reference_wrapper<Registry> _registry;
    std::vector<std::function<void()>> _commands;
    mutable std::mutex _commandsMutex;

    std::unordered_map<std::uint32_t, Entity> _placeholdertoReal;
    std::uint32_t _nextPlaceholderId = 0;
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_CORE_COMMANDBUFFER_HPP_

#ifndef SRC_ENGINE_ECS_CORE_COMMANDBUFFER_IMPL_HPP_
#define SRC_ENGINE_ECS_CORE_COMMANDBUFFER_IMPL_HPP_

namespace ECS {

template <typename T, typename... Args>
void CommandBuffer::emplaceComponentDeferred(Entity entity, Args&&... args) {
    std::lock_guard lock(_commandsMutex);

    auto captured_args = std::make_tuple(std::forward<Args>(args)...);

    _commands.push_back(
        [this, entity, captured_args = std::move(captured_args)]() mutable {
            Entity target_entity = entity;
            auto iter = _placeholdertoReal.find(entity.id);
            if (iter != _placeholdertoReal.end()) {
                target_entity = iter->second;
            }

            std::apply(
                [this, target_entity](auto&&... args) {
                    _registry.get().template emplaceComponent<T>(
                        target_entity, std::forward<decltype(args)>(args)...);
                },
                std::move(captured_args));
        });
}

template <typename T>
void CommandBuffer::removeComponentDeferred(Entity entity) {
    std::lock_guard lock(_commandsMutex);
    _commands.push_back([this, entity]() {
        Entity target_entity = entity;
        auto iter = _placeholdertoReal.find(entity.id);
        if (iter != _placeholdertoReal.end()) {
            target_entity = iter->second;
        }

        _registry.get().template removeComponent<T>(target_entity);
    });
}

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_CORE_COMMANDBUFFER_HPP_
