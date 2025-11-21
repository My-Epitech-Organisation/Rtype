/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SystemScheduler implementation
*/

#include "SystemScheduler.hpp"
#include "../Core/Registry.hpp"
#include <queue>
#include <sstream>

namespace ECS {

    void SystemScheduler::add_system(const std::string& name, SystemFunc func,
                                     const std::vector<std::string>& dependencies) {
        if (systems.find(name) != systems.end()) {
            throw std::runtime_error("System '" + name + "' already registered");
        }

        systems[name] = SystemNode{name, std::move(func), dependencies, true};
        needs_reorder = true;
    }

    void SystemScheduler::remove_system(const std::string& name) {
        systems.erase(name);
        needs_reorder = true;
    }

    void SystemScheduler::run() {
        if (needs_reorder) {
            recompute_order();
            needs_reorder = false;
        }

        for (const auto& system_name : execution_order) {
            auto it = systems.find(system_name);
            if (it != systems.end() && it->second.enabled) {
                it->second.func(registry);
            }
        }
    }

    void SystemScheduler::run_system(const std::string& name) {
        auto it = systems.find(name);
        if (it == systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        if (it->second.enabled) {
            it->second.func(registry);
        }
    }

    void SystemScheduler::clear() {
        systems.clear();
        execution_order.clear();
        needs_reorder = true;
    }

    std::vector<std::string> SystemScheduler::get_execution_order() const {
        return execution_order;
    }

    void SystemScheduler::set_system_enabled(const std::string& name, bool enabled) {
        auto it = systems.find(name);
        if (it == systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        it->second.enabled = enabled;
    }

    bool SystemScheduler::is_system_enabled(const std::string& name) const {
        auto it = systems.find(name);
        if (it == systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        return it->second.enabled;
    }

    void SystemScheduler::recompute_order() {
        if (has_cycle()) {
            throw std::runtime_error("Circular dependency detected in system graph");
        }
        topological_sort();
    }

    void SystemScheduler::topological_sort() {
        execution_order.clear();
        std::unordered_map<std::string, int> in_degree;
        std::unordered_map<std::string, std::vector<std::string>> adjacency_list;

        for (const auto& [name, node] : systems) {
            in_degree[name] = 0;
        }

        for (const auto& [name, node] : systems) {
            for (const auto& dep : node.dependencies) {
                if (systems.find(dep) == systems.end()) {
                    throw std::runtime_error("System '" + name + "' depends on non-existent system '" + dep + "'");
                }
                adjacency_list[dep].push_back(name);
                in_degree[name]++;
            }
        }
        std::queue<std::string> zero_degree;
        for (const auto& [name, degree] : in_degree) {
            if (degree == 0) {
                zero_degree.push(name);
            }
        }

        while (!zero_degree.empty()) {
            std::string current = zero_degree.front();
            zero_degree.pop();
            execution_order.push_back(current);

            for (const auto& neighbor : adjacency_list[current]) {
                in_degree[neighbor]--;
                if (in_degree[neighbor] == 0) {
                    zero_degree.push(neighbor);
                }
            }
        }

        if (execution_order.size() != systems.size()) {
            throw std::runtime_error("Failed to compute execution order (possible cycle)");
        }
    }

    bool SystemScheduler::has_cycle() const {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> rec_stack;

        std::function<bool(const std::string&)> dfs = [&](const std::string& name) -> bool {
            visited.insert(name);
            rec_stack.insert(name);

            auto it = systems.find(name);
            if (it != systems.end()) {
                for (const auto& dep : it->second.dependencies) {
                    if (visited.find(dep) == visited.end()) {
                        if (dfs(dep)) return true;
                    } else if (rec_stack.find(dep) != rec_stack.end()) {
                        return true;
                    }
                }
            }

            rec_stack.erase(name);
            return false;
        };

        for (const auto& [name, _] : systems) {
            if (visited.find(name) == visited.end()) {
                if (dfs(name)) return true;
            }
        }

        return false;
    }

} // namespace ECS
