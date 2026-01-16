/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerInputHandler - Handles player input processing
*/

#ifndef SRC_SERVER_SERVERAPP_PLAYER_PLAYERINPUTHANDLER_PLAYERINPUTHANDLER_HPP_
#define SRC_SERVER_SERVERAPP_PLAYER_PLAYERINPUTHANDLER_PLAYERINPUTHANDLER_HPP_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "GameStateManager.hpp"
#include "IGameConfig.hpp"

namespace rtype::server {

class ServerNetworkSystem;

/**
 * @brief Callback for player shooting
 * @param networkId Player's network ID
 * @param x X position
 * @param y Y position
 * @return Projectile network ID (0 if failed)
 */
using ShootCallback =
    std::function<std::uint32_t(std::uint32_t networkId, float x, float y)>;

/**
 * @brief Callback for player charged shot
 * @param networkId Player's network ID
 * @param x X position
 * @param y Y position
 * @param chargeLevel Charge level (1-3)
 * @return Projectile network ID (0 if failed)
 */
using ChargedShotCallback = std::function<std::uint32_t(
    std::uint32_t networkId, float x, float y, std::uint8_t chargeLevel)>;

/**
 * @brief Callback for Force Pod launch/recall
 * @param playerNetworkId Player's network ID
 */
using ForcePodLaunchCallback =
    std::function<void(std::uint32_t playerNetworkId)>;

/**
 * @brief Callback for laser beam input
 * @param playerEntity Player entity
 * @param playerNetworkId Player's network ID
 * @param isFiring True if fire button is held, false if released
 */
using LaserInputCallback = std::function<void(
    ECS::Entity playerEntity, std::uint32_t playerNetworkId, bool isFiring)>;

/**
 * @brief Handles player input processing
 *
 * Processes movement and shooting inputs from players,
 * updating velocity and triggering projectile spawning.
 */
class PlayerInputHandler {
   public:
    /**
     * @brief Default player speed
     */
    static constexpr float DEFAULT_PLAYER_SPEED = 250.0F;

    /**
     * @brief Construct a PlayerInputHandler
     * @param registry ECS registry
     * @param networkSystem Network system for position updates
     * @param stateManager Game state manager
     * @param gameConfig Game configuration (optional)
     * @param verbose Enable verbose logging
     */
    PlayerInputHandler(std::shared_ptr<ECS::Registry> registry,
                       std::shared_ptr<ServerNetworkSystem> networkSystem,
                       std::shared_ptr<GameStateManager> stateManager,
                       std::shared_ptr<const IGameConfig> gameConfig = nullptr,
                       bool verbose = false);

    ~PlayerInputHandler() = default;

    /**
     * @brief Handle input from a player
     * @param userId The player's user ID
     * @param inputMask Input bitmask
     * @param entity Player entity (if resolved)
     */
    void handleInput(std::uint32_t userId, std::uint16_t inputMask,
                     std::optional<ECS::Entity> entity);

    /**
     * @brief Set callback for shooting
     */
    void setShootCallback(ShootCallback callback) {
        _shootCallback = std::move(callback);
    }

    /**
     * @brief Set callback for charged shot
     */
    void setChargedShotCallback(ChargedShotCallback callback) {
        _chargedShotCallback = std::move(callback);
    }

    /**
     * @brief Set callback for Force Pod launch/recall
     */
    void setForcePodLaunchCallback(ForcePodLaunchCallback callback) {
        _forcePodCallback = std::move(callback);
    }

    /**
     * @brief Set callback for laser beam input
     */
    void setLaserInputCallback(LaserInputCallback callback) {
        _laserCallback = std::move(callback);
    }

    /**
     * @brief Set player speed override
     */
    void setPlayerSpeed(float speed) { _playerSpeed = speed; }

   private:
    /**
     * @brief Process movement input
     */
    void processMovement(ECS::Entity entity, std::uint16_t inputMask);

    /**
     * @brief Process shoot input
     */
    void processShoot(std::uint32_t userId, ECS::Entity entity);

    /**
     * @brief Process charged shot input
     * @param chargeLevel Charge level (1-3)
     */
    void processChargedShot(std::uint32_t userId, ECS::Entity entity,
                            std::uint8_t chargeLevel);

    /**
     * @brief Process Force Pod launch/recall input
     */
    void processForcePodLaunch(std::uint32_t userId);

    /**
     * @brief Process weapon switch input
     */
    void processWeaponSwitch(ECS::Entity entity);

    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<ServerNetworkSystem> _networkSystem;
    std::shared_ptr<GameStateManager> _stateManager;
    std::shared_ptr<const IGameConfig> _gameConfig;
    ShootCallback _shootCallback;
    ChargedShotCallback _chargedShotCallback;
    ForcePodLaunchCallback _forcePodCallback;
    LaserInputCallback _laserCallback;
    float _playerSpeed{DEFAULT_PLAYER_SPEED};
    bool _verbose;
    std::unordered_map<std::uint32_t, bool> _weaponSwitchStates;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_PLAYER_PLAYERINPUTHANDLER_PLAYERINPUTHANDLER_HPP_
