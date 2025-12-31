/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AnimationSystem.hpp
*/

#ifndef R_TYPE_ANIMATIONSYSTEM_HPP
#define R_TYPE_ANIMATIONSYSTEM_HPP

#include "BoxingSystem.hpp"

#include <memory>
#include "../Components/ImageComponent.hpp"

namespace rtype::games::rtype::client {
    class AnimationSystem : public ::rtype::engine::ASystem {
    public:
        AnimationSystem();
        void update(ECS::Registry& registry, float dt) override;
    };
}  // namespace rtype::games::rtype::client


#endif //R_TYPE_ANIMATIONSYSTEM_HPP