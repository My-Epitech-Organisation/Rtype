/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for RTypeGameConfig and RTypeConfigParser
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "Config/RTypeConfig.hpp"

using namespace rtype::game::config;

class RTypeConfigParserTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "rtype_config_test";
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
// RTypeGameConfig Tests
// ============================================================================

TEST(RTypeGameConfigTest, DefaultValuesAreValid) {
    RTypeGameConfig config = RTypeGameConfig::createDefault();
    auto errors = config.validate();
    EXPECT_TRUE(errors.empty()) << "Default config should be valid";
}

TEST(RTypeGameConfigTest, ValidateVideoSettings) {
    RTypeGameConfig config;

    // Invalid width
    config.video.width = 0;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());

    // Reset and test invalid height
    config = RTypeGameConfig::createDefault();
    config.video.height = 10000;
    errors = config.validate();
    EXPECT_FALSE(errors.empty());

    // Reset and test invalid maxFps
    config = RTypeGameConfig::createDefault();
    config.video.maxFps = 0;
    errors = config.validate();
    EXPECT_FALSE(errors.empty());
}

TEST(RTypeGameConfigTest, ValidateAudioSettings) {
    RTypeGameConfig config;

    // Invalid master volume
    config.audio.masterVolume = 1.5F;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());

    // Reset and test negative volume
    config = RTypeGameConfig::createDefault();
    config.audio.musicVolume = -0.5F;
    errors = config.validate();
    EXPECT_FALSE(errors.empty());
}

TEST(RTypeGameConfigTest, ValidateNetworkSettings) {
    RTypeGameConfig config;

    // Invalid server port
    config.network.serverPort = 0;
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());

    // Reset and test empty server address
    config = RTypeGameConfig::createDefault();
    config.network.serverAddress = "";
    errors = config.validate();
    EXPECT_FALSE(errors.empty());
}

TEST(RTypeGameConfigTest, ValidateGameplaySettings) {
    RTypeGameConfig config;

    // Invalid difficulty
    config.gameplay.difficulty = "impossible";
    auto errors = config.validate();
    EXPECT_FALSE(errors.empty());

    // Valid difficulties
    config = RTypeGameConfig::createDefault();
    config.gameplay.difficulty = "easy";
    errors = config.validate();
    EXPECT_TRUE(errors.empty());

    config.gameplay.difficulty = "normal";
    errors = config.validate();
    EXPECT_TRUE(errors.empty());

    config.gameplay.difficulty = "hard";
    errors = config.validate();
    EXPECT_TRUE(errors.empty());

    config.gameplay.difficulty = "nightmare";
    errors = config.validate();
    EXPECT_TRUE(errors.empty());
}

TEST(RTypeGameConfigTest, ApplyDefaultsFixesInvalidValues) {
    RTypeGameConfig config;
    config.video.width = 0;
    config.audio.masterVolume = 5.0F;
    config.network.serverPort = 0;
    config.gameplay.difficulty = "invalid";

    config.applyDefaults();
    auto errors = config.validate();
    EXPECT_TRUE(errors.empty()) << "After applying defaults, config should be valid";
}

// ============================================================================
// RTypeConfigParser TOML Tests
// ============================================================================

TEST_F(RTypeConfigParserTest, ParseValidToml) {
    const std::string toml = R"(
[video]
width = 1920
height = 1080
fullscreen = true
vsync = false
maxFps = 144

[audio]
masterVolume = 0.8
musicVolume = 0.5
sfxVolume = 1.0
muted = false

[network]
serverAddress = "192.168.1.1"
serverPort = 5000
tickrate = 128

[gameplay]
difficulty = "hard"
startingLives = 5
waves = 20
)";

    writeFile("config.toml", toml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "config.toml");

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->video.width, 1920u);
    EXPECT_EQ(config->video.height, 1080u);
    EXPECT_TRUE(config->video.fullscreen);
    EXPECT_FALSE(config->video.vsync);
    EXPECT_EQ(config->video.maxFps, 144u);
    EXPECT_FLOAT_EQ(config->audio.masterVolume, 0.8F);
    EXPECT_FLOAT_EQ(config->audio.musicVolume, 0.5F);
    EXPECT_EQ(config->network.serverAddress, "192.168.1.1");
    EXPECT_EQ(config->network.serverPort, 5000);
    EXPECT_EQ(config->gameplay.difficulty, "hard");
    EXPECT_EQ(config->gameplay.startingLives, 5u);
}

TEST_F(RTypeConfigParserTest, ParsePartialTomlUsesDefaults) {
    const std::string toml = R"(
[video]
width = 1920
# height is missing, should use default

[gameplay]
difficulty = "easy"
# other fields missing
)";

    writeFile("partial.toml", toml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "partial.toml");

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->video.width, 1920u);
    EXPECT_EQ(config->video.height, 720u);  // Default
    EXPECT_EQ(config->gameplay.difficulty, "easy");
    EXPECT_EQ(config->gameplay.startingLives, 3u);  // Default
}

TEST_F(RTypeConfigParserTest, ParseEmptyTomlUsesAllDefaults) {
    writeFile("empty.toml", "");

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "empty.toml");

    ASSERT_TRUE(config.has_value());
    RTypeGameConfig defaults = RTypeGameConfig::createDefault();
    EXPECT_EQ(config->video.width, defaults.video.width);
    EXPECT_EQ(config->video.height, defaults.video.height);
}

TEST_F(RTypeConfigParserTest, ParseInvalidTomlReturnsNullopt) {
    const std::string invalidToml = R"(
[video
width = 1920
missing bracket
)";

    writeFile("invalid.toml", invalidToml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "invalid.toml");

    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST_F(RTypeConfigParserTest, ParseResolutionString) {
    const std::string toml = R"(
[video]
resolution = "1920x1080"
)";

    writeFile("resolution.toml", toml);

    RTypeConfigParser parser;
    auto config = parser.loadFromFile(testDir / "resolution.toml");

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->video.width, 1920u);
    EXPECT_EQ(config->video.height, 1080u);
}

TEST_F(RTypeConfigParserTest, FileNotFoundReturnsNullopt) {
    RTypeConfigParser parser;
    auto config = parser.loadFromFile("nonexistent.toml");

    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

// ============================================================================
// RTypeConfigParser Serialization Tests
// ============================================================================

TEST_F(RTypeConfigParserTest, SerializeToTomlAndReload) {
    RTypeGameConfig original;
    original.video.width = 1600;
    original.video.height = 900;
    original.audio.masterVolume = 0.75F;
    original.gameplay.difficulty = "hard";

    RTypeConfigParser parser;
    EXPECT_TRUE(parser.saveToFile(original, testDir / "saved.toml"));

    auto loaded = parser.loadFromFile(testDir / "saved.toml");
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->video.width, original.video.width);
    EXPECT_EQ(loaded->video.height, original.video.height);
    EXPECT_FLOAT_EQ(loaded->audio.masterVolume, original.audio.masterVolume);
    EXPECT_EQ(loaded->gameplay.difficulty, original.gameplay.difficulty);
}

// ============================================================================
// Error Callback Tests
// ============================================================================

TEST_F(RTypeConfigParserTest, ErrorCallbackIsCalled) {
    const std::string toml = R"(
[video]
width = 0
)";  // Invalid width, should trigger validation error

    writeFile("errors.toml", toml);

    std::vector<ConfigError> capturedErrors;
    RTypeConfigParser parser;
    parser.setErrorCallback([&capturedErrors](const ConfigError& error) {
        capturedErrors.push_back(error);
    });

    auto config = parser.loadFromFile(testDir / "errors.toml");

    // Width of 0 is invalid, but config should still load with default applied
    EXPECT_TRUE(config.has_value());
    EXPECT_FALSE(capturedErrors.empty());
}
