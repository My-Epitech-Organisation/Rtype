/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameOverState.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMEOVERSTATE_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMEOVERSTATE_HPP_

#include <cstdint>

namespace rtype::games::rtype::client {
struct GameOverState {
    std::uint32_t finalScore = 0;
    bool isVictory = false;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMEOVERSTATE_HPP_
