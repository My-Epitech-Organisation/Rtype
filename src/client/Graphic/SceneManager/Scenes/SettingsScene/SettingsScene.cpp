/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.cpp
*/

#include "SettingsScene.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "AudioLib/AudioLib.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/Accessibility.hpp"
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
        this->_registry, this->_assetsManager, "Input Bindings",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_keybindSectionEntities.insert(this->_keybindSectionEntities.end(),
                                         sectionEntities.begin(),
                                         sectionEntities.end());
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    float y = sectionY + 100;
    float x = sectionX + 50;

    for (auto action : actions) {
        std::string textStr = SettingsSceneUtils::actionToString(action) + ": ";

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

                InputMode mode = this->_keybinds->getInputMode();
                LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[SettingsScene] Button clicked for action: "
                          << static_cast<int>(action) << ", Mode: "
                          << (mode == InputMode::Keyboard ? "Keyboard"
                                                          : "Controller"));

                if (mode == InputMode::Keyboard ||
                    action == GameAction::SHOOT ||
                    action == GameAction::PAUSE ||
                    action == GameAction::CHANGE_AMMO ||
                    action == GameAction::MOVE_UP ||
                    action == GameAction::MOVE_DOWN) {
                    this->_actionToRebind = action;
                    LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[SettingsScene] Waiting for input for action: "
                              << static_cast<int>(action));
                    ECS::Entity entity = this->_actionButtons[action];
                    if (this->_registry
                            ->hasComponent<rtype::games::rtype::client::Text>(
                                entity)) {
                        auto& textComp = this->_registry->getComponent<
                            rtype::games::rtype::client::Text>(entity);
                        std::string waitText =
                            SettingsSceneUtils::actionToString(action) +
                            (mode == InputMode::Keyboard
                                 ? ": Press any key..."
                                 : ": Press any button...");
                        textComp.textContent = waitText;
                        textComp.text.setString(waitText);
                    }
                }
            }));
        this->_actionButtons[action] = btn;
        this->_keybindSectionEntities.push_back(btn);
        this->_listEntity.push_back(btn);
        y += 65;
    }

    this->_refreshKeybindSection();
}

void SettingsScene::_refreshKeybindSection() {
    InputMode mode = this->_keybinds->getInputMode();

    std::vector<GameAction> actions = {
        GameAction::MOVE_UP,    GameAction::MOVE_DOWN, GameAction::MOVE_LEFT,
        GameAction::MOVE_RIGHT, GameAction::SHOOT,     GameAction::CHANGE_AMMO,
        GameAction::PAUSE};

    for (auto action : actions) {
        if (this->_actionButtons.find(action) == this->_actionButtons.end())
            continue;

        ECS::Entity entity = this->_actionButtons[action];
        if (!this->_registry->hasComponent<rtype::games::rtype::client::Text>(
                entity))
            continue;

        auto& textComp =
            this->_registry->getComponent<rtype::games::rtype::client::Text>(
                entity);

        std::string textStr = SettingsSceneUtils::actionToString(action) + ": ";

        if (mode == InputMode::Keyboard) {
            auto keyOpt = this->_keybinds->getKeyBinding(action);
            textStr +=
                keyOpt ? SettingsSceneUtils::keyToString(*keyOpt) : "None";
        } else {
            auto btnOpt = this->_keybinds->getJoyButtonBinding(action);
            if (btnOpt.has_value()) {
                textStr += KeyboardActions::getXboxButtonName(*btnOpt);
            } else if (action == GameAction::MOVE_UP ||
                       action == GameAction::MOVE_DOWN ||
                       action == GameAction::MOVE_LEFT ||
                       action == GameAction::MOVE_RIGHT) {
                if (action == GameAction::MOVE_UP ||
                    action == GameAction::MOVE_DOWN) {
                    bool inverted =
                        this->_keybinds->isJoyAxisInverted(GameAction::MOVE_UP);
                    textStr +=
                        (inverted ? "Left Stick Y (Inverted)" : "Left Stick Y");
                } else {
                    textStr += "Left Stick X";
                }
            } else {
                textStr += "Not mapped";
            }
        }

        textComp.textContent = textStr;
        textComp.text.setString(textStr);
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

void SettingsScene::_initInputModeSection() {
    float sectionX = 1180;
    float sectionY = 50;
    float sectionW = 600;
    float sectionH = 150;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Input Device",
        sf::FloatRect(sf::Vector2f(sectionX, sectionY),
                      sf::Vector2f(sectionW, sectionH)));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "Keyboard"),
        rtype::games::rtype::shared::Position(sectionX + 50, sectionY + 60),
        rtype::games::rtype::client::Rectangle({200, 60}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([this]() {
            this->_keybinds->setInputMode(InputMode::Keyboard);
            this->_refreshInputModeLabel();
            this->_refreshKeybindSection();
        })));

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "Controller"),
        rtype::games::rtype::shared::Position(sectionX + 280, sectionY + 60),
        rtype::games::rtype::client::Rectangle({250, 60}, sf::Color::Blue,
                                               sf::Color::Red),
        this->_assetsManager, std::function<void()>([this]() {
            this->_keybinds->setInputMode(InputMode::Controller);
            this->_refreshInputModeLabel();
            this->_refreshKeybindSection();
        })));

    _inputModeLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Current: Keyboard", "main_font",
        sf::Vector2f(sectionX + sectionW - 220, sectionY + 20), 20);
    this->_listEntity.push_back(_inputModeLabel);

    this->_refreshInputModeLabel();
}

void SettingsScene::_refreshInputModeLabel() {
    if (!this->_registry->isAlive(_inputModeLabel)) return;
    if (!this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            _inputModeLabel))
        return;

    InputMode mode = this->_keybinds->getInputMode();
    std::string modeStr =
        (mode == InputMode::Keyboard) ? "Keyboard" : "Controller";

    auto& text =
        this->_registry->getComponent<rtype::games::rtype::client::Text>(
            _inputModeLabel);
    text.textContent = "Current: " + modeStr;
    text.text.setString(text.textContent);
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

    if (!this->_registry->hasSingleton<AccessibilitySettings>()) {
        this->_registry->setSingleton<AccessibilitySettings>(
            AccessibilitySettings{});
    }

    auto makeButton = [&](const std::string& label, float x, float y,
                          ColorBlindMode mode) {
        this->_listEntity.push_back(EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                this->_assetsManager->fontManager->get("main_font"),
                sf::Color::White, 24, label),
            rtype::games::rtype::shared::Position(x, y),
            rtype::games::rtype::client::Rectangle(
                {400, 55}, sf::Color(60, 60, 120), sf::Color(80, 80, 180)),
            this->_assetsManager,
            std::function<void()>([this, mode]() { _setColorMode(mode); })));
    };

    float startX = sectionX + 40;
    float startY = sectionY + 80;
    float gapY = 70;

    makeButton("Color: None", startX, startY + gapY * 0, ColorBlindMode::None);
    makeButton("Protanopia", startX, startY + gapY * 1,
               ColorBlindMode::Protanopia);
    makeButton("Deuteranopia", startX, startY + gapY * 2,
               ColorBlindMode::Deuteranopia);
    makeButton("Tritanopia", startX, startY + gapY * 3,
               ColorBlindMode::Tritanopia);
    makeButton("Achromatopsia (grayscale)", startX, startY + gapY * 4,
               ColorBlindMode::Achromatopsia);
    makeButton("High Contrast", startX, startY + gapY * 5,
               ColorBlindMode::HighContrast);

    float sliderY = startY + gapY * 6;
    float labelX = startX;
    float minusX = startX + 300;
    float plusX = startX + 420;

    float currentIntensity =
        this->_registry->getSingleton<AccessibilitySettings>().intensity;
    int percent = static_cast<int>(
        std::clamp(currentIntensity, 0.0f, 1.5f) * 100.f + 0.5f);

    this->_intensityLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "Intensity: " + std::to_string(percent) + "%", "main_font",
        sf::Vector2f(labelX, sliderY), 24);
    this->_listEntity.push_back(*this->_intensityLabel);

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "-"),
        rtype::games::rtype::shared::Position(minusX, sliderY - 10),
        rtype::games::rtype::client::Rectangle({60, 50}, sf::Color(40, 40, 90),
                                               sf::Color(70, 70, 140)),
        this->_assetsManager,
        std::function<void()>([this]() { _adjustColorIntensity(-0.1f); })));

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("main_font"),
            sf::Color::White, 28, "+"),
        rtype::games::rtype::shared::Position(plusX, sliderY - 10),
        rtype::games::rtype::client::Rectangle({60, 50}, sf::Color(40, 40, 90),
                                               sf::Color(70, 70, 140)),
        this->_assetsManager,
        std::function<void()>([this]() { _adjustColorIntensity(0.1f); })));
}

void SettingsScene::_setColorMode(ColorBlindMode mode) {
    auto& acc = this->_registry->hasSingleton<AccessibilitySettings>()
                    ? this->_registry->getSingleton<AccessibilitySettings>()
                    : this->_registry->setSingleton<AccessibilitySettings>(
                          AccessibilitySettings{});
    acc.colorMode = mode;
}

void SettingsScene::_adjustColorIntensity(float delta) {
    auto& acc = this->_registry->hasSingleton<AccessibilitySettings>()
                    ? this->_registry->getSingleton<AccessibilitySettings>()
                    : this->_registry->setSingleton<AccessibilitySettings>(
                          AccessibilitySettings{});
    acc.intensity = std::clamp(acc.intensity + delta, 0.0f, 1.5f);
    _refreshIntensityLabel();
}

void SettingsScene::_refreshIntensityLabel() {
    if (!this->_intensityLabel.has_value()) return;
    if (!this->_registry->isAlive(*this->_intensityLabel)) return;
    if (!this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            *this->_intensityLabel))
        return;

    float value =
        this->_registry->getSingleton<AccessibilitySettings>().intensity;
    int percent =
        static_cast<int>(std::clamp(value, 0.0f, 1.5f) * 100.f + 0.5f);

    auto& text =
        this->_registry->getComponent<rtype::games::rtype::client::Text>(
            *this->_intensityLabel);
    text.textContent = "Intensity: " + std::to_string(percent) + "%";
    text.text.setString(text.textContent);
}

void SettingsScene::update(float dt) {}

void SettingsScene::render(std::shared_ptr<sf::RenderWindow> window) {}

void SettingsScene::pollEvents(const sf::Event& e) {
    if (this->_actionToRebind.has_value()) {
        GameAction action = *this->_actionToRebind;
        InputMode mode = this->_keybinds->getInputMode();
        LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[SettingsScene] In rebind mode for action: "
                  << static_cast<int>(action));

        if (mode == InputMode::Keyboard) {
            if (const auto& keyEvent = e.getIf<sf::Event::KeyPressed>()) {
                LOG_DEBUG_CAT(::rtype::LogCategory::Input, "[SettingsScene] Keyboard key pressed: "
                          << static_cast<int>(keyEvent->code));
                sf::Keyboard::Key key = keyEvent->code;

                this->_keybinds->setKeyBinding(action, key);

                std::string keyName = SettingsSceneUtils::keyToString(key);
                std::string text =
                    SettingsSceneUtils::actionToString(action) + ": " + keyName;

                ECS::Entity entity = this->_actionButtons[action];
                if (this->_registry
                        ->hasComponent<rtype::games::rtype::client::Text>(
                            entity)) {
                    auto& textComp =
                        this->_registry
                            ->getComponent<rtype::games::rtype::client::Text>(
                                entity);
                    textComp.textContent = text;
                    textComp.text.setString(text);
                }

                this->_actionToRebind = std::nullopt;
            }
        } else if (mode == InputMode::Controller) {
            if (const auto& btnEvent =
                    e.getIf<sf::Event::JoystickButtonPressed>()) {
                unsigned int button = btnEvent->button;
                LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                    "[SettingsScene] Controller button pressed: " << button);

                if (action == GameAction::SHOOT ||
                    action == GameAction::PAUSE ||
                    action == GameAction::CHANGE_AMMO) {
                    this->_keybinds->setJoyButtonBinding(action, button);

                    std::string buttonName =
                        KeyboardActions::getXboxButtonName(button);
                    std::string text =
                        SettingsSceneUtils::actionToString(action) + ": " +
                        buttonName;

                    ECS::Entity entity = this->_actionButtons[action];
                    if (this->_registry
                            ->hasComponent<rtype::games::rtype::client::Text>(
                                entity)) {
                        auto& textComp = this->_registry->getComponent<
                            rtype::games::rtype::client::Text>(entity);
                        textComp.textContent = text;
                        textComp.text.setString(text);
                    }

                    this->_actionToRebind = std::nullopt;
                } else if (action == GameAction::MOVE_UP ||
                           action == GameAction::MOVE_DOWN) {
                    bool currentInvert =
                        this->_keybinds->isJoyAxisInverted(GameAction::MOVE_UP);
                    this->_keybinds->setJoyAxisInverted(GameAction::MOVE_UP,
                                                        !currentInvert);
                    this->_keybinds->setJoyAxisInverted(GameAction::MOVE_DOWN,
                                                        !currentInvert);

                    std::string text =
                        SettingsSceneUtils::actionToString(action) +
                        ((!currentInvert) ? ": Left Stick Y (Inverted)"
                                          : ": Left Stick Y");

                    ECS::Entity entity = this->_actionButtons[action];
                    if (this->_registry
                            ->hasComponent<rtype::games::rtype::client::Text>(
                                entity)) {
                        auto& textComp = this->_registry->getComponent<
                            rtype::games::rtype::client::Text>(entity);
                        textComp.textContent = text;
                        textComp.text.setString(text);
                    }

                    this->_actionToRebind = std::nullopt;
                }
            }
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
    this->_initInputModeSection();

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
                LOG_ERROR_CAT(::rtype::LogCategory::UI, std::string("Error switching to Main Menu: ") +
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
