/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.cpp
*/

#include "SettingsScene.hpp"

#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "SceneManager/SceneException.hpp"

static std::vector<ECS::Entity> _createSection(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assets, const std::string& title, float x,
    float y, float width, float height) {
    std::vector<ECS::Entity> entities;
    auto bg = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(bg, x, y);
    registry->emplaceComponent<rtype::games::rtype::client::Rectangle>(
        bg, std::pair<float, float>{width, height}, sf::Color(0, 0, 0, 150),
        sf::Color(0, 0, 0, 150));

    if (registry->hasComponent<rtype::games::rtype::client::Rectangle>(bg)) {
        auto& rect =
            registry->getComponent<rtype::games::rtype::client::Rectangle>(bg);
        rect.outlineThickness = 2.0f;
        rect.outlineColor = sf::Color::White;
    }

    entities.push_back(bg);

    auto titleEnt = registry->spawnEntity();
    registry->emplaceComponent<rtype::games::rtype::shared::Position>(
        titleEnt, x + 20, y + 10);
    registry->emplaceComponent<rtype::games::rtype::client::Text>(
        titleEnt, assets->fontManager->get("title_font"), sf::Color::White, 30,
        title);
    registry->emplaceComponent<rtype::games::rtype::client::StaticTextTag>(
        titleEnt);
    entities.push_back(titleEnt);
    return entities;
}

void SettingsScene::_initKeybindSection() {
    std::vector<GameAction> actions = {
        GameAction::MOVE_UP,    GameAction::MOVE_DOWN, GameAction::MOVE_LEFT,
        GameAction::MOVE_RIGHT, GameAction::SHOOT,     GameAction::PAUSE};

    float sectionX = 50;
    float sectionY = 225;
    float sectionW = 600;
    float sectionH = 600;
    std::vector<ECS::Entity> sectionEntities = _createSection(
        this->_registry, this->_assetsManager, "Keyboard Assignment", sectionX,
        sectionY, sectionW, sectionH);
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

        auto btn = createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                this->_assetsManager->fontManager->get("title_font"),
                sf::Color::White, 24, textStr),
            rtype::games::rtype::shared::Position(x, y),
            rtype::games::rtype::client::Rectangle({500, 50}, sf::Color::Blue,
                                                   sf::Color::Red),
            std::function<void()>([this, action]() {
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
        y += 80;
    }
}

void SettingsScene::_initAudioSection() {
    float sectionX = 665;
    float sectionY = 225;
    float sectionW = 500;
    float sectionH = 200;

    std::vector<ECS::Entity> sectionEntities =
        _createSection(this->_registry, this->_assetsManager, "Audio", sectionX,
                       sectionY, sectionW, sectionH);
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());
}

void SettingsScene::_initWindowSection() {
    float sectionX = 665;
    float sectionY = 440;
    float sectionW = 500;
    float sectionH = 385;

    std::vector<ECS::Entity> sectionEntities =
        _createSection(this->_registry, this->_assetsManager, "Window",
                       sectionX, sectionY, sectionW, sectionH);
    this->_listEntity.insert(this->_listEntity.end(), sectionEntities.begin(),
                             sectionEntities.end());
}

void SettingsScene::update() {}

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
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<KeyboardActions> keybinds)
    : AScene(ecs, textureManager, window), _keybinds(keybinds) {
    this->_listEntity =
        (createBackground(this->_registry, this->_assetsManager, "Settings"));

    this->_initKeybindSection();
    this->_initAudioSection();
    this->_initWindowSection();

    this->_listEntity.push_back(createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            this->_assetsManager->fontManager->get("title_font"),
            sf::Color::White, 36, "Back"),
        rtype::games::rtype::shared::Position(100, 900),
        rtype::games::rtype::client::Rectangle({400, 75}, sf::Color::Blue,
                                               sf::Color::Red),
        std::function<void()>([switchToScene]() {
            try {
                switchToScene(SceneManager::MAIN_MENU);
            } catch (SceneNotFound& e) {
                std::cerr << "Error switching to Main Menu: " << e.what()
                          << std::endl;
            }
        })));
}

SettingsScene::~SettingsScene() {
    for (auto& entity : this->_listEntity) {
        this->_registry->killEntity(entity);
    }
}
