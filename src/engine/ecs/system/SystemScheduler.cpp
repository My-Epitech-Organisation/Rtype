/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SystemScheduler implementation
*/

#include "SystemScheduler.hpp"

#include <queue>
#include <sstream>
#include <utility>

#include "../core/Registry/Registry.hpp"

namespace ECS {

void SystemScheduler::addSystem(const std::string& name, const SystemFunc& func,
                                const std::vector<std::string>& dependencies) {
    if (_systems.find(name) != _systems.end()) {
        throw std::runtime_error("System '" + name + "' already registered");
    }

    _systems[name] = SystemNode{.name = name,
                                .func = func,
                                .dependencies = dependencies,
                                .enabled = true};
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
        auto iter = _systems.find(system_name);
        if (iter != _systems.end() && iter->second.enabled) {
            iter->second.func(registry.get());
        }
    }
}

void SystemScheduler::runSystem(const std::string& name) {
    auto iter = _systems.find(name);
    if (iter == _systems.end()) {
        throw std::runtime_error("System '" + name + "' not found");
    }
    if (iter->second.enabled) {
        iter->second.func(registry.get());
    }
}

void SystemScheduler::clear() {
    _systems.clear();
    _executionOrder.clear();
    _needsReorder = true;
}

auto SystemScheduler::getExecutionOrder() const -> std::vector<std::string> {
    return _executionOrder;
}

void SystemScheduler::setSystemEnabled(const std::string& name, bool enabled) {
    auto iter = _systems.find(name);
    if (iter == _systems.end()) {
        throw std::runtime_error("System '" + name + "' not found");
    }
    iter->second.enabled = enabled;
}

auto SystemScheduler::isSystemEnabled(const std::string& name) const -> bool {
    auto iter = _systems.find(name);
    if (iter == _systems.end()) {
        throw std::runtime_error("System '" + name + "' not found");
    }
    return iter->second.enabled;
}

void SystemScheduler::recomputeOrder() {
    if (hasCycle()) {
        throw std::runtime_error(
            "Circular dependency detected in system graph");
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
                std::string errorMsg = "System '";
                errorMsg += name;
                errorMsg += "' depends on non-existent system '";
                errorMsg += dep;
                errorMsg += "'";
                throw std::runtime_error(errorMsg);
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
        throw std::runtime_error(
            "Failed to compute execution order (possible cycle)");
    }
}

auto SystemScheduler::hasCycle() const -> bool {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> rec_stack;

    std::function<bool(const std::string&)> dfs =
        [&](const std::string& name) -> bool {
        visited.insert(name);
        rec_stack.insert(name);

        auto iter = _systems.find(name);
        if (iter != _systems.end()) {
            for (const auto& dep : iter->second.dependencies) {
                if (visited.find(dep) == visited.end()) {
                    if (dfs(dep)) {
                        return true;
                    }
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
            if (dfs(name)) {
                return true;
            }
        }
    }

    return false;
}

}  // namespace ECS
