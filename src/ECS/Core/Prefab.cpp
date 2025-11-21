/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Prefab
*/

#include "Prefab.hpp"
#include "Registry.hpp"
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <vector>

namespace ECS {

    void PrefabManager::register_prefab(const std::string& name, PrefabFunc func) {
        std::unique_lock lock(prefab_mutex);
        prefabs[name] = std::move(func);
    }

    Entity PrefabManager::instantiate(const std::string& name) {
        PrefabFunc func;
        {
            std::shared_lock lock(prefab_mutex);
            auto it = prefabs.find(name);
            if (it == prefabs.end()) {
                throw std::runtime_error("Prefab not found: " + name);
            }
            func = it->second;
        }

        auto entity = registry.spawn_entity();
        func(registry, entity);
        return entity;
    }

    Entity PrefabManager::instantiate(const std::string& name, PrefabFunc customizer) {
        auto entity = instantiate(name);
        customizer(registry, entity);
        return entity;
    }

    std::vector<Entity> PrefabManager::instantiate_multiple(const std::string& name, size_t count) {
        PrefabFunc func;
        {
            std::shared_lock lock(prefab_mutex);
            auto it = prefabs.find(name);
            if (it == prefabs.end()) {
                throw std::runtime_error("Prefab not found: " + name);
            }
            func = it->second;
        }

        std::vector<Entity> entities;
        entities.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            auto entity = registry.spawn_entity();
            func(registry, entity);
            entities.push_back(entity);
        }

        return entities;
    }

    bool PrefabManager::has_prefab(const std::string& name) const {
        std::shared_lock lock(prefab_mutex);
        return prefabs.find(name) != prefabs.end();
    }

    void PrefabManager::unregister_prefab(const std::string& name) {
        std::unique_lock lock(prefab_mutex);
        prefabs.erase(name);
    }

    std::vector<std::string> PrefabManager::get_prefab_names() const {
        std::shared_lock lock(prefab_mutex);
        std::vector<std::string> names;
        names.reserve(prefabs.size());

        for (const auto& [name, _] : prefabs) {
            names.push_back(name);
        }

        std::sort(names.begin(), names.end());
        return names;
    }

    void PrefabManager::clear() {
        std::unique_lock lock(prefab_mutex);
        prefabs.clear();
    }

    void PrefabManager::create_from_entity(const std::string& name, Entity template_entity) {
        if (!registry.is_alive(template_entity)) {
            throw std::runtime_error("Cannot create prefab from dead entity");
        }

        const auto& component_types = registry.get_entity_components(template_entity);
        if (component_types.empty()) {
            throw std::runtime_error("Cannot create prefab from entity with no components");
        }

        // Note: This is a simplified version. A full implementation would need
        // to serialize component data and store it. For now, we just create
        // an empty prefab that needs manual configuration.
        std::unique_lock lock(prefab_mutex);
        prefabs[name] = [](Registry&, Entity) {
            // Placeholder - would need component serialization
        };
    }

} // namespace ECS
