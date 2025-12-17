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
    if (this->_audio) {
        this->_audio->pauseMusic();
        this->_assetsManager->audioManager->load("gameover_music", this->_assetsManager->configGameAssets.assets.music.gameOver);
        this->_audio->loadMusic(this->_assetsManager->audioManager->get("gameover_music"));
        this->_audio->play();
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

    auto popUpOverlay = EntityFactory::createRectangle(
        _registry,
        sf::Vector2i{
            rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH,
            rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT},
        sf::Color(0, 0, 0, 200),
        sf::Vector2f{100.f, 75.f});
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popUpOverlay,
        rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
    this->_registry->emplaceComponent<rtype::games::rtype::shared::Position>(
    popUpOverlay, 0, 0);
    this->_listEntity.push_back(popUpOverlay);

    const float centerX = static_cast<float>(rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH) / 2.0f - 70.f;

        auto title = EntityFactory::createStaticText(
            _registry, _assetsManager, "GAME OVER", "title_font",
            sf::Vector2f{centerX - 260.f, 180.f}, 96.f);

        _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            title, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
        _listEntity.push_back(title);


        auto score = EntityFactory::createStaticText(
            _registry, _assetsManager, "SCORE: " + std::to_string(finalScore),
            "main_font", sf::Vector2f{centerX - 140.f, 320.f}, 72.f);

        _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            score, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
        _listEntity.push_back(score);


        float btnWidth = 300.f;
        float btnHeight = 70.f;
        float btnX = 1920 / 2 - (btnWidth / 2.0f) - 20.f;

        auto button = EntityFactory::createButton(
            _registry,
            rtype::games::rtype::client::Text(
                _assetsManager->fontManager->get("main_font"), sf::Color::White, 36,
                "Back to Menu"),
            rtype::games::rtype::shared::Position(btnX, 650.f), // Utilise la variable calculée
            rtype::games::rtype::client::Rectangle(
                {(int)btnWidth, (int)btnHeight}, sf::Color(0, 150, 200), sf::Color(0, 200, 255)),
            _assetsManager, std::function<void()>{[this]() {
                if (_switchToScene) {
                    _switchToScene(SceneManager::MAIN_MENU);
                }
            }});

        _registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            button, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
        _listEntity.push_back(button);
}
