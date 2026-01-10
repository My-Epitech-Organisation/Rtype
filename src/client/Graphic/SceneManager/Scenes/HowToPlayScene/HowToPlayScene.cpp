/*registry
** EPITECH PROJECT, 2025
** r-type
** File description:
** HowToPlayScene.cpp
*/

#include "HowToPlayScene.hpp"

#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic/SceneManager/Scenes/SettingsScene/SettingsSceneUtils.hpp"
#include "Logger/Macros.hpp"
#include "SceneManager/SceneException.hpp"

namespace {
static constexpr float kSectionX = 350.f;
static constexpr float kSectionY = 250.f;
static constexpr float kSectionW = 1220.f;
static constexpr float kSectionH = 600.f;
}  // namespace

HowToPlayScene::HowToPlayScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(std::move(ecs), std::move(assetsManager), std::move(window),
             std::move(audio)),
      _keybinds(std::move(keybinds)),
      _switchToScene(std::move(switchToScene)) {
    this->_listEntity = EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "How to Play");
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
    std::vector<ECS::Entity> sectionEntities = EntityFactory::createSection(
        this->_registry, this->_assetsManager, "How to Play",
        rtype::display::Rect<float>(kSectionX, kSectionY, kSectionW,
                                    kSectionH));
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());

    float textX = kSectionX + kSectionW / 2;
    float startY = kSectionY + kSectionH / 2 - 150.f;
    float lineGap = 55.f;

    const std::vector<std::string> lines = {
        "Objective: survive waves and destroy Bydos ships.",
        "Movement: " + _keyName(GameAction::MOVE_UP) + "/" +
            _keyName(GameAction::MOVE_DOWN) + " / " +
            _keyName(GameAction::MOVE_LEFT) + " / " +
            _keyName(GameAction::MOVE_RIGHT),
        "Shoot: press " + _keyName(GameAction::SHOOT) +
            " to fire your current weapon.",
        "Change ammo: use " + _keyName(GameAction::CHANGE_AMMO) +
            " to cycle special shots.",
        "Pause: press " + _keyName(GameAction::PAUSE) +
            " or use the pause menu button.",
        "Visual cues: flashes mark shots and explosions even when muted.",
        "Tips: stay centered, watch the colored outlines for enemies/ally."};

    auto fontId = "main_font";
    for (std::size_t i = 0; i < lines.size(); ++i) {
        float y = startY + static_cast<float>(i) * lineGap;
        auto text = EntityFactory::createStaticText(
            this->_registry, this->_assetsManager, lines[i], fontId,
            rtype::display::Vector2<float>{textX, y}, 28.f);
        this->_registry
            ->emplaceComponent<rtype::games::rtype::client::CenteredTextTag>(
                text);
        this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
            text, 2);
        this->_listEntity.push_back(text);
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
