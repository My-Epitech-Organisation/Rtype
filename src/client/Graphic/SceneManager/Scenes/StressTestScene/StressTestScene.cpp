/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** StressTestScene.cpp - Interactive visual stress test implementation
*/

#include "StressTestScene.hpp"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "AllComponents.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"

// Namespace aliases
namespace rc = rtype::games::rtype::client;
namespace rs = rtype::games::rtype::shared;

StressTestScene::StressTestScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, assetsManager, window), _switchToScene(switchToScene) {
    // Reserve space for frame times
    _frameTimes.reserve(1000);

    // Create background
    auto bgEntities = EntityFactory::createBackground(_registry, _assetsManager,
                                                      "STRESS TEST");
    for (auto& e : bgEntities) {
        _listEntity.push_back(e);
    }

    _createUI();
    _createButtons();

    _frameClock.restart();
    _testClock.restart();
}

StressTestScene::~StressTestScene() { _destroyAllTestEntities(); }

void StressTestScene::_createUI() {
    auto& font = _assetsManager->fontManager->get("title_font");

    // FPS counter (top right)
    _fpsText = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(_fpsText, 1500.0f, 50.0f);
    _registry->emplaceComponent<rc::Text>(_fpsText, font, sf::Color::Green, 32,
                                          "FPS: 0");
    _registry->emplaceComponent<rc::StaticTextTag>(_fpsText);
    _listEntity.push_back(_fpsText);

    // Entity count (top right, below FPS)
    _entityCountText = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(_entityCountText, 1500.0f,
                                              100.0f);
    _registry->emplaceComponent<rc::Text>(_entityCountText, font,
                                          sf::Color::Yellow, 28, "Entities: 0");
    _registry->emplaceComponent<rc::StaticTextTag>(_entityCountText);
    _listEntity.push_back(_entityCountText);

    // Phase indicator (top center)
    _phaseText = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(_phaseText, 800.0f, 50.0f);
    _registry->emplaceComponent<rc::Text>(_phaseText, font, sf::Color::Cyan, 36,
                                          "Phase: IDLE");
    _registry->emplaceComponent<rc::StaticTextTag>(_phaseText);
    _listEntity.push_back(_phaseText);

    // Instructions (bottom left)
    _instructionsText = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(_instructionsText, 50.0f, 900.0f);
    _registry->emplaceComponent<rc::Text>(
        _instructionsText, font, sf::Color::White, 24,
        "[SPACE] Add 50 | [A] Auto Mode | [C] Clear | [ESC] Back to Menu");
    _registry->emplaceComponent<rc::StaticTextTag>(_instructionsText);
    _listEntity.push_back(_instructionsText);

    // Stats panel (left side)
    _statsText = _registry->spawnEntity();
    _registry->emplaceComponent<rs::Position>(_statsText, 50.0f, 150.0f);
    _registry->emplaceComponent<rc::Text>(
        _statsText, font, sf::Color::White, 22,
        "Avg: 0.00ms\nMin: 0.00ms\nMax: 0.00ms\nFrames: 0");
    _registry->emplaceComponent<rc::StaticTextTag>(_statsText);
    _listEntity.push_back(_statsText);
}

void StressTestScene::_createButtons() {
    auto& font = _assetsManager->fontManager->get("title_font");

    // Add 100 entities button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "+100 Entities"),
        rs::Position(50, 350),
        rc::Rectangle({250, 60}, sf::Color(0, 100, 0), sf::Color(0, 150, 0)),
        std::function<void()>([this]() { _spawnTestEntities(100); })));

    // Add 500 entities button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "+500 Entities"),
        rs::Position(50, 430),
        rc::Rectangle({250, 60}, sf::Color(0, 100, 0), sf::Color(0, 150, 0)),
        std::function<void()>([this]() { _spawnTestEntities(500); })));

    // Remove 100 entities button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "-100 Entities"),
        rs::Position(50, 510),
        rc::Rectangle({250, 60}, sf::Color(150, 50, 0), sf::Color(200, 75, 0)),
        std::function<void()>([this]() { _destroyTestEntities(100); })));

    // Clear all button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "Clear All"),
        rs::Position(50, 590),
        rc::Rectangle({250, 60}, sf::Color(150, 0, 0), sf::Color(200, 0, 0)),
        std::function<void()>([this]() { _destroyAllTestEntities(); })));

    // Toggle auto mode button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "Auto Mode"),
        rs::Position(50, 670),
        rc::Rectangle({250, 60}, sf::Color(100, 0, 100),
                      sf::Color(150, 0, 150)),
        std::function<void()>([this]() {
            _autoMode = !_autoMode;
            _currentPhase = _autoMode ? TestPhase::SPAWNING : TestPhase::IDLE;
        })));

    // Back to menu button
    _listEntity.push_back(EntityFactory::createButton(
        _registry, rc::Text(font, sf::Color::White, 28, "Back to Menu"),
        rs::Position(50, 750),
        rc::Rectangle({250, 60}, sf::Color::Blue, sf::Color(50, 50, 200)),
        std::function<void()>([this]() {
            try {
                _switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Main Menu: " << e.what()
                          << std::endl;
            }
        })));
}

void StressTestScene::_spawnTestEntities(std::size_t count) {
    if (_currentEntityCount >= MAX_ENTITY_COUNT) {
        return;
    }

    std::uniform_real_distribution<float> posX(400.0f, 1800.0f);
    std::uniform_real_distribution<float> posY(100.0f, 1000.0f);
    std::uniform_real_distribution<float> velX(-100.0f, 100.0f);
    std::uniform_real_distribution<float> velY(-100.0f, 100.0f);
    std::uniform_int_distribution<int> zIndex(-3, 3);

    auto& texture = _assetsManager->textureManager->get("player_vessel");

    std::size_t toSpawn =
        std::min(count, MAX_ENTITY_COUNT - _currentEntityCount);

    for (std::size_t i = 0; i < toSpawn; ++i) {
        auto entity = _registry->spawnEntity();

        _registry->emplaceComponent<rc::Image>(entity, texture);
        _registry->emplaceComponent<rc::TextureRect>(
            entity, std::pair<int, int>({0, 0}), std::pair<int, int>({33, 17}));
        _registry->emplaceComponent<rs::Position>(entity, posX(_rng),
                                                  posY(_rng));
        _registry->emplaceComponent<rc::Size>(entity, 2.0f, 2.0f);
        _registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{velX(_rng), velY(_rng)});
        _registry->emplaceComponent<rc::ZIndex>(entity, zIndex(_rng));

        _testEntities.push_back(entity);
    }

    _currentEntityCount = _testEntities.size();
}

void StressTestScene::_destroyTestEntities(std::size_t count) {
    std::size_t toDestroy = std::min(count, _testEntities.size());

    for (std::size_t i = 0; i < toDestroy; ++i) {
        if (!_testEntities.empty()) {
            _registry->killEntity(_testEntities.back());
            _testEntities.pop_back();
        }
    }

    _currentEntityCount = _testEntities.size();
}

void StressTestScene::_destroyAllTestEntities() {
    for (auto entity : _testEntities) {
        _registry->killEntity(entity);
    }
    _testEntities.clear();
    _currentEntityCount = 0;
    _currentPhase = TestPhase::IDLE;
    _autoMode = false;

    // Reset metrics
    _frameTimes.clear();
    _minFrameTime = 999.0f;
    _maxFrameTime = 0.0f;
    _avgFrameTime = 0.0f;
    _frameCount = 0;
}

void StressTestScene::_runAutoMode(float deltaTime) {
    if (!_autoMode) return;

    _spawnTimer += deltaTime;

    switch (_currentPhase) {
        case TestPhase::SPAWNING:
            if (_spawnTimer >= SPAWN_INTERVAL) {
                _spawnTimer = 0.0f;
                _spawnTestEntities(SPAWN_BATCH_SIZE);

                if (_currentEntityCount >= _targetEntityCount) {
                    _currentPhase = TestPhase::SUSTAINED;
                    _testClock.restart();
                }
            }
            break;

        case TestPhase::SUSTAINED:
            // Run for 5 seconds at target count
            if (_testClock.getElapsedTime().asSeconds() >= 5.0f) {
                _currentPhase = TestPhase::DESTROYING;
            }
            break;

        case TestPhase::DESTROYING:
            if (_spawnTimer >= SPAWN_INTERVAL) {
                _spawnTimer = 0.0f;
                _destroyTestEntities(SPAWN_BATCH_SIZE);

                if (_currentEntityCount == 0) {
                    _currentPhase = TestPhase::COMPLETED;
                    _autoMode = false;
                }
            }
            break;

        case TestPhase::COMPLETED:
        case TestPhase::IDLE:
        default:
            break;
    }
}

void StressTestScene::_updateMetrics(float deltaTime) {
    ++_frameCount;

    float frameTimeMs = deltaTime * 1000.0f;
    _frameTimes.push_back(frameTimeMs);

    // Keep only last 100 frame times for rolling average
    if (_frameTimes.size() > 100) {
        _frameTimes.erase(_frameTimes.begin());
    }

    // Calculate metrics
    if (!_frameTimes.empty()) {
        float sum =
            std::accumulate(_frameTimes.begin(), _frameTimes.end(), 0.0f);
        _avgFrameTime = sum / static_cast<float>(_frameTimes.size());
    }

    _minFrameTime = std::min(_minFrameTime, frameTimeMs);
    _maxFrameTime = std::max(_maxFrameTime, frameTimeMs);

    if (deltaTime > 0.0f) {
        _currentFps = 1.0f / deltaTime;
    }
}

void StressTestScene::_updateUI() {
    // Update FPS text
    if (_registry->hasComponent<rc::Text>(_fpsText)) {
        auto& fpsTextComp = _registry->getComponent<rc::Text>(_fpsText);
        std::ostringstream fpsStream;
        fpsStream << "FPS: " << std::fixed << std::setprecision(1)
                  << _currentFps;
        fpsTextComp.textContent = fpsStream.str();
    }

    // Update entity count text
    if (_registry->hasComponent<rc::Text>(_entityCountText)) {
        auto& countTextComp =
            _registry->getComponent<rc::Text>(_entityCountText);
        countTextComp.textContent =
            "Entities: " + std::to_string(_currentEntityCount);
    }

    // Update phase text
    if (_registry->hasComponent<rc::Text>(_phaseText)) {
        auto& phaseTextComp = _registry->getComponent<rc::Text>(_phaseText);
        phaseTextComp.textContent = "Phase: " + _getPhaseString();

        // Color based on phase
        switch (_currentPhase) {
            case TestPhase::SPAWNING:
                phaseTextComp.color = sf::Color::Green;
                break;
            case TestPhase::SUSTAINED:
                phaseTextComp.color = sf::Color::Yellow;
                break;
            case TestPhase::DESTROYING:
                phaseTextComp.color = sf::Color::Red;
                break;
            case TestPhase::COMPLETED:
                phaseTextComp.color = sf::Color::Cyan;
                break;
            default:
                phaseTextComp.color = sf::Color::White;
                break;
        }
    }

    // Update stats text
    if (_registry->hasComponent<rc::Text>(_statsText)) {
        auto& statsTextComp = _registry->getComponent<rc::Text>(_statsText);
        std::ostringstream statsStream;
        statsStream << std::fixed << std::setprecision(2)
                    << "Avg: " << _avgFrameTime << "ms\n"
                    << "Min: " << _minFrameTime << "ms\n"
                    << "Max: " << _maxFrameTime << "ms\n"
                    << "Frames: " << _frameCount;
        statsTextComp.textContent = statsStream.str();
    }
}

std::string StressTestScene::_getPhaseString() const {
    switch (_currentPhase) {
        case TestPhase::IDLE:
            return "IDLE";
        case TestPhase::SPAWNING:
            return "SPAWNING";
        case TestPhase::SUSTAINED:
            return "SUSTAINED";
        case TestPhase::DESTROYING:
            return "DESTROYING";
        case TestPhase::COMPLETED:
            return "COMPLETED";
        default:
            return "UNKNOWN";
    }
}

void StressTestScene::pollEvents(const sf::Event& e) {
    if (const auto* keyEvent = e.getIf<sf::Event::KeyPressed>()) {
        switch (keyEvent->code) {
            case sf::Keyboard::Key::Escape:
                try {
                    _switchToScene(SceneManager::MAIN_MENU);
                } catch (SceneNotFound& ex) {
                    std::cerr << "Error: " << ex.what() << std::endl;
                }
                break;

            case sf::Keyboard::Key::Space:
                _spawnTestEntities(SPAWN_BATCH_SIZE);
                break;

            case sf::Keyboard::Key::A:
                _autoMode = !_autoMode;
                _currentPhase =
                    _autoMode ? TestPhase::SPAWNING : TestPhase::IDLE;
                break;

            case sf::Keyboard::Key::C:
                _destroyAllTestEntities();
                break;

            case sf::Keyboard::Key::Num1:
                _spawnTestEntities(100);
                break;

            case sf::Keyboard::Key::Num5:
                _spawnTestEntities(500);
                break;

            default:
                break;
        }
    }
}

void StressTestScene::update() {
    float deltaTime = _frameClock.restart().asSeconds();

    _updateMetrics(deltaTime);
    _runAutoMode(deltaTime);
    _updateUI();

    // Bounce entities off screen edges (simple physics)
    for (auto entity : _testEntities) {
        if (!_registry->hasComponent<rs::Position>(entity) ||
            !_registry->hasComponent<rs::VelocityComponent>(entity)) {
            continue;
        }

        auto& pos = _registry->getComponent<rs::Position>(entity);
        auto& vel = _registry->getComponent<rs::VelocityComponent>(entity);

        // Bounce off edges
        if (pos.x < 350.0f || pos.x > 1850.0f) {
            vel.vx = -vel.vx;
            pos.x = std::clamp(pos.x, 350.0f, 1850.0f);
        }
        if (pos.y < 50.0f || pos.y > 1030.0f) {
            vel.vy = -vel.vy;
            pos.y = std::clamp(pos.y, 50.0f, 1030.0f);
        }
    }
}

void StressTestScene::render(std::shared_ptr<sf::RenderWindow> window) {
    // Rendering is handled by the main Graphic class via ECS systems
}
