/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include <time.h>

#include <functional>

#include <SFML/Graphics/Text.hpp>

#include "Components/Common/PositionComponent.hpp"
#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Graphic/TagComponent.hpp"
#include "Components/Graphic/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/BoxingComponent.hpp"
#include "Graphic/PrallaxComponent.hpp"
#include "Graphic/SizeComponent.hpp"
#include "Graphic/TextureRectComponent.hpp"
#include "Graphic/VelocityComponent.hpp"
#include "SceneManager/SceneException.hpp"

void MainMenuScene::update() {}

void MainMenuScene::render(sf::RenderWindow& window) {}

void MainMenuScene::pollEvents(const sf::Event& e) {}

MainMenuScene::MainMenuScene(
    const std::shared_ptr<ECS::Registry>& ecs,
    const std::shared_ptr<AssetManager>& assetsManager,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    sf::RenderWindow& window)
    : AScene(ecs, assetsManager) {
    auto seed = static_cast<unsigned int>(time(nullptr));

    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "R-TYPE"));
    auto astroneerVessel = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<Image>(
        astroneerVessel,
        this->_assetsManager->textureManager->get("astro_vessel"));
    this->_registry->emplaceComponent<Position>(astroneerVessel, 1900, 1060);
    this->_registry->emplaceComponent<Size>(astroneerVessel, 0.3, 0.3);
    this->_registry->emplaceComponent<Velocity>(astroneerVessel, -135.f, -75.f);
    this->_listEntity.push_back(astroneerVessel);

    for (int i = 0; i < 7; i++) {
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
