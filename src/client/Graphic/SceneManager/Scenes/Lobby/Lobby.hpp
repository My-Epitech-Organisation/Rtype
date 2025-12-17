/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_

#include <unordered_map>

#include "../AScene.hpp"
#include "SceneManager.hpp"

class Lobby : public AScene {
   private:
    bool _isConnected = false;
    unsigned int _nbrUser = 0;
    std::unordered_map<uint32_t, std::vector<ECS::Entity>> _listUser;
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    void _initInfoMenu();

    void _createPlayerInfoMenu(uint32_t id, int index = 0);

    void _removePlayerInfoMenu(uint32_t userId);

   public:
    /**
     * @brief Construct a new Lobby scene
     * @param networkClient Shared pointer to the network client
     * @param networkSystem Shared pointer to the client network system
     * @param switchToScene Scene switch callback
     */
    Lobby(std::shared_ptr<ECS::Registry> ecs,
          std::shared_ptr<AssetManager> textureManager,
          std::shared_ptr<sf::RenderWindow> window,
          std::function<void(const SceneManager::Scene&)> switchToScene,
          std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
          std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
              nullptr,
          std::shared_ptr<AudioLib> audioLib = nullptr);

    void update(float dt) override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
    void pollEvents(const sf::Event& e) override;

    /**
     * @brief Destroy the Lobby scene
     */
    ~Lobby() override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_
