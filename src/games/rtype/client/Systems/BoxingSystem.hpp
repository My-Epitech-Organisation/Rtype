/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_

#include <memory>

#include "ASystem.hpp"
#include "ECS.hpp"
#include "rtype/display/IDisplay.hpp"

namespace rtype::games::rtype::client {
class BoxingSystem : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<::rtype::display::IDisplay> _display;

   public:
    explicit BoxingSystem(std::shared_ptr<::rtype::display::IDisplay> display);
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BOXINGSYSTEM_HPP_
