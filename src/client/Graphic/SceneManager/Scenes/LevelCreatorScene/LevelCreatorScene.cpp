/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LevelCreatorScene.cpp
*/

#include "LevelCreatorScene.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Components/HiddenComponent.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/TextInputComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"

void LevelCreatorScene::createSection(
    const std::string& id, const std::string& title,
    const rtype::display::Rect<float>& bounds) {
    ScrollSection section;
    section.id = id;
    section.bounds = bounds;

    auto sectionEnts = EntityFactory::createSection(_registry, _assetsManager,
                                                    title, bounds, 0);
    _uiEntities.insert(_uiEntities.end(), sectionEnts.begin(),
                       sectionEnts.end());
    _sections.push_back(section);
}

void LevelCreatorScene::addElementToSection(const std::string& sectionId,
                                            ECS::Entity entity,
                                            float relativeY) {
    for (auto& sec : _sections) {
        if (sec.id == sectionId) {
            sec.entities.push_back({entity, relativeY});
            _uiEntities.push_back(entity);

            if (_registry->hasComponent<
                    rtype::games::rtype::shared::TransformComponent>(entity)) {
                _registry
                    ->getComponent<
                        rtype::games::rtype::shared::TransformComponent>(entity)
                    .y = sec.bounds.top + relativeY - sec.currentScroll;
            }
            return;
        }
    }
}

LevelCreatorScene::LevelCreatorScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<KeyboardActions> keybinds, std::shared_ptr<AudioLib> audio,
    std::map<std::string, std::shared_ptr<IBackground>> libBackgrounds,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const SceneManager::Scene&)> switchToScene)
    : AScene(std::move(ecs), std::move(assetsManager), window,
             std::move(audio)),
      _textInputSystem(
          std::make_shared<rtype::games::rtype::client::TextInputSystem>(
              window)),
      _switchToScene(std::move(switchToScene)),
      _libBackgrounds(std::move(libBackgrounds)) {
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Level Creator", nullptr));

    this->_bgIteratorFst = _libBackgrounds.begin();
    this->_bgIteratorCurrent = _libBackgrounds.begin();

    this->_bgPluginName = this->_bgIteratorCurrent->first;

    float startX = kLevelSectionPosLeft;
    float startY = kLevelSectionPosTop;

    createSection("settings", "Level Settings",
                  {startX, startY, kLevelSectionWidth, kLevelSectionHeight});

    float labelX = startX + 25;
    float inputX = startX + 250;
    float inputW = 400;
    float gapY = 50.f;
    float currentY = 100.f;

    addElementToSection(
        "settings",
        EntityFactory::createStaticText(_registry, _assetsManager,
                                        "ID:", "main_font", {labelX, 0}, 24.f),
        currentY);
    _levelIdInput = EntityFactory::createTextInput(
        _registry, _assetsManager, {inputX, 0}, {inputW, 40}, "level_1",
        "level_1", 50, false);
    addElementToSection("settings", _levelIdInput, currentY);

    currentY += gapY;
    addElementToSection(
        "settings",
        EntityFactory::createStaticText(
            _registry, _assetsManager, "Name:", "main_font", {labelX, 0}, 24.f),
        currentY);
    _levelNameInput = EntityFactory::createTextInput(
        _registry, _assetsManager, {inputX, 0}, {inputW, 40}, "Map Name",
        "Space Station", 50, false);
    addElementToSection("settings", _levelNameInput, currentY);

    currentY += gapY;
    addElementToSection("settings",
                        EntityFactory::createStaticText(
                            _registry, _assetsManager,
                            "Background:", "main_font", {labelX, 0}, 24.f),
                        currentY);
    _levelBackgroundBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text("main_font",
                                          rtype::display::Color::White(), 20,
                                          this->_bgPluginName),
        rtype::games::rtype::shared::TransformComponent(inputX, 0),
        rtype::games::rtype::client::Rectangle(
            {120, 40}, rtype::display::Color(0, 180, 0, 255),
            rtype::display::Color(0, 135, 0, 255)),
        this->_assetsManager, std::function<void()>([this]() {
            if (this->_libBackgrounds.empty()) return;
            ++this->_bgIteratorCurrent;
            if (this->_bgIteratorCurrent == this->_libBackgrounds.end()) {
                this->_bgIteratorCurrent = this->_libBackgrounds.begin();
            }
            this->_bgPluginName = this->_bgIteratorCurrent->first;

            if (this->_registry
                    ->hasComponent<rtype::games::rtype::client::Text>(
                        this->_levelBackgroundBtn)) {
                this->_registry
                    ->getComponent<rtype::games::rtype::client::Text>(
                        this->_levelBackgroundBtn)
                    .textContent = this->_bgPluginName;
            }
        }));
    addElementToSection("settings", _levelBackgroundBtn, currentY);

    currentY += gapY;
    addElementToSection("settings",
                        EntityFactory::createStaticText(
                            _registry, _assetsManager,
                            "Scroll Speed:", "main_font", {labelX, 0}, 24.f),
                        currentY);
    _scrollSpeedInput =
        EntityFactory::createTextInput(_registry, _assetsManager, {inputX, 0},
                                       {inputW, 40}, "50.0", "50.0", 10, true);
    addElementToSection("settings", _scrollSpeedInput, currentY);

    currentY += gapY;
    addElementToSection(
        "settings",
        EntityFactory::createStaticText(
            _registry, _assetsManager, "Boss:", "main_font", {labelX, 0}, 24.f),
        currentY);
    _bossInput = EntityFactory::createTextInput(_registry, _assetsManager,
                                                {inputX, 0}, {inputW, 40},
                                                "boss_1", "boss_1", 50, false);

    addElementToSection("settings", _bossInput, currentY);

    currentY += 80.f;

    auto btnAdd = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "Add Wave"),
        rtype::games::rtype::shared::TransformComponent(startX + 40, 0),
        rtype::games::rtype::client::Rectangle(
            {120, 40}, rtype::display::Color(0, 180, 0, 255),
            rtype::display::Color(0, 135, 0, 255)),
        this->_assetsManager,
        std::function<void()>([this]() { this->addWave(); }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnAdd, 1);
    addElementToSection("settings", btnAdd, currentY);

    auto btnPrev = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "< Prev"),
        rtype::games::rtype::shared::TransformComponent(startX + 180, 0),
        rtype::games::rtype::client::Rectangle(
            {80, 40}, rtype::display::Color(100, 100, 100, 255),
            rtype::display::Color(150, 150, 150, 255)),
        this->_assetsManager, std::function<void()>([this]() {
            if (_currentWaveIndex > 0) switchWave(_currentWaveIndex - 1);
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnPrev, 1);
    addElementToSection("settings", btnPrev, currentY);

    auto btnNext = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "Next >"),
        rtype::games::rtype::shared::TransformComponent(startX + 280, 0),
        rtype::games::rtype::client::Rectangle(
            {80, 40}, rtype::display::Color(100, 100, 100, 255),
            rtype::display::Color(150, 150, 150, 255)),
        this->_assetsManager, std::function<void()>([this]() {
            if (_currentWaveIndex < static_cast<int>(_waves.size()) - 1)
                switchWave(_currentWaveIndex + 1);
        }));
    this->_registry->emplaceComponent<rtype::games::rtype::client::ZIndex>(
        btnNext, 1);
    addElementToSection("settings", btnNext, currentY);

    _listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 24, "GENERATE TOML"),
        rtype::games::rtype::shared::TransformComponent(startX, 840),
        rtype::games::rtype::client::Rectangle({250, 60},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager,
        std::function<void()>([this]() { this->saveToToml(); })));

    _listEntity.push_back(EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "Back"),
        rtype::games::rtype::shared::TransformComponent(startX, 920),
        rtype::games::rtype::client::Rectangle({250, 60},
                                               rtype::display::Color::Blue(),
                                               rtype::display::Color::Red()),
        this->_assetsManager, std::function<void()>([this]() {
            _switchToScene(SceneManager::Scene::MAIN_MENU);
        })));

    if (_waves.empty()) addWave();

    refreshWaveUI();
}

static const std::vector<std::string> ENEMY_TYPES = {"basic", "wave", "chaser",
                                                     "heavy", "boss"};
static const std::vector<std::string> POWERUP_TYPES = {
    "health_small", "force_pod", "shield", "rapid_fire", "double_damage"};

static std::string cycleOption(const std::string& current,
                               const std::vector<std::string>& options) {
    auto it = std::find(options.begin(), options.end(), current);
    if (it == options.end() || it + 1 == options.end()) {
        return options[0];
    }
    return *(it + 1);
}

std::string LevelCreatorScene::getInputValue(ECS::Entity entity) {
    if (entity == ECS::Entity(0)) return "";
    if (_registry->isAlive(entity) &&
        _registry->hasComponent<rtype::games::rtype::client::TextInput>(
            entity)) {
        return _registry
            ->getComponent<rtype::games::rtype::client::TextInput>(entity)
            .content;
    }
    return "";
}

void LevelCreatorScene::saveCurrentWaveStats() {
    if (_currentWaveIndex >= 0 &&
        _currentWaveIndex < static_cast<int>(_waves.size())) {
        auto& w = _waves[_currentWaveIndex];

        if (w.spawnDelayInputVal != ECS::Entity(0) &&
            _registry->isAlive(w.spawnDelayInputVal) &&
            _registry->hasComponent<rtype::games::rtype::client::TextInput>(
                w.spawnDelayInputVal)) {
            try {
                w.spawn_delay = std::stof(getInputValue(w.spawnDelayInputVal));
            } catch (...) {
            }
        }

        for (auto& s : w.spawns) {
            // Enemy is updated via button click, no need to read
            if (s.delayInputVal != ECS::Entity(0) &&
                _registry->isAlive(s.delayInputVal) &&
                _registry->hasComponent<rtype::games::rtype::client::TextInput>(
                    s.delayInputVal)) {
                try {
                    s.delay = std::stof(getInputValue(s.delayInputVal));
                } catch (...) {
                }
            }
            if (s.countInputVal != ECS::Entity(0) &&
                _registry->isAlive(s.countInputVal) &&
                _registry->hasComponent<rtype::games::rtype::client::TextInput>(
                    s.countInputVal)) {
                try {
                    s.count = std::stoi(getInputValue(s.countInputVal));
                } catch (...) {
                }
            }
        }

        for (auto& p : w.powerups) {
            if (p.delayInputVal != ECS::Entity(0) &&
                _registry->isAlive(p.delayInputVal) &&
                _registry->hasComponent<rtype::games::rtype::client::TextInput>(
                    p.delayInputVal)) {
                try {
                    p.delay = std::stof(getInputValue(p.delayInputVal));
                } catch (...) {
                }
            }
            if (p.yInputVal != ECS::Entity(0) &&
                _registry->isAlive(p.yInputVal) &&
                _registry->hasComponent<rtype::games::rtype::client::TextInput>(
                    p.yInputVal)) {
                try {
                    p.y = std::stof(getInputValue(p.yInputVal));
                } catch (...) {
                }
            }
        }
    }
}

void LevelCreatorScene::saveToToml() {
    saveCurrentWaveStats();

    std::string filename = "level_config.toml";
    std::string lvlId = getInputValue(_levelIdInput);
    if (!lvlId.empty()) filename = lvlId + ".toml";

    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR_CAT(rtype::LogCategory::UI, std::string("Failed to open ") +
                                                  filename + " for writing.");
        return;
    }

    file << "# "
            "=================================================================="
            "==========="
         << std::endl;
    file << "# R-Type Level Configuration" << std::endl;
    file << "# "
            "=================================================================="
            "==========="
         << std::endl
         << std::endl;

    file << "[level]" << std::endl;
    file << "id = \"" << getInputValue(_levelIdInput) << "\"" << std::endl;
    file << "name = \"" << getInputValue(_levelNameInput) << "\"" << std::endl;
    file << "background = \"" << this->_bgPluginName << "\"" << std::endl;
    file << "scroll_speed = "
         << (getInputValue(_scrollSpeedInput).empty()
                 ? "0.0"
                 : getInputValue(_scrollSpeedInput))
         << std::endl;
    file << "boss = \"" << getInputValue(_bossInput) << "\"" << std::endl;
    file << std::endl;

    for (const auto& wave : _waves) {
        file << "# "
                "--------------------------------------------------------------"
                "---------------"
             << std::endl;
        file << "# Wave " << wave.number << std::endl;
        file << "# "
                "--------------------------------------------------------------"
                "---------------"
             << std::endl;
        file << "[[wave]]" << std::endl;
        file << "number = " << wave.number << std::endl;
        file << "spawn_delay = " << wave.spawn_delay << std::endl;
        file << std::endl;

        for (const auto& spawn : wave.spawns) {
            file << "[[wave.spawn]]" << std::endl;
            file << "enemy = \"" << spawn.enemy << "\"" << std::endl;
            file << "delay = " << spawn.delay << std::endl;
            file << "count = " << spawn.count << std::endl;
            file << std::endl;
        }

        for (const auto& powerup : wave.powerups) {
            file << "[[wave.powerup]]" << std::endl;
            file << "id = \"" << powerup.id << "\"" << std::endl;
            file << "delay = " << powerup.delay << std::endl;
            file << "y = " << powerup.y << std::endl;
            file << std::endl;
        }
    }

    file.close();
    std::cout << "Level configuration saved to " << filename << std::endl;

    if (_statusMessageEntity != ECS::Entity(0) &&
        _registry->isAlive(_statusMessageEntity)) {
        _registry->killEntity(_statusMessageEntity);
    }

    float startX = kLevelSectionPosLeft;
    _statusMessageEntity = EntityFactory::createStaticText(
        _registry, _assetsManager,
        "File: " + filename + ", generated successfully and saved", "main_font",
        {startX, 810.f}, 20.f);

    try {
        auto& textComp =
            _registry->getComponent<rtype::games::rtype::client::Text>(
                _statusMessageEntity);
        textComp.color = rtype::display::Color::Green();
    } catch (const std::exception&) {
    }
    _listEntity.push_back(_statusMessageEntity);
}

void LevelCreatorScene::addWave() {
    if (this->_waves.size() >= kMaxWaves) return;
    saveCurrentWaveStats();
    Wave newWave;
    newWave.number = this->_waves.size() + 1;
    newWave.spawn_delay = 1.0f;
    this->_waves.push_back(newWave);

    this->_currentWaveIndex = this->_waves.size() - 1;
    refreshWaveUI();
}

void LevelCreatorScene::switchWave(int index) {
    if (index < 0 || index >= static_cast<int>(_waves.size())) return;
    saveCurrentWaveStats();
    _currentWaveIndex = index;
    refreshWaveUI();
}

void LevelCreatorScene::refreshWaveUI() {
    for (auto it = _sections.begin(); it != _sections.end();) {
        if (it->id == "wave_config") {
            // Kill all entities tracked in this section
            for (auto& pair : it->entities) {
                if (_registry->isAlive(pair.first))
                    _registry->killEntity(pair.first);
            }
            it = _sections.erase(it);
        } else {
            ++it;
        }
    }

    for (auto e : _waveUiEntities) {
        if (_registry->isAlive(e)) _registry->killEntity(e);
    }
    _waveUiEntities.clear();

    _uiEntities.erase(std::remove_if(_uiEntities.begin(), _uiEntities.end(),
                                     [this](ECS::Entity e) {
                                         return !_registry->isAlive(e);
                                     }),
                      _uiEntities.end());

    if (_currentWaveIndex < 0 ||
        _currentWaveIndex >= static_cast<int>(_waves.size()))
        return;

    auto& wave = _waves[_currentWaveIndex];
    int waveIdx = _currentWaveIndex;

    float startX = kLevelSectionPosLeft + kLevelSectionWidth + 30.f;
    float startY = 200.f;
    float sectionW = 1920.f - startX - 50.f;
    float sectionH = 750.f;

    auto sectionEnts = EntityFactory::createSection(
        _registry, _assetsManager,
        "Wave " + std::to_string(wave.number) + " Configuration",
        {startX, startY, sectionW, sectionH}, 0);
    _uiEntities.insert(_uiEntities.end(), sectionEnts.begin(),
                       sectionEnts.end());
    _waveUiEntities = sectionEnts;

    ScrollSection waveSection;
    waveSection.id = "wave_config";
    waveSection.bounds = {startX, startY, sectionW, sectionH};
    waveSection.currentScroll = 0.0f;
    _sections.push_back(waveSection);

    float contentX = startX + 30.f;
    float contentY = 100.f;

    auto addToWave = [&](ECS::Entity e, float y) {
        if (this->_registry->hasComponent<rtype::games::rtype::client::ZIndex>(
                e))
            this->_registry
                ->getComponent<rtype::games::rtype::client::ZIndex>(e)
                .depth = 1;
        else
            this->_registry
                ->emplaceComponent<rtype::games::rtype::client::ZIndex>(e, 1);
        addElementToSection("wave_config", e, y);
    };

    addToWave(EntityFactory::createStaticText(_registry, _assetsManager,
                                              "Spawn Delay (s):", "main_font",
                                              {contentX, 0}, 24.f),
              contentY);

    wave.spawnDelayInputVal = EntityFactory::createTextInput(
        _registry, _assetsManager, {contentX + 220, 0}, {120, 35}, "1.0",
        std::to_string(wave.spawn_delay), 10, true);
    addToWave(wave.spawnDelayInputVal, contentY);

    contentY += 60.f;

    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Enemies",
                                        "main_font", {contentX, 0}, 28.f),
        contentY);

    auto addSpawnBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "+ Add Enemy"),
        rtype::games::rtype::shared::TransformComponent(contentX + 150, 0),
        rtype::games::rtype::client::Rectangle(
            {140, 35}, rtype::display::Color(0, 180, 0, 255),
            rtype::display::Color(0, 135, 0, 255)),
        this->_assetsManager, std::function<void()>([this, waveIdx]() {
            if (waveIdx < static_cast<int>(this->_waves.size())) {
                this->_waves[waveIdx].spawns.push_back(Spawn{});
                this->refreshWaveUI();
            }
        }));
    addToWave(addSpawnBtn, contentY);

    contentY += 50.f;

    float col1 = contentX;
    float col2 = contentX + 320;
    float col3 = contentX + 500;

    addToWave(EntityFactory::createStaticText(_registry, _assetsManager, "Type",
                                              "main_font", {col1, 0}, 20.f),
              contentY);
    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Wait (s)",
                                        "main_font", {col2, 0}, 20.f),
        contentY);
    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Count",
                                        "main_font", {col3, 0}, 20.f),
        contentY);

    contentY += 30.f;
    int spawnIdx = 0;
    for (auto& spawn : wave.spawns) {
        auto enemyBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                "main_font", rtype::display::Color::White(), 18, spawn.enemy),
            rtype::games::rtype::shared::TransformComponent(col1, 0),
            rtype::games::rtype::client::Rectangle(
                {300, 35}, rtype::display::Color(50, 50, 200, 255),
                rtype::display::Color(70, 70, 220, 255)),
            this->_assetsManager,
            std::function<void()>([this, waveIdx, spawnIdx]() {
                if (waveIdx < static_cast<int>(this->_waves.size()) &&
                    spawnIdx <
                        static_cast<int>(this->_waves[waveIdx].spawns.size())) {
                    auto& s = this->_waves[waveIdx].spawns[spawnIdx];
                    s.enemy = cycleOption(s.enemy, ENEMY_TYPES);
                    this->refreshWaveUI();
                }
            }));
        addToWave(enemyBtn, contentY);

        spawn.delayInputVal = EntityFactory::createTextInput(
            _registry, _assetsManager, {col2, 0}, {120, 35}, "0.0",
            std::to_string(spawn.delay), 10, true);
        addToWave(spawn.delayInputVal, contentY);

        spawn.countInputVal = EntityFactory::createTextInput(
            _registry, _assetsManager, {col3, 0}, {120, 35}, "1",
            std::to_string(spawn.count), 10, true);
        addToWave(spawn.countInputVal, contentY);

        contentY += 45.f;
        spawnIdx++;
    }

    contentY += 30.f;
    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Powerups",
                                        "main_font", {contentX, 0}, 28.f),
        contentY);

    auto addPuBtn = EntityFactory::createButton(
        this->_registry,
        rtype::games::rtype::client::Text(
            "main_font", rtype::display::Color::White(), 20, "+ Add Powerup"),
        rtype::games::rtype::shared::TransformComponent(contentX + 160, 0),
        rtype::games::rtype::client::Rectangle(
            {160, 35}, rtype::display::Color(0, 180, 0, 255),
            rtype::display::Color(0, 135, 0, 255)),
        this->_assetsManager, std::function<void()>([this, waveIdx]() {
            if (waveIdx < static_cast<int>(this->_waves.size())) {
                this->_waves[waveIdx].powerups.push_back(Powerup{});
                this->refreshWaveUI();
            }
        }));
    addToWave(addPuBtn, contentY);

    contentY += 50.f;

    addToWave(EntityFactory::createStaticText(_registry, _assetsManager, "Type",
                                              "main_font", {col1, 0}, 20.f),
              contentY);
    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Wait (s)",
                                        "main_font", {col2, 0}, 20.f),
        contentY);
    addToWave(
        EntityFactory::createStaticText(_registry, _assetsManager, "Y Pos",
                                        "main_font", {col3, 0}, 20.f),
        contentY);

    contentY += 30.f;
    int puIdx = 0;
    for (auto& pu : wave.powerups) {
        auto idBtn = EntityFactory::createButton(
            this->_registry,
            rtype::games::rtype::client::Text(
                "main_font", rtype::display::Color::White(), 18, pu.id),
            rtype::games::rtype::shared::TransformComponent(col1, 0),
            rtype::games::rtype::client::Rectangle(
                {300, 35}, rtype::display::Color(200, 50, 50, 255),
                rtype::display::Color(220, 70, 70, 255)),
            this->_assetsManager,
            std::function<void()>([this, waveIdx, puIdx]() {
                if (waveIdx < static_cast<int>(this->_waves.size()) &&
                    puIdx < static_cast<int>(
                                this->_waves[waveIdx].powerups.size())) {
                    auto& p = this->_waves[waveIdx].powerups[puIdx];
                    p.id = cycleOption(p.id, POWERUP_TYPES);
                    this->refreshWaveUI();
                }
            }));
        addToWave(idBtn, contentY);

        pu.delayInputVal = EntityFactory::createTextInput(
            _registry, _assetsManager, {col2, 0}, {120, 35}, "Dly",
            std::to_string(pu.delay), 10, true);
        addToWave(pu.delayInputVal, contentY);

        pu.yInputVal = EntityFactory::createTextInput(
            _registry, _assetsManager, {col3, 0}, {120, 35}, "Y",
            std::to_string(pu.y), 10, true);
        addToWave(pu.yInputVal, contentY);

        contentY += 45.f;
        puIdx++;
    }

    for (auto& sec : _sections) {
        if (sec.id == "wave_config") {
            float usedHeight = contentY + 50.f;  // + padding
            if (usedHeight > sectionH) {
                sec.maxScroll = usedHeight - sectionH + 20.f;
            } else {
                sec.maxScroll = 0.0f;
            }
            break;
        }
    }

    updateScrollPositions();
}

void LevelCreatorScene::pollEvents(const rtype::display::Event& e) {
    if (_textInputSystem) {
        _textInputSystem->handleEvent(*_registry, e);
    }

    if (e.type == rtype::display::EventType::MouseWheelScrolled) {
        int mx = e.mouseWheel.x;
        int my = e.mouseWheel.y;
        float delta = e.mouseWheel.delta;

        float scrollSpeed = 40.0f;

        for (auto& sec : _sections) {
            if (mx >= sec.bounds.left &&
                mx <= (sec.bounds.left + sec.bounds.width) &&
                my >= sec.bounds.top &&
                my <= (sec.bounds.top + sec.bounds.height)) {
                sec.currentScroll -= delta * scrollSpeed;

                if (sec.currentScroll < 0.0f) sec.currentScroll = 0.0f;
                if (sec.currentScroll > sec.maxScroll)
                    sec.currentScroll = sec.maxScroll;

                break;  // Only scroll one section
            }
        }

        updateScrollPositions();
    }
}

void LevelCreatorScene::updateScrollPositions() {
    auto applyAlpha = [&](ECS::Entity e, uint8_t a) {
        if (_registry->hasComponent<rtype::games::rtype::client::Text>(e)) {
            _registry->getComponent<rtype::games::rtype::client::Text>(e)
                .color.a = a;
        }
        if (_registry->hasComponent<rtype::games::rtype::client::Rectangle>(
                e)) {
            auto& rect =
                _registry->getComponent<rtype::games::rtype::client::Rectangle>(
                    e);
            rect.currentColor.a = a;
            rect.outlineColor.a = a;
            rect.mainColor.a = a;
            rect.hoveredColor.a = a;
        }
        if (_registry->hasComponent<rtype::games::rtype::client::TextInput>(
                e)) {
            auto& input =
                _registry->getComponent<rtype::games::rtype::client::TextInput>(
                    e);
            input.textColor.a = a;
            input.backgroundColor.a = a;
            input.focusedBorderColor.a = a;
            input.unfocusedBorderColor.a = a;
        }
    };

    for (const auto& sec : _sections) {
        float sectionTop = sec.bounds.top;
        float headerHeight = 60.f;
        float clipTop = sectionTop + headerHeight;
        float sectionBottom = sectionTop + sec.bounds.height;
        float fadeHeight = 150.f;

        for (auto& pair : sec.entities) {
            ECS::Entity entity = pair.first;
            float relativeY = pair.second;

            if (!_registry->isAlive(entity)) continue;

            float newY = sec.bounds.top + relativeY - sec.currentScroll;

            if (_registry->hasComponent<
                    rtype::games::rtype::shared::TransformComponent>(entity)) {
                _registry
                    ->getComponent<
                        rtype::games::rtype::shared::TransformComponent>(entity)
                    .y = newY;
            }

            bool isVisible = (newY >= clipTop - 10.f && newY <= sectionBottom);

            if (isVisible) {
                uint8_t alpha = 255;
                if (newY > sectionBottom - fadeHeight) {
                    float ratio = (sectionBottom - newY) / fadeHeight;
                    if (ratio < 0) ratio = 0;
                    if (ratio > 1) ratio = 1;
                    alpha = static_cast<uint8_t>(255 * ratio);
                }
                applyAlpha(entity, alpha);

                if (_registry->hasComponent<
                        rtype::games::rtype::client::HiddenComponent>(entity)) {
                    _registry
                        ->getComponent<
                            rtype::games::rtype::client::HiddenComponent>(
                            entity)
                        .isHidden = false;
                }
            } else {
                if (!_registry->hasComponent<
                        rtype::games::rtype::client::HiddenComponent>(entity)) {
                    _registry->emplaceComponent<
                        rtype::games::rtype::client::HiddenComponent>(entity,
                                                                      true);
                } else {
                    _registry
                        ->getComponent<
                            rtype::games::rtype::client::HiddenComponent>(
                            entity)
                        .isHidden = true;
                }
            }
        }
    }
}

LevelCreatorScene::~LevelCreatorScene() {
    for (auto e : _listEntity) {
        if (_registry->isAlive(e)) {
            _registry->killEntity(e);
        }
    }
    for (auto e : _uiEntities) {
        if (_registry->isAlive(e)) {
            _registry->killEntity(e);
        }
    }
}

void LevelCreatorScene::update(float dt) {}

void LevelCreatorScene::render(
    std::shared_ptr<rtype::display::IDisplay> window) {}
