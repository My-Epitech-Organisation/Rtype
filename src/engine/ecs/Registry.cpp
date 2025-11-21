/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Registry
*/

#include "rtype/engine/ecs/Registry.hpp"

namespace rtype::engine::ecs {

Registry::Registry() : nextId_(1) {}

Registry::~Registry() {
    clear();
}

Entity Registry::createEntity() {
    EntityId id = nextId_++;
    entities_.push_back(id);
    return Entity(id);
}

void Registry::destroyEntity(Entity entity) {
    for (auto it = entities_.begin(); it != entities_.end(); ++it) {
        if (*it == entity.id()) {
            entities_.erase(it);
            break;
        }
    }
}

void Registry::clear() {
    entities_.clear();
    nextId_ = 1;
}

size_t Registry::entityCount() const {
    return entities_.size();
}

} // namespace rtype::engine::ecs
