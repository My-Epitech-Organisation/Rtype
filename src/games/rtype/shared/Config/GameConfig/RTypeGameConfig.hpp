/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameConfig - R-Type specific game configuration schema
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_GAMECONFIG_RTYPEGAMECONFIG_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_GAMECONFIG_RTYPEGAMECONFIG_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace rtype::game::config {

/**
 * @brief Maximum number of players supported by the game
 */
inline constexpr uint32_t MAX_PLAYER_COUNT = 4;

/**
 * @brief Schema version for configuration file compatibility
 */
constexpr uint32_t CONFIG_SCHEMA_VERSION = 1;

/**
 * @brief Video/Graphics configuration section
 */
struct VideoConfig {
    uint32_t width = 1280;
    uint32_t height = 720;
    bool fullscreen = false;
    bool vsync = true;
    uint32_t maxFps = 60;
    float uiScale = 1.0F;

    bool operator==(const VideoConfig&) const = default;
};

/**
 * @brief Audio configuration section
 */
struct AudioConfig {
    float masterVolume = 1.0F;
    float musicVolume = 0.8F;
    float sfxVolume = 1.0F;
    bool muted = false;

    bool operator==(const AudioConfig&) const = default;
};

/**
 * @brief Network configuration section
 */
struct NetworkConfig {
    std::string serverAddress = "127.0.0.1";
    uint16_t serverPort = 4000;
    uint16_t clientPort = 0;  // 0 = auto-assign
    uint32_t connectionTimeout = 5000;
    uint32_t maxRetries = 3;
    uint32_t tickrate = 60;

    bool operator==(const NetworkConfig&) const = default;
};

/**
 * @brief Fonts configuration section
 */
struct FontsConfig {
    std::string MainFont;
    std::string TitleFont;

    bool operator==(const FontsConfig&) const = default;
};

/**
 * @brief Background Textures configuration section
 */

struct BackgroundTextureConfig {
    std::string background;
    std::string sun;
    std::string planet1;
    std::string planet2;
    std::string planet3;
    std::string bigAsteroids;
    std::string smallAsteroids;
    std::string fstPlanAsteroids;
    std::string sndPlanAsteroids;

    bool operator==(const BackgroundTextureConfig&) const = default;
};

/**
 * @brief Wall Textures configuration section
 */

struct WallTextureConfig {
    std::string engrenage1;
    std::string engrenage2;
    std::string metal1;
    std::string metal2;
    std::string metal3;
    std::string metal4;
    std::string panneau1;
    std::string panneau2;
    std::string panneau3;
    std::string truc;
    std::string tubeMetal;

    bool operator==(const WallTextureConfig&) const = default;
};

/**
 * @brief Textures configuration section
 */
struct TexturesConfig {
    BackgroundTextureConfig backgroundTexture;
    WallTextureConfig wallTexture;
    std::string Player;
    std::string EnemyNormal;
    std::string EnemyChaser;
    std::string EnemyShooter;
    std::string EnemyHeavy;
    std::string EnemyPatrol;
    std::string EnemyWave;
    std::string astroVessel;
    std::string missileLaser;
    std::string chargedShot;
    std::string forcePod;

    // Power-ups
    std::string healthSmall;
    std::string healthLarge;
    std::string speedBoost;
    std::string weaponUpgrade;
    std::string shield;
    std::string rapidFire;
    std::string damageBoost;
    std::string extraLife;
    std::string laserUpgrade;
    std::string laserBeam;

    bool operator==(const TexturesConfig&) const = default;
};

/**
 * @brief Music configuration section
 */
struct MusicConfig {
    std::string mainMenu;
    std::string game;
    std::string settings;
    std::string gameOver;

    bool operator==(const MusicConfig&) const = default;
};

/**
 * @brief Sfx configuration section
 */
struct SfxConfig {
    std::string hoverButton;
    std::string clickButton;
    std::string laser;
    std::string enemySpawn;
    std::string enemyDeath;
    std::string playerSpawn;
    std::string playerDeath;
    std::string forcePodLaunch;
    std::string forcePodReturn;
    std::string chargedShot;
    std::string chargedShotMax;

    bool operator==(const SfxConfig&) const = default;
};

/**
 * @brief Assets configuration section
 */
struct AssetsConfig {
    MusicConfig music;
    SfxConfig sfx;
    TexturesConfig textures;
    FontsConfig fonts;

    bool operator==(const AssetsConfig&) const = default;
};

/**
 * @brief Server-specific configuration section
 */
struct ServerConfig {
    uint16_t port = 4000;
    uint32_t maxPlayers = 8;
    uint32_t tickrate = 60;
    std::string mapName = "default";

    uint16_t adminPort = 8080;
    bool adminEnabled = true;
    bool adminLocalhostOnly = true;
    std::string adminToken = "";

    bool operator==(const ServerConfig&) const = default;
};

/**
 * @brief Laser weapon configuration
 */
struct LaserConfig {
    float damagePerSecond = 50.0F;  ///< DPS damage
    float startupDelay = 0.56F;     ///< Delay before damage activates (seconds)
    float maxDuration = 3.0F;       ///< Maximum fire duration (seconds)
    float cooldownDuration = 2.0F;  ///< Cooldown after release (seconds)
    float hitboxWidth = 614.0F;     ///< Beam hitbox width (pixels)
    float hitboxHeight = 50.0F;     ///< Beam hitbox height (pixels)
    float offsetX = 340.0F;         ///< Offset from player position (pixels)

    bool operator==(const LaserConfig&) const = default;
};

/**
 * @brief Gameplay configuration section
 */
struct GameplayConfig {
    std::string difficulty = "normal";
    uint32_t startingLives = 3;
    uint32_t waves = 10;
    float playerSpeed = 260.0F;
    float enemySpeedMultiplier = 1.0F;
    bool friendlyFire = false;

    LaserConfig laser;  ///< Laser weapon settings

    bool operator==(const GameplayConfig&) const = default;
};

/**
 * @brief Input/Controls configuration section
 */
struct InputConfig {
    std::string moveUp = "Up";
    std::string moveDown = "Down";
    std::string moveLeft = "Left";
    std::string moveRight = "Right";
    std::string fire = "Space";
    std::string pause = "Escape";
    float mouseSensitivity = 1.0F;

    bool operator==(const InputConfig&) const = default;
};

/**
 * @brief Paths configuration section
 */
struct PathsConfig {
    std::string assetsPath = "assets";
    std::string savesPath = "saves";
    std::string logsPath = "logs";
    std::string configPath = "config";

    bool operator==(const PathsConfig&) const = default;
};

/**
 * @brief Configuration validation error
 */
struct ConfigError {
    std::string section;
    std::string key;
    std::string message;

    [[nodiscard]] std::string toString() const {
        if (key.empty()) {
            return "[" + section + "] " + message;
        }
        return "[" + section + "." + key + "] " + message;
    }
};

/**
 * @class RTypeGameConfig
 * @brief Complete R-Type game configuration with validation and defaults
 *
 * Manages all game configuration sections with:
 * - Default values for missing keys
 * - Validation of value ranges
 * - Error reporting for invalid configurations
 *
 * Example usage:
 * @code
 * RTypeGameConfig config;
 * config.video.width = 1920;
 * config.video.height = 1080;
 *
 * auto errors = config.validate();
 * if (!errors.empty()) {
 *     for (const auto& error : errors) {
 *         LOG_ERROR("Config error: {}", error.toString());
 *     }
 * }
 * @endcode
 */
class RTypeGameConfig {
   public:
    /// @brief Schema version for this configuration
    uint32_t schemaVersion = CONFIG_SCHEMA_VERSION;

    /// @brief Video/Graphics settings
    VideoConfig video;

    /// @brief Audio settings
    AudioConfig audio;

    /// @brief Network settings
    NetworkConfig network;

    /// @brief Assets settings
    AssetsConfig assets;

    /// @brief Server settings (server-only)
    ServerConfig server;

    /// @brief Gameplay settings
    GameplayConfig gameplay;

    /// @brief Input/Controls settings
    InputConfig input;

    /// @brief Path settings
    PathsConfig paths;

    /**
     * @brief Validate all configuration values
     * @return Vector of validation errors (empty if valid)
     */
    [[nodiscard]] std::vector<ConfigError> validate() const;

    /**
     * @brief Apply default values to missing/invalid fields
     */
    void applyDefaults();

    /**
     * @brief Create a configuration with all default values
     * @return Default configuration
     */
    [[nodiscard]] static RTypeGameConfig createDefault();

    bool operator==(const RTypeGameConfig&) const = default;
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_GAMECONFIG_RTYPEGAMECONFIG_HPP_
