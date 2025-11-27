/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Prefab - Entity templates for efficient spawning
*/

#ifndef SRC_ENGINE_ECS_CORE_PREFAB_HPP_
#define SRC_ENGINE_ECS_CORE_PREFAB_HPP_

#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Entity.hpp"

namespace ECS {

class Registry;

/**
 * @brief Template for spawning pre-configured entities.
 *
 * Prefabs allow you to define entity "blueprints" with predefined component sets.
 * This is useful for:
 * - Game object templates (Player, Enemy, Bullet, etc.)
 * - Level design with reusable entities
 * - Network entity synchronization
 * - Save/Load systems
 *
 * Example:
 *   PrefabManager prefabs(registry);
 *
 *   // Define prefab
 *   prefabs.registerPrefab("Player", [](Registry& r, Entity e) {
 *       r.emplaceComponent<Position>(e, 0.0f, 0.0f);
 *       r.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
 *       r.emplaceComponent<Player>(e);
 *   });
 *
 *   // Spawn from prefab
 *   auto player = prefabs.instantiate("Player");
 */
class PrefabManager {
 public:
    using PrefabFunc = std::function<void(Registry&, Entity)>;

    explicit PrefabManager(std::reference_wrapper<Registry> reg) : _registry(reg) {}

        /**
         * @brief Registers a new prefab template.
         * @param name Unique prefab identifier
         * @param func Function that configures entity components
         */
        void registerPrefab(const std::string& name, PrefabFunc func);

        /**
         * @brief Spawns entity from prefab template.
         * @param name Prefab name
         * @return Newly created entity
         * @throws std::runtime_error if prefab not found
         */
        auto instantiate(const std::string& name) -> Entity;

        /**
         * @brief Spawns entity from prefab and applies additional configuration.
         * @param name Prefab name
         * @param customizer Additional configuration function
         * @return Newly created entity
         */
        auto instantiate(const std::string& name,
                         const PrefabFunc& customizer) -> Entity;

        /**
         * @brief Spawns multiple entities from same prefab.
         * @param name Prefab name
         * @param count Number of instances to create
         * @return Vector of created entities
         */
        auto instantiateMultiple(const std::string& name,
                                  size_t count) -> std::vector<Entity>;

        /**
         * @brief Checks if prefab exists.
         */
        auto hasPrefab(const std::string& name) const -> bool;

        /**
         * @brief Removes prefab definition.
         */
        void unregisterPrefab(const std::string& name);

        /**
         * @brief Gets all registered prefab names.
         */
        auto getPrefabNames() const -> std::vector<std::string>;

        /**
         * @brief Clears all prefab definitions.
         */
        void clear();

        /**
         * @brief Creates prefab from existing entity (saves its configuration).
         * Useful for level editors or runtime prefab creation.
         * @param name New prefab name
         * @param template_entity Entity to use as template
         * @throws std::runtime_error if entity is dead or has no components
         */
        void createFromEntity(const std::string& name, Entity template_entity);

    private:
        std::reference_wrapper<Registry> _registry;
        std::unordered_map<std::string, PrefabFunc> _prefabs;
    mutable std::shared_mutex _prefabMutex;
};

}  // namespace ECS

#endif  // SRC_ENGINE_ECS_CORE_PREFAB_HPP_
