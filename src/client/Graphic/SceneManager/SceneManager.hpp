/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_

#include <iostream>
#include <map>
#include <memory>
#include <optional>

#include "../../network/ClientNetworkSystem.hpp"
#include "../../network/NetworkClient.hpp"
#include "../KeyboardActions.hpp"
#include "AssetManager/AssetManager.hpp"
#include "AudioLib/AudioLib.hpp"
#include "ECS.hpp"
#include "Scenes/IScene.hpp"

class SceneManager {
   public:
    enum Scene {
        MAIN_MENU,
        IN_GAME,
        SETTINGS_MENU,
        HOW_TO_PLAY,
        LOBBY,
        NONE,
    };

   private:
    std::optional<Scene> _nextScene = std::nullopt;  // Scène en attente
    Scene _currentScene = NONE;

    std::map<Scene, std::function<std::unique_ptr<IScene>()>> _sceneList;
    std::unique_ptr<IScene> _activeScene;
    std::shared_ptr<sf::RenderWindow> _window;
    std::shared_ptr<AudioLib> _audio;

    std::function<void(const Scene&)> _switchToScene;

    std::shared_ptr<KeyboardActions> _keybinds;

    /// @brief Network client for server communication
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system for ECS synchronization
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    void _applySceneChange();

   public:
    [[nodiscard]] Scene getCurrentScene() const { return _currentScene; }
    void setCurrentScene(Scene scene);

    void pollEvents(const sf::Event& e);
    void update(float dt);
    void draw();

    bool operator==(const Scene& data) const {
        if (data == this->_currentScene) return true;
        return false;
    }

    /**
     * @brief Get the network client
     * @return Shared pointer to network client
     */
    [[nodiscard]] std::shared_ptr<rtype::client::NetworkClient>
    getNetworkClient() const {
        return _networkClient;
    }

    /**
     * @brief Get the network system
     * @return Shared pointer to network system
     */
    [[nodiscard]] std::shared_ptr<rtype::client::ClientNetworkSystem>
    getNetworkSystem() const {
        return _networkSystem;
    }

    SceneManager(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> assetManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
            nullptr,
        std::shared_ptr<AudioLib> audioLib = nullptr);
    ~SceneManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_
