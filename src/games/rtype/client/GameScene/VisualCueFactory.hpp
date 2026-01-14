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

#include "ECS.hpp"
#include "rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Helper factory to spawn short-lived visual cues that mirror audio
 * events.
 */
class VisualCueFactory {
   public:
    static void createFlash(ECS::Registry& registry,
                            const ::rtype::display::Vector2f& center,
                            const ::rtype::display::Color& color,
                            float size = 64.f, float lifetime = 0.35f,
                            int zIndex = 50);

    /**
     * @brief Create a floating damage number popup (WoW style)
     * @param registry The ECS registry
     * @param position Position where the number appears
     * @param damage The damage amount to display
     * @param fontName The font name to use
     * @param color Text color (default red for damage)
     */
    static void createDamagePopup(
        ECS::Registry& registry, const ::rtype::display::Vector2f& position,
        int damage, const std::string& fontName,
        const ::rtype::display::Color& color = ::rtype::display::Color::Red());

    /**
     * @brief Create a power-up collection popup text
     * @param registry The ECS registry
     * @param position Position where the popup appears
     * @param powerUpName Name of the power-up (e.g. "+Speed", "+Shield")
     * @param font The font to use
     * @param color Text color
     */
    static void createPowerUpPopup(
        ECS::Registry& registry,
        const ::rtype::display::Vector2<float>& position,
        const std::string& powerUpName, const std::string& font,
        const ::rtype::display::Color& color);

    /**
     * @brief Create a confetti effect with falling colored particles
     * @param registry The ECS registry
     * @param screenWidth Width of the screen for particle spread
     * @param screenHeight Height of the screen for spawn position
     * @param particleCount Number of confetti particles to spawn
     */
    static void createConfetti(ECS::Registry& registry, float screenWidth,
                               float screenHeight, int particleCount = 50);
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_VISUALCUEFACTORY_HPP_
