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
#include "Graphic.hpp"
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
        this->_assetsManager->audioManager->load(
            "gameover_music",
            this->_assetsManager->configGameAssets.assets.music.gameOver);
        this->_audio->loadMusic(
            this->_assetsManager->audioManager->get("gameover_music"));
        this->_audio->play();
    }
}

void GameOverScene::pollEvents(const sf::Event& e) { (void)e; }

void GameOverScene::update(float dt) { (void)dt; }

void GameOverScene::render(std::shared_ptr<sf::RenderWindow> window) {
    (void)window;
}

void GameOverScene::_buildLayout() {
    auto backgroundEntities = EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "");
    this->_listEntity.insert(this->_listEntity.end(),
                             backgroundEntities.begin(),
                             backgroundEntities.end());

    std::uint32_t finalScore = 0;
    if (this->_registry
            ->hasSingleton<rtype::games::rtype::client::GameOverState>()) {
        finalScore =
            this->_registry
                ->getSingleton<rtype::games::rtype::client::GameOverState>()
                .finalScore;
    }

    auto popUpOverlay = EntityFactory::createRectangle(
        this->_registry,
        sf::Vector2i{
            rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH,
            rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT},
        sf::Color(0, 0, 0, 200), sf::Vector2f{0.f, 0.f});
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popUpOverlay,
        rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
    this->_listEntity.push_back(popUpOverlay);

    const float centerX = static_cast<float>(Graphic::WINDOW_WIDTH) / 2.0f;

    auto title = EntityFactory::createStaticText(
        this->_registry, _assetsManager, "GAME OVER", "title_font",
        sf::Vector2f{
            centerX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_TITLE_Y},
        96.f);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    this->_listEntity.push_back(title);

    auto score = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "SCORE: " + std::to_string(finalScore), "main_font",
        sf::Vector2f{
            centerX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_SCORE_Y},
        72.f);

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        score, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    this->_listEntity.push_back(score);

    float btnWidth =
        rtype::games::rtype::client::GraphicsConfig::GAME_OVER_BUTTON_WIDTH;
    float btnHeight =
        rtype::games::rtype::client::GraphicsConfig::GAME_OVER_BUTTON_HEIGHT;
    float btnX =
        rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH / 2 -
        (btnWidth / 2.0f) -
        rtype::games::rtype::client::GraphicsConfig::GAME_OVER_BUTTON_X_OFFSET;

    auto button = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Back to Menu"),
        rtype::games::rtype::shared::Position(
            btnX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_BUTTON_Y),
        rtype::games::rtype::client::Rectangle(
            {static_cast<int>(btnWidth), static_cast<int>(btnHeight)},
            sf::Color(0, 150, 200), sf::Color(0, 200, 255)),
        this->_assetsManager, std::function<void()>{[this]() {
            if (this->_switchToScene) {
                this->_switchToScene(SceneManager::MAIN_MENU);
            }
        }});

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        button, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    this->_listEntity.push_back(button);
}
