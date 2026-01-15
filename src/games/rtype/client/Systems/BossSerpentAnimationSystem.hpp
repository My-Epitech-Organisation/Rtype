/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BossSerpentAnimationSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOSSSERPENTANIMATIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOSSSERPENTANIMATIONSYSTEM_HPP_

#include "ASystem.hpp"

namespace rtype::games::rtype::client {

/**
 * @class BossSerpentAnimationSystem
 * @brief Handles boss serpent visual animation (head direction, body segments)
 */
class BossSerpentAnimationSystem : public ::rtype::engine::ASystem {
   public:
    BossSerpentAnimationSystem();
    void update(ECS::Registry& registry, float dt) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOSSSERPENTANIMATIONSYSTEM_HPP_
