/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextureRectComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
#include <utility>

#include "../../../../../include/rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

struct TextureRect {
   public:
    display::IntRect rect;
    TextureRect() : rect({0, 0, 0, 0}) {}
    TextureRect(std::pair<int, int> position, std::pair<int, int> size)
        : rect({position.first, position.second, size.first, size.second}) {}
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
