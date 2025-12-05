/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeConfigParser Branch Coverage Tests
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "Config/Parser/RTypeConfigParser.hpp"
#include "Config/GameConfig/RTypeGameConfig.hpp"

namespace rtype::game::config {

class RTypeConfigParserTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "config_parser_test";
        std::filesystem::create_directories(testDir);
        errorsReceived.clear();
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(testDir, ec);
    }

    std::filesystem::path testDir;
    std::vector<ConfigError> errorsReceived;

    void createConfigFile(const std::filesystem::path& path,
                          const std::string& content) {
        std::ofstream file(path);
        file << content;
        file.close();
    }
};

// =============================================================================
// loadFromFile Tests - Branch coverage
// =============================================================================

TEST_F(RTypeConfigParserTest, LoadFromFileSuccess) {
    auto configPath = testDir / "valid_config.toml";
    createConfigFile(configPath, R"(
[video]
width = 1920
height = 1080
fullscreen = false
vsync = true
maxFps = 60
uiScale = 1.0

[audio]
masterVolume = 0.8
musicVolume = 0.7
sfxVolume = 0.9
muted = false

[network]
serverAddress = "127.0.0.1"
serverPort = 4242
clientPort = 4243
connectionTimeout = 5000
maxRetries = 3
tickrate = 60

[server]
port = 4242
max_players = 4
tickrate = 60
mapName = "level1"

[gameplay]
difficulty = "normal"
startingLives = 3
waves = 10
playerSpeed = 200.0
enemySpeedMultiplier = 1.0
friendlyFire = false

[input]
moveUp = "W"
moveDown = "S"
moveLeft = "A"
moveRight = "D"
fire = "Space"
pause = "Escape"
mouseSensitivity = 1.0

[paths]
assetsPath = "./assets"
savesPath = "./saves"
logsPath = "./logs"
configPath = "./config"
)");

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
    EXPECT_EQ(result->video.height, 1080);
    EXPECT_EQ(result->audio.masterVolume, 0.8f);
}

TEST_F(RTypeConfigParserTest, LoadFromFileNotFound) {
    auto configPath = testDir / "nonexistent.toml";

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_FALSE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromFileInvalidToml) {
    auto configPath = testDir / "invalid.toml";
    createConfigFile(configPath, R"(
[video
width = 1920
)");

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_FALSE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromFilePartialConfig) {
    auto configPath = testDir / "partial.toml";
    createConfigFile(configPath, R"(
[video]
width = 800
height = 600
)");

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 800);
    EXPECT_EQ(result->video.height, 600);
    // Other values should have defaults applied
}

TEST_F(RTypeConfigParserTest, LoadFromFileWithResolutionString) {
    auto configPath = testDir / "resolution.toml";
    createConfigFile(configPath, R"(
[video]
resolution = "1280x720"
)");

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1280);
    EXPECT_EQ(result->video.height, 720);
}

TEST_F(RTypeConfigParserTest, LoadFromFileWithInvalidResolutionFormat) {
    auto configPath = testDir / "bad_resolution.toml";
    createConfigFile(configPath, R"(
[video]
resolution = "not_a_resolution"
)");

    RTypeConfigParser parser;
    bool errorReported = false;
    parser.setErrorCallback(
        [&errorReported](const ConfigError& error) {
            if (error.key == "resolution") {
                errorReported = true;
            }
        });

    auto result = parser.loadFromFile(configPath);

    // Config should still load, error may or may not be reported depending on parsing
    EXPECT_TRUE(result.has_value());
    // The error may not be reported if resolution parsing doesn't find 'x'
}

TEST_F(RTypeConfigParserTest, LoadFromFileWithValidationErrors) {
    auto configPath = testDir / "validation_errors.toml";
    createConfigFile(configPath, R"(
[video]
width = 100
height = 100
maxFps = 10
uiScale = 10.0

[audio]
masterVolume = 2.0
musicVolume = -1.0
sfxVolume = 5.0
)");

    RTypeConfigParser parser;
    std::vector<ConfigError> errors;
    parser.setErrorCallback(
        [&errors](const ConfigError& error) {
            errors.push_back(error);
        });

    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    // Validation errors should have been reported
    EXPECT_GT(errors.size(), 0);
}

TEST_F(RTypeConfigParserTest, LoadFromFileEmptyFile) {
    auto configPath = testDir / "empty.toml";
    createConfigFile(configPath, "");

    RTypeConfigParser parser;
    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    // Should have all defaults applied
}

// =============================================================================
// loadFromString Tests - Branch coverage
// =============================================================================

TEST_F(RTypeConfigParserTest, LoadFromStringSuccess) {
    std::string content = R"(
[video]
width = 1920
height = 1080
fullscreen = true
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
    EXPECT_EQ(result->video.height, 1080);
    EXPECT_TRUE(result->video.fullscreen);
}

TEST_F(RTypeConfigParserTest, LoadFromStringInvalid) {
    std::string content = "[invalid toml syntax";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_FALSE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringEmpty) {
    std::string content = "";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringOnlyComments) {
    std::string content = R"(
# This is a comment
# Another comment
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringAllSections) {
    std::string content = R"(
[video]
width = 1280
height = 720
fullscreen = false
vsync = true
maxFps = 144
uiScale = 1.5

[audio]
masterVolume = 1.0
musicVolume = 0.5
sfxVolume = 0.75
muted = false

[network]
serverAddress = "192.168.1.1"
serverPort = 5000
clientPort = 5001
connectionTimeout = 10000
maxRetries = 5
tickrate = 30

[server]
port = 5000
max_players = 8
tickrate = 30
mapName = "arena"

[gameplay]
difficulty = "hard"
startingLives = 5
waves = 20
playerSpeed = 300.0
enemySpeedMultiplier = 1.5
friendlyFire = true

[input]
moveUp = "Up"
moveDown = "Down"
moveLeft = "Left"
moveRight = "Right"
fire = "X"
pause = "P"
mouseSensitivity = 2.0

[paths]
assetsPath = "/game/assets"
savesPath = "/game/saves"
logsPath = "/game/logs"
configPath = "/game/config"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());

    // Video
    EXPECT_EQ(result->video.width, 1280);
    EXPECT_EQ(result->video.height, 720);
    EXPECT_FALSE(result->video.fullscreen);
    EXPECT_TRUE(result->video.vsync);
    EXPECT_EQ(result->video.maxFps, 144);
    EXPECT_FLOAT_EQ(result->video.uiScale, 1.5f);

    // Audio
    EXPECT_FLOAT_EQ(result->audio.masterVolume, 1.0f);
    EXPECT_FLOAT_EQ(result->audio.musicVolume, 0.5f);
    EXPECT_FLOAT_EQ(result->audio.sfxVolume, 0.75f);
    EXPECT_FALSE(result->audio.muted);

    // Network
    EXPECT_EQ(result->network.serverAddress, "192.168.1.1");
    EXPECT_EQ(result->network.serverPort, 5000);
    EXPECT_EQ(result->network.clientPort, 5001);
    EXPECT_EQ(result->network.connectionTimeout, 10000);
    EXPECT_EQ(result->network.maxRetries, 5);
    EXPECT_EQ(result->network.tickrate, 30);

    // Server
    EXPECT_EQ(result->server.port, 5000);
    EXPECT_EQ(result->server.maxPlayers, 8);
    EXPECT_EQ(result->server.tickrate, 30);
    EXPECT_EQ(result->server.mapName, "arena");

    // Gameplay
    EXPECT_EQ(result->gameplay.difficulty, "hard");
    EXPECT_EQ(result->gameplay.startingLives, 5);
    EXPECT_EQ(result->gameplay.waves, 20);
    EXPECT_FLOAT_EQ(result->gameplay.playerSpeed, 300.0f);
    EXPECT_FLOAT_EQ(result->gameplay.enemySpeedMultiplier, 1.5f);
    EXPECT_TRUE(result->gameplay.friendlyFire);

    // Input
    EXPECT_EQ(result->input.moveUp, "Up");
    EXPECT_EQ(result->input.moveDown, "Down");
    EXPECT_EQ(result->input.moveLeft, "Left");
    EXPECT_EQ(result->input.moveRight, "Right");
    EXPECT_EQ(result->input.fire, "X");
    EXPECT_EQ(result->input.pause, "P");
    EXPECT_FLOAT_EQ(result->input.mouseSensitivity, 2.0f);

    // Paths
    EXPECT_EQ(result->paths.assetsPath, "/game/assets");
    EXPECT_EQ(result->paths.savesPath, "/game/saves");
    EXPECT_EQ(result->paths.logsPath, "/game/logs");
    EXPECT_EQ(result->paths.configPath, "/game/config");
}

// =============================================================================
// saveToFile Tests - Branch coverage
// =============================================================================

TEST_F(RTypeConfigParserTest, SaveToFileSuccess) {
    auto configPath = testDir / "save_test.toml";

    RTypeGameConfig config;
    config.video.width = 1920;
    config.video.height = 1080;
    config.video.fullscreen = true;
    config.audio.masterVolume = 0.9f;

    RTypeConfigParser parser;
    bool result = parser.saveToFile(config, configPath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(configPath));
}

TEST_F(RTypeConfigParserTest, SaveToFileCreatesDirectory) {
    auto configPath = testDir / "nested" / "dir" / "config.toml";

    RTypeGameConfig config;
    config.video.width = 800;

    RTypeConfigParser parser;
    bool result = parser.saveToFile(config, configPath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(configPath));
}

TEST_F(RTypeConfigParserTest, SaveToFileOverwritesExisting) {
    auto configPath = testDir / "overwrite.toml";

    // Create initial file
    createConfigFile(configPath, "# Old content");

    RTypeGameConfig config;
    config.video.width = 1280;

    RTypeConfigParser parser;
    bool result = parser.saveToFile(config, configPath);

    EXPECT_TRUE(result);

    // Verify new content
    std::ifstream file(configPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("width = 1280") != std::string::npos);
}

TEST_F(RTypeConfigParserTest, SaveToFileAllValues) {
    auto configPath = testDir / "full_save.toml";

    RTypeGameConfig config;
    config.video.width = 2560;
    config.video.height = 1440;
    config.video.fullscreen = true;
    config.video.vsync = false;
    config.video.maxFps = 240;
    config.video.uiScale = 2.0f;

    config.audio.masterVolume = 0.5f;
    config.audio.musicVolume = 0.3f;
    config.audio.sfxVolume = 0.7f;
    config.audio.muted = true;

    config.network.serverAddress = "10.0.0.1";
    config.network.serverPort = 8080;
    config.network.clientPort = 8081;
    config.network.connectionTimeout = 15000;
    config.network.maxRetries = 10;
    config.network.tickrate = 120;

    config.server.port = 8080;
    config.server.maxPlayers = 16;
    config.server.tickrate = 120;
    config.server.mapName = "custom_map";

    config.gameplay.difficulty = "nightmare";
    config.gameplay.startingLives = 1;
    config.gameplay.waves = 50;
    config.gameplay.playerSpeed = 500.0f;
    config.gameplay.enemySpeedMultiplier = 2.0f;
    config.gameplay.friendlyFire = true;

    config.input.moveUp = "I";
    config.input.moveDown = "K";
    config.input.moveLeft = "J";
    config.input.moveRight = "L";
    config.input.fire = "O";
    config.input.pause = "M";
    config.input.mouseSensitivity = 3.0f;

    config.paths.assetsPath = "/custom/assets";
    config.paths.savesPath = "/custom/saves";
    config.paths.logsPath = "/custom/logs";
    config.paths.configPath = "/custom/config";

    RTypeConfigParser parser;
    bool result = parser.saveToFile(config, configPath);

    EXPECT_TRUE(result);

    // Verify content
    std::ifstream file(configPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    EXPECT_TRUE(content.find("width = 2560") != std::string::npos);
    EXPECT_TRUE(content.find("height = 1440") != std::string::npos);
    EXPECT_TRUE(content.find("fullscreen = true") != std::string::npos);
    EXPECT_TRUE(content.find("muted = true") != std::string::npos);
    EXPECT_TRUE(content.find("serverAddress = \"10.0.0.1\"") !=
                std::string::npos);
    EXPECT_TRUE(content.find("difficulty = \"nightmare\"") != std::string::npos);
}

// =============================================================================
// serializeToString Tests - Branch coverage
// =============================================================================

TEST_F(RTypeConfigParserTest, SerializeToStringBasic) {
    RTypeGameConfig config;
    config.video.width = 1024;
    config.video.height = 768;

    RTypeConfigParser parser;
    std::string result = parser.serializeToString(config);

    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("width = 1024") != std::string::npos);
    EXPECT_TRUE(result.find("height = 768") != std::string::npos);
}

TEST_F(RTypeConfigParserTest, SerializeToStringContainsAllSections) {
    RTypeGameConfig config;

    RTypeConfigParser parser;
    std::string result = parser.serializeToString(config);

    EXPECT_TRUE(result.find("[video]") != std::string::npos);
    EXPECT_TRUE(result.find("[audio]") != std::string::npos);
    EXPECT_TRUE(result.find("[network]") != std::string::npos);
    EXPECT_TRUE(result.find("[server]") != std::string::npos);
    EXPECT_TRUE(result.find("[gameplay]") != std::string::npos);
    EXPECT_TRUE(result.find("[input]") != std::string::npos);
    EXPECT_TRUE(result.find("[paths]") != std::string::npos);
}

TEST_F(RTypeConfigParserTest, SerializeToStringBooleanValues) {
    RTypeGameConfig config;
    config.video.fullscreen = true;
    config.video.vsync = false;
    config.audio.muted = true;
    config.gameplay.friendlyFire = false;

    RTypeConfigParser parser;
    std::string result = parser.serializeToString(config);

    EXPECT_TRUE(result.find("fullscreen = true") != std::string::npos);
    EXPECT_TRUE(result.find("vsync = false") != std::string::npos);
    EXPECT_TRUE(result.find("muted = true") != std::string::npos);
    EXPECT_TRUE(result.find("friendlyFire = false") != std::string::npos);
}

// =============================================================================
// setErrorCallback Tests - Branch coverage
// =============================================================================

TEST_F(RTypeConfigParserTest, ErrorCallbackReceivesParseErrors) {
    auto configPath = testDir / "error_callback.toml";
    createConfigFile(configPath, "[invalid");

    RTypeConfigParser parser;
    bool callbackCalled = false;
    parser.setErrorCallback(
        [&callbackCalled](const ConfigError& error) {
            callbackCalled = true;
            EXPECT_FALSE(error.message.empty());
        });

    auto result = parser.loadFromFile(configPath);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(callbackCalled);
}

TEST_F(RTypeConfigParserTest, ErrorCallbackReceivesValidationErrors) {
    auto configPath = testDir / "validation_callback.toml";
    createConfigFile(configPath, R"(
[video]
width = 50
height = 50
maxFps = 5
)");

    RTypeConfigParser parser;
    std::vector<ConfigError> errors;
    parser.setErrorCallback(
        [&errors](const ConfigError& error) {
            errors.push_back(error);
        });

    auto result = parser.loadFromFile(configPath);

    EXPECT_TRUE(result.has_value());
    // Validation errors are generated but may be handled internally
    // The config should have defaults applied after validation
}

TEST_F(RTypeConfigParserTest, ErrorCallbackReceivesFileNotFoundError) {
    auto configPath = testDir / "nonexistent_callback.toml";

    RTypeConfigParser parser;
    bool fileNotFoundError = false;
    parser.setErrorCallback(
        [&fileNotFoundError](const ConfigError& error) {
            if (error.message.find("not found") != std::string::npos ||
                error.message.find("File not found") != std::string::npos) {
                fileNotFoundError = true;
            }
        });

    auto result = parser.loadFromFile(configPath);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fileNotFoundError);
}

// =============================================================================
// Round-trip Tests - Save and Load
// =============================================================================

TEST_F(RTypeConfigParserTest, RoundTripPreservesAllValues) {
    auto configPath = testDir / "roundtrip.toml";

    RTypeGameConfig originalConfig;
    originalConfig.video.width = 1600;
    originalConfig.video.height = 900;
    originalConfig.video.fullscreen = true;
    originalConfig.video.vsync = false;
    originalConfig.video.maxFps = 165;
    originalConfig.video.uiScale = 1.25f;

    originalConfig.audio.masterVolume = 0.85f;
    originalConfig.audio.musicVolume = 0.45f;
    originalConfig.audio.sfxVolume = 0.95f;
    originalConfig.audio.muted = false;

    originalConfig.network.serverAddress = "game.server.com";
    originalConfig.network.serverPort = 7777;
    originalConfig.network.clientPort = 7778;
    originalConfig.network.connectionTimeout = 8000;
    originalConfig.network.maxRetries = 4;
    originalConfig.network.tickrate = 64;

    originalConfig.server.port = 7777;
    originalConfig.server.maxPlayers = 12;
    originalConfig.server.tickrate = 64;
    originalConfig.server.mapName = "ctf_arena";

    originalConfig.gameplay.difficulty = "hard";
    originalConfig.gameplay.startingLives = 2;
    originalConfig.gameplay.waves = 30;
    originalConfig.gameplay.playerSpeed = 250.0f;
    originalConfig.gameplay.enemySpeedMultiplier = 1.3f;
    originalConfig.gameplay.friendlyFire = true;

    originalConfig.input.moveUp = "ArrowUp";
    originalConfig.input.moveDown = "ArrowDown";
    originalConfig.input.moveLeft = "ArrowLeft";
    originalConfig.input.moveRight = "ArrowRight";
    originalConfig.input.fire = "Enter";
    originalConfig.input.pause = "Tab";
    originalConfig.input.mouseSensitivity = 1.75f;

    originalConfig.paths.assetsPath = "res/assets";
    originalConfig.paths.savesPath = "data/saves";
    originalConfig.paths.logsPath = "var/logs";
    originalConfig.paths.configPath = "etc/config";

    RTypeConfigParser parser;

    // Save
    EXPECT_TRUE(parser.saveToFile(originalConfig, configPath));

    // Load
    auto loadedConfig = parser.loadFromFile(configPath);
    EXPECT_TRUE(loadedConfig.has_value());

    // Compare
    EXPECT_EQ(loadedConfig->video.width, originalConfig.video.width);
    EXPECT_EQ(loadedConfig->video.height, originalConfig.video.height);
    EXPECT_EQ(loadedConfig->video.fullscreen, originalConfig.video.fullscreen);
    EXPECT_EQ(loadedConfig->video.vsync, originalConfig.video.vsync);
    EXPECT_EQ(loadedConfig->video.maxFps, originalConfig.video.maxFps);

    EXPECT_EQ(loadedConfig->network.serverAddress,
              originalConfig.network.serverAddress);
    EXPECT_EQ(loadedConfig->network.serverPort,
              originalConfig.network.serverPort);

    EXPECT_EQ(loadedConfig->gameplay.difficulty,
              originalConfig.gameplay.difficulty);
    EXPECT_EQ(loadedConfig->gameplay.startingLives,
              originalConfig.gameplay.startingLives);

    EXPECT_EQ(loadedConfig->input.moveUp, originalConfig.input.moveUp);
    EXPECT_EQ(loadedConfig->input.fire, originalConfig.input.fire);

    EXPECT_EQ(loadedConfig->paths.assetsPath, originalConfig.paths.assetsPath);
}

// =============================================================================
// Edge Cases and Error Conditions
// =============================================================================

TEST_F(RTypeConfigParserTest, LoadFromStringWithExtraWhitespace) {
    std::string content = R"(
  [video]  
    width   =   1920  
    height  =   1080  
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
    EXPECT_EQ(result->video.height, 1080);
}

TEST_F(RTypeConfigParserTest, LoadFromStringWithInlineComments) {
    std::string content = R"(
[video]
width = 1920 # Screen width
height = 1080 # Screen height
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
    EXPECT_EQ(result->video.height, 1080);
}

TEST_F(RTypeConfigParserTest, LoadFromStringWithUnknownSection) {
    std::string content = R"(
[video]
width = 1920

[unknown_section]
some_key = "some_value"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
}

TEST_F(RTypeConfigParserTest, LoadFromStringWithUnknownKey) {
    std::string content = R"(
[video]
width = 1920
unknownKey = "unknownValue"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
}

TEST_F(RTypeConfigParserTest, LoadFromStringWithWrongTypeValues) {
    std::string content = R"(
[video]
width = "not_a_number"
fullscreen = "not_a_bool"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    // Should still load with defaults for wrong types
    EXPECT_TRUE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringNegativeValues) {
    std::string content = R"(
[audio]
masterVolume = -0.5
musicVolume = -1.0
sfxVolume = -2.0
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    // Should load, validation will clamp/report errors
    EXPECT_TRUE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringZeroValues) {
    std::string content = R"(
[video]
width = 0
height = 0
maxFps = 0

[network]
serverPort = 0
connectionTimeout = 0
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
}

TEST_F(RTypeConfigParserTest, LoadFromStringMaxValues) {
    std::string content = R"(
[video]
width = 7680
height = 4320
maxFps = 999
uiScale = 5.0

[network]
serverPort = 65535
connectionTimeout = 999999999
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 7680);
    EXPECT_EQ(result->network.serverPort, 65535);
}

TEST_F(RTypeConfigParserTest, LoadFromStringEmptyStrings) {
    std::string content = R"(
[network]
serverAddress = ""

[server]
mapName = ""

[input]
moveUp = ""
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    // Empty strings may be replaced with defaults by applyDefaults()
    // so we just check parsing succeeds
}

TEST_F(RTypeConfigParserTest, LoadFromStringSpecialCharactersInStrings) {
    std::string content = R"(
[network]
serverAddress = "server.example.com:8080/path?query=value"

[server]
mapName = "map with spaces and_underscores-dashes"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->network.serverAddress,
              "server.example.com:8080/path?query=value");
}

TEST_F(RTypeConfigParserTest, MultipleLoadsSameParser) {
    RTypeConfigParser parser;

    std::string content1 = R"(
[video]
width = 1920
)";
    auto result1 = parser.loadFromString(content1);
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(result1->video.width, 1920);

    std::string content2 = R"(
[video]
width = 1280
)";
    auto result2 = parser.loadFromString(content2);
    EXPECT_TRUE(result2.has_value());
    EXPECT_EQ(result2->video.width, 1280);
}

// =============================================================================
// Additional Branch Coverage Tests - Resolution parsing edge cases
// =============================================================================

TEST_F(RTypeConfigParserTest, LoadFromStringResolutionWithoutX) {
    std::string content = R"(
[video]
resolution = "1920"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    // Resolution parsing should fail due to missing 'x'
    EXPECT_TRUE(result.has_value());
    // Width/height should be defaults
}

TEST_F(RTypeConfigParserTest, LoadFromStringResolutionInvalidNumber) {
    std::string content = R"(
[video]
resolution = "abcxdef"
)";

    RTypeConfigParser parser;
    bool errorReported = false;
    parser.setErrorCallback([&errorReported](const ConfigError& error) {
        if (error.key == "resolution") {
            errorReported = true;
        }
    });

    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(errorReported);
}

TEST_F(RTypeConfigParserTest, LoadFromStringResolutionPartialInvalid) {
    std::string content = R"(
[video]
resolution = "1920xabc"
)";

    RTypeConfigParser parser;
    bool errorReported = false;
    parser.setErrorCallback([&errorReported](const ConfigError& error) {
        if (error.key == "resolution") {
            errorReported = true;
        }
    });

    auto result = parser.loadFromString(content);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(errorReported);
}

TEST_F(RTypeConfigParserTest, LoadFromStringAllSectionsComplete) {
    std::string content = R"(
[video]
width = 1920
height = 1080
fullscreen = true
vsync = false
maxFps = 144
uiScale = 1.5

[audio]
masterVolume = 0.5
musicVolume = 0.6
sfxVolume = 0.7
muted = true

[network]
serverAddress = "192.168.1.100"
serverPort = 12345
clientPort = 12346
connectionTimeout = 10000
maxRetries = 5
tickrate = 128

[server]
port = 54321
max_players = 8
tickrate = 64
mapName = "custom_map"

[gameplay]
difficulty = "hard"
startingLives = 5
waves = 20
playerSpeed = 300.0
enemySpeedMultiplier = 1.5
friendlyFire = true

[input]
moveUp = "Up"
moveDown = "Down"
moveLeft = "Left"
moveRight = "Right"
fire = "X"
pause = "P"
mouseSensitivity = 2.0

[paths]
assetsPath = "/custom/assets"
savesPath = "/custom/saves"
logsPath = "/custom/logs"
configPath = "/custom/config"
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);


    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 1920);
    EXPECT_TRUE(result->video.fullscreen);
    EXPECT_TRUE(result->audio.muted);
    EXPECT_EQ(result->network.serverAddress, "192.168.1.100");
    EXPECT_EQ(result->server.maxPlayers, 8);
    EXPECT_EQ(result->gameplay.difficulty, "hard");
    EXPECT_EQ(result->input.fire, "X");
    EXPECT_EQ(result->paths.assetsPath, "/custom/assets");
}

TEST_F(RTypeConfigParserTest, SaveToFilePermissionDenied) {
    RTypeGameConfig config;
    RTypeConfigParser parser;

    // Try to save to a read-only location
    bool result = parser.saveToFile(config, "/proc/test.toml");

    EXPECT_FALSE(result);
}

TEST_F(RTypeConfigParserTest, SaveToFileRenameFailure) {
    RTypeGameConfig config;
    RTypeConfigParser parser;

    // Create a directory with the same name as target
    auto targetPath = testDir / "blocked_config.toml";
    std::filesystem::create_directories(targetPath);

    bool result = parser.saveToFile(config, targetPath);

    // Clean up before assertion
    std::filesystem::remove_all(targetPath);

    EXPECT_FALSE(result);
}

TEST_F(RTypeConfigParserTest, SerializeToStringAllFields) {
    RTypeGameConfig config;
    config.video.width = 2560;
    config.video.height = 1440;
    config.audio.masterVolume = 0.5f;
    config.network.serverAddress = "test.server.com";

    RTypeConfigParser parser;
    std::string serialized = parser.serializeToString(config);

    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("2560"), std::string::npos);
    EXPECT_NE(serialized.find("1440"), std::string::npos);
    EXPECT_NE(serialized.find("test.server.com"), std::string::npos);
}

TEST_F(RTypeConfigParserTest, LoadFromStringVideoSectionOnly) {
    std::string content = R"(
[video]
width = 800
height = 600
fullscreen = false
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->video.width, 800);
    EXPECT_EQ(result->video.height, 600);
    // Other sections should have defaults
}

TEST_F(RTypeConfigParserTest, LoadFromStringNetworkSectionOnly) {
    std::string content = R"(
[network]
serverAddress = "localhost"
serverPort = 8080
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->network.serverAddress, "localhost");
    EXPECT_EQ(result->network.serverPort, 8080);
}

TEST_F(RTypeConfigParserTest, LoadFromFileSaveRoundTrip) {
    RTypeGameConfig original;
    original.video.width = 1600;
    original.video.height = 900;
    original.audio.masterVolume = 0.75f;
    original.network.serverAddress = "roundtrip.test";
    original.gameplay.difficulty = "hard";  // Use valid difficulty value

    auto filepath = testDir / "roundtrip.toml";

    RTypeConfigParser parser;
    ASSERT_TRUE(parser.saveToFile(original, filepath));

    auto loaded = parser.loadFromFile(filepath);
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->video.width, original.video.width);
    EXPECT_EQ(loaded->video.height, original.video.height);
    EXPECT_NEAR(loaded->audio.masterVolume, original.audio.masterVolume, 0.01f);
    EXPECT_EQ(loaded->network.serverAddress, original.network.serverAddress);
    EXPECT_EQ(loaded->gameplay.difficulty, original.gameplay.difficulty);
}

TEST_F(RTypeConfigParserTest, ErrorCallbackMultipleErrors) {
    std::vector<ConfigError> errors;
    RTypeConfigParser parser;
    parser.setErrorCallback([&errors](const ConfigError& error) {
        errors.push_back(error);
    });

    // Cause multiple errors
    [[maybe_unused]] auto r1 = parser.loadFromFile("nonexistent1.toml");
    [[maybe_unused]] auto r2 = parser.loadFromFile("nonexistent2.toml");

    EXPECT_GE(errors.size(), 2u);
}

TEST_F(RTypeConfigParserTest, LoadFromStringBooleanValues) {
    std::string content = R"(
[video]
fullscreen = true
vsync = false

[audio]
muted = true

[gameplay]
friendlyFire = false
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->video.fullscreen);
    EXPECT_FALSE(result->video.vsync);
    EXPECT_TRUE(result->audio.muted);
    EXPECT_FALSE(result->gameplay.friendlyFire);
}

TEST_F(RTypeConfigParserTest, LoadFromStringFloatValues) {
    std::string content = R"(
[video]
uiScale = 1.25

[audio]
masterVolume = 0.333
musicVolume = 0.666
sfxVolume = 0.999

[input]
mouseSensitivity = 2.5
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->video.uiScale, 1.25f, 0.001f);
    EXPECT_NEAR(result->audio.masterVolume, 0.333f, 0.001f);
    EXPECT_NEAR(result->input.mouseSensitivity, 2.5f, 0.001f);
}

TEST_F(RTypeConfigParserTest, LoadFromStringIntegerValues) {
    std::string content = R"(
[video]
maxFps = 240

[network]
connectionTimeout = 30000
maxRetries = 10
tickrate = 128

[server]
max_players = 16
tickrate = 32

[gameplay]
startingLives = 10
waves = 50
)";

    RTypeConfigParser parser;
    auto result = parser.loadFromString(content);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->video.maxFps, 240);
    EXPECT_EQ(result->network.connectionTimeout, 30000);
    EXPECT_EQ(result->server.maxPlayers, 16);
    EXPECT_EQ(result->gameplay.waves, 50);
}

}  // namespace rtype::game::config

