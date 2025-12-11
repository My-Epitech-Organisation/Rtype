/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.cpp
*/

#include "SettingsScene.hpp"

#include "AudioLib/AudioLib.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

void SettingsScene::_initKeybindSection() {
    std::vector<GameAction> actions = {
        GameAction::MOVE_UP,    GameAction::MOVE_DOWN, GameAction::MOVE_LEFT,
        GameAction::MOVE_RIGHT, GameAction::SHOOT,     GameAction::CHANGE_AMMO,
        GameAction::PAUSE};

    float sectionX = 50;
    float sectionY = 225;
    float sectionW = 600;
    float sectionH = 600;
    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Keyboard Assignment",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    float y = sectionY + 100;
    float x = sectionX + 50;

    for (auto action : actions) {
        auto keyOpt = this->_keybinds->getKeyBinding(action);
        std::string keyName =
            keyOpt ? SettingsSceneUtils::keyToString(*keyOpt) : "None";
        std::string textStr =
            SettingsSceneUtils::actionToString(action) + ": " + keyName;

        auto btn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                this->_assetsManager->fontManager->get("main_font"),
                sf::Color::White, 24, textStr),
            rtype::games::rtype::shared::Position(x, y),
            rtype::games::rtype::client::Rectangle({500, 50}, sf::Color::Blue,
                                                   sf::Color::Red),
            this->_assetsManager, std::function<void()>([this, action]() {
                if (this->_actionToRebind.has_value()) return;
                this->_actionToRebind = action;
                ECS::Entity entity = this->_actionButtons[action];
                if (this->_registry
                        ->hasComponent<rtype::games::rtype::client::Text>(
                            entity)) {
                    auto& textComp =
                        this->_registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                entity);
                    std::string waitText =
                        SettingsSceneUtils::actionToString(action) +
                        ": Press any key...";
                    textComp.textContent = waitText;
                    textComp.text.setString(waitText);
                }
            }));
        this->_actionButtons[action] = btn;
        this->_listEntity.push_back(btn);
        y += 65;
    }
}

void SettingsScene::_initAudioSection() {
    float sectionX = 665;
    float sectionY = 225;
    float sectionW = 500;
    float sectionH = 200;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Audio",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    auto& font = this->_assetsManager->fontManager->get("main_font");
    float startY = sectionY + 70;
    float gapY = 60;

    auto createVolumeControl = [&](std::string label, float y, bool isMusic) {
        float labelX = sectionX + 30;
        float minusX = sectionX + 250;
        float valueX = sectionX + 320;
        float plusX = sectionX + 400;

        auto valueEntity = this->_registry->spawnEntity();
        float currentVol = isMusic ? this->_audio->getMusicVolume()
                                   : this->_audio->getSFXVolume();
        std::string textStr =
            label + ": " + std::to_string(static_cast<int>(currentVol));

        this->_registry->emplaceComponent<rtype::games::rtype::client::Text>(
            valueEntity, rtype::games::rtype::client::Text(
                             font, sf::Color::White, 24, textStr));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
                valueEntity);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::Position>(
                valueEntity,
                rtype::games::rtype::shared::Position(labelX, y + 10));
        this->_listEntity.push_back(valueEntity);

        auto minusBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(font, sf::Color::White, 24, "-"),
            rtype::games::rtype::shared::Position(minusX, y),
            rtype::games::rtype::client::Rectangle({50, 50}, sf::Color::Blue,
                                                   sf::Color::Red),
            this->_assetsManager,
            std::function<void()>([this, valueEntity, isMusic, label]() {
                float vol = isMusic ? this->_audio->getMusicVolume()
                                    : this->_audio->getSFXVolume();
                vol = std::max(0.0f, vol - 5.0f);
                if (isMusic)
                    this->_audio->setMusicVolume(vol);
                else
                    this->_audio->setSFXVolume(vol);

                if (this->_registry
                        ->hasComponent<rtype::games::rtype::client::Text>(
                            valueEntity)) {
                    auto& textComp =
                        this->_registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                valueEntity);
                    std::string s =
                        label + ": " + std::to_string(static_cast<int>(vol));
                    textComp.textContent = s;
                    textComp.text.setString(s);
                }
            }));
        this->_listEntity.push_back(minusBtn);

        auto plusBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(font, sf::Color::White, 24, "+"),
            rtype::games::rtype::shared::Position(plusX, y),
            rtype::games::rtype::client::Rectangle({50, 50}, sf::Color::Blue,
                                                   sf::Color::Red),
            this->_assetsManager,
            std::function<void()>([this, valueEntity, isMusic, label]() {
                float vol = isMusic ? this->_audio->getMusicVolume()
                                    : this->_audio->getSFXVolume();
                vol = std::min(100.0f, vol + 5.0f);
                if (isMusic)
                    this->_audio->setMusicVolume(vol);
                else
                    this->_audio->setSFXVolume(vol);

                if (this->_registry
                        ->hasComponent<rtype::games::rtype::client::Text>(
                            valueEntity)) {
                    auto& textComp =
                        this->_registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                valueEntity);
                    std::string s =
                        label + ": " + std::to_string(static_cast<int>(vol));
                    textComp.textContent = s;
                    textComp.text.setString(s);
                }
            }));
        this->_listEntity.push_back(plusBtn);
    };

    createVolumeControl("Music", startY, true);
    createVolumeControl("SFX", startY + gapY, false);
}

void SettingsScene::_initWindowSection() {
    float sectionX = 665;
    float sectionY = 440;
    float sectionW = 500;
    float sectionH = 385;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Window",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());
}

void SettingsScene::_initAccessibilitySection() {
    float sectionX = 1180;
    float sectionY = 225;
    float sectionW = 600;
    float sectionH = 600;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Accessibility",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());
}

void SettingsScene::update(float dt) {}

void SettingsScene::render(std::shared_ptr<sf::RenderWindow> window) {}

void SettingsScene::pollEvents(const sf::Event& e) {
    if (this->_actionToRebind.has_value()) {
        if (const auto& keyEvent = e.getIf<sf::Event::KeyPressed>()) {
            sf::Keyboard::Key key = keyEvent->code;

            this->_keybinds->setKeyBinding(*this->_actionToRebind, key);

            std::string keyName = SettingsSceneUtils::keyToString(key);
            std::string text =
                SettingsSceneUtils::actionToString(*this->_actionToRebind) +
                ": " + keyName;

            ECS::Entity entity = this->_actionButtons[*this->_actionToRebind];
            if (this->_registry
                    ->hasComponent<rtype::games::rtype::client::Text>(entity)) {
                auto& textComp =
                    this->_registry
                        ->getComponent<rtype::games::rtype::client::Text>(
                            entity);
                textComp.textContent = text;
                textComp.text.setString(text);
            }

            this->_actionToRebind = std::nullopt;
        }
    }
}

SettingsScene::SettingsScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, textureManager, window, audio), _keybinds(keybinds) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Settings"));

    this->_initKeybindSection();
    this->_initAudioSection();
    this->_initWindowSection();
    this->_initAccessibilitySection();

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 36, "Back"),
        rtype::games::rtype::shared::Position(100, 900),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Main Menu: ") +
                          std::string(e.what()));
            }
        })));

    this->_assetsManager->audioManager->load(
        "main_settings_music",
        this->_assetsManager->configGameAssets.assets.music.settings);
    auto settings =
        this->_assetsManager->audioManager->get("main_settings_music");
    this->_audio->loadMusic(settings);
    this->_audio->setLoop(true);
    this->_audio->play();
}
