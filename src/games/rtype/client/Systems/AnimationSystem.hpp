/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AnimationSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP_

#include <memory>

#include "../Components/ImageComponent.hpp"
#include "BoxingSystem.hpp"

namespace rtype::games::rtype::client {
class AnimationSystem : public ::rtype::engine::ASystem {
   public:
    AnimationSystem();
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP_
