/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
#define SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_

#include <memory>

#include <SFML/Graphics.hpp>
#include <rtype/ecs.hpp>

#include "../../games/rtype/client/GraphicsConstants.hpp"
#include "../../games/rtype/client/Systems/BoxingSystem.hpp"
#include "../../games/rtype/client/Systems/ButtonUpdateSystem.hpp"
#include "../../games/rtype/client/Systems/EventSystem.hpp"
#include "../../games/rtype/client/Systems/MovementSystem.hpp"
#include "../../games/rtype/client/Systems/ParallaxScrolling.hpp"
#include "../../games/rtype/client/Systems/RenderSystem.hpp"
#include "../../games/rtype/client/Systems/ResetTriggersSystem.hpp"
#include "../../games/rtype/shared/Systems/Lifetime/LifetimeSystem.hpp"
#include "../../games/rtype/shared/Systems/Projectile/ProjectileSystem.hpp"
#include "AssetManager/AssetManager.hpp"
#include "AudioLib/AudioLib.hpp"
#include "KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"

/**
 * @brief Main graphics class managing the game window and rendering pipeline.
 *
 * Uses an ECS SystemScheduler for proper system execution ordering with
 * dependency management. Systems are stored as unique_ptr members and
 * accessed via the scheduler.
 *
 * System execution order:
 * 1. ResetTriggers - Resets input states
 * 2. ButtonUpdate - Updates button states (depends on ResetTriggers)
 * 3. Parallax - Updates parallax backgrounds (depends on ButtonUpdate)
 * 4. Movement - Updates entity positions (depends on Parallax)
 * 5. Render - Draws all entities (depends on Movement)
 * 6. Boxing - Draws debug boxes (depends on Render)
 */
class Graphic {
   public:
    /// @brief Window width from centralized config
    static constexpr unsigned int WINDOW_WIDTH =
        ::rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH;
    /// @brief Window height from centralized config
    static constexpr unsigned int WINDOW_HEIGHT =
        ::rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT;

   private:
    /// @brief Background scroll speed from centralized config
    static constexpr float scrollSpeed =
        ::rtype::games::rtype::client::GraphicsConfig::SCROLL_SPEED;

    static constexpr float ProjectileSpeed =
        ::rtype::games::rtype::client::GraphicsConfig::PROJECTILE_SPEED_LASER;

    // ========================================================================
    // Shared resources (owned by Graphic, shared with subsystems)
    // ========================================================================

    /// @brief ECS registry shared with SceneManager and all systems
    std::shared_ptr<ECS::Registry> _registry;

    /// @brief Asset manager shared with SceneManager for texture/font loading
    std::shared_ptr<AssetManager> _assetsManager;

    /// @brief Keyboard action mappings shared with SceneManager
    std::shared_ptr<KeyboardActions> _keybinds;

    /// @brief SFML window shared with rendering systems and SceneManager
    std::shared_ptr<sf::RenderWindow> _window;

    /// @brief Audio lib shared with SceneManager
    std::shared_ptr<AudioLib> _audioLib;

    /// @brief Camera view shared with ParallaxScrolling system
    std::shared_ptr<sf::View> _view;

    // ========================================================================
    // Owned resources (unique ownership)
    // ========================================================================

    /// @brief Scene state machine (owns scenes, uses shared resources)
    std::unique_ptr<SceneManager> _sceneManager;

    /// @brief System scheduler for ordered system execution
    std::unique_ptr<ECS::SystemScheduler> _systemScheduler;

    // ========================================================================
    // ECS Systems (unique ownership, registered with scheduler)
    // ========================================================================

    std::unique_ptr<::rtype::games::rtype::client::MovementSystem>
        _movementSystem;
    std::unique_ptr<::rtype::games::rtype::client::ButtonUpdateSystem>
        _buttonUpdateSystem;
    std::unique_ptr<::rtype::games::rtype::client::ParallaxScrolling>
        _parallaxScrolling;
    std::unique_ptr<::rtype::games::rtype::client::RenderSystem> _renderSystem;
    std::unique_ptr<::rtype::games::rtype::client::BoxingSystem> _boxingSystem;
    std::unique_ptr<::rtype::games::rtype::client::ResetTriggersSystem>
        _resetTriggersSystem;
    std::unique_ptr<::rtype::games::rtype::client::EventSystem> _eventSystem;
    std::unique_ptr<::rtype::games::rtype::shared::ProjectileSystem>
        _projectileSystem;
    std::unique_ptr<::rtype::games::rtype::shared::LifetimeSystem>
        _lifetimeSystem;

    // ========================================================================
    // Runtime state
    // ========================================================================

    /// @brief Main clock for delta time calculation
    sf::Clock _mainClock;

    /// @brief Current frame delta time (seconds)
    float _currentDeltaTime = 0.0f;

    // ========================================================================
    // Private methods (game loop phases)
    // ========================================================================

    /// @brief Process window events (input, close, etc.)
    void _pollEvents();

    /// @brief Update delta time from clock
    void _updateDeltaTime();

    /// @brief Update camera/view scrolling based on delta time
    void _updateViewScrolling();

    /// @brief Run update phase systems and scene logic
    void _update();

    /// @brief Clear window, run render systems, display
    void _display();

    /// @brief Initialize and register all systems with the scheduler
    void _initializeSystems();

   public:
    void loop();

    explicit Graphic(std::shared_ptr<ECS::Registry> registry);
    ~Graphic() = default;

    Graphic(const Graphic&) = delete;
    Graphic& operator=(const Graphic&) = delete;
    Graphic(Graphic&&) = delete;
    Graphic& operator=(Graphic&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_GRAPHIC_HPP_
