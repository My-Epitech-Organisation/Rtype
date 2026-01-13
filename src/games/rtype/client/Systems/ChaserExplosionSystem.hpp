/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserExplosionSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASEREXPLOSIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASEREXPLOSIONSYSTEM_HPP_

#include "../../../../lib/engine/src/ASystem.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System to manage Chaser explosion animation
 * Prevents animation from looping back after explosion starts
 */
class ChaserExplosionSystem : public ::rtype::engine::ASystem {
   public:
    ChaserExplosionSystem();
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASEREXPLOSIONSYSTEM_HPP_
