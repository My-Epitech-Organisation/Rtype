/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** HowToPlayScene.cpp
*/

#include "HowToPlayScene.hpp"

#include "Components/SizeComponent.hpp"
#include "Components/TextureRectComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/SceneManager/Scenes/SettingsScene/SettingsSceneUtils.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

namespace {
static constexpr float kControlsSectionX = 100.f;
static constexpr float kControlsSectionY = 200.f;
static constexpr float kControlsSectionW = 800.f;
static constexpr float kControlsSectionH = 450.f;

static constexpr float kPowerupsSectionX = 950.f;
static constexpr float kPowerupsSectionY = 200.f;
static constexpr float kPowerupsSectionW = 870.f;
static constexpr float kPowerupsSectionH = 700.f;
}  // namespace

HowToPlayScene::HowToPlayScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(std::move(ecs), std::move(assetsManager), std::move(window),
             std::move(audio)),
      _keybinds(std::move(keybinds)),
      _switchToScene(std::move(switchToScene)) {
    this->_listEntity = EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "How to Play", nullptr);
    this->_initLayout();
    auto backBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 36, "Back"),
        rtype::games::rtype::shared::TransformComponent(100, 900),
        rtype::games::rtype::client::Rectangle({400, 75},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            try {
                this->_switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                LOG_ERROR_CAT(::rtype::LogCategory::UI,
                              std::string("Error switching to Main Menu: ") +
                                  std::string(e.what()));
            }
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        backBtn, 1);
    this->_listEntity.push_back(backBtn);
}

std::string HowToPlayScene::_keyName(GameAction action) const {
    auto key = _keybinds->getKeyBinding(action);
    if (!key.has_value()) return "Unbound";
    return SettingsSceneUtils::keyToString(*key);
}

void HowToPlayScene::_initLayout() {
    std::vector<ECS::Entity> controlsSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Controls",
        rtype::display::Rect<float>(kControlsSectionX, kControlsSectionY,
                                    kControlsSectionW, kControlsSectionH));
    this->_listEntity.insert(this->_listEntity.end(), controlsSection.begin(),
                             controlsSection.end());

    float textX = kControlsSectionX + 30.f;
    float startY = kControlsSectionY + 80.f;
    float lineGap = 50.f;

    const std::vector<std::string> lines = {
        "Objective: survive waves and destroy Bydos.",
        "Move: " + _keyName(GameAction::MOVE_UP) + "/" +
            _keyName(GameAction::MOVE_DOWN) + "/" +
            _keyName(GameAction::MOVE_LEFT) + "/" +
            _keyName(GameAction::MOVE_RIGHT),
        "Shoot: " + _keyName(GameAction::SHOOT),
        "Change ammo: " + _keyName(GameAction::CHANGE_AMMO),
        "Pause: " + _keyName(GameAction::PAUSE),
        "Tips: stay centered, watch colored outlines."};

    auto fontId = "main_font";
    for (std::size_t i = 0; i < lines.size(); ++i) {
        float y = startY + static_cast<float>(i) * lineGap;
        auto text = EntityFactory::createStaticText(
            this->_registry, this->_assetsManager, lines[i], fontId,
            rtype::display::Vector2<float>{textX, y}, 22.f);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            text, 2);
        this->_listEntity.push_back(text);
    }

    std::vector<ECS::Entity> powerupsSection = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "Power-Ups",
        rtype::display::Rect<float>(kPowerupsSectionX, kPowerupsSectionY,
                                    kPowerupsSectionW, kPowerupsSectionH));
    this->_listEntity.insert(this->_listEntity.end(), powerupsSection.begin(),
                             powerupsSection.end());

    struct PowerupInfo {
        std::string textureId;
        std::string name;
        std::string description;
    };

    const std::vector<PowerupInfo> powerups = {
        {"health_small", "Health Pack", "Restores 25 HP instantly"},
        {"health_large", "Large Health", "Restores 75 HP instantly"},
        {"speed_boost", "Speed Boost", "50% faster for 10s"},
        {"weapon_upgrade", "Weapon Up", "Permanent weapon upgrade"},
        {"shield", "Shield", "100 HP shield for 15s"},
        {"rapid_fire", "Rapid Fire", "Faster firing for 12s"},
        {"double_damage", "Double Damage", "2x damage for 15s"},
        {"extra_life", "Extra Life", "+1 life"},
        {"force_pod", "Force Pod", "Orbiting attack drone"},
        {"laser_upgrade", "Laser", "Unlocks laser beam"}};

    float puStartY = kPowerupsSectionY + 60.f;
    float puLineGap = 48.f;
    float iconX = kPowerupsSectionX + 20.f;
    float nameX = kPowerupsSectionX + 70.f;
    float descX = kPowerupsSectionX + 220.f;
    constexpr float targetIconSize = 40.f;
    constexpr float textVerticalOffset = 35.f;

    for (std::size_t i = 0; i < powerups.size(); ++i) {
        float y = puStartY + static_cast<float>(i) * puLineGap;
        float iconY = y;
        if (powerups[i].textureId == "force_pod") {
            iconY += 10.f;
        }

        auto icon = EntityFactory::createStaticImage(
            this->_registry, powerups[i].textureId, {iconX, iconY}, 1.0f);
        try {
            auto texture = this->_assetsManager->textureManager->get(
                powerups[i].textureId);
            auto texSize = texture->getSize();

            int frameWidth;
            int frameHeight;
            constexpr int numFrames = 4;

            if (powerups[i].textureId == "force_pod") {
                frameWidth = 16;
                frameHeight = 16;
            } else {
                frameWidth = static_cast<int>(texSize.x) / numFrames;
                frameHeight = static_cast<int>(texSize.y);
            }
            int frameOffsetX =
                (powerups[i].textureId == "laser_upgrade") ? frameWidth * 2 : 0;
            this->_registry
                ->emplaceComponent<rtype::games::rtype::client::TextureRect>(
                    icon, std::pair<int, int>{frameOffsetX, 0},
                    std::pair<int, int>{frameWidth, frameHeight});
            float scale = targetIconSize / static_cast<float>(frameWidth);
            this->_registry
                ->getComponent<rtype::games::rtype::client::Size>(icon)
                .x = scale;
            this->_registry
                ->getComponent<rtype::games::rtype::client::Size>(icon)
                .y = scale;
        } catch (const std::exception& e) {
            LOG_WARNING_CAT(
                ::rtype::LogCategory::UI,
                "Could not get texture for powerup: " + powerups[i].textureId);
        }

        this->_listEntity.push_back(icon);

        auto name = EntityFactory::createStaticText(
            this->_registry, this->_assetsManager, powerups[i].name, fontId,
            {nameX, y + textVerticalOffset}, 18.f);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            name, 2);
        this->_listEntity.push_back(name);

        auto desc = EntityFactory::createStaticText(
            this->_registry, this->_assetsManager, powerups[i].description,
            fontId, {descX, y + textVerticalOffset}, 16.f);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            desc, 2);
        this->_listEntity.push_back(desc);
    }
}

void HowToPlayScene::pollEvents(const rtype::display::Event& e) {
    if (e.type == rtype::display::EventType::KeyPressed) {
        if (e.key.code == rtype::display::Key::Escape) {
            try {
                _switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& err) {
                LOG_ERROR_CAT(::rtype::LogCategory::UI,
                              std::string("Error switching to Main Menu: ") +
                                  std::string(err.what()));
            }
        }
    }
}

void HowToPlayScene::update(float /*dt*/) {}

void HowToPlayScene::render(
    std::shared_ptr<rtype::display::IDisplay> /*window*/) {}
