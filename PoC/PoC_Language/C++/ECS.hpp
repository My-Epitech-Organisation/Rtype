/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ECS
*/

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <optional>

using Entity = std::size_t;

struct Component {
    virtual ~Component() = default;
};

struct TransformComponent : public Component {
    float x;
    float y;
    explicit TransformComponent(float x_pos = 0.0f, float y_pos = 0.0f)
        : x(x_pos), y(y_pos) {}
};

struct VelocityComponent : public Component {
    float vx;
    float vy;
    explicit VelocityComponent(float x_vel = 0.0f, float y_vel = 0.0f)
        : vx(x_vel), vy(y_vel) {}
};

class Registry {
public:
    Entity createEntity() {
        return nextEntityId++;
    }

    template <typename T, typename... Args>
    void addComponent(Entity entity, Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        components[entity][std::type_index(typeid(T))] = std::move(component);
    }

    template <typename T>
    std::optional<std::reference_wrapper<T>> getComponent(Entity entity) {
        auto entityIt = components.find(entity);
        if (entityIt == components.end()) {
            return std::nullopt;
        }
        auto compIt = entityIt->second.find(std::type_index(typeid(T)));
        if (compIt == entityIt->second.end()) {
            return std::nullopt;
        }
        return std::ref(static_cast<T&>(*compIt->second));
    }

    template <typename T>
    bool hasComponent(Entity entity) const {
        return getComponent<T>(entity).has_value();
    }

    void update(float dt) {
        for (auto& [entity, comps] : components) {
            auto opt_transform = getComponent<TransformComponent>(entity);
            auto opt_velocity = getComponent<VelocityComponent>(entity);
            if (opt_transform && opt_velocity) {
                opt_transform->get().x += opt_velocity->get().vx * dt;
                opt_transform->get().y += opt_velocity->get().vy * dt;
                std::cout << "Entity " << entity << " moved to ("
                          << opt_transform->get().x << ", " << opt_transform->get().y << ")" << std::endl;
            }
        }
    }

private:
    Entity nextEntityId = 0;
    std::unordered_map<Entity, std::unordered_map<std::type_index, std::unique_ptr<Component>>> components;
};
