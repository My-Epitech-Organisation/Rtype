/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** StressTestScene.hpp - Interactive visual stress test scene
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_STRESSTESTSCENE_STRESSTESTSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_STRESSTESTSCENE_STRESSTESTSCENE_HPP_

#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "../../SceneManager.hpp"
#include "../AScene.hpp"
#include "ecs/ECS.hpp"

/**
 * @brief Interactive stress test scene for visual performance testing.
 *
 * This scene allows users to:
 * - Add/remove entities dynamically
 * - Monitor FPS and frame times
 * - Test different entity counts
 * - Stop the test and return to menu
 */
class StressTestScene : public AScene {
   public:
    /// @brief Stress test phases
    enum class TestPhase {
        IDLE,        ///< Waiting for user to start
        SPAWNING,    ///< Adding entities
        SUSTAINED,   ///< Running at target entity count
        DESTROYING,  ///< Removing entities
        COMPLETED    ///< Test finished
    };

   private:
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    // Test configuration
    static constexpr std::size_t SPAWN_BATCH_SIZE = 50;
    static constexpr std::size_t TARGET_ENTITY_COUNT = 1000;
    static constexpr std::size_t MAX_ENTITY_COUNT = 1000000;
    static constexpr float SPAWN_INTERVAL = 0.05f;  // seconds between spawns

    // Test state
    TestPhase _currentPhase = TestPhase::IDLE;
    std::size_t _currentEntityCount = 0;
    std::size_t _targetEntityCount = TARGET_ENTITY_COUNT;
    bool _autoMode = false;
    float _spawnTimer = 0.0f;

    // Performance metrics
    sf::Clock _frameClock;
    sf::Clock _testClock;
    std::vector<float> _frameTimes;
    float _currentFps = 0.0f;
    float _avgFrameTime = 0.0f;
    float _minFrameTime = 999.0f;
    float _maxFrameTime = 0.0f;
    std::size_t _frameCount = 0;

    // Spawned test entities (separate from UI entities in _listEntity)
    std::vector<ECS::Entity> _testEntities;

    // UI entities
    ECS::Entity _fpsText;
    ECS::Entity _entityCountText;
    ECS::Entity _phaseText;
    ECS::Entity _instructionsText;
    ECS::Entity _statsText;

    // Random number generation
    std::mt19937 _rng;

    // Private methods
    void _createUI();
    void _createButtons();
    void _updateMetrics(float deltaTime);
    void _updateUI();
    void _spawnTestEntities(std::size_t count);
    void _destroyTestEntities(std::size_t count);
    void _destroyAllTestEntities();
    void _runAutoMode(float deltaTime);

    std::string _getPhaseString() const;

   public:
    StressTestScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::function<void(const SceneManager::Scene&)> switchToScene);

    ~StressTestScene() override;

    void pollEvents(const sf::Event& e) override;
    void update() override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_STRESSTESTSCENE_STRESSTESTSCENE_HPP_
