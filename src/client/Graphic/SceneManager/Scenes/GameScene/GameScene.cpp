/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include "Components/CountdownComponent.hpp"
#include "Components/HiddenComponent.hpp"
#include "Components/ImageComponent.hpp"
#include "Components/LifetimeComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SizeComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/Tags.hpp"
#include "Components/VelocityComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic.hpp"
#include "SceneManager/SceneException.hpp"

void GameScene::_updateUserMovementUp() {
    auto keyMoveUp = this->_keybinds->getKeyBinding(GameAction::MOVE_UP);
    if (!keyMoveUp.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveUp)) {
        this->_registry
            ->view<rtype::games::rtype::shared::VelocityComponent,
                   rtype::games::rtype::client::ControllableTag>()
            .each([](auto, auto& velocity, auto) {
                velocity.vy -= PlayerMovementSpeed;
            });
    }
}

void GameScene::_updateUserMovementDown() {
    auto keyMoveDown = this->_keybinds->getKeyBinding(GameAction::MOVE_DOWN);
    if (!keyMoveDown.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveDown)) {
        this->_registry
            ->view<rtype::games::rtype::shared::VelocityComponent,
                   rtype::games::rtype::client::ControllableTag>()
            .each([](auto, auto& velocity, auto) {
                velocity.vy += PlayerMovementSpeed;
            });
    }
}

void GameScene::_updateUserMovementLeft() {
    auto keyMoveLeft = this->_keybinds->getKeyBinding(GameAction::MOVE_LEFT);
    if (!keyMoveLeft.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveLeft)) {
        this->_registry
            ->view<rtype::games::rtype::shared::VelocityComponent,
                   rtype::games::rtype::client::ControllableTag>()
            .each([](auto, auto& velocity, auto) {
                velocity.vx -= PlayerMovementSpeed;
            });
    }
}

void GameScene::_updateUserMovementRight() {
    auto keyMoveRight = this->_keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
    if (!keyMoveRight.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveRight)) {
        this->_registry
            ->view<rtype::games::rtype::shared::VelocityComponent,
                   rtype::games::rtype::client::ControllableTag>()
            .each([](auto, auto& velocity, auto) {
                velocity.vx += PlayerMovementSpeed;
            });
    }
}

void GameScene::_handleShoot() {
    auto playerView =
        this->_registry->view<rtype::games::rtype::shared::Position,
                              rtype::games::rtype::client::ControllableTag>();

    playerView.each([this](auto, auto& pos, auto) {
        auto projectile = EntityFactory::createProjectile(
            this->_registry, this->_assetsManager,
            sf::Vector2f(pos.x + 80, pos.y));

        this->_listEntity.push_back(projectile);
    });
}

void GameScene::_handleKeyReleasedEvent(const sf::Event& event) {
    auto eventKeyRelease = event.getIf<sf::Event::KeyReleased>();
    auto keyPause = this->_keybinds->getKeyBinding(GameAction::PAUSE);
    if (!eventKeyRelease || !keyPause.has_value()) return;
    if (eventKeyRelease->code == *keyPause) {
        this->_registry
            ->view<rtype::games::rtype::client::HiddenComponent,
                   rtype::games::rtype::client::PauseMenuTag>()
            .each([](auto, rtype::games::rtype::client::HiddenComponent& hidden,
                     auto) { hidden.isHidden = !hidden.isHidden; });
    }
}

void GameScene::_updateUserShoot(float deltaTime) {
    this->_registry
        ->view<rtype::games::rtype::client::CountdownPlayer,
               rtype::games::rtype::client::ControllableTag>()
        .each([this, deltaTime](auto, auto& cd, auto) {
            cd.laserCD -= deltaTime;
            if (cd.laserCD > 0) return;
            if (sf::Keyboard::isKeyPressed(
                    *this->_keybinds->getKeyBinding(GameAction::SHOOT))) {
                cd.laserCD =
                    rtype::games::rtype::client::GraphicsConfig::PROJECTILE_CD;
                this->_handleShoot();
            }
        });
}

void GameScene::update(float deltaTime) {
    this->_registry
        ->view<rtype::games::rtype::shared::VelocityComponent,
               rtype::games::rtype::client::ControllableTag>()
        .each([](auto, auto& velocity, auto) {
            velocity.vx = 0;
            velocity.vy = 0;
        });
    this->_updateUserMovementUp();
    this->_updateUserMovementDown();
    this->_updateUserMovementLeft();
    this->_updateUserMovementRight();
    this->_updateUserShoot(deltaTime);
}

void GameScene::render(std::shared_ptr<sf::RenderWindow> window) {}

void GameScene::pollEvents(const sf::Event& e) {
    if (e.is<sf::Event::KeyReleased>()) {
        this->_handleKeyReleasedEvent(e);
    }
}

GameScene::GameScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, textureManager, window, audio), _keybinds(keybinds) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, ""));
    auto fakePlayer = EntityFactory::createPlayer(
        this->_registry, this->_assetsManager, {4, 4}, true);
    this->_listEntity.push_back(fakePlayer);
    auto sectionX = (Graphic::WINDOW_WIDTH - SizeXPauseMenu) / 2;
    auto sectionY = (Graphic::WINDOW_HEIGHT - SizeYPauseMenu) / 2;
    auto pauseEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "", sectionX, sectionY,
        SizeXPauseMenu, SizeYPauseMenu);
    auto titleEntity = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, PauseMenuTitle, "title_font",
        (sectionX + SizeXPauseMenu / 2) -
            ((PauseMenuTitle.length() - 2) * (SizeFontPauseMenu / 2)),
        sectionY, SizeFontPauseMenu);
    auto& titleText =
        this->_registry->getComponent<rtype::games::rtype::client::Text>(
            titleEntity);
    sf::FloatRect bounds = titleText.text.getLocalBounds();
    float centeredX = sectionX + (SizeXPauseMenu - bounds.size.x) / 2;
    auto& titlePos =
        this->_registry->getComponent<rtype::games::rtype::shared::Position>(
            titleEntity);
    titlePos.x = centeredX;
    pauseEntities.push_back(titleEntity);

    pauseEntities.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 30, "Menu"),
        rtype::games::rtype::shared::Position(
            sectionX + ((SizeXPauseMenu / 2) - (150 / 2)),
            sectionY + SizeYPauseMenu - 75),
        rtype::games::rtype::client::Rectangle({150, 55}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Main Menu: " << e.what()
                          << std::endl;
            }
        })));

    for (auto& entt : pauseEntities) {
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::HiddenComponent>(
                entt, true);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::PauseMenuTag>(entt);
    }
    this->_listEntity.insert(this->_listEntity.end(), pauseEntities.begin(),
                             pauseEntities.end());
    this->_assetsManager->textureManager->load("projectile_player_laser",
                                               "./assets/missileLaser.gif");
    this->_assetsManager->audioManager->load(
        "main_game_music",
        this->_assetsManager->configGameAssets.assets.music.game);
    auto bgMusic = this->_assetsManager->audioManager->get("main_game_music");
    this->_audio->loadMusic(bgMusic);
    this->_audio->setLoop(true);
    this->_audio->play();
}
