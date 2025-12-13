/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** VisualCueFactory.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMESCENE_VISUALCUEFACTORY_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMESCENE_VISUALCUEFACTORY_HPP_

#include <memory>
#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>

#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Helper factory to spawn short-lived visual cues that mirror audio
 * events.
 */
class VisualCueFactory {
   public:
    static void createFlash(ECS::Registry& registry, const sf::Vector2f& center,
                            const sf::Color& color, float size = 64.f,
                            float lifetime = 0.35f, int zIndex = 50);

    /**
     * @brief Create a floating damage number popup (WoW style)
     * @param registry The ECS registry
     * @param position Position where the number appears
     * @param damage The damage amount to display
     * @param font The font to use
     * @param color Text color (default red for damage)
     */
    static void createDamagePopup(ECS::Registry& registry,
                                  const sf::Vector2f& position, int damage,
                                  const sf::Font& font,
                                  const sf::Color& color = sf::Color::Red);
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_VISUALCUEFACTORY_HPP_
