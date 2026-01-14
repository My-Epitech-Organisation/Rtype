/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeGameScene.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEGAMESCENE_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEGAMESCENE_HPP_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "../Systems/LaserBeamAnimationSystem.hpp"
#include "../lib/display/Clock/Clock.hpp"
#include "AudioLib/AudioLib.hpp"
#include "Graphic/SceneManager/Scenes/GameScene/AGameScene.hpp"
#include "games/rtype/shared/Systems/Movements/MovementSystem.hpp"
#include "rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief R-Type specific game scene implementation
 *
 * This class contains all R-Type specific game logic including:
 * - Entity creation (players, enemies, missiles)
 * - Input handling for the R-Type game
 * - Pause menu creation
 *
 * Logic is delegated to specialized classes:
 * - RtypeEntityFactory: Entity creation
 * - RtypePauseMenu: Pause menu management
 * - RtypeInputHandler: Input processing
 */
class RtypeGameScene : public AGameScene {
   public:
    RtypeGameScene(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<::rtype::display::IDisplay> display,
        std::shared_ptr<KeyboardActions> keybinds,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::function<void(const std::string&)> setBackground,
        std::function<void(const std::string&)> setLevelMusic,
        std::shared_ptr<::rtype::client::NetworkClient> networkClient,
        std::shared_ptr<::rtype::client::ClientNetworkSystem> networkSystem,
        std::shared_ptr<AudioLib> audioLib = nullptr);

    ~RtypeGameScene() override;

    /**
     * @brief Initialize R-Type specific game entities
     * @return Vector of created entities
     */
    std::vector<ECS::Entity> initialize() override;

    void _handleShoot();

    void _updateUserShoot(float deltaTime);

    /**
     * @brief Update R-Type game logic
     */
    void update() override;

    /**
     * @brief Render R-Type specific elements
     * @param display The display interface
     */
    void render(::rtype::display::IDisplay& display) override;

    /**
     * @brief Handle R-Type specific events
     * @param event The display event
     */
    void pollEvents(const ::rtype::display::Event& event) override;

    /**
     * @brief Get current input mask for R-Type controls
     * @return Input mask
     */
    [[nodiscard]] std::uint16_t getInputMask() const override;

    /**
     * @brief Set up the entity factory for R-Type entities
     */
    void setupEntityFactory() override;

    /**
     * @brief Set up local player callback for R-Type
     */
    void setupLocalPlayerCallback() override;

   private:
    /**
     * @brief Initialize the HUD with lives display text
     */
    void setupHud();

    /**
     * @brief Setup the level announcement callback
     */
    void setupLevelAnnounceCallback();

    /**
     * @brief Show the level announcement
     * @param levelName The name of the level
     */
    void showLevelAnnounce(const std::string& levelName);

    /**
     * @brief Update the level announcement visual
     * @param dt Delta time
     */
    void updateLevelAnnounce(float dt);

    /**
     * @brief Update the lives display text with current health values
     * @param current Current lives/health
     * @param max Maximum lives/health
     */
    void updateLivesDisplay(int current, int max);

    /**
     * @brief Handle entity health update events from the network
     * @param event Health update event containing entity ID and health values
     * @note Only updates display for the local player
     */
    void handleHealthUpdate(const ::rtype::client::EntityHealthEvent& event);

    std::optional<ECS::Entity> _livesTextEntity;
    std::optional<ECS::Entity> _healthBarBgEntity;
    std::optional<ECS::Entity> _healthBarFillEntity;
    std::optional<ECS::Entity> _healthTextEntity;
    std::optional<ECS::Entity> _pingTextEntity;
    std::optional<std::uint32_t> _localPlayerId;
    std::optional<ECS::Entity> _localPlayerEntity;
    int _lastKnownLives{0};
    int _lastKnownMaxLives{0};
    float _damageFlashTimer{0.0F};
    float _uiTimer{0.0F};

    static constexpr int kVignetteLayers = 6;
    std::vector<ECS::Entity> _vignetteEntities;
    float _vignetteAlpha{0.0F};
    static constexpr float kVignetteFadeSpeed = 300.0F;
    static constexpr float kVignetteMaxAlpha = 180.0F;
    ::rtype::display::Vector2i _lastVignetteSize{0, 0};

    static constexpr float kShootSendInterval = 0.05F;

    ::rtype::display::Clock _shootInputClock;

    void setupDamageVignette();
    void refreshDamageVignetteLayout();
    void clearDamageVignette();
    void setHealthBarVisible(bool visible);
    void updateDamageVignette(float deltaTime);
    void updateHealthBar(int current, int max);
    void updatePingDisplay();
    void triggerDamageFlash(int damageAmount);
    void resetHudColors();
    void spawnDamagePopup(int damage);

    void setupDisconnectCallback();
    void showDisconnectModal(network::DisconnectReason reason);
    void cleanupDisconnectModal();
    std::string getDisconnectMessage(network::DisconnectReason reason) const;
    std::optional<ECS::Entity> _disconnectOverlayEntity;
    std::optional<ECS::Entity> _disconnectPanelEntity;
    std::optional<ECS::Entity> _disconnectTitleEntity;
    std::optional<ECS::Entity> _disconnectMessageEntity;
    std::optional<ECS::Entity> _disconnectButtonEntity;
    bool _isDisconnected{false};

    // Level Announcement
    std::optional<ECS::Entity> _levelAnnounceTextEntity;
    std::optional<ECS::Entity> _levelAnnounceBgEntity;
    float _levelAnnounceTimer = 0.0f;
    bool _isFirstLevelAnnounce = true;

    std::unique_ptr<::rtype::games::rtype::shared::MovementSystem>
        _movementSystem;
    std::unique_ptr<LaserBeamAnimationSystem> _laserBeamAnimationSystem;

    bool _lowBandwidthMode{false};
    std::uint8_t _lowBandwidthActiveCount{0};
    std::optional<ECS::Entity> _bandwidthIndicatorEntity;
    std::optional<ECS::Entity> _bandwidthNotificationEntity;
    float _bandwidthNotificationTimer{0.0F};
    void toggleLowBandwidthMode();
    void updateBandwidthIndicator();
    void showBandwidthNotification(std::uint32_t userId, bool enabled,
                                   std::uint8_t activeCount);
    void setupBandwidthModeCallback();
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEGAMESCENE_HPP_
