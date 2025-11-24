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
#include <utility>

namespace ECS {

    void SystemScheduler::addSystem(const std::string& name, SystemFunc func,
                                     const std::vector<std::string>& dependencies) {
        if (_systems.find(name) != _systems.end()) {
            throw std::runtime_error("System '" + name + "' already registered");
        }

        _systems[name] = SystemNode{name, std::move(func), dependencies, true};
        _needsReorder = true;
    }

    void SystemScheduler::removeSystem(const std::string& name) {
        _systems.erase(name);
        _needsReorder = true;
    }

    void SystemScheduler::run() {
        if (_needsReorder) {
            recomputeOrder();
            _needsReorder = false;
        }

        for (const auto& system_name : _executionOrder) {
            auto it = _systems.find(system_name);
            if (it != _systems.end() && it->second.enabled) {
                it->second.func(registry);
            }
        }
    }

    void SystemScheduler::runSystem(const std::string& name) {
        auto it = _systems.find(name);
        if (it == _systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        if (it->second.enabled) {
            it->second.func(registry);
        }
    }

    void SystemScheduler::clear() {
        _systems.clear();
        _executionOrder.clear();
        _needsReorder = true;
    }

    std::vector<std::string> SystemScheduler::getExecutionOrder() const {
        return _executionOrder;
    }

    void SystemScheduler::setSystemEnabled(const std::string& name, bool enabled) {
        auto it = _systems.find(name);
        if (it == _systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        it->second.enabled = enabled;
    }

    bool SystemScheduler::isSystemEnabled(const std::string& name) const {
        auto it = _systems.find(name);
        if (it == _systems.end()) {
            throw std::runtime_error("System '" + name + "' not found");
        }
        return it->second.enabled;
    }

    void SystemScheduler::recomputeOrder() {
        if (hasCycle()) {
            throw std::runtime_error("Circular dependency detected in system graph");
        }
        topologicalSort();
    }

    void SystemScheduler::topologicalSort() {
        _executionOrder.clear();
        std::unordered_map<std::string, int> in_degree;
        std::unordered_map<std::string, std::vector<std::string>> adjacency_list;

        for (const auto& [name, node] : _systems) {
            in_degree[name] = 0;
        }

        for (const auto& [name, node] : _systems) {
            for (const auto& dep : node.dependencies) {
                if (_systems.find(dep) == _systems.end()) {
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
            _executionOrder.push_back(current);

            for (const auto& neighbor : adjacency_list[current]) {
                in_degree[neighbor]--;
                if (in_degree[neighbor] == 0) {
                    zero_degree.push(neighbor);
                }
            }
        }

        if (_executionOrder.size() != _systems.size()) {
            throw std::runtime_error("Failed to compute execution order (possible cycle)");
        }
    }

    bool SystemScheduler::hasCycle() const {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> rec_stack;

        std::function<bool(const std::string&)> dfs = [&](const std::string& name) -> bool {
            visited.insert(name);
            rec_stack.insert(name);

            auto it = _systems.find(name);
            if (it != _systems.end()) {
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

        for (const auto& [name, _] : _systems) {
            if (visited.find(name) == visited.end()) {
                if (dfs(name)) return true;
            }
        }

        return false;
    }

} // namespace ECS
