/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../AScene.hpp"
#include "SceneManager.hpp"

class Lobby : public AScene {
   private:
    bool _isConnected = false;
    unsigned int _nbrUser = 0;
    bool _isReady = false;
    bool _countdownActive = false;
    float _countdownTimer = 3.0f;
    ECS::Entity _readyButtonEntity;
    ECS::Entity _countdownTextEntity;
    std::unordered_map<uint32_t, std::vector<ECS::Entity>> _listUser;
    std::unordered_set<uint32_t> _playersToPosition;
    std::unordered_map<uint32_t, int> _playerIndexMap;
    std::unordered_map<uint32_t, ECS::Entity> _playerReadyIndicators;
    std::unordered_set<uint32_t> _playerReadyStates;
    std::unordered_set<uint32_t> _disconnectedPlayers;
    std::vector<uint32_t> _pendingPlayerRemovals;
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    void _initInfoMenu();

    void _createPlayerInfoMenu(uint32_t id, int index = 0);

    void _removePlayerInfoMenu(uint32_t userId);

    void _updatePlayerReadyIndicator(uint32_t userId, bool isReady);

   public:
    // Test helpers
    void onEntityDestroyEvent(std::uint32_t entityId);
    const std::vector<uint32_t>& getPendingPlayerRemovals() const noexcept {
        return _pendingPlayerRemovals;
    }
    void addUserForTest(uint32_t userId,
                        const std::vector<ECS::Entity>& entities) {
        _listUser[userId] = entities;
    }

    // Test helpers
    bool isCountdownActive() const noexcept { return _countdownActive; }
    float getCountdownTimer() const noexcept { return _countdownTimer; }
    /**
     * @brief Construct a new Lobby scene
     * @param networkClient Shared pointer to the network client
     * @param networkSystem Shared pointer to the client network system
     * @param switchToScene Scene switch callback
     */
    Lobby(std::shared_ptr<ECS::Registry> ecs,
          std::shared_ptr<AssetManager> textureManager,
          std::shared_ptr<::rtype::display::IDisplay> window,
          std::function<void(const SceneManager::Scene&)> switchToScene,
          std::shared_ptr<rtype::client::NetworkClient> networkClient = nullptr,
          std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem =
              nullptr,
          std::shared_ptr<AudioLib> audioLib = nullptr);

    void update(float dt) override;
    void render(std::shared_ptr<rtype::display::IDisplay> window) override;
    void pollEvents(const rtype::display::Event& e) override;

    /**
     * @brief Destroy the Lobby scene
     */
    ~Lobby() override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_LOBBY_LOBBY_HPP_
