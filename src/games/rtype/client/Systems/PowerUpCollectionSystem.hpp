/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PowerUpCollectionSystem - Detects power-up collection and shows popups
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_POWERUPCOLLECTIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_POWERUPCOLLECTIONSYSTEM_HPP_

#include <memory>
#include <unordered_map>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include "../../shared/Components/PowerUpComponent.hpp"
#include "rtype/display/IDisplay.hpp"
#include "rtype/engine.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief System that detects when players collect power-ups and displays popup
 * notifications
 */
class PowerUpCollectionSystem : public ::rtype::engine::ASystem {
   public:
    explicit PowerUpCollectionSystem(const std::string &font);
    ~PowerUpCollectionSystem() override = default;

    void update(ECS::Registry& registry, float dt) override;

   private:
    const std::string &_font;

    struct PowerUpState {
        ::rtype::games::rtype::shared::PowerUpType type;
        float remainingTime;
    };
    std::unordered_map<uint32_t, PowerUpState> _lastPowerUpState;

    std::string getPowerUpDisplayName(
        ::rtype::games::rtype::shared::PowerUpType type) const;
    ::rtype::display::Color getPowerUpColor(
        ::rtype::games::rtype::shared::PowerUpType type) const;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_POWERUPCOLLECTIONSYSTEM_HPP_
