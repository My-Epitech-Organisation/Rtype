/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.cpp
*/

#include "SettingsScene.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"

void SettingsScene::update() {
}

void SettingsScene::render(sf::RenderWindow &window) {
}

void SettingsScene::pollEvents(const sf::Event &e) {
}

SettingsScene::SettingsScene(const std::shared_ptr<ECS::Registry> &ecs,
    const std::shared_ptr<AssetManager> &textureManager, std::function<void(const SceneManager::Scene &)> switchToScene,
    sf::RenderWindow &window) : AScene(ecs, textureManager)
{
    this->_listEntity = (EntityFactory::createBackground(this->_registry, this->_assetsManager, "Settings"));
    this->_listEntity.push_back(EntityFactory::createButton(
        *this->_registry,
        Text(this->_assetsManager->fontManager->get("title_font"),
             sf::Color::White, 36, "Menu"),
        Position(100, 460),
        Rectangle({400, 75}, sf::Color::Blue, sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Settings Menu: " << e.what()
                          << std::endl;
            }
    })));
}

SettingsScene::~SettingsScene() {
    for (auto& entity : this->_listEntity) {
        this->_registry->killEntity(entity);
    }
}
