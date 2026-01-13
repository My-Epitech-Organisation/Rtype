/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserExplosionComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHASEREXPLOSIONCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHASEREXPLOSIONCOMPONENT_HPP_

namespace rtype::games::rtype::client {

/**
 * @brief Component to track explosion state of Chaser enemy
 */
struct ChaserExplosion {
    bool isExploding;
    float explosionTimer;

    explicit ChaserExplosion(bool exploding = false, float timer = 0.0f)
        : isExploding(exploding), explosionTimer(timer) {}
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHASEREXPLOSIONCOMPONENT_HPP_
