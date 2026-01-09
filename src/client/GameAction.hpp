/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameAction.hpp
*/

#ifndef SRC_CLIENT_GAMEACTION_HPP_
#define SRC_CLIENT_GAMEACTION_HPP_

#include <cstdint>

enum class GameAction : std::uint8_t {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    FORCE_POD,
    CHANGE_AMMO,
    FORCE_POD,
    PAUSE,
    NONE
};

#endif  // SRC_CLIENT_GAMEACTION_HPP_
