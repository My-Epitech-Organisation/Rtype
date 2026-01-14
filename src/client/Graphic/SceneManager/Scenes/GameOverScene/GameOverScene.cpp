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

#include "AllComponents.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/client/GameOverState.hpp"
#include "games/rtype/client/GraphicsConstants.hpp"

GameOverScene::GameOverScene(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<AudioLib> audio,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(std::move(registry), std::move(assetsManager), std::move(window),
             std::move(audio)),
      _switchToScene(std::move(switchToScene)) {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameOverScene] Constructing Game Over scene");
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

void GameOverScene::pollEvents(const rtype::display::Event& e) { (void)e; }

void GameOverScene::update(float dt) { (void)dt; }

void GameOverScene::render(std::shared_ptr<rtype::display::IDisplay> window) {
    (void)window;
}

void GameOverScene::_buildLayout() {
    auto backgroundEntities = EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "", nullptr);
    this->_listEntity.insert(this->_listEntity.end(),
                             backgroundEntities.begin(),
                             backgroundEntities.end());

    std::uint32_t finalScore = 0;
    bool isVictory = false;
    if (this->_registry
            ->hasSingleton<rtype::games::rtype::client::GameOverState>()) {
        const auto& state =
            this->_registry
                ->getSingleton<rtype::games::rtype::client::GameOverState>();
        finalScore = state.finalScore;
        isVictory = state.isVictory;
    }

    auto popUpOverlay = EntityFactory::createRectangle(
        this->_registry,
        rtype::display::Vector2<int>{
            rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH,
            rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT},
        rtype::display::Color(0, 0, 0, 200),
        rtype::display::Vector2<float>{0.f, 0.f});
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        popUpOverlay,
        rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
    this->_listEntity.push_back(popUpOverlay);

    if (!isVictory) {
        auto bloodTop = EntityFactory::createRectangle(
            this->_registry,
            rtype::display::Vector2<int>{
                rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH, 80},
            rtype::display::Color(139, 0, 0, 180),
            rtype::display::Vector2<float>{0.f, 0.f});
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            bloodTop,
            rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
        this->_listEntity.push_back(bloodTop);

        auto bloodBottom = EntityFactory::createRectangle(
            this->_registry,
            rtype::display::Vector2<int>{
                rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH, 100},
            rtype::display::Color(139, 0, 0, 200),
            rtype::display::Vector2<float>{
                0.f,
                static_cast<float>(
                    rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT -
                    100)});
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            bloodBottom,
            rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
        this->_listEntity.push_back(bloodBottom);

        auto bloodLeft = EntityFactory::createRectangle(
            this->_registry,
            rtype::display::Vector2<int>{
                60, rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT},
            rtype::display::Color(139, 0, 0, 150),
            rtype::display::Vector2<float>{0.f, 0.f});
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            bloodLeft,
            rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
        this->_listEntity.push_back(bloodLeft);

        auto bloodRight = EntityFactory::createRectangle(
            this->_registry,
            rtype::display::Vector2<int>{
                60, rtype::games::rtype::client::GraphicsConfig::WINDOW_HEIGHT},
            rtype::display::Color(139, 0, 0, 150),
            rtype::display::Vector2<float>{
                static_cast<float>(
                    rtype::games::rtype::client::GraphicsConfig::WINDOW_WIDTH -
                    60),
                0.f});
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            bloodRight,
            rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI - 1);
        this->_listEntity.push_back(bloodRight);
    }

    const float centerX = static_cast<float>(Graphic::WINDOW_WIDTH) / 2.0f;

    std::string titleText = isVictory ? "YOU WIN" : "YOU DIED";
    auto title = EntityFactory::createStaticText(
        this->_registry, _assetsManager, titleText, "title_font",
        rtype::display::Vector2<float>{
            centerX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_TITLE_Y},
        96.f);

    if (this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            title)) {
        auto& textComp =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                title);
        if (isVictory) {
            textComp.color = rtype::display::Color::Green();
        } else {
            textComp.color = rtype::display::Color::Red();
        }
    }

    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredTextTag>(title);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        title, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    this->_listEntity.push_back(title);

    auto score = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "SCORE: " + std::to_string(finalScore), "main_font",
        rtype::display::Vector2<float>{
            centerX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_SCORE_Y},
        72.f);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredTextTag>(score);

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
            "main_font", rtype::display::Color::White(), 36, "Back to Menu"),
        rtype::games::rtype::shared::TransformComponent(
            btnX,
            rtype::games::rtype::client::GraphicsConfig::GAME_OVER_BUTTON_Y),
        rtype::games::rtype::client::Rectangle(
            {static_cast<int>(btnWidth), static_cast<int>(btnHeight)},
            rtype::display::Color(0, 150, 200, 255),
            rtype::display::Color(0, 200, 255, 255)),
        this->_assetsManager, std::function<void()>{[this]() {
            if (this->_switchToScene) {
                this->_switchToScene(SceneManager::MAIN_MENU);
            }
        }});

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        button, rtype::games::rtype::client::GraphicsConfig::ZINDEX_UI);
    this->_listEntity.push_back(button);
}
