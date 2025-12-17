/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeConfigParser - Implementation
*/

#include "RTypeConfigParser.hpp"

#include <fstream>
#include <sstream>

namespace rtype::game::config {

std::optional<RTypeGameConfig> RTypeConfigParser::loadFromFile(
    const std::filesystem::path& filepath) {
    auto table = _parser.parseFile(filepath);
    if (!table) {
        return std::nullopt;
    }

    RTypeGameConfig config = parseFromTable(*table);

    auto validationErrors = config.validate();
    for (const auto& error : validationErrors) {
        _parser.reportError({error.section, error.key, error.message});
    }

    config.applyDefaults();
    return config;
}

std::optional<RTypeGameConfig> RTypeConfigParser::loadFromString(
    const std::string& content) {
    auto table = _parser.parseString(content);
    if (!table) {
        return std::nullopt;
    }

    RTypeGameConfig config = parseFromTable(*table);

    auto validationErrors = config.validate();
    for (const auto& error : validationErrors) {
        _parser.reportError({error.section, error.key, error.message});
    }

    config.applyDefaults();
    return config;
}

bool RTypeConfigParser::saveToFile(const RTypeGameConfig& config,
                                   const std::filesystem::path& filepath) {
    std::string content = serializeToToml(config);

    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }

    auto tempPath = filepath.string() + ".tmp";
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        _parser.reportError(
            {"file", "", "Cannot create file: " + filepath.string()});
        return false;
    }

    file << content;
    file.close();

    if (!file) {
        _parser.reportError(
            {"file", "", "Failed to write to file: " + filepath.string()});
        std::filesystem::remove(tempPath);
        return false;
    }

    try {
        std::filesystem::rename(tempPath, filepath);
    } catch (const std::exception& e) {
        _parser.reportError(
            {"file", "",
             std::string("Failed to save file: ") + std::string(e.what())});
        std::filesystem::remove(tempPath);
        return false;
    }

    return true;
}

std::string RTypeConfigParser::serializeToString(
    const RTypeGameConfig& config) {
    return serializeToToml(config);
}

void RTypeConfigParser::setErrorCallback(ErrorCallback callback) {
    _parser.setErrorCallback(
        [callback](const rtype::config::ParseError& error) {
            callback({error.section, error.key, error.message});
        });
}

RTypeGameConfig RTypeConfigParser::parseFromTable(const toml::table& table) {
    RTypeGameConfig config;
    RTypeGameConfig defaults;

    config.video.width = _parser.getValue<uint32_t>(table, "video", "width",
                                                    defaults.video.width);
    config.video.height = _parser.getValue<uint32_t>(table, "video", "height",
                                                     defaults.video.height);
    config.video.fullscreen = _parser.getValue<bool>(
        table, "video", "fullscreen", defaults.video.fullscreen);
    config.video.vsync =
        _parser.getValue<bool>(table, "video", "vsync", defaults.video.vsync);
    config.video.maxFps = _parser.getValue<uint32_t>(table, "video", "maxFps",
                                                     defaults.video.maxFps);
    config.video.uiScale = static_cast<float>(_parser.getValue<double>(
        table, "video", "uiScale", defaults.video.uiScale));
    if (auto* sec = table["video"].as_table()) {
        if (auto res = (*sec)["resolution"].value<std::string>()) {
            auto resStr = *res;
            auto xPos = resStr.find('x');
            if (xPos != std::string::npos) {
                try {
                    config.video.width = std::stoul(resStr.substr(0, xPos));
                    config.video.height = std::stoul(resStr.substr(xPos + 1));
                } catch (...) {
                    _parser.reportError(
                        {"video", "resolution", "Invalid resolution format"});
                }
            }
        }
    }

    // Audio section
    config.audio.masterVolume = static_cast<float>(_parser.getValue<double>(
        table, "audio", "masterVolume", defaults.audio.masterVolume));
    config.audio.musicVolume = static_cast<float>(_parser.getValue<double>(
        table, "audio", "musicVolume", defaults.audio.musicVolume));
    config.audio.sfxVolume = static_cast<float>(_parser.getValue<double>(
        table, "audio", "sfxVolume", defaults.audio.sfxVolume));
    config.audio.muted =
        _parser.getValue<bool>(table, "audio", "muted", defaults.audio.muted);

    // Network section
    config.network.serverAddress = _parser.getString(
        table, "network", "serverAddress", defaults.network.serverAddress);
    config.network.serverPort = static_cast<uint16_t>(_parser.getValue<int64_t>(
        table, "network", "serverPort", defaults.network.serverPort));
    config.network.clientPort = static_cast<uint16_t>(_parser.getValue<int64_t>(
        table, "network", "clientPort", defaults.network.clientPort));
    config.network.connectionTimeout =
        _parser.getValue<uint32_t>(table, "network", "connectionTimeout",
                                   defaults.network.connectionTimeout);
    config.network.maxRetries = _parser.getValue<uint32_t>(
        table, "network", "maxRetries", defaults.network.maxRetries);
    config.network.tickrate = _parser.getValue<uint32_t>(
        table, "network", "tickrate", defaults.network.tickrate);

    // Server section
    config.server.port = static_cast<uint16_t>(_parser.getValue<int64_t>(
        table, "server", "port", defaults.server.port));
    config.server.maxPlayers = _parser.getValue<uint32_t>(
        table, "server", "max_players", defaults.server.maxPlayers);
    config.server.tickrate = _parser.getValue<uint32_t>(
        table, "server", "tickrate", defaults.server.tickrate);
    config.server.mapName =
        _parser.getString(table, "server", "mapName", defaults.server.mapName);

    // Gameplay section
    config.gameplay.difficulty = _parser.getString(
        table, "gameplay", "difficulty", defaults.gameplay.difficulty);
    config.gameplay.startingLives = _parser.getValue<uint32_t>(
        table, "gameplay", "startingLives", defaults.gameplay.startingLives);
    config.gameplay.waves = _parser.getValue<uint32_t>(
        table, "gameplay", "waves", defaults.gameplay.waves);
    config.gameplay.playerSpeed = static_cast<float>(_parser.getValue<double>(
        table, "gameplay", "playerSpeed", defaults.gameplay.playerSpeed));
    config.gameplay.enemySpeedMultiplier = static_cast<float>(
        _parser.getValue<double>(table, "gameplay", "enemySpeedMultiplier",
                                 defaults.gameplay.enemySpeedMultiplier));
    config.gameplay.friendlyFire = _parser.getValue<bool>(
        table, "gameplay", "friendlyFire", defaults.gameplay.friendlyFire);

    // Input section
    config.input.moveUp =
        _parser.getString(table, "input", "moveUp", defaults.input.moveUp);
    config.input.moveDown =
        _parser.getString(table, "input", "moveDown", defaults.input.moveDown);
    config.input.moveLeft =
        _parser.getString(table, "input", "moveLeft", defaults.input.moveLeft);
    config.input.moveRight = _parser.getString(table, "input", "moveRight",
                                               defaults.input.moveRight);
    config.input.fire =
        _parser.getString(table, "input", "fire", defaults.input.fire);
    config.input.pause =
        _parser.getString(table, "input", "pause", defaults.input.pause);
    config.input.mouseSensitivity = static_cast<float>(_parser.getValue<double>(
        table, "input", "mouseSensitivity", defaults.input.mouseSensitivity));

    // Paths section
    config.paths.assetsPath = _parser.getString(table, "paths", "assetsPath",
                                                defaults.paths.assetsPath);
    config.paths.savesPath = _parser.getString(table, "paths", "savesPath",
                                               defaults.paths.savesPath);
    config.paths.logsPath =
        _parser.getString(table, "paths", "logsPath", defaults.paths.logsPath);
    config.paths.configPath = _parser.getString(table, "paths", "configPath",
                                                defaults.paths.configPath);

    // Assets
    config.assets.fonts.MainFont =
        "assets/" + _parser.getString(table, "Fonts", "MainFont",
                                      defaults.assets.fonts.MainFont);
    config.assets.fonts.TitleFont =
        "assets/" + _parser.getString(table, "Fonts", "TitleFont",
                                      defaults.assets.fonts.TitleFont);

    // Textures
    config.assets.textures.background =
        "assets/" + _parser.getString(table, "Textures", "Background",
                                      defaults.assets.textures.background);
    config.assets.textures.planet1 =
        "assets/" + _parser.getString(table, "Textures", "Planet1",
                                      defaults.assets.textures.planet1);
    config.assets.textures.planet2 =
        "assets/" + _parser.getString(table, "Textures", "Planet2",
                                      defaults.assets.textures.planet2);
    config.assets.textures.planet3 =
        "assets/" + _parser.getString(table, "Textures", "Planet3",
                                      defaults.assets.textures.planet3);
    config.assets.textures.astroVessel =
        "assets/" + _parser.getString(table, "Textures", "AstroVessel",
                                      defaults.assets.textures.astroVessel);
    config.assets.textures.Player =
        "assets/" + _parser.getString(table, "Textures", "Player",
                                      defaults.assets.textures.Player);
    config.assets.textures.Enemy =
        "assets/" + _parser.getString(table, "Textures", "Enemy",
                                      defaults.assets.textures.Enemy);
    config.assets.textures.missileLaser =
        "assets/" + _parser.getString(table, "Textures", "MissileLaser",
                                      defaults.assets.textures.missileLaser);
    // Music
    config.assets.music.mainMenu =
        "assets/" + _parser.getString(table, "Music", "MainMenu",
                                      defaults.assets.music.mainMenu);
    config.assets.music.game =
        "assets/" +
        _parser.getString(table, "Music", "Game", defaults.assets.music.game);
    config.assets.music.settings =
        "assets/" + _parser.getString(table, "Music", "Settings",
                                      defaults.assets.music.settings);
    config.assets.music.gameOver =
        "assets/" + _parser.getString(table, "Music", "GameOver",
                                      defaults.assets.music.gameOver);
    // SFX
    config.assets.sfx.clickButton =
        "assets/" + _parser.getString(table, "SFX", "ClickButton",
                                      defaults.assets.sfx.clickButton);
    config.assets.sfx.hoverButton =
        "assets/" + _parser.getString(table, "SFX", "HoverButton",
                                      defaults.assets.sfx.hoverButton);
    config.assets.sfx.laser =
        "assets/" +
        _parser.getString(table, "SFX", "Laser", defaults.assets.sfx.laser);
    config.assets.sfx.playerSpawn =
        "assets/" + _parser.getString(table, "SFX", "PlayerSpawn",
                                      defaults.assets.sfx.playerSpawn);
    config.assets.sfx.playerDeath =
        "assets/" + _parser.getString(table, "SFX", "PlayerDeath",
                                      defaults.assets.sfx.playerDeath);
    config.assets.sfx.enemySpawn =
        "assets/" + _parser.getString(table, "SFX", "EnemySpawn",
                                      defaults.assets.sfx.enemySpawn);
    config.assets.sfx.enemyDeath =
        "assets/" + _parser.getString(table, "SFX", "EnemyDeath",
                                      defaults.assets.sfx.enemyDeath);

    return config;
}

std::string RTypeConfigParser::serializeToToml(const RTypeGameConfig& config) {
    std::ostringstream ss;

    ss << "# R-Type Configuration File\n";
    ss << "# Schema version: " << config.schemaVersion << "\n\n";

    ss << "[video]\n";
    ss << "width = " << config.video.width << "\n";
    ss << "height = " << config.video.height << "\n";
    ss << "fullscreen = " << (config.video.fullscreen ? "true" : "false")
       << "\n";
    ss << "vsync = " << (config.video.vsync ? "true" : "false") << "\n";
    ss << "maxFps = " << config.video.maxFps << "\n";
    ss << "uiScale = " << config.video.uiScale << "\n\n";

    ss << "[audio]\n";
    ss << "masterVolume = " << config.audio.masterVolume << "\n";
    ss << "musicVolume = " << config.audio.musicVolume << "\n";
    ss << "sfxVolume = " << config.audio.sfxVolume << "\n";
    ss << "muted = " << (config.audio.muted ? "true" : "false") << "\n\n";

    ss << "[network]\n";
    ss << "serverAddress = \"" << config.network.serverAddress << "\"\n";
    ss << "serverPort = " << config.network.serverPort << "\n";
    ss << "clientPort = " << config.network.clientPort << "\n";
    ss << "connectionTimeout = " << config.network.connectionTimeout << "\n";
    ss << "maxRetries = " << config.network.maxRetries << "\n";
    ss << "tickrate = " << config.network.tickrate << "\n\n";

    ss << "[server]\n";
    ss << "port = " << config.server.port << "\n";
    ss << "max_players = " << config.server.maxPlayers << "\n";
    ss << "tickrate = " << config.server.tickrate << "\n";
    ss << "mapName = \"" << config.server.mapName << "\"\n\n";

    ss << "[gameplay]\n";
    ss << "difficulty = \"" << config.gameplay.difficulty << "\"\n";
    ss << "startingLives = " << config.gameplay.startingLives << "\n";
    ss << "waves = " << config.gameplay.waves << "\n";
    ss << "playerSpeed = " << config.gameplay.playerSpeed << "\n";
    ss << "enemySpeedMultiplier = " << config.gameplay.enemySpeedMultiplier
       << "\n";
    ss << "friendlyFire = " << (config.gameplay.friendlyFire ? "true" : "false")
       << "\n\n";

    ss << "[input]\n";
    ss << "moveUp = \"" << config.input.moveUp << "\"\n";
    ss << "moveDown = \"" << config.input.moveDown << "\"\n";
    ss << "moveLeft = \"" << config.input.moveLeft << "\"\n";
    ss << "moveRight = \"" << config.input.moveRight << "\"\n";
    ss << "fire = \"" << config.input.fire << "\"\n";
    ss << "pause = \"" << config.input.pause << "\"\n";
    ss << "mouseSensitivity = " << config.input.mouseSensitivity << "\n\n";

    ss << "[paths]\n";
    ss << "assetsPath = \"" << config.paths.assetsPath << "\"\n";
    ss << "savesPath = \"" << config.paths.savesPath << "\"\n";
    ss << "logsPath = \"" << config.paths.logsPath << "\"\n";
    ss << "configPath = \"" << config.paths.configPath << "\"\n";

    return ss.str();
}

}  // namespace rtype::game::config
