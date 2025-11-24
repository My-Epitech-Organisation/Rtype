/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SystemScheduler - Formal system management with dependency graph
*/

#ifndef ECS_SYSTEM_SYSTEM_SCHEDULER_HPP
    #define ECS_SYSTEM_SYSTEM_SCHEDULER_HPP
    #include "../Core/Entity.hpp"
    #include <functional>
    #include <vector>
    #include <unordered_map>
    #include <unordered_set>
    #include <string>
    #include <algorithm>
    #include <stdexcept>
    #include <thread>

namespace ECS {

    class Registry;

    /**
     * @brief System scheduler with automatic dependency resolution.
     *
     * Features:
     * - Topological sorting of systems based on dependencies
     * - Parallel execution of independent systems
     * - Named systems for easy management
     * - Before/after dependency specification
     *
     * Example:
     *   SystemScheduler scheduler(registry);
     *   scheduler.addSystem("physics", physics_system);
     *   scheduler.addSystem("render", render_system, {"physics"}); // runs after physics
     *   scheduler.run(); // Execute all systems in order
     */
    class SystemScheduler {
    public:
        using SystemFunc = std::function<void(Registry&)>;

        explicit SystemScheduler(Registry& reg) : registry(reg) {}

        /**
         * @brief Registers a system with optional dependencies.
         * @param name Unique system identifier
         * @param func System function to execute
         * @param dependencies List of system names that must run before this one
         */
        void addSystem(const std::string& name, SystemFunc func,
                       const std::vector<std::string>& dependencies = {});

        /**
         * @brief Removes a system by name.
         */
        void removeSystem(const std::string& name);

        /**
         * @brief Executes all systems in dependency order.
         * Independent systems may run in parallel.
         */
        void run();

        /**
         * @brief Executes a specific system by name.
         */
        void runSystem(const std::string& name);

        /**
         * @brief Clears all registered systems.
         */
        void clear();

        /**
         * @brief Returns execution order of systems (for debugging).
         */
        std::vector<std::string> getExecutionOrder() const;

        /**
         * @brief Enables or disables a system without removing it.
         */
        void setSystemEnabled(const std::string& name, bool enabled);

        /**
         * @brief Checks if a system is enabled.
         */
        bool isSystemEnabled(const std::string& name) const;

    private:
        struct SystemNode {
            std::string name;
            SystemFunc func;
            std::vector<std::string> dependencies;
            bool enabled = true;
        };

        Registry& registry;
        std::unordered_map<std::string, SystemNode> _systems;
        std::vector<std::string> _executionOrder;
        bool _needsReorder = true;

        void recomputeOrder();
        void topologicalSort();
        bool hasCycle() const;
    };

} // namespace ECS

#endif // ECS_SYSTEM_SYSTEM_SCHEDULER_HPP
