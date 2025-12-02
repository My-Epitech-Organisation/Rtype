/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include <functional>

#include <time.h>

#include <SFML/Graphics/Text.hpp>

#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Graphic/TagComponent.hpp"
#include "Components/Graphic/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/PrallaxComponent.hpp"
#include "Graphic/SizeComponent.hpp"
#include "Graphic/TextureRectComponent.hpp"
#include "Graphic/VelocityComponent.hpp"
#include "SceneManager/SceneException.hpp"
#include "assets/Audiowide_Regular.h"
#include "assets/bgMenu.h"
#include "assets/playerVessel.h"

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
    this->_assetsManager->textureManager->load(
        "player_vessel", playerVessel_gif, playerVessel_gif_len);
    this->_assetsManager->textureManager->get("bg_menu").setRepeated(true);

    auto background = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<Image>(
        background, this->_assetsManager->textureManager->get("bg_menu"));
    this->_registry->emplaceComponent<Position>(background, 0, 0);
    this->_registry->emplaceComponent<Parallax>(background, 0.8, true);

    auto seed = static_cast<unsigned int>(time(nullptr));

    for (int i = 0; i < 15; i++) {
        auto fakePlayer = this->_registry->spawnEntity();
        this->_registry->emplaceComponent<Image>(
            fakePlayer,
            this->_assetsManager->textureManager->get("player_vessel"));
        this->_registry->emplaceComponent<TextureRect>(
            fakePlayer, std::pair<int, int>({0, 0}),
            std::pair<int, int>({33, 17}));
        this->_registry->emplaceComponent<Position>(
            fakePlayer, (-10 * (rand_r(&seed) % 150) + 50),
            72 * (rand_r(&seed) % 15));
        this->_registry->emplaceComponent<Size>(fakePlayer, 2.2, 2.2);
        this->_registry->emplaceComponent<Velocity>(
            fakePlayer, (rand_r(&seed) % 150) + 75, 0.f);
        this->_listEntity.push_back(fakePlayer);
    }

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
