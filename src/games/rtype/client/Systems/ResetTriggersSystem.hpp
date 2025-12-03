/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ResetTriggersSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_

#include <memory>

#include <SFML/Graphics.hpp>

#include "ecs/ECS.hpp"

class ResetTriggersSystem {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RESETTRIGGERSSYSTEM_HPP_
