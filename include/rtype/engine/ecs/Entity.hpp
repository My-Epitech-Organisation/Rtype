/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Entity
*/

#pragma once

namespace rtype::engine::ecs {

using EntityId = unsigned int;

class Entity {
public:
    Entity() : id_(0) {}
    explicit Entity(EntityId id) : id_(id) {}

    EntityId id() const { return id_; }
    bool valid() const { return id_ != 0; }

    bool operator==(const Entity& other) const { return id_ == other.id_; }
    bool operator!=(const Entity& other) const { return id_ != other.id_; }

private:
    EntityId id_;
};

} // namespace rtype::engine::ecs
