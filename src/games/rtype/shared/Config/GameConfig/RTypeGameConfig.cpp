/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameConfig - Implementation
*/

#include "RTypeGameConfig.hpp"

namespace rtype::game::config {

std::vector<ConfigError> RTypeGameConfig::validate() const {
    std::vector<ConfigError> errors;

    // Video validation
    if (video.width == 0 || video.width > 7680) {
        errors.push_back(
            {"video", "width", "Width must be between 1 and 7680"});
    }
    if (video.height == 0 || video.height > 4320) {
        errors.push_back(
            {"video", "height", "Height must be between 1 and 4320"});
    }
    if (video.maxFps == 0 || video.maxFps > 500) {
        errors.push_back(
            {"video", "maxFps", "MaxFps must be between 1 and 500"});
    }
    if (video.uiScale < 0.5F || video.uiScale > 3.0F) {
        errors.push_back(
            {"video", "uiScale", "UI scale must be between 0.5 and 3.0"});
    }

    // Audio validation
    if (audio.masterVolume < 0.0F || audio.masterVolume > 1.0F) {
        errors.push_back({"audio", "masterVolume",
                          "Master volume must be between 0.0 and 1.0"});
    }
    if (audio.musicVolume < 0.0F || audio.musicVolume > 1.0F) {
        errors.push_back({"audio", "musicVolume",
                          "Music volume must be between 0.0 and 1.0"});
    }
    if (audio.sfxVolume < 0.0F || audio.sfxVolume > 1.0F) {
        errors.push_back(
            {"audio", "sfxVolume", "SFX volume must be between 0.0 and 1.0"});
    }

    // Network validation
    if (network.serverAddress.empty()) {
        errors.push_back(
            {"network", "serverAddress", "Server address cannot be empty"});
    }
    if (network.serverPort == 0) {
        errors.push_back(
            {"network", "serverPort", "Server port must be greater than 0"});
    }
    if (network.connectionTimeout == 0) {
        errors.push_back({"network", "connectionTimeout",
                          "Connection timeout must be greater than 0"});
    }
    if (network.tickrate == 0 || network.tickrate > 240) {
        errors.push_back(
            {"network", "tickrate", "Tickrate must be between 1 and 240"});
    }

    // Server validation
    if (server.port == 0) {
        errors.push_back({"server", "port", "Port must be greater than 0"});
    }
    if (server.maxPlayers == 0 || server.maxPlayers > 64) {
        errors.push_back(
            {"server", "maxPlayers", "Max players must be between 1 and 64"});
    }
    if (server.tickrate == 0 || server.tickrate > 240) {
        errors.push_back(
            {"server", "tickrate", "Tickrate must be between 1 and 240"});
    }

    // Gameplay validation
    if (gameplay.difficulty != "easy" && gameplay.difficulty != "normal" &&
        gameplay.difficulty != "hard" && gameplay.difficulty != "nightmare") {
        errors.push_back(
            {"gameplay", "difficulty",
             "Difficulty must be one of: easy, normal, hard, nightmare"});
    }
    if (gameplay.startingLives == 0 || gameplay.startingLives > 99) {
        errors.push_back({"gameplay", "startingLives",
                          "Starting lives must be between 1 and 99"});
    }
    if (gameplay.waves == 0) {
        errors.push_back({"gameplay", "waves", "Waves must be greater than 0"});
    }
    if (gameplay.playerSpeed <= 0.0F) {
        errors.push_back(
            {"gameplay", "playerSpeed", "Player speed must be positive"});
    }
    if (gameplay.enemySpeedMultiplier <= 0.0F) {
        errors.push_back({"gameplay", "enemySpeedMultiplier",
                          "Enemy speed multiplier must be positive"});
    }

    // Input validation
    if (input.moveUp.empty()) {
        errors.push_back({"input", "moveUp", "Move up key cannot be empty"});
    }
    if (input.moveDown.empty()) {
        errors.push_back(
            {"input", "moveDown", "Move down key cannot be empty"});
    }
    if (input.moveLeft.empty()) {
        errors.push_back(
            {"input", "moveLeft", "Move left key cannot be empty"});
    }
    if (input.moveRight.empty()) {
        errors.push_back(
            {"input", "moveRight", "Move right key cannot be empty"});
    }
    if (input.fire.empty()) {
        errors.push_back({"input", "fire", "Fire key cannot be empty"});
    }
    if (input.mouseSensitivity <= 0.0F || input.mouseSensitivity > 10.0F) {
        errors.push_back({"input", "mouseSensitivity",
                          "Mouse sensitivity must be between 0.0 and 10.0"});
    }

    return errors;
}

void RTypeGameConfig::applyDefaults() {
    RTypeGameConfig defaults = createDefault();

    // Only apply defaults if current values are invalid
    if (video.width == 0 || video.width > 7680)
        video.width = defaults.video.width;
    if (video.height == 0 || video.height > 4320)
        video.height = defaults.video.height;
    if (video.maxFps == 0 || video.maxFps > 500)
        video.maxFps = defaults.video.maxFps;
    if (video.uiScale < 0.5F || video.uiScale > 3.0F)
        video.uiScale = defaults.video.uiScale;

    if (audio.masterVolume < 0.0F || audio.masterVolume > 1.0F)
        audio.masterVolume = defaults.audio.masterVolume;
    if (audio.musicVolume < 0.0F || audio.musicVolume > 1.0F)
        audio.musicVolume = defaults.audio.musicVolume;
    if (audio.sfxVolume < 0.0F || audio.sfxVolume > 1.0F)
        audio.sfxVolume = defaults.audio.sfxVolume;

    if (network.serverAddress.empty())
        network.serverAddress = defaults.network.serverAddress;
    if (network.serverPort == 0)
        network.serverPort = defaults.network.serverPort;
    if (network.connectionTimeout == 0)
        network.connectionTimeout = defaults.network.connectionTimeout;
    if (network.tickrate == 0 || network.tickrate > 240)
        network.tickrate = defaults.network.tickrate;

    if (server.port == 0) server.port = defaults.server.port;
    if (server.maxPlayers == 0 || server.maxPlayers > 64)
        server.maxPlayers = defaults.server.maxPlayers;
    if (server.tickrate == 0 || server.tickrate > 240)
        server.tickrate = defaults.server.tickrate;

    if (gameplay.difficulty != "easy" && gameplay.difficulty != "normal" &&
        gameplay.difficulty != "hard" && gameplay.difficulty != "nightmare") {
        gameplay.difficulty = defaults.gameplay.difficulty;
    }
    if (gameplay.startingLives == 0 || gameplay.startingLives > 99)
        gameplay.startingLives = defaults.gameplay.startingLives;
    if (gameplay.waves == 0) gameplay.waves = defaults.gameplay.waves;
    if (gameplay.playerSpeed <= 0.0F)
        gameplay.playerSpeed = defaults.gameplay.playerSpeed;
    if (gameplay.enemySpeedMultiplier <= 0.0F)
        gameplay.enemySpeedMultiplier = defaults.gameplay.enemySpeedMultiplier;

    if (input.moveUp.empty()) input.moveUp = defaults.input.moveUp;
    if (input.moveDown.empty()) input.moveDown = defaults.input.moveDown;
    if (input.moveLeft.empty()) input.moveLeft = defaults.input.moveLeft;
    if (input.moveRight.empty()) input.moveRight = defaults.input.moveRight;
    if (input.fire.empty()) input.fire = defaults.input.fire;
    if (input.pause.empty()) input.pause = defaults.input.pause;
    if (input.mouseSensitivity <= 0.0F || input.mouseSensitivity > 10.0F)
        input.mouseSensitivity = defaults.input.mouseSensitivity;
}

RTypeGameConfig RTypeGameConfig::createDefault() {
    return RTypeGameConfig{};  // Uses default member initializers
}

}  // namespace rtype::game::config
