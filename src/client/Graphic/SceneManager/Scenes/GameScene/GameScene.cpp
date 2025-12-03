/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include "Components/HiddenComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"

void GameScene::_updateUserMovementUp() {
    auto keyMoveUp = this->_keybinds->getKeyBinding(GameAction::MOVE_UP);
    if (!keyMoveUp.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveUp)) {
        this->_registry->view<Velocity, ControllableTag>().each(
            [](auto, auto& velocity, auto) { velocity.y -= 300; });
    }
}

void GameScene::_updateUserMovementDown() {
    auto keyMoveDown = this->_keybinds->getKeyBinding(GameAction::MOVE_DOWN);
    if (!keyMoveDown.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveDown)) {
        this->_registry->view<Velocity, ControllableTag>().each(
            [](auto, auto& velocity, auto) { velocity.y += 300; });
    }
}

void GameScene::_updateUserMovementLeft() {
    auto keyMoveLeft = this->_keybinds->getKeyBinding(GameAction::MOVE_LEFT);
    if (!keyMoveLeft.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveLeft)) {
        this->_registry->view<Velocity, ControllableTag>().each(
            [](auto, auto& velocity, auto) { velocity.x -= 300; });
    }
}

void GameScene::_updateUserMovementRight() {
    auto keyMoveRight = this->_keybinds->getKeyBinding(GameAction::MOVE_RIGHT);
    if (!keyMoveRight.has_value()) {
        return;
    }
    if (sf::Keyboard::isKeyPressed(*keyMoveRight)) {
        this->_registry->view<Velocity, ControllableTag>().each(
            [](auto, auto& velocity, auto) { velocity.x += 300; });
    }
}

void GameScene::_handleKeyReleasedEvent(const sf::Event& event) {
    auto eventKeyRelease = event.getIf<sf::Event::KeyReleased>();
    if (eventKeyRelease->code ==
        this->_keybinds->getKeyBinding(GameAction::PAUSE)) {
        this->_registry->view<HiddenComponent, PauseMenuTag>().each(
            [](auto, HiddenComponent& hidden, auto) {
                hidden.isHidden = !hidden.isHidden;
            });
    }
}

void GameScene::update() {
    this->_registry->view<Velocity, ControllableTag>().each(
        [](auto, auto& velocity, auto) {
            velocity.x = 0;
            velocity.y = 0;
        });
    this->_updateUserMovementUp();
    this->_updateUserMovementDown();
    this->_updateUserMovementLeft();
    this->_updateUserMovementRight();
}

void GameScene::render(const std::shared_ptr<sf::RenderWindow>& window) {}

void GameScene::pollEvents(const sf::Event& e) {
    if (e.is<sf::Event::KeyReleased>()) {
        this->_handleKeyReleasedEvent(e);
    }
}

GameScene::GameScene(
    const std::shared_ptr<ECS::Registry>& ecs,
    const std::shared_ptr<AssetManager>& textureManager,
    const std::shared_ptr<sf::RenderWindow>& window,
    const std::shared_ptr<KeyboardActions>& keybinds,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, textureManager, window), _keybinds(keybinds) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, ""));
    auto fakePlayer = EntityFactory::createPlayer(
        this->_registry, this->_assetsManager, {4, 4}, true);
    this->_listEntity.push_back(fakePlayer);
    auto sectionX = (1920 - SIZE_X_PAUSE_MENU) / 2;
    auto sectionY = (1080 - SIZE_Y_PAUSE_MENU) / 2;
    auto pauseEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "", sectionX, sectionY,
        SIZE_X_PAUSE_MENU, SIZE_Y_PAUSE_MENU);
    pauseEntities.push_back(EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, PAUSE_MENU_TITLE, "title_font",
        (sectionX + SIZE_X_PAUSE_MENU / 2) -
            ((PAUSE_MENU_TITLE.length() - 2) * (SIZE_FONT_PAUSE_MENU / 2)),
        sectionY, SIZE_FONT_PAUSE_MENU));

    pauseEntities.push_back(EntityFactory::createButton(
        this->_registry,
        Text(this->_assetsManager->fontManager->get("title_font"),
             sf::Color::White, 30, "Menu"),
        Position(sectionX + ((SIZE_X_PAUSE_MENU / 2) - (150 / 2)),
                 sectionY + SIZE_Y_PAUSE_MENU - 75),
        Rectangle({150, 55}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Main Menu: " << e.what()
                          << std::endl;
            }
        })));

    for (auto& entt : pauseEntities) {
        this->_registry->emplaceComponent<HiddenComponent>(entt, true);
        this->_registry->emplaceComponent<PauseMenuTag>(entt);
    }
    this->_listEntity.insert(this->_listEntity.end(), pauseEntities.begin(),
                             pauseEntities.end());
}
