/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ChaserRotationSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASERROTATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASERROTATIONSYSTEM_HPP_

#include <rtype/engine.hpp>

namespace rtype::games::rtype::client {

/**
 * @brief System that rotates chaser enemies to face their target player
 */
class ChaserRotationSystem : public ::rtype::engine::ASystem {
   public:
    ChaserRotationSystem();
    ~ChaserRotationSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHASERROTATIONSYSTEM_HPP_
