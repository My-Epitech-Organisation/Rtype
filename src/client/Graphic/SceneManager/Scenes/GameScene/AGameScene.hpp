/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AGameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_AGAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_AGAMESCENE_HPP_

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "IGameScene.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"

// Forward declarations
namespace rtype::client {
class NetworkClient;
class ClientNetworkSystem;
}  // namespace rtype::client

/**
 * @brief Abstract base class for game-specific scene logic
 *
 * This class provides common functionality for game scenes
 * while allowing derived classes to implement game-specific behavior.
 */
class AGameScene : public IGameScene {
   protected:
    std::shared_ptr<ECS::Registry> _registry;
    std::shared_ptr<AssetManager> _assetsManager;
    std::shared_ptr<sf::RenderWindow> _window;
    std::shared_ptr<KeyboardActions> _keybinds;
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    /// @brief Network client for server communication
    std::shared_ptr<rtype::client::NetworkClient> _networkClient;

    /// @brief Network system for ECS synchronization
    std::shared_ptr<rtype::client::ClientNetworkSystem> _networkSystem;

    /// @brief Last input mask sent to server (to avoid flooding)
    std::uint8_t _lastInputMask = 0;

   public:
    AGameScene(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::shared_ptr<rtype::client::NetworkClient> networkClient,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
        : _registry(std::move(registry)),
          _assetsManager(std::move(assetsManager)),
          _window(std::move(window)),
          _keybinds(std::move(keybinds)),
          _switchToScene(std::move(switchToScene)),
          _networkClient(std::move(networkClient)),
          _networkSystem(std::move(networkSystem)) {}

    ~AGameScene() override = default;

    // Default implementations that can be overridden
    void render(std::shared_ptr<sf::RenderWindow> window) override {}
    void pollEvents(const sf::Event& event) override {}

    /**
     * @brief Get the last input mask
     * @return The last input mask sent
     */
    [[nodiscard]] std::uint8_t getLastInputMask() const {
        return _lastInputMask;
    }

    /**
     * @brief Set the last input mask
     * @param mask The input mask to store
     */
    void setLastInputMask(std::uint8_t mask) { _lastInputMask = mask; }

    /**
     * @brief Get the network system
     * @return Shared pointer to the network system
     */
    [[nodiscard]] std::shared_ptr<rtype::client::ClientNetworkSystem>
    getNetworkSystem() const {
        return _networkSystem;
    }
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_AGAMESCENE_HPP_
