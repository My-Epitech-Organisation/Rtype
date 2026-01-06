/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "AudioLib/AudioLib.hpp"

static constexpr int nbr_vessels = 7;
#include "../../../../network/ClientNetworkSystem.hpp"
#include "../../../../network/NetworkClient.hpp"
#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"
#include "Systems/TextInputSystem.hpp"

class MainMenuScene : public AScene {
   private:
    bool _connectPopUpVisible = false;
    /// @brief Network client for server communication
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system for ECS synchronization
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    /// @brief Text input system for handling input fields
    std::shared_ptr<rtype::games::rtype::client::TextInputSystem>
        _textInputSystem;

    /// @brief IP input field entity
    ECS::Entity _ipInputEntity;

    /// @brief Port input field entity
    ECS::Entity _portInputEntity;

    /// @brief Status text entity
    ECS::Entity _statusEntity;

    void _createAstroneerVessel();
    void _createFakePlayer();

    /**
     * @brief Create the connection panel UI
     * @param switchToScene Scene switch callback
     */
    void _createConnectionPanel(
        std::function<void(const SceneManager::Scene&)> switchToScene);

    /**
     * @brief Handle connect button click
     * @param switchToScene Scene switch callback
     */
    void _onConnectClicked(
        std::function<void(const SceneManager::Scene&)> switchToScene);

    /**
     * @brief Update the status text
     * @param message Status message
     * @param color Text color
     */
    void _updateStatus(const std::string& message,
                       ::rtype::display::Color color);

   public:
    void update(float dt) override;
    void render(std::shared_ptr<::rtype::display::IDisplay> window) override;
    void pollEvents(const ::rtype::display::Event& e) override;

    MainMenuScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> textureManager,
        std::shared_ptr<::rtype::display::IDisplay> window,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
            nullptr,
        std::shared_ptr<AudioLib> audioLib = nullptr);

    /**
     * @brief Destructor - clears network callbacks to prevent use-after-free
     */
    ~MainMenuScene();

   private:
    std::shared_ptr<AudioLib> _audioLib;
    std::vector<rtype::client::NetworkClient::CallbackId> _connectedCallbackIds;
    std::vector<rtype::client::NetworkClient::CallbackId>
        _disconnectedCallbackIds;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_
