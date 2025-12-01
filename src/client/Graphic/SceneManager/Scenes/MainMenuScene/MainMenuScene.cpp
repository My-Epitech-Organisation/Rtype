/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include <functional>

#include <SFML/Graphics/Text.hpp>

#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Graphic/TagComponent.hpp"
#include "Components/Graphic/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"
#include "assets/Audiowide_Regular.h"
#include "assets/bgMenu.h"

void MainMenuScene::update() {}

void MainMenuScene::render(sf::RenderWindow& window) {}

void MainMenuScene::pollEvents(const sf::Event& e) {}

MainMenuScene::MainMenuScene(
    const std::shared_ptr<ECS::Registry>& ecs,
    const std::shared_ptr<AssetManager>& assetsManager,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    sf::RenderWindow& window)
    : AScene(ecs, assetsManager) {
    this->_assetsManager->fontManager->load("title_font", Audiowide_Regular_ttf,
                                            Audiowide_Regular_ttf_len);
    this->_assetsManager->textureManager->load("bg_menu", bgMainMenu_png,
                                               bgMainMenu_png_len);

    auto background = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<Image>(
        background, this->_assetsManager->textureManager->get("bg_menu"),
        sf::IntRect{{0, 0}, {0, 0}});
    this->_registry->emplaceComponent<Position>(background, 0, 0);

    auto appTitle = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<Text>(
        appTitle, this->_assetsManager->fontManager->get("title_font"),
        sf::Color::White, 72, "R-TYPE");
    this->_registry->emplaceComponent<Position>(appTitle, 50, 50);
    this->_registry->emplaceComponent<StaticTextTag>(appTitle);

    this->_listEntity.push_back(background);
    this->_listEntity.push_back(appTitle);
    this->_listEntity.push_back(EntityFactory::createButton(
        *this->_registry,
        Text(this->_assetsManager->fontManager->get("title_font"),
             sf::Color::White, 36, "Start Game"),
        Position(100, 350),
        Rectangle({400, 75}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::IN_GAME);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Game Menu: " << e.what()
                          << std::endl;
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        *this->_registry,
        Text(this->_assetsManager->fontManager->get("title_font"),
             sf::Color::White, 36, "Settings"),
        Position(100, 460),
        Rectangle({400, 75}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::SETTINGS_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Settings Menu: " << e.what()
                          << std::endl;
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        *this->_registry,
        Text(this->_assetsManager->fontManager->get("title_font"),
             sf::Color::White, 36, "Quit"),
        Position(100, 570),
        Rectangle({400, 75}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([&window]() { window.close(); })

            ));
}

MainMenuScene::~MainMenuScene() {
    for (auto& entity : this->_listEntity) {
        this->_registry->killEntity(entity);
    }
}
