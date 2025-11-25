/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Registry
*/

#pragma once

#include <vector>

#include "Entity.hpp"

namespace rtype::engine::ecs {

class Registry {
   public:
    Registry();
    ~Registry();

    Entity createEntity();
    void destroyEntity(Entity entity);

    void clear();
    size_t entityCount() const;

   private:
    EntityId nextId_;
    std::vector<EntityId> entities_;
};

}  // namespace rtype::engine::ecs
