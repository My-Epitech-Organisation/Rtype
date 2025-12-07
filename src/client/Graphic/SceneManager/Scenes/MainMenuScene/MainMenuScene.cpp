/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <random>

#include <SFML/Graphics/Text.hpp>

#include "AllComponents.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"

void MainMenuScene::_createAstroneerVessel() {
    auto astroneerVessel = this->_registry->spawnEntity();
    this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
        astroneerVessel,
        this->_assetsManager->textureManager->get("astro_vessel"));
    this->_registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        astroneerVessel, 1900, 1060);
    this->_registry->emplaceComponent<rtype::games::rtype::client::Size>(
        astroneerVessel, 0.3, 0.3);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::shared::VelocityComponent>(
            astroneerVessel, -135.f, -75.f);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        astroneerVessel, -1);
    this->_listEntity.push_back(astroneerVessel);
}

void MainMenuScene::_createFakePlayer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib150(1, 150);
    std::uniform_int_distribution<> distrib15(1, 15);

    for (int i = 0; i < nbr_vessels; i++) {
        auto fakePlayer = this->_registry->spawnEntity();
        this->_registry->emplaceComponent<rtype::games::rtype::client::Image>(
            fakePlayer,
            this->_assetsManager->textureManager->get("player_vessel"));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::TextureRect>(
                fakePlayer, std::pair<int, int>({0, 0}),
                std::pair<int, int>({33, 17}));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::Position>(
                fakePlayer, (-10 * (distrib150(gen) + 50)),
                72 * (distrib15(gen) % 15));
        this->_registry->emplaceComponent<rtype::games::rtype::client::Size>(
            fakePlayer, 2.2, 2.2);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::VelocityComponent>(
                fakePlayer, (distrib150(gen) % 150) + 75, 0.f);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            fakePlayer, 0);
        this->_listEntity.push_back(fakePlayer);
    }
}

void MainMenuScene::update() {}

void MainMenuScene::render(std::shared_ptr<sf::RenderWindow> window) {}

void MainMenuScene::pollEvents(const sf::Event& e) {}

MainMenuScene::MainMenuScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, assetsManager, window) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "R-TYPE"));
    this->_createAstroneerVessel();
    this->_createFakePlayer();

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Start Game"),
        rtype::games::rtype::shared::Position(100, 350),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::IN_GAME);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Game Menu: " << e.what()
                          << std::endl;
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Settings"),
        rtype::games::rtype::shared::Position(100, 460),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::SETTINGS_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Settings Menu: " << e.what()
                          << std::endl;
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Stress Test"),
        rtype::games::rtype::shared::Position(100, 570),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color(128, 0, 128),
                                               sf::Color(180, 0, 180)),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::STRESS_TEST);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Stress Test: " << e.what()
                          << std::endl;
            }
        })));
    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Quit"),
        rtype::games::rtype::shared::Position(100, 680),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([this]() { this->_window->close(); })

            ));
}
