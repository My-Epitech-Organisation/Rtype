/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Prefab
*/

#include "Prefab.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Registry/Registry.hpp"

namespace ECS {

void PrefabManager::registerPrefab(const std::string& name, PrefabFunc func) {
    std::unique_lock lock(_prefabMutex);
    _prefabs[name] = std::move(func);
}

auto PrefabManager::instantiate(const std::string& name) -> Entity {
    PrefabFunc func;
    {
        std::shared_lock lock(_prefabMutex);
        auto iter = _prefabs.find(name);
        if (iter == _prefabs.end()) {
            throw std::runtime_error("Prefab not found: " + name);
        }
        func = iter->second;
    }

    auto entity = _registry.get().spawnEntity();
    func(_registry.get(), entity);
    return entity;
}

auto PrefabManager::instantiate(const std::string& name,
                                const PrefabFunc& customizer) -> Entity {
    auto entity = instantiate(name);
    customizer(_registry.get(), entity);
    return entity;
}

auto PrefabManager::instantiateMultiple(const std::string& name, size_t count)
    -> std::vector<Entity> {
    PrefabFunc func;
    {
        std::shared_lock lock(_prefabMutex);
        auto iter = _prefabs.find(name);
        if (iter == _prefabs.end()) {
            throw std::runtime_error("Prefab not found: " + name);
        }
        func = iter->second;
    }

    std::vector<Entity> entities;
    entities.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        auto entity = _registry.get().spawnEntity();
        func(_registry.get(), entity);
        entities.push_back(entity);
    }

    return entities;
}

auto PrefabManager::hasPrefab(const std::string& name) const -> bool {
    std::shared_lock lock(_prefabMutex);
    return _prefabs.find(name) != _prefabs.end();
}

void PrefabManager::unregisterPrefab(const std::string& name) {
    std::unique_lock lock(_prefabMutex);
    _prefabs.erase(name);
}

auto PrefabManager::getPrefabNames() const -> std::vector<std::string> {
    std::shared_lock lock(_prefabMutex);
    std::vector<std::string> names;
    names.reserve(_prefabs.size());

    for (const auto& [name, _] : _prefabs) {
        names.push_back(name);
    }

    std::ranges::sort(names);
    return names;
}

void PrefabManager::clear() {
    std::unique_lock lock(_prefabMutex);
    _prefabs.clear();
}

void PrefabManager::createFromEntity(const std::string& name,
                                     Entity template_entity) {
    if (!_registry.get().isAlive(template_entity)) {
        throw std::runtime_error("Cannot create prefab from dead entity");
    }
    (void)name;  // To avoid unused parameter warning

    const auto& component_types =
        _registry.get().getEntityComponents(template_entity);
    if (component_types.empty()) {
        throw std::runtime_error(
            "Cannot create prefab from entity with no components");
    }

    // TODO(SamTess): Full implementation requires component serialization
    // system This would need:
    // 1. A way to clone/copy component data generically
    // 2. Component serialization/deserialization
    // 3. Type-safe component copying
    //
    // For now, this function is NOT IMPLEMENTED.
    // Use registerPrefab() with a manual configuration function instead.
    throw std::runtime_error(
        "createFromEntity is not yet implemented. Use registerPrefab() "
        "instead.");
}

}  // namespace ECS
