/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeVisualSystem - Handles charge shot visual effects
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEVISUALSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEVISUALSYSTEM_HPP_

#include <memory>

#include "ASystem.hpp"
#include "AudioLib/AudioLib.hpp"
#include "rtype/display/IDisplay.hpp"

namespace rtype::games::rtype::client {

/**
 * @class ChargeVisualSystem
 * @brief System that handles visual feedback for charge shot mechanic
 *
 * Manages:
 * - Progressive ship glow (3 stages: dim → bright → max)
 * - Screen shake effect on max charge release
 * - Charge bar UI updates
 */
class ChargeVisualSystem : public ::rtype::engine::ASystem {
   public:
    /**
     * @brief Construct ChargeVisualSystem
     * @param display Display interface for screen effects
     * @param audioLib Audio library for charging sounds
     */
    ChargeVisualSystem(std::shared_ptr<::rtype::display::IDisplay> display,
                       std::shared_ptr<AudioLib> audioLib);

    /**
     * @brief Update charge visual effects
     * @param registry ECS registry
     * @param dt Delta time
     */
    void update(ECS::Registry& registry, float dt) override;

   private:
    /**
     * @brief Apply screen shake effect
     * @param intensity Shake intensity
     */
    void applyScreenShake(float intensity);

    /**
     * @brief Reset screen shake
     */
    void resetScreenShake();

    std::shared_ptr<::rtype::display::IDisplay> _display;
    std::shared_ptr<AudioLib> _audioLib;
    ::rtype::display::Vector2f _originalViewCenter;
    ::rtype::display::Vector2f _originalViewSize;
    bool _isShaking = false;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEVISUALSYSTEM_HPP_
