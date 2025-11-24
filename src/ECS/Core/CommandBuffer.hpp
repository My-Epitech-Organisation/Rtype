/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CommandBuffer
*/

#ifndef ECS_CORE_COMMAND_BUFFER_HPP
    #define ECS_CORE_COMMAND_BUFFER_HPP
    #include "Entity.hpp"
    #include "Registry/Registry.hpp"
    #include <vector>
    #include <functional>
    #include <mutex>
    #include <any>
    #include <typeindex>
    #include <unordered_map>
    #include <utility>

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
     *   registry.parallel_view<Position>().each([&](Entity e, Position& p) {
     *       if (p.x > 100) {
     *           cmd.destroy_entity_deferred(e);  // Safe during parallel iteration
     *       }
     *   });
     *   cmd.flush();  // Apply all changes at once
     */
    class CommandBuffer {
    public:
        explicit CommandBuffer(Registry& reg) : registry(reg) {}

        /**
         * @brief Records entity creation for later execution.
         * @return Placeholder entity (real entity created on flush)
         */
        Entity spawn_entity_deferred();

        /**
         * @brief Records entity destruction for later execution.
         */
        void destroy_entity_deferred(Entity entity);

        /**
         * @brief Records component addition for later execution.
         */
        template<typename T, typename... Args>
        void emplace_component_deferred(Entity entity, Args&&... args);

        /**
         * @brief Records component removal for later execution.
         */
        template<typename T>
        void remove_component_deferred(Entity entity);

        /**
         * @brief Applies all recorded commands and clears the buffer.
         * NOT thread-safe: Call from main thread only.
         */
        void flush();

        /**
         * @brief Returns number of pending commands.
         */
        size_t pending_count() const;

        /**
         * @brief Clears all pending commands without executing them.
         */
        void clear();

    private:
        Registry& registry;
        std::vector<std::function<void()>> commands;
        mutable std::mutex commands_mutex;

        std::unordered_map<std::uint32_t, Entity> placeholder_to_real;
        std::uint32_t next_placeholder_id = 0;
    };

} // namespace ECS

#endif // ECS_CORE_COMMAND_BUFFER_HPP

// Template implementations (must be after Registry.hpp is included)
#ifndef ECS_COMMAND_BUFFER_IMPL_HPP
    #define ECS_COMMAND_BUFFER_IMPL_HPP

namespace ECS {

    template<typename T, typename... Args>
    void CommandBuffer::emplace_component_deferred(Entity entity, Args&&... args) {
        std::lock_guard lock(commands_mutex);

        auto captured_args = std::make_tuple(std::forward<Args>(args)...);

        commands.push_back([this, entity, captured_args = std::move(captured_args)]() mutable {
            Entity target_entity = entity;
            auto it = placeholder_to_real.find(entity.id);
            if (it != placeholder_to_real.end()) {
                target_entity = it->second;
            }

            std::apply([this, target_entity](auto&&... args) {
                registry.template emplace_component<T>(target_entity, std::forward<decltype(args)>(args)...);
            }, std::move(captured_args));
        });
    }

    template<typename T>
    void CommandBuffer::remove_component_deferred(Entity entity) {
        std::lock_guard lock(commands_mutex);
        commands.push_back([this, entity]() {
            Entity target_entity = entity;
            auto it = placeholder_to_real.find(entity.id);
            if (it != placeholder_to_real.end()) {
                target_entity = it->second;
            }

            registry.template remove_component<T>(target_entity);
        });
    }

} // namespace ECS

#endif // ECS_COMMAND_BUFFER_IMPL_HPP
