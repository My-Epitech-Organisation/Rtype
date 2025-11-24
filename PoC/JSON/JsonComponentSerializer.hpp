/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** JsonComponentSerializer - JSON-based ECS component serialization
*/

#ifndef JSON_COMPONENT_SERIALIZER_HPP
    #define JSON_COMPONENT_SERIALIZER_HPP

#include <ECS/ECS.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

namespace PoC {

    /**
     * @brief Position component for ECS
     */
    struct Position {
        float x;
        float y;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Position, x, y)
    };

    /**
     * @brief Velocity component for ECS
     */
    struct Velocity {
        float dx;
        float dy;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Velocity, dx, dy)
    };

    /**
     * @brief Health component for ECS
     */
    struct Health {
        int current;
        int max;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Health, current, max)
    };

    /**
     * @brief Tag component for entity identification
     */
    struct PlayerTag {};
    struct EnemyTag {};

    /**
     * @brief Generic JSON component serializer for ECS
     * @tparam T Component type (must have nlohmann::json support)
     */
    template<typename T>
    class JsonComponentSerializer : public ECS::IComponentSerializer {
    public:
        std::string serialize(ECS::Entity entity, ECS::Registry* registry) const override {
            if (registry->hasComponent<T>(entity)) {
                const T& component = registry->getComponent<T>(entity);
                nlohmann::json j = component;
                return j.dump();
            }
            return "{}";
        }

        void deserialize(ECS::Entity entity, const std::string& data, ECS::Registry* registry) const override {
            nlohmann::json j = nlohmann::json::parse(data);
            T component = j.get<T>();
            registry->emplaceComponent<T>(entity, component);
        }
    };

    /**
     * @brief Save ECS entities to JSON file
     * @param registry ECS registry
     * @param filename Output filename
     */
    inline void saveEntitiesToJson(ECS::Registry& registry, const std::string& filename) {
        nlohmann::json output;
        output["entities"] = nlohmann::json::array();

        // Iterate through all entities with Position component
        registry.view<Position>().each([&](ECS::Entity entity, const Position& pos) {
            nlohmann::json entityData;
            entityData["id"] = entity.id;

            // Serialize Position
            entityData["position"] = pos;

            // Serialize Velocity
            if (registry.hasComponent<Velocity>(entity)) {
                entityData["velocity"] = registry.getComponent<Velocity>(entity);
            }

            // Serialize Health
            if (registry.hasComponent<Health>(entity)) {
                entityData["health"] = registry.getComponent<Health>(entity);
            }

            // Check for tags
            if (registry.hasComponent<PlayerTag>(entity)) {
                entityData["tag"] = "player";
            } else if (registry.hasComponent<EnemyTag>(entity)) {
                entityData["tag"] = "enemy";
            }

            output["entities"].push_back(entityData);
        });

        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create file: " + filename);
        }
        file << output.dump(4);
    }

    /**
     * @brief Load ECS entities from JSON file
     * @param registry ECS registry
     * @param filename Input filename
     */
    inline void loadEntitiesFromJson(ECS::Registry& registry, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        nlohmann::json input;
        file >> input;

        for (const auto& entityData : input["entities"]) {
            ECS::Entity entity = registry.spawnEntity();

            // Deserialize Position
            if (entityData.contains("position")) {
                Position pos = entityData["position"].get<Position>();
                registry.emplaceComponent<Position>(entity, pos);
            }

            // Deserialize Velocity
            if (entityData.contains("velocity")) {
                Velocity vel = entityData["velocity"].get<Velocity>();
                registry.emplaceComponent<Velocity>(entity, vel);
            }

            // Deserialize Health
            if (entityData.contains("health")) {
                Health health = entityData["health"].get<Health>();
                registry.emplaceComponent<Health>(entity, health);
            }

            // Deserialize tags
            if (entityData.contains("tag")) {
                std::string tag = entityData["tag"].get<std::string>();
                if (tag == "player") {
                    registry.emplaceComponent<PlayerTag>(entity);
                } else if (tag == "enemy") {
                    registry.emplaceComponent<EnemyTag>(entity);
                }
            }
        }
    }

} // namespace PoC

#endif // JSON_COMPONENT_SERIALIZER_HPP
