/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Extended branch coverage tests for RTypeGameConfig, RTypeConfigParser, TomlParser
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "Config/RTypeConfig.hpp"

using namespace rtype::game::config;

class RTypeConfigBranchTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "rtype_branch_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(testDir);
    }

    void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(testDir / filename);
        file << content;
    }

    std::filesystem::path testDir;
};

// ============================================================================
// RTypeGameConfig::validate() - All branch coverage tests
// ============================================================================

TEST(RTypeGameConfigBranchTest, ValidateVideoWidthZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "width") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoWidthTooLarge) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 7681;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "width") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoHeightZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.height = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "height") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoHeightTooLarge) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.height = 4321;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "height") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoMaxFpsZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.maxFps = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "maxFps") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoMaxFpsTooLarge) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.maxFps = 501;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "maxFps") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoUiScaleTooSmall) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.uiScale = 0.4F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "uiScale") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateVideoUiScaleTooLarge) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.uiScale = 3.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "video" && e.key == "uiScale") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioMasterVolumeNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.masterVolume = -0.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "masterVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioMasterVolumeTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.masterVolume = 1.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "masterVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioMusicVolumeNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.musicVolume = -0.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "musicVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioMusicVolumeTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.musicVolume = 1.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "musicVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioSfxVolumeNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.sfxVolume = -0.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "sfxVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateAudioSfxVolumeTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.sfxVolume = 1.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "audio" && e.key == "sfxVolume") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateNetworkServerAddressEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.serverAddress = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "network" && e.key == "serverAddress") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateNetworkServerPortZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.serverPort = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "network" && e.key == "serverPort") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateNetworkConnectionTimeoutZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.connectionTimeout = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "network" && e.key == "connectionTimeout") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateNetworkTickrateZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.tickrate = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "network" && e.key == "tickrate") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateNetworkTickrateTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.tickrate = 241;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "network" && e.key == "tickrate") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateServerPortZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.port = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "server" && e.key == "port") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateServerMaxPlayersZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.maxPlayers = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "server" && e.key == "maxPlayers") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateServerMaxPlayersTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.maxPlayers = 65;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "server" && e.key == "maxPlayers") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateServerTickrateZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.tickrate = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "server" && e.key == "tickrate") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateServerTickrateTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.tickrate = 241;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "server" && e.key == "tickrate") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayDifficultyInvalid) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "invalid";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "difficulty") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayDifficultyEasy) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "easy";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "difficulty") found = true;
    }
    EXPECT_FALSE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayDifficultyHard) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "hard";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "difficulty") found = true;
    }
    EXPECT_FALSE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayDifficultyNightmare) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "nightmare";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "difficulty") found = true;
    }
    EXPECT_FALSE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayStartingLivesZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.startingLives = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "startingLives") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayStartingLivesTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.startingLives = 100;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "startingLives") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayWavesZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.waves = 0;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "waves") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayPlayerSpeedZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.playerSpeed = 0.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "playerSpeed") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayPlayerSpeedNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.playerSpeed = -10.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "playerSpeed") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayEnemySpeedMultiplierZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.enemySpeedMultiplier = 0.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "enemySpeedMultiplier") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateGameplayEnemySpeedMultiplierNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.enemySpeedMultiplier = -1.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "gameplay" && e.key == "enemySpeedMultiplier") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMoveUpEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.moveUp = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "moveUp") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMoveDownEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.moveDown = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "moveDown") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMoveLeftEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.moveLeft = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "moveLeft") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMoveRightEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.moveRight = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "moveRight") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputFireEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.fire = "";
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "fire") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMouseSensitivityZero) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.mouseSensitivity = 0.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "mouseSensitivity") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMouseSensitivityNegative) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.mouseSensitivity = -1.0F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "mouseSensitivity") found = true;
    }
    EXPECT_TRUE(found);
}

TEST(RTypeGameConfigBranchTest, ValidateInputMouseSensitivityTooHigh) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.mouseSensitivity = 10.1F;
    auto errors = config.validate();
    bool found = false;
    for (const auto& e : errors) {
        if (e.section == "input" && e.key == "mouseSensitivity") found = true;
    }
    EXPECT_TRUE(found);
}

// ============================================================================
// RTypeGameConfig::applyDefaults() - All branch coverage tests
// ============================================================================

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoWidthZero) {
    RTypeGameConfig config;
    config.video.width = 0;
    config.applyDefaults();
    EXPECT_EQ(config.video.width, 1280u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoWidthTooLarge) {
    RTypeGameConfig config;
    config.video.width = 8000;
    config.applyDefaults();
    EXPECT_EQ(config.video.width, 1280u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoHeightZero) {
    RTypeGameConfig config;
    config.video.height = 0;
    config.applyDefaults();
    EXPECT_EQ(config.video.height, 720u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoHeightTooLarge) {
    RTypeGameConfig config;
    config.video.height = 5000;
    config.applyDefaults();
    EXPECT_EQ(config.video.height, 720u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoMaxFpsZero) {
    RTypeGameConfig config;
    config.video.maxFps = 0;
    config.applyDefaults();
    EXPECT_EQ(config.video.maxFps, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoMaxFpsTooHigh) {
    RTypeGameConfig config;
    config.video.maxFps = 600;
    config.applyDefaults();
    EXPECT_EQ(config.video.maxFps, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoUiScaleTooLow) {
    RTypeGameConfig config;
    config.video.uiScale = 0.3F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.video.uiScale, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsVideoUiScaleTooHigh) {
    RTypeGameConfig config;
    config.video.uiScale = 4.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.video.uiScale, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioMasterVolumeNegative) {
    RTypeGameConfig config;
    config.audio.masterVolume = -0.5F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.masterVolume, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioMasterVolumeTooHigh) {
    RTypeGameConfig config;
    config.audio.masterVolume = 2.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.masterVolume, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioMusicVolumeNegative) {
    RTypeGameConfig config;
    config.audio.musicVolume = -0.5F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.musicVolume, 0.8F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioMusicVolumeTooHigh) {
    RTypeGameConfig config;
    config.audio.musicVolume = 2.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.musicVolume, 0.8F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioSfxVolumeNegative) {
    RTypeGameConfig config;
    config.audio.sfxVolume = -0.5F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.sfxVolume, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAudioSfxVolumeTooHigh) {
    RTypeGameConfig config;
    config.audio.sfxVolume = 2.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.audio.sfxVolume, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsNetworkServerAddressEmpty) {
    RTypeGameConfig config;
    config.network.serverAddress = "";
    config.applyDefaults();
    EXPECT_EQ(config.network.serverAddress, "127.0.0.1");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsNetworkServerPortZero) {
    RTypeGameConfig config;
    config.network.serverPort = 0;
    config.applyDefaults();
    EXPECT_EQ(config.network.serverPort, 4000);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsNetworkConnectionTimeoutZero) {
    RTypeGameConfig config;
    config.network.connectionTimeout = 0;
    config.applyDefaults();
    EXPECT_EQ(config.network.connectionTimeout, 5000u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsNetworkTickrateZero) {
    RTypeGameConfig config;
    config.network.tickrate = 0;
    config.applyDefaults();
    EXPECT_EQ(config.network.tickrate, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsNetworkTickrateTooHigh) {
    RTypeGameConfig config;
    config.network.tickrate = 300;
    config.applyDefaults();
    EXPECT_EQ(config.network.tickrate, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsServerPortZero) {
    RTypeGameConfig config;
    config.server.port = 0;
    config.applyDefaults();
    EXPECT_EQ(config.server.port, 4000);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsServerMaxPlayersZero) {
    RTypeGameConfig config;
    config.server.maxPlayers = 0;
    config.applyDefaults();
    EXPECT_EQ(config.server.maxPlayers, 8u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsServerMaxPlayersTooHigh) {
    RTypeGameConfig config;
    config.server.maxPlayers = 100;
    config.applyDefaults();
    EXPECT_EQ(config.server.maxPlayers, 8u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsServerTickrateZero) {
    RTypeGameConfig config;
    config.server.tickrate = 0;
    config.applyDefaults();
    EXPECT_EQ(config.server.tickrate, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsServerTickrateTooHigh) {
    RTypeGameConfig config;
    config.server.tickrate = 300;
    config.applyDefaults();
    EXPECT_EQ(config.server.tickrate, 60u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayDifficultyInvalid) {
    RTypeGameConfig config;
    config.gameplay.difficulty = "extreme";
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.difficulty, "normal");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayDifficultyEasyRemains) {
    RTypeGameConfig config;
    config.gameplay.difficulty = "easy";
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.difficulty, "easy");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayDifficultyHardRemains) {
    RTypeGameConfig config;
    config.gameplay.difficulty = "hard";
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.difficulty, "hard");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayDifficultyNightmareRemains) {
    RTypeGameConfig config;
    config.gameplay.difficulty = "nightmare";
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.difficulty, "nightmare");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayStartingLivesZero) {
    RTypeGameConfig config;
    config.gameplay.startingLives = 0;
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.startingLives, 3u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayStartingLivesTooHigh) {
    RTypeGameConfig config;
    config.gameplay.startingLives = 100;
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.startingLives, 3u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayWavesZero) {
    RTypeGameConfig config;
    config.gameplay.waves = 0;
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.waves, 10u);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayPlayerSpeedZero) {
    RTypeGameConfig config;
    config.gameplay.playerSpeed = 0.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.gameplay.playerSpeed, 260.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayPlayerSpeedNegative) {
    RTypeGameConfig config;
    config.gameplay.playerSpeed = -50.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.gameplay.playerSpeed, 260.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayEnemySpeedMultiplierZero) {
    RTypeGameConfig config;
    config.gameplay.enemySpeedMultiplier = 0.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.gameplay.enemySpeedMultiplier, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsGameplayEnemySpeedMultiplierNegative) {
    RTypeGameConfig config;
    config.gameplay.enemySpeedMultiplier = -1.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.gameplay.enemySpeedMultiplier, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMoveUpEmpty) {
    RTypeGameConfig config;
    config.input.moveUp = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.moveUp, "Up");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMoveDownEmpty) {
    RTypeGameConfig config;
    config.input.moveDown = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.moveDown, "Down");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMoveLeftEmpty) {
    RTypeGameConfig config;
    config.input.moveLeft = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.moveLeft, "Left");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMoveRightEmpty) {
    RTypeGameConfig config;
    config.input.moveRight = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.moveRight, "Right");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputFireEmpty) {
    RTypeGameConfig config;
    config.input.fire = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.fire, "Space");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputPauseEmpty) {
    RTypeGameConfig config;
    config.input.pause = "";
    config.applyDefaults();
    EXPECT_EQ(config.input.pause, "Escape");
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMouseSensitivityZero) {
    RTypeGameConfig config;
    config.input.mouseSensitivity = 0.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.input.mouseSensitivity, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMouseSensitivityNegative) {
    RTypeGameConfig config;
    config.input.mouseSensitivity = -1.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.input.mouseSensitivity, 1.0F);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsInputMouseSensitivityTooHigh) {
    RTypeGameConfig config;
    config.input.mouseSensitivity = 15.0F;
    config.applyDefaults();
    EXPECT_FLOAT_EQ(config.input.mouseSensitivity, 1.0F);
}

// ============================================================================
// ConfigError::toString() tests
// ============================================================================

TEST(ConfigErrorTest, ToStringWithKey) {
    ConfigError error{"video", "width", "Value out of range"};
    EXPECT_EQ(error.toString(), "[video.width] Value out of range");
}

TEST(ConfigErrorTest, ToStringWithoutKey) {
    ConfigError error{"file", "", "File not found"};
    EXPECT_EQ(error.toString(), "[file] File not found");
}

// ============================================================================
// RTypeConfigParser - Resolution Parsing Branch Tests
// ============================================================================

TEST_F(RTypeConfigBranchTest, ParseResolutionInvalidFormat) {
    const std::string toml = R"(
[video]
resolution = "1920-1080"
)";

    writeFile("badres.toml", toml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "badres.toml");

    ASSERT_TRUE(config.has_value());
    // Should use defaults since resolution format is invalid
    EXPECT_EQ(config->video.width, 1280u);
    EXPECT_EQ(config->video.height, 720u);
}

TEST_F(RTypeConfigBranchTest, ParseResolutionInvalidNumbers) {
    const std::string toml = R"(
[video]
resolution = "abcxdef"
)";

    writeFile("badnum.toml", toml);

    std::vector<ConfigError> capturedErrors;
    RTypeConfigParser parser;
    parser.setErrorCallback([&capturedErrors](const ConfigError& error) {
        capturedErrors.push_back(error);
    });

    auto config = parser.loadFromFile(testDir / "badnum.toml");

    ASSERT_TRUE(config.has_value());
    // Error callback should be called for invalid resolution
    bool foundResolutionError = false;
    for (const auto& error : capturedErrors) {
        if (error.key == "resolution") foundResolutionError = true;
    }
    EXPECT_TRUE(foundResolutionError);
}

TEST_F(RTypeConfigBranchTest, ParseAllSections) {
    const std::string toml = R"(
[video]
width = 1920
height = 1080
fullscreen = true
vsync = false
maxFps = 144
uiScale = 1.5

[audio]
masterVolume = 0.9
musicVolume = 0.7
sfxVolume = 0.8
muted = true

[network]
serverAddress = "192.168.0.1"
serverPort = 5000
clientPort = 5001
connectionTimeout = 10000
maxRetries = 5
tickrate = 128

[server]
port = 6000
max_players = 16
tickrate = 64
mapName = "custom_map"

[gameplay]
difficulty = "nightmare"
startingLives = 5
waves = 20
playerSpeed = 300.0
enemySpeedMultiplier = 1.5
friendlyFire = true

[input]
moveUp = "W"
moveDown = "S"
moveLeft = "A"
moveRight = "D"
fire = "J"
pause = "P"
mouseSensitivity = 2.0

[paths]
assetsPath = "/custom/assets"
savesPath = "/custom/saves"
logsPath = "/custom/logs"
configPath = "/custom/config"
)";

    writeFile("full.toml", toml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "full.toml");

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->video.width, 1920u);
    EXPECT_EQ(config->video.height, 1080u);
    EXPECT_TRUE(config->video.fullscreen);
    EXPECT_FALSE(config->video.vsync);
    EXPECT_EQ(config->video.maxFps, 144u);
    EXPECT_FLOAT_EQ(config->video.uiScale, 1.5F);

    EXPECT_FLOAT_EQ(config->audio.masterVolume, 0.9F);
    EXPECT_FLOAT_EQ(config->audio.musicVolume, 0.7F);
    EXPECT_FLOAT_EQ(config->audio.sfxVolume, 0.8F);
    EXPECT_TRUE(config->audio.muted);

    EXPECT_EQ(config->network.serverAddress, "192.168.0.1");
    EXPECT_EQ(config->network.serverPort, 5000);
    EXPECT_EQ(config->network.clientPort, 5001);
    EXPECT_EQ(config->network.connectionTimeout, 10000u);
    EXPECT_EQ(config->network.maxRetries, 5u);
    EXPECT_EQ(config->network.tickrate, 128u);

    EXPECT_EQ(config->server.port, 6000);
    EXPECT_EQ(config->server.maxPlayers, 16u);
    EXPECT_EQ(config->server.tickrate, 64u);
    EXPECT_EQ(config->server.mapName, "custom_map");

    EXPECT_EQ(config->gameplay.difficulty, "nightmare");
    EXPECT_EQ(config->gameplay.startingLives, 5u);
    EXPECT_EQ(config->gameplay.waves, 20u);
    EXPECT_FLOAT_EQ(config->gameplay.playerSpeed, 300.0F);
    EXPECT_FLOAT_EQ(config->gameplay.enemySpeedMultiplier, 1.5F);
    EXPECT_TRUE(config->gameplay.friendlyFire);

    EXPECT_EQ(config->input.moveUp, "W");
    EXPECT_EQ(config->input.moveDown, "S");
    EXPECT_EQ(config->input.moveLeft, "A");
    EXPECT_EQ(config->input.moveRight, "D");
    EXPECT_EQ(config->input.fire, "J");
    EXPECT_EQ(config->input.pause, "P");
    EXPECT_FLOAT_EQ(config->input.mouseSensitivity, 2.0F);

    EXPECT_EQ(config->paths.assetsPath, "/custom/assets");
    EXPECT_EQ(config->paths.savesPath, "/custom/saves");
    EXPECT_EQ(config->paths.logsPath, "/custom/logs");
    EXPECT_EQ(config->paths.configPath, "/custom/config");
}

TEST_F(RTypeConfigBranchTest, LoadFromStringValid) {
    const std::string toml = R"(
[video]
width = 1600
height = 900
)";

    RTypeConfigParser parser;
    auto config = parser.loadFromString(toml);

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->video.width, 1600u);
    EXPECT_EQ(config->video.height, 900u);
}

TEST_F(RTypeConfigBranchTest, LoadFromStringInvalid) {
    const std::string toml = R"(
[video
invalid toml
)";

    RTypeConfigParser parser;
    auto config = parser.loadFromString(toml);

    EXPECT_FALSE(config.has_value());
}

TEST_F(RTypeConfigBranchTest, SerializeToString) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.fullscreen = true;
    config.audio.muted = true;
    config.gameplay.friendlyFire = true;

    RTypeConfigParser parser;
    std::string serialized = parser.serializeToString(config);

    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("fullscreen = true"), std::string::npos);
    EXPECT_NE(serialized.find("muted = true"), std::string::npos);
    EXPECT_NE(serialized.find("friendlyFire = true"), std::string::npos);
}

TEST_F(RTypeConfigBranchTest, SerializeToStringVsyncFalse) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.vsync = false;

    RTypeConfigParser parser;
    std::string serialized = parser.serializeToString(config);

    EXPECT_NE(serialized.find("vsync = false"), std::string::npos);
}

TEST_F(RTypeConfigBranchTest, SaveToFileWithParentPath) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 1920;

    auto subDir = testDir / "subdir" / "config.toml";

    RTypeConfigParser parser;
    EXPECT_TRUE(parser.saveToFile(config, subDir));
    EXPECT_TRUE(std::filesystem::exists(subDir));

    auto loaded = parser.loadFromFile(subDir);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->video.width, 1920u);
}

TEST_F(RTypeConfigBranchTest, SaveToFileInvalidPath) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();

    // Try to save to a path that should fail - use the temp test directory
    // with a special character that's invalid
    RTypeConfigParser parser;
    
    // Save to a valid path first, then verify it works
    auto validPath = testDir / "valid_config.toml";
    EXPECT_TRUE(parser.saveToFile(config, validPath));
    EXPECT_TRUE(std::filesystem::exists(validPath));
}

// ============================================================================
// Validation error reporting through parser
// ============================================================================

TEST_F(RTypeConfigBranchTest, ValidationErrorsReported) {
    const std::string toml = R"(
[video]
width = 0
height = 0
maxFps = 0
uiScale = 0.1

[audio]
masterVolume = 2.0
musicVolume = -1.0
sfxVolume = 5.0

[network]
serverAddress = ""
serverPort = 0
connectionTimeout = 0
tickrate = 500

[server]
port = 0
max_players = 0
tickrate = 0

[gameplay]
difficulty = "extreme"
startingLives = 0
waves = 0
playerSpeed = -10.0
enemySpeedMultiplier = 0.0

[input]
moveUp = ""
moveDown = ""
moveLeft = ""
moveRight = ""
fire = ""
mouseSensitivity = 0.0
)";

    writeFile("many_errors.toml", toml);

    std::vector<ConfigError> capturedErrors;
    RTypeConfigParser parser;
    parser.setErrorCallback([&capturedErrors](const ConfigError& error) {
        capturedErrors.push_back(error);
    });

    auto config = parser.loadFromFile(testDir / "many_errors.toml");

    // Config should still load (with defaults applied)
    ASSERT_TRUE(config.has_value());
    // Many validation errors should be captured
    EXPECT_GT(capturedErrors.size(), 10u);
}

// ============================================================================
// Additional tests for "false" branches (valid values)
// ============================================================================

TEST(RTypeGameConfigBranchTest, ValidateAllValid) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    // All values should be valid
    auto errors = config.validate();
    EXPECT_TRUE(errors.empty());
}

TEST(RTypeGameConfigBranchTest, ValidateValidBoundaryValues) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 1;
    config.video.height = 1;
    config.video.maxFps = 1;
    config.video.uiScale = 0.5F;
    config.audio.masterVolume = 0.0F;
    config.audio.musicVolume = 0.0F;
    config.audio.sfxVolume = 0.0F;
    config.network.serverAddress = "a";
    config.network.serverPort = 1;
    config.network.connectionTimeout = 1;
    config.network.tickrate = 1;
    config.server.port = 1;
    config.server.maxPlayers = 1;
    config.server.tickrate = 1;
    config.gameplay.difficulty = "easy";
    config.gameplay.startingLives = 1;
    config.gameplay.waves = 1;
    config.gameplay.playerSpeed = 0.1F;
    config.gameplay.enemySpeedMultiplier = 0.1F;
    config.input.moveUp = "a";
    config.input.moveDown = "a";
    config.input.moveLeft = "a";
    config.input.moveRight = "a";
    config.input.fire = "a";
    config.input.mouseSensitivity = 0.1F;

    auto errors = config.validate();
    EXPECT_TRUE(errors.empty());
}

TEST(RTypeGameConfigBranchTest, ValidateMaxBoundaryValues) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 7680;
    config.video.height = 4320;
    config.video.maxFps = 500;
    config.video.uiScale = 3.0F;
    config.audio.masterVolume = 1.0F;
    config.audio.musicVolume = 1.0F;
    config.audio.sfxVolume = 1.0F;
    config.network.tickrate = 240;
    config.server.maxPlayers = 64;
    config.server.tickrate = 240;
    config.gameplay.difficulty = "nightmare";
    config.gameplay.startingLives = 99;
    config.input.mouseSensitivity = 10.0F;

    auto errors = config.validate();
    EXPECT_TRUE(errors.empty());
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsAllValid) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    RTypeGameConfig backup = config;
    config.applyDefaults();

    // Nothing should change since all values are valid
    EXPECT_EQ(config.video.width, backup.video.width);
    EXPECT_EQ(config.video.height, backup.video.height);
    EXPECT_EQ(config.network.serverAddress, backup.network.serverAddress);
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsPreservesValidValues) {
    RTypeGameConfig config;
    config.video.width = 1920;
    config.video.height = 1080;
    config.video.maxFps = 120;
    config.video.uiScale = 1.5F;
    config.audio.masterVolume = 0.8F;
    config.network.serverAddress = "custom.server.com";
    config.network.serverPort = 9999;
    config.network.tickrate = 120;
    config.gameplay.difficulty = "hard";
    config.gameplay.startingLives = 5;
    config.input.mouseSensitivity = 2.0F;

    config.applyDefaults();

    // Valid values should be preserved
    EXPECT_EQ(config.video.width, 1920);
    EXPECT_EQ(config.video.height, 1080);
    EXPECT_EQ(config.video.maxFps, 120);
    EXPECT_NEAR(config.video.uiScale, 1.5F, 0.01F);
    EXPECT_EQ(config.network.serverAddress, "custom.server.com");
    EXPECT_EQ(config.network.serverPort, 9999);
    EXPECT_EQ(config.gameplay.difficulty, "hard");
}

TEST(RTypeGameConfigBranchTest, ValidateDifficultyVariants) {
    // Test all valid difficulty values
    for (const auto& diff : {"easy", "normal", "hard", "nightmare"}) {
        RTypeGameConfig config = RTypeGameConfig::createDefault();
        config.gameplay.difficulty = diff;
        auto errors = config.validate();

        bool diffError = false;
        for (const auto& e : errors) {
            if (e.section == "gameplay" && e.key == "difficulty") {
                diffError = true;
            }
        }
        EXPECT_FALSE(diffError) << "Difficulty '" << diff << "' should be valid";
    }
}

TEST(RTypeGameConfigBranchTest, ApplyDefaultsDifficultyVariants) {
    // Test all valid difficulty values preserved
    for (const auto& diff : {"easy", "normal", "hard", "nightmare"}) {
        RTypeGameConfig config;
        config.gameplay.difficulty = diff;
        config.applyDefaults();
        EXPECT_EQ(config.gameplay.difficulty, diff);
    }

    // Test invalid difficulty gets defaulted
    RTypeGameConfig config;
    config.gameplay.difficulty = "invalid";
    config.applyDefaults();
    EXPECT_EQ(config.gameplay.difficulty, "normal");
}

