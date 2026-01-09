/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameConfig upper bounds branch coverage tests
*/

#include <gtest/gtest.h>

#include "Config/RTypeConfig.hpp"

using namespace rtype::game::config;

// ============================================================================
// Tests for upper bound branches in validate()
// ============================================================================

TEST(RTypeGameConfigBounds, VideoWidthUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test width > 7680
    config.video.width = 8000;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test width = 7680 (should be valid)
    config.video.width = 7680;
    errors = config.validate();
    bool hasWidthError = false;
    for (const auto& err : errors) {
        if (err.key == "width") hasWidthError = true;
    }
    EXPECT_FALSE(hasWidthError);
}

TEST(RTypeGameConfigBounds, VideoHeightUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test height > 4320
    config.video.height = 5000;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test height = 4320 (should be valid)
    config.video.height = 4320;
    errors = config.validate();
    bool hasHeightError = false;
    for (const auto& err : errors) {
        if (err.key == "height") hasHeightError = true;
    }
    EXPECT_FALSE(hasHeightError);
}

TEST(RTypeGameConfigBounds, VideoMaxFpsUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test maxFps > 500
    config.video.maxFps = 600;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test maxFps = 500 (should be valid)
    config.video.maxFps = 500;
    errors = config.validate();
    bool hasFpsError = false;
    for (const auto& err : errors) {
        if (err.key == "maxFps") hasFpsError = true;
    }
    EXPECT_FALSE(hasFpsError);
}

TEST(RTypeGameConfigBounds, VideoUiScaleUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test uiScale > 3.0
    config.video.uiScale = 4.0F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test uiScale = 3.0 (should be valid)
    config.video.uiScale = 3.0F;
    errors = config.validate();
    bool hasScaleError = false;
    for (const auto& err : errors) {
        if (err.key == "uiScale") hasScaleError = true;
    }
    EXPECT_FALSE(hasScaleError);
}

TEST(RTypeGameConfigBounds, VideoUiScaleLowerBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test uiScale < 0.5
    config.video.uiScale = 0.3F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test uiScale = 0.5 (should be valid)
    config.video.uiScale = 0.5F;
    errors = config.validate();
    bool hasScaleError = false;
    for (const auto& err : errors) {
        if (err.key == "uiScale") hasScaleError = true;
    }
    EXPECT_FALSE(hasScaleError);
}

TEST(RTypeGameConfigBounds, AudioMasterVolumeUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test masterVolume > 1.0
    config.audio.masterVolume = 1.5F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test masterVolume = 1.0 (should be valid)
    config.audio.masterVolume = 1.0F;
    errors = config.validate();
    bool hasVolumeError = false;
    for (const auto& err : errors) {
        if (err.key == "masterVolume") hasVolumeError = true;
    }
    EXPECT_FALSE(hasVolumeError);
}

TEST(RTypeGameConfigBounds, AudioMusicVolumeUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test musicVolume > 1.0
    config.audio.musicVolume = 2.0F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test musicVolume = 1.0 (should be valid)
    config.audio.musicVolume = 1.0F;
    errors = config.validate();
    bool hasVolumeError = false;
    for (const auto& err : errors) {
        if (err.key == "musicVolume") hasVolumeError = true;
    }
    EXPECT_FALSE(hasVolumeError);
}

TEST(RTypeGameConfigBounds, AudioSfxVolumeUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test sfxVolume > 1.0
    config.audio.sfxVolume = 1.2F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test sfxVolume = 1.0 (should be valid)
    config.audio.sfxVolume = 1.0F;
    errors = config.validate();
    bool hasVolumeError = false;
    for (const auto& err : errors) {
        if (err.key == "sfxVolume") hasVolumeError = true;
    }
    EXPECT_FALSE(hasVolumeError);
}

TEST(RTypeGameConfigBounds, NetworkTickrateUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test tickrate > 240
    config.network.tickrate = 300;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test tickrate = 240 (should be valid)
    config.network.tickrate = 240;
    errors = config.validate();
    bool hasTickrateError = false;
    for (const auto& err : errors) {
        if (err.key == "tickrate" && err.section == "network") hasTickrateError = true;
    }
    EXPECT_FALSE(hasTickrateError);
}

TEST(RTypeGameConfigBounds, ServerMaxPlayersUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test maxPlayers > 64
    config.server.maxPlayers = 100;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test maxPlayers = 64 (should be valid)
    config.server.maxPlayers = 64;
    errors = config.validate();
    bool hasMaxPlayersError = false;
    for (const auto& err : errors) {
        if (err.key == "maxPlayers") hasMaxPlayersError = true;
    }
    EXPECT_FALSE(hasMaxPlayersError);
}

TEST(RTypeGameConfigBounds, ServerTickrateUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test tickrate > 240
    config.server.tickrate = 500;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test tickrate = 240 (should be valid)
    config.server.tickrate = 240;
    errors = config.validate();
    bool hasTickrateError = false;
    for (const auto& err : errors) {
        if (err.key == "tickrate" && err.section == "server") hasTickrateError = true;
    }
    EXPECT_FALSE(hasTickrateError);
}

TEST(RTypeGameConfigBounds, GameplayStartingLivesUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test startingLives > 99
    config.gameplay.startingLives = 150;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test startingLives = 99 (should be valid)
    config.gameplay.startingLives = 99;
    errors = config.validate();
    bool hasLivesError = false;
    for (const auto& err : errors) {
        if (err.key == "startingLives") hasLivesError = true;
    }
    EXPECT_FALSE(hasLivesError);
}

TEST(RTypeGameConfigBounds, InputMouseSensitivityUpperBound) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    // Test mouseSensitivity > 10.0
    config.input.mouseSensitivity = 15.0F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
    
    // Test mouseSensitivity = 10.0 (should be valid)
    config.input.mouseSensitivity = 10.0F;
    errors = config.validate();
    bool hasSensitivityError = false;
    for (const auto& err : errors) {
        if (err.key == "mouseSensitivity") hasSensitivityError = true;
    }
    EXPECT_FALSE(hasSensitivityError);
}

// ============================================================================
// Tests for upper bound branches in applyDefaults()
// ============================================================================

TEST(RTypeGameConfigBoundsDefaults, VideoWidthUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.width = 10000;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.video.width, 1280);
}

TEST(RTypeGameConfigBoundsDefaults, VideoHeightUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.height = 8000;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.video.height, 720);
}

TEST(RTypeGameConfigBoundsDefaults, VideoMaxFpsUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.maxFps = 1000;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.video.maxFps, 60u);
}

TEST(RTypeGameConfigBoundsDefaults, VideoUiScaleUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.uiScale = 5.0F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.video.uiScale, 1.0F);
}

TEST(RTypeGameConfigBoundsDefaults, VideoUiScaleLowerBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.video.uiScale = 0.1F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.video.uiScale, 1.0F);
}

TEST(RTypeGameConfigBoundsDefaults, AudioMasterVolumeUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.masterVolume = 2.5F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.audio.masterVolume, 1.0F);
}

TEST(RTypeGameConfigBoundsDefaults, AudioMusicVolumeUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.musicVolume = 3.0F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.audio.musicVolume, 0.8F);
}

TEST(RTypeGameConfigBoundsDefaults, AudioSfxVolumeUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.audio.sfxVolume = 1.5F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.audio.sfxVolume, 1.0F);
}

TEST(RTypeGameConfigBoundsDefaults, NetworkTickrateUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.network.tickrate = 500;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.network.tickrate, 60u);
}

TEST(RTypeGameConfigBoundsDefaults, ServerMaxPlayersUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.maxPlayers = 200;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.server.maxPlayers, 8u);
}

TEST(RTypeGameConfigBoundsDefaults, ServerTickrateUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.server.tickrate = 1000;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.server.tickrate, 60u);
}

TEST(RTypeGameConfigBoundsDefaults, GameplayStartingLivesUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.startingLives = 200;
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.startingLives, 3u);
}

TEST(RTypeGameConfigBoundsDefaults, InputMouseSensitivityUpperBoundApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.input.mouseSensitivity = 20.0F;
    
    config.applyDefaults();
    
    EXPECT_FLOAT_EQ(config.input.mouseSensitivity, 1.0F);
}

// ============================================================================
// Tests for all difficulty string combinations
// ============================================================================

TEST(RTypeGameConfigBounds, DifficultyEasy) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "easy";
    
    auto errors = config.validate();
    bool hasDifficultyError = false;
    for (const auto& err : errors) {
        if (err.key == "difficulty") hasDifficultyError = true;
    }
    EXPECT_FALSE(hasDifficultyError);
}

TEST(RTypeGameConfigBounds, DifficultyNormal) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "normal";
    
    auto errors = config.validate();
    bool hasDifficultyError = false;
    for (const auto& err : errors) {
        if (err.key == "difficulty") hasDifficultyError = true;
    }
    EXPECT_FALSE(hasDifficultyError);
}

TEST(RTypeGameConfigBounds, DifficultyHard) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "hard";
    
    auto errors = config.validate();
    bool hasDifficultyError = false;
    for (const auto& err : errors) {
        if (err.key == "difficulty") hasDifficultyError = true;
    }
    EXPECT_FALSE(hasDifficultyError);
}

TEST(RTypeGameConfigBounds, DifficultyNightmare) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "nightmare";
    
    auto errors = config.validate();
    bool hasDifficultyError = false;
    for (const auto& err : errors) {
        if (err.key == "difficulty") hasDifficultyError = true;
    }
    EXPECT_FALSE(hasDifficultyError);
}

TEST(RTypeGameConfigBounds, DifficultyInvalid) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "impossible";
    
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
}

TEST(RTypeGameConfigBounds, DifficultyEmpty) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "";
    
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());
}

TEST(RTypeGameConfigBoundsDefaults, DifficultyEasyApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "easy";
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.difficulty, "easy");
}

TEST(RTypeGameConfigBoundsDefaults, DifficultyNormalApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "normal";
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.difficulty, "normal");
}

TEST(RTypeGameConfigBoundsDefaults, DifficultyHardApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "hard";
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.difficulty, "hard");
}

TEST(RTypeGameConfigBoundsDefaults, DifficultyNightmareApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "nightmare";
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.difficulty, "nightmare");
}

TEST(RTypeGameConfigBoundsDefaults, DifficultyInvalidApplied) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "invalid";
    
    config.applyDefaults();
    
    EXPECT_EQ(config.gameplay.difficulty, "normal");
}

// ============================================================================
// Combined boundary tests
// ============================================================================

TEST(RTypeGameConfigBounds, MultipleUpperBounds) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    
    config.video.width = 10000;
    config.video.height = 5000;
    config.audio.masterVolume = 2.0F;
    config.server.maxPlayers = 100;
    
    auto errors = config.validate();
    EXPECT_GE(errors.size(), 4u);
}

TEST(RTypeGameConfigBounds, AllUpperBoundsAtLimit) {
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
    config.gameplay.startingLives = 99;
    config.input.mouseSensitivity = 10.0F;
    
    auto errors = config.validate();
    
    // Should have no errors for numeric fields at their upper limits
    bool hasNumericError = false;
    for (const auto& err : errors) {
        if (err.key == "width" || err.key == "height" || err.key == "maxFps" ||
            err.key == "uiScale" || err.key == "masterVolume" || 
            err.key == "musicVolume" || err.key == "sfxVolume" ||
            err.key == "tickrate" || err.key == "maxPlayers" ||
            err.key == "startingLives" || err.key == "mouseSensitivity") {
            hasNumericError = true;
        }
    }
    EXPECT_FALSE(hasNumericError);
}
