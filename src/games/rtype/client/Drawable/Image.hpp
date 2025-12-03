/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Image
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_DRAWABLE_IMAGE_HPP_
#define SRC_GAMES_RTYPE_CLIENT_DRAWABLE_IMAGE_HPP_
#include "../Components/ImageComponent.hpp"
#include "../Components/ZIndexComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "ecs/core/Entity.hpp"

struct DrawableImage {
    ECS::Entity entity;
    Image* img;
    Position* pos;
    ZIndex* zindex;
};
#endif  // SRC_GAMES_RTYPE_CLIENT_DRAWABLE_IMAGE_HPP_
