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
#include <utility>
#include <vector>

#include "AudioLib/AudioLib.hpp"
#include "Graphic/SceneManager/Scenes/GameScene/AGameScene.hpp"

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
        std::shared_ptr<sf::RenderWindow> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::shared_ptr<::rtype::client::NetworkClient> networkClient,
        std::shared_ptr<::rtype::client::ClientNetworkSystem> networkSystem,
        std::shared_ptr<AudioLib> audioLib = nullptr);

    ~RtypeGameScene() override = default;

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
     * @param window The render window
     */
    void render(std::shared_ptr<sf::RenderWindow> window) override;

    /**
     * @brief Handle R-Type specific events
     * @param event The SFML event
     */
    void pollEvents(const sf::Event& event) override;

    /**
     * @brief Get current input mask for R-Type controls
     * @return Input mask
     */
    [[nodiscard]] std::uint8_t getInputMask() const override;

    /**
     * @brief Set up the entity factory for R-Type entities
     */
    void setupEntityFactory() override;

    /**
     * @brief Set up local player callback for R-Type
     */
    void setupLocalPlayerCallback() override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GAMESCENE_RTYPEGAMESCENE_HPP_
