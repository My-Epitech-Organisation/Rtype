/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_

#include <cstdint>

#include "../../../../network/ClientNetworkSystem.hpp"
#include "../../../../network/NetworkClient.hpp"
#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

static constexpr int SizeXPauseMenu = 600;
static constexpr int SizeYPauseMenu = 600;
static constexpr int SizeFontPauseMenu = 40;
static constexpr std::string PauseMenuTitle = "Pause";

class GameScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;

    /// @brief Network client for server communication
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system for ECS synchronization
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    /// @brief Last input mask sent to server (to avoid flooding)
    std::uint8_t _lastInputMask = 0;

    void _handleKeyReleasedEvent(const sf::Event& event);

   public:
    void update() override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
    void pollEvents(const sf::Event& e) override;

    GameScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> textureManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
            nullptr);
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
