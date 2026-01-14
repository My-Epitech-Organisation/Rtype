/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** IGameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_IGAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_IGAMESCENE_HPP_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "ECS.hpp"
#include "Graphic/AssetManager/AssetManager.hpp"
#include "rtype/display/IDisplay.hpp"

/**
 * @brief Interface for game-specific scene logic
 *
 * This interface allows different games to implement their own
 * game scene logic while keeping the main GameScene generic.
 */
class IGameScene {
   public:
    virtual ~IGameScene() = default;

    /**
     * @brief Initialize the game scene
     *
     * Called once when the game scene is created.
     * Should set up entities, components, and any game-specific state.
     *
     * @return Vector of entities created during initialization
     */
    virtual std::vector<ECS::Entity> initialize() = 0;

    /**
     * @brief Update the game scene
     *
     * Called every frame to update game-specific logic.
     */
    virtual void update() = 0;

    /**
     * @brief Render game-specific elements
     *
     * @param display The display interface
     */
    virtual void render(rtype::display::IDisplay& display) = 0;

    /**
     * @brief Handle game-specific events
     *
     * @param event The display event
     */
    virtual void pollEvents(const rtype::display::Event& event) = 0;

    /**
     * @brief Get current input mask based on game controls
     *
     * @return Input mask representing current inputs
     */
    virtual std::uint16_t getInputMask() const = 0;

    /**
     * @brief Set up the entity factory for network entity creation
     *
     * Called to configure how entities should be created when
     * received from the network.
     */
    virtual void setupEntityFactory() = 0;

    /**
     * @brief Set up the local player callback
     *
     * Called to configure what happens when a local player is assigned.
     */
    virtual void setupLocalPlayerCallback() = 0;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_IGAMESCENE_HPP_
