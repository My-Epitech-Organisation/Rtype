/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_

#include <cstdint>
#include <memory>

#include "../../../../network/ClientNetworkSystem.hpp"
#include "../../../../network/NetworkClient.hpp"
#include "../AScene.hpp"
#include "IGameScene.hpp"
#include "AudioLib/AudioLib.hpp"
#include "SceneManager/SceneManager.hpp"

/**
 * @brief Generic GameScene that delegates game-specific logic to IGameScene
 *
 * This class provides the shell for a game scene, delegating all game-specific
 * logic (entity creation, input handling, etc.) to an IGameScene
 * implementation. This allows different games to be plugged in without
 * modifying this class.
 */
class GameScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;

    /// @brief Network client for server communication
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system for ECS synchronization
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    /// @brief Game-specific scene implementation
    std::unique_ptr<IGameScene> _gameScene;

    /// @brief Last input mask sent to server (to avoid flooding)
    std::uint8_t _lastInputMask = 0;

   public:
    void update() override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
    void pollEvents(const sf::Event& e) override;

    /**
     * @brief Construct a new GameScene
     *
     * @param ecs The ECS registry
     * @param textureManager Asset manager
     * @param window Render window
     * @param keybinds Keyboard bindings
     * @param switchToScene Scene switch callback
     * @param gameScene Game-specific scene implementation
     * @param networkClient Network client (optional)
     * @param networkSystem Network system (optional)
     */
    GameScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> textureManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::unique_ptr<IGameScene> gameScene,
        std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
            nullptr,
        std::shared_ptr<AudioLib> audio = nullptr);

   private:
        std::shared_ptr<AudioLib> _audio;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
