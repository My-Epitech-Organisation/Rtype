/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeInputSystem - Handles charge shot input processing
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEINPUTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEINPUTSYSTEM_HPP_

#include <functional>
#include <memory>
#include <utility>

#include "ASystem.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Callback when charged shot is released
 * @param chargeLevel The charge level achieved
 */
using ChargeReleaseCallback =
    std::function<void(shared::ChargeLevel chargeLevel)>;

/**
 * @class ChargeInputSystem
 * @brief System that handles charge shot input processing
 *
 * Tracks:
 * - Fire button hold duration
 * - Charge level calculation
 * - Release detection and event triggering
 */
class ChargeInputSystem : public ::rtype::engine::ASystem {
   public:
    ChargeInputSystem();

    /**
     * @brief Update charge input state
     * @param registry ECS registry
     * @param dt Delta time
     */
    void update(ECS::Registry& registry, float dt) override;

    /**
     * @brief Set whether shoot button is currently pressed
     * @param pressed True if shoot button is held
     */
    void setShootPressed(bool pressed) noexcept { _shootPressed = pressed; }

    /**
     * @brief Check if shoot button is pressed
     * @return true if shoot button is held
     */
    [[nodiscard]] bool isShootPressed() const noexcept { return _shootPressed; }

    /**
     * @brief Set callback for charge release
     * @param callback Function to call when charge is released
     */
    void setChargeReleaseCallback(ChargeReleaseCallback callback) {
        _releaseCallback = std::move(callback);
    }

    /**
     * @brief Check if a charged shot was just released
     * @return true if charged shot was released this frame
     */
    [[nodiscard]] bool wasChargedShotReleased() const noexcept {
        return _chargedShotReleased;
    }

    /**
     * @brief Get the last released charge level
     * @return Last charge level that was released
     */
    [[nodiscard]] shared::ChargeLevel getLastReleasedLevel() const noexcept {
        return _lastReleasedLevel;
    }

    /**
     * @brief Clear the charged shot released flag
     */
    void clearChargedShotReleased() noexcept { _chargedShotReleased = false; }

   private:
    bool _shootPressed = false;
    bool _wasShootPressed = false;
    bool _chargedShotReleased = false;
    shared::ChargeLevel _lastReleasedLevel = shared::ChargeLevel::None;
    ChargeReleaseCallback _releaseCallback;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CHARGEINPUTSYSTEM_HPP_
