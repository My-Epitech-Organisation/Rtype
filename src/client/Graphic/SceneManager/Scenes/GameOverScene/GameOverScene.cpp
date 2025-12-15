/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameOverScene.cpp
*/

#include "GameOverScene.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <SFML/Graphics/Color.hpp>

#include "AllComponents.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/client/GameOverState.hpp"
#include "games/rtype/client/GraphicsConstants.hpp"

GameOverScene::GameOverScene(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<AudioLib> audio,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(std::move(registry), std::move(assetsManager), std::move(window),
             std::move(audio)),
      _switchToScene(std::move(switchToScene)) {
    LOG_DEBUG("[GameOverScene] Constructing Game Over scene");
    _buildLayout();
    if (_audio) {
        _audio->pauseMusic();
    }
}

void GameOverScene::pollEvents(const sf::Event& e) { (void)e; }

void GameOverScene::update(float dt) { (void)dt; }

void GameOverScene::render(std::shared_ptr<sf::RenderWindow> window) {
    (void)window;
}

void GameOverScene::_buildLayout() {
    auto backgroundEntities =
        EntityFactory::createBackground(_registry, _assetsManager, "");
    _listEntity.insert(_listEntity.end(), backgroundEntities.begin(),
                       backgroundEntities.end());

    std::uint32_t finalScore = 0;
    if (_registry->hasSingleton<rtype::games::rtype::client::GameOverState>()) {
        finalScore =
            _registry
                ->getSingleton<rtype::games::rtype::client::GameOverState>()
                .finalScore;
    }

    const float centerX =
        static_cast<float>(
            rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH) /
        2.0f;

    auto title = EntityFactory::createStaticText(
        _registry, _assetsManager, "GAME OVER", "title_font",
        sf::Vector2f{centerX - 200.f, 180.f}, 96.f);
    _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    _listEntity.push_back(title);

    auto score = EntityFactory::createStaticText(
        _registry, _assetsManager, "SCORE: " + std::to_string(finalScore),
        "title_font", sf::Vector2f{centerX - 180.f, 320.f}, 72.f);
    _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        score, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    _listEntity.push_back(score);

    auto button = EntityFactory::createButton(
        _registry,
        rtype::games::rtype::client::Text(
            _assetsManager->fontManager->get("main_font"), sf::Color::White, 36,
            "Back to Menu"),
        rtype::games::rtype::shared::Position(centerX - 120.f, 520.f),
        rtype::games::rtype::client::Rectangle(
            {240, 70}, sf::Color(0, 150, 200), sf::Color(0, 200, 255)),
        _assetsManager, std::function<void()>{[this]() {
            if (_switchToScene) {
                _switchToScene(SceneManager::MAIN_MENU);
            }
        }});
    _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        button, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    _listEntity.push_back(button);
}
