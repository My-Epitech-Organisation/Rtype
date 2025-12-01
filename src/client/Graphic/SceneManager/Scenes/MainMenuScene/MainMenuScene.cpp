/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.cpp
*/

#include "MainMenuScene.hpp"

#include "Components/Graphic/ImageComponent.hpp"
#include "Components/Common/PositionComponent.hpp"
#include "assets/bgMenu.h"

void MainMenuScene::update() {
}

void MainMenuScene::render(sf::RenderWindow &window) {
    this->_registry->view<Image, Position>().each([this, &window](auto _, auto& img, auto& pos) {
        sf::Sprite sprite(this->_assetsManager->textureManager->get(img.textureId));
        // sprite.setTextureRect(img.textureRect);
        sprite.setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y)});

        window.draw(sprite);
    });
}

void MainMenuScene::pollEvents(const sf::Event &e) {
}

MainMenuScene::MainMenuScene(const std::shared_ptr<ECS::Registry> &ecs, const std::shared_ptr<AssetManager> &assetsManager) :
    AScene(ecs, assetsManager)
{
    this->_background = this->_registry->spawnEntity();
    this->_assetsManager->textureManager->load("bg_menu", bgMainMenu_png, bgMainMenu_png_len);
    this->_registry->emplaceComponent<Image>(this->_background, "bg_menu", sf::IntRect{{0, 0}, {0, 0}});
    this->_registry->emplaceComponent<Position>(this->_background, 0, 0);
}

MainMenuScene::~MainMenuScene() {
    this->_registry->killEntity(this->_background);
}


