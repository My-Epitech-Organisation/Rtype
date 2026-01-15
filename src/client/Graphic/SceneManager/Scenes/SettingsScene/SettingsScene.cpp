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
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/Accessibility.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

SettingsScene::SettingsScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(ecs, textureManager, window, audio), _keybinds(keybinds) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Settings", nullptr));

    this->_initKeybindSection();
    this->_initAudioSection();
    this->_initWindowSection();
    this->_initAccessibilitySection();
    this->_initInputModeSection();

    this->_listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, "Back"),
        rtype::games::rtype::shared::TransformComponent(100.f, 900.f),
        rtype::games::rtype::client::Rectangle({400.f, 75.f},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR(std::string("Error switching to Main Menu: ") +
                          std::string(e.what()));
            }
        })));
    this->_listEntity.push_back(EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "Loaded display lib: " + this->_window->getLibName(), "main_font",
        rtype::display::Vector2<float>(1500.f, 932.5f), 24));

    this->_assetsManager->audioManager->load(
        "main_settings_music",
        this->_assetsManager->configGameAssets.assets.music.settings);
    auto settings =
        this->_assetsManager->audioManager->get("main_settings_music");
    this->_audio->loadMusic(settings);
    this->_audio->setLoop(true);
    this->_audio->play();
}

void SettingsScene::_initKeybindSection() {
    std::vector<GameAction> actions = {
        GameAction::MOVE_UP,   GameAction::MOVE_DOWN,
        GameAction::MOVE_LEFT, GameAction::MOVE_RIGHT,
        GameAction::SHOOT,     GameAction::CHARGE_SHOT,
        GameAction::FORCE_POD, GameAction::CHANGE_AMMO,
        GameAction::PAUSE,     GameAction::TOGGLE_LOW_BANDWIDTH};

    float sectionX = 50;
    float sectionY = 180;
    float sectionW = 550;
    float sectionH = 680;
    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Input Bindings",
        rtype::display::Rect<float>(sectionX, sectionY, sectionW, sectionH));
    this->_keybindSectionEntities.insert(this->_keybindSectionEntities.end(),
                                         sectionEntities.begin(),
                                         sectionEntities.end());
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    float y = sectionY + 80;
    float x = sectionX + 25;

    for (auto action : actions) {
        std::string textStr = SettingsSceneUtils::actionToString(action) + ": ";

        auto btn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                "main_font", rtype::display::Color::White(), 18, textStr),
            rtype::games::rtype::shared::TransformComponent(x, y),
            rtype::games::rtype::client::Rectangle(
                {500, 45}, rtype::display::Color::Blue(),
                rtype::display::Color::Red()),
            this->_assetsManager, std::function<void()>([this, action]() {
                if (this->_actionToRebind.has_value()) return;

                InputMode mode = this->_keybinds->getInputMode();
                LOG_DEBUG_CAT(
                    ::rtype::LogCategory::Input,
                    "[SettingsScene] Button clicked for action: "
                        << static_cast<int>(action) << ", Mode: "
                        << (mode == InputMode::Keyboard ? "Keyboard"
                                                        : "Controller"));

                if (mode == InputMode::Keyboard ||
                    action == GameAction::SHOOT ||
                    action == GameAction::CHARGE_SHOT ||
                    action == GameAction::FORCE_POD ||
                    action == GameAction::PAUSE ||
                    action == GameAction::CHANGE_AMMO ||
                    action == GameAction::MOVE_UP ||
                    action == GameAction::MOVE_DOWN) {
                    this->_actionToRebind = action;
                    LOG_DEBUG_CAT(
                        ::rtype::LogCategory::Input,
                        "[SettingsScene] Waiting for input for action: "
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
                    }
                }
            }));
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            btn, 2);
        this->_actionButtons[action] = btn;
        this->_keybindSectionEntities.push_back(btn);
        this->_listEntity.push_back(btn);
        y += 55;
    }

    this->_refreshKeybindSection();
}

void SettingsScene::_refreshKeybindSection() {
    InputMode mode = this->_keybinds->getInputMode();

    std::vector<GameAction> actions = {
        GameAction::MOVE_UP,   GameAction::MOVE_DOWN,
        GameAction::MOVE_LEFT, GameAction::MOVE_RIGHT,
        GameAction::SHOOT,     GameAction::CHARGE_SHOT,
        GameAction::FORCE_POD, GameAction::CHANGE_AMMO,
        GameAction::PAUSE,     GameAction::TOGGLE_LOW_BANDWIDTH};

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
    }
}

void SettingsScene::_initAudioSection() {
    float sectionX = 665;
    float sectionY = 225;
    float sectionW = 500;
    float sectionH = 200;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Audio",
        rtype::display::Rect<float>(sectionX, sectionY, sectionW, sectionH));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    auto font = "main_font";
    float startY = sectionY + 70;
    float gapY = 60;

    auto createVolumeControl = [&](std::string label, float y, bool isMusic) {
        float labelX = sectionX + 30;
        float minusX = sectionX + 250;
        float plusX = sectionX + 400;

        auto valueEntity = this->_registry->spawnEntity();
        float currentVol = isMusic ? this->_audio->getMusicVolume()
                                   : this->_audio->getSFXVolume();
        std::string textStr =
            label + ": " + std::to_string(static_cast<int>(currentVol));

        this->_registry->emplaceComponent<rtype::games::rtype::client::Text>(
            valueEntity,
            rtype::games::rtype::client::Text(
                font, rtype::display::Color::White(), 24, textStr));
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
                valueEntity);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::shared::TransformComponent>(
                valueEntity, rtype::games::rtype::shared::TransformComponent(
                                 labelX, y + 10));
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            valueEntity, 1);
        this->_listEntity.push_back(valueEntity);

        auto minusBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                font, rtype::display::Color::White(), 24, "-"),
            rtype::games::rtype::shared::TransformComponent(minusX, y),
            rtype::games::rtype::client::Rectangle(
                {50, 50}, rtype::display::Color::Blue(),
                rtype::display::Color::Red()),
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
                }
            }));
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            minusBtn, 1);
        this->_listEntity.push_back(minusBtn);

        auto plusBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                font, rtype::display::Color::White(), 24, "+"),
            rtype::games::rtype::shared::TransformComponent(plusX, y),
            rtype::games::rtype::client::Rectangle(
                {50, 50}, rtype::display::Color::Blue(),
                rtype::display::Color::Red()),
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
                }
            }));
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            plusBtn, 1);
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
        rtype::display::Rect<float>(sectionX, sectionY, sectionW, sectionH));

    sectionEntities.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text("main_font",
                                          rtype::display::Color::White(), 24,
                                          "Toggle Fullscreen"),
        rtype::games::rtype::shared::TransformComponent(sectionX + 50,
                                                        sectionY + 80),
        rtype::games::rtype::client::Rectangle({400, 60},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            bool isFullscreen = this->_window->isFullscreen();
            this->_window->setFullscreen(!isFullscreen);
        })));

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
        rtype::display::Rect<float>(sectionX, sectionY, sectionW, sectionH));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    auto keyboardBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 28, "Keyboard"),
        rtype::games::rtype::shared::TransformComponent(sectionX + 50,
                                                        sectionY + 60),
        rtype::games::rtype::client::Rectangle({200, 60},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            this->_keybinds->setInputMode(InputMode::Keyboard);
            this->_refreshInputModeLabel();
            this->_refreshKeybindSection();
        }));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        keyboardBtn, 1);
    this->_listEntity.push_back(keyboardBtn);

    auto controllerBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 28, "Controller"),
        rtype::games::rtype::shared::TransformComponent(sectionX + 280,
                                                        sectionY + 60),
        rtype::games::rtype::client::Rectangle({250, 60},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            this->_keybinds->setInputMode(InputMode::Controller);
            this->_refreshInputModeLabel();
            this->_refreshKeybindSection();
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        controllerBtn, 1);
    this->_listEntity.push_back(controllerBtn);

    this->_inputModeLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager, "Current: Keyboard", "main_font",
        rtype::display::Vector2<float>(sectionX + sectionW - 215,
                                       sectionY + 35),
        20);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        this->_inputModeLabel, 1);
    this->_listEntity.push_back(this->_inputModeLabel);

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
}

void SettingsScene::_initAccessibilitySection() {
    float sectionX = 1180;
    float sectionY = 225;
    float sectionW = 600;
    float sectionH = 600;

    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Accessibility",
        rtype::display::Rect<float>(sectionX, sectionY, sectionW, sectionH));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    if (!this->_registry->hasSingleton<AccessibilitySettings>()) {
        this->_registry->setSingleton<AccessibilitySettings>(
            AccessibilitySettings{});
    }

    auto makeButton = [&](const std::string& label, float x, float y,
                          ColorBlindMode mode) {
        auto btn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                "main_font", rtype::display::Color::White(), 24, label),
            rtype::games::rtype::shared::TransformComponent(x, y),
            rtype::games::rtype::client::Rectangle(
                {400, 55}, rtype::display::Color(60, 60, 120, 255),
                rtype::display::Color(80, 80, 180, 255)),
            this->_assetsManager,
            std::function<void()>([this, mode]() { _setColorMode(mode); }));
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            btn, 1);
        this->_listEntity.push_back(btn);
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
    float labelX = startX + strlen("Intensity") / 2 * 24;
    float minusX = startX + 280;
    float plusX = startX + 340;

    float currentIntensity =
        this->_registry->getSingleton<AccessibilitySettings>().intensity;
    int percent = static_cast<int>(
        std::clamp(currentIntensity, 0.0f, 1.5f) * 100.f + 0.5f);

    this->_intensityLabel = EntityFactory::createStaticText(
        this->_registry, this->_assetsManager,
        "Intensity: " + std::to_string(percent) + "%", "main_font",
        rtype::display::Vector2<float>(labelX, sliderY + 50 / 2), 24);
    this->_registry
        ->emplaceComponent<rtype::games::rtype::client::CenteredTextTag>(
            *this->_intensityLabel);
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        *this->_intensityLabel, 1);
    this->_listEntity.push_back(*this->_intensityLabel);

    auto btnMinus = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 28, "-"),
        rtype::games::rtype::shared::TransformComponent(minusX, sliderY),
        rtype::games::rtype::client::Rectangle(
            {60, 50}, rtype::display::Color(40, 40, 90, 255),
            rtype::display::Color(70, 70, 140, 255)),
        this->_assetsManager,
        std::function<void()>([this]() { _adjustColorIntensity(-0.1f); }));

    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnMinus, 1);
    this->_listEntity.push_back(btnMinus);

    auto btnPlus = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 28, "+"),
        rtype::games::rtype::shared::TransformComponent(plusX, sliderY),
        rtype::games::rtype::client::Rectangle(
            {60, 50}, rtype::display::Color(40, 40, 90, 255),
            rtype::display::Color(70, 70, 140, 255)),
        this->_assetsManager,
        std::function<void()>([this]() { _adjustColorIntensity(0.1f); }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnPlus, 1);
    this->_listEntity.push_back(btnPlus);
}

void SettingsScene::pollEvents(const ::rtype::display::Event& e) {
    if (this->_actionToRebind.has_value()) {
        GameAction action = *this->_actionToRebind;
        InputMode mode = this->_keybinds->getInputMode();
        LOG_DEBUG("[SettingsScene] In rebind mode for action: "
                  << static_cast<int>(action));

        if (mode == InputMode::Keyboard) {
            if (e.type == rtype::display::EventType::KeyPressed) {
                auto key = e.key.code;
                if (key != rtype::display::Key::Escape) {
                    this->_keybinds->setKeyBinding(action, key);
                    LOG_DEBUG("[SettingsScene] Rebound action "
                              << static_cast<int>(action) << " to key "
                              << static_cast<int>(key));
                }
                this->_actionToRebind.reset();
                this->_refreshKeybindSection();
            }
        } else {
            if (e.type == rtype::display::EventType::JoystickButtonPressed) {
                unsigned int btn = e.joystickButton.button;
                this->_keybinds->setJoyButtonBinding(action, btn);
                LOG_DEBUG("[SettingsScene] Rebound action "
                          << static_cast<int>(action) << " to button " << btn);
                this->_actionToRebind.reset();
                this->_refreshKeybindSection();
            }
        }
    }
}

void SettingsScene::update(float dt) {
    (void)dt;
    if (this->_audio) {
        this->_audio->update();
    }
}

void SettingsScene::render(std::shared_ptr<rtype::display::IDisplay> window) {
    (void)window;
}

void SettingsScene::_setColorMode(ColorBlindMode mode) {
    auto& settings = this->_registry->getSingleton<AccessibilitySettings>();
    settings.colorMode = mode;
    // Accessibility::apply(this->_registry, settings);
}

void SettingsScene::_adjustColorIntensity(float delta) {
    auto& settings = this->_registry->getSingleton<AccessibilitySettings>();
    settings.intensity = std::clamp(settings.intensity + delta, 0.0f, 1.5f);
    // Accessibility::apply(this->_registry, settings);
    _refreshIntensityLabel();
}

void SettingsScene::_refreshIntensityLabel() {
    if (!this->_intensityLabel.has_value() ||
        !this->_registry->isAlive(*this->_intensityLabel))
        return;

    if (!this->_registry->hasComponent<rtype::games::rtype::client::Text>(
            *this->_intensityLabel))
        return;

    auto& settings = this->_registry->getSingleton<AccessibilitySettings>();
    int percent = static_cast<int>(settings.intensity * 100.f + 0.5f);

    auto& text =
        this->_registry->getComponent<rtype::games::rtype::client::Text>(
            *this->_intensityLabel);
    text.textContent = "Intensity: " + std::to_string(percent) + "%";
}
