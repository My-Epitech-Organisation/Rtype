/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GraphicsConstants.hpp - Centralized graphics configuration constants
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_GRAPHICSCONSTANTS_HPP_
#define SRC_GAMES_RTYPE_CLIENT_GRAPHICSCONSTANTS_HPP_

namespace rtype::games::rtype::client {

/**
 * @brief Centralized graphics configuration constants
 *
 * This namespace contains all magic numbers and configuration values
 * used across the graphics subsystem. Modifying these values allows
 * easy tuning of visual parameters without code changes.
 */
namespace GraphicsConfig {

// ==========================================================================
// Window Configuration
// ==========================================================================

/// @brief Default window width in pixels
inline constexpr unsigned int WINDOW_WIDTH = 1920;

/// @brief Default window height in pixels
inline constexpr unsigned int WINDOW_HEIGHT = 1080;

// ==========================================================================
// Parallax Configuration
// ==========================================================================

/// @brief Background layer scroll factor (slowest)
inline constexpr float PARALLAX_BACKGROUND = 0.2f;

/// @brief Planet layer 1 scroll factor (fastest parallax layer)
inline constexpr float PARALLAX_PLANET_1 = 0.7f;

/// @brief Big Asteroids plan layer 1 scroll factor (fastest parallax layer)
inline constexpr float PARALLAX_BIG_ASTEROIDS = 0.7f;

/// @brief Small Asteroids plan layer 1 scroll factor (fastest parallax layer)
inline constexpr float PARALLAX_SMALL_ASTEROIDS = 0.42f;

/// @brief Asteroids fst plan layer 1 scroll factor (fastest parallax layer)
inline constexpr float PARALLAX_ASTEROIDS_FST_PLAN = 0.44f;

/// @brief Asteroids snd plan layer 1 scroll factor (fastest parallax layer)
inline constexpr float PARALLAX_ASTEROIDS_SND_PLAN = 0.35f;

/// @brief Planet layer 2 scroll factor (medium)
inline constexpr float PARALLAX_PLANET_2 = 0.4f;

/// @brief Planet layer 3 scroll factor (same as background)
inline constexpr float PARALLAX_PLANET_3 = 0.2f;

// ==========================================================================
// Scrolling Configuration
// ==========================================================================

/// @brief Background auto-scroll speed in pixels per second
inline constexpr float SCROLL_SPEED = 50.0f;

// ==========================================================================
// Projectile Configuration
// ==========================================================================

/// @brief Projectile speed in pixels per second
inline constexpr float PROJECTILE_SPEED_LASER = 800.0f;

/// @brief Projectile laser cooldown in seconds
inline constexpr float PROJECTILE_CD = 0.175f;

/// @brief Projectile lifeline in seconds
inline constexpr float LIFETIME_PROJECTILE = 5.0f;

inline constexpr float PROJECTILE_SPAWN_OFFSET_X = 80.0f;

// ==========================================================================
// Z-Index Layers
// ==========================================================================

/// @brief Z-index for far background layer
inline constexpr int ZINDEX_BACKGROUND = -10;

/// @brief Z-index for far sun layer
inline constexpr int ZINDEX_SUN = -9;

/// @brief Z-index for first planet asteroids layer
inline constexpr int ZINDEX_SND_PLAN_ASTEROIDS = -6;

/// @brief Z-index for parallax planet layers
inline constexpr int ZINDEX_PLANETS = -5;

/// @brief Z-index for app title
inline constexpr int ZINDEX_APP_TITLE = -1;

/// @brief Z-index for Big and small asteroids layers
inline constexpr int ZINDEX_BIG_SMALL_ASTEROIDS = -4;

/// @brief Z-index for first planet asteroids layer
inline constexpr int ZINDEX_FST_PLAN_ASTEROIDS = -4;

/// @brief Z-index for game entities (players, enemies, etc.)
inline constexpr int ZINDEX_ENTITIES = 0;

/// @brief Z-index for UI elements
inline constexpr int ZINDEX_UI = 10;

// ==========================================================================
// UI Configuration
// ==========================================================================

/// @brief Default outline thickness for UI rectangles
inline constexpr float UI_OUTLINE_THICKNESS = 2.0f;

/// @brief Default title font size
inline constexpr unsigned int TITLE_FONT_SIZE = 72;

/// @brief Default section title font size
inline constexpr unsigned int SECTION_TITLE_FONT_SIZE = 30;

/// @brief Section title X offset from section border
inline constexpr float SECTION_TITLE_OFFSET_X = 20.0f;

/// @brief Section title Y offset from section border
inline constexpr float SECTION_TITLE_OFFSET_Y = 10.0f;

/// @brief Default section background alpha
inline constexpr unsigned int SECTION_BG_ALPHA = 150;

// ==========================================================================
// Sprite Scaling
// ==========================================================================

/// @brief Astroneer vessel scale factor
inline constexpr float ASTRONEER_VESSEL_SCALE = 0.3f;

/// @brief Fake player (preview) scale factor
inline constexpr float FAKE_PLAYER_SCALE = 2.2f;

// ==========================================================================
// Player Configuration
// ==========================================================================

/// @brief Maximum number of player sprites (colors) available
inline constexpr unsigned int MAX_PLAYER_SPRITES = 4;

// ==========================================================================
// Obstacle Configuration
// ==========================================================================

/// @brief Obstacle nbr of sprites (variety)
inline constexpr unsigned int NBR_MAX_OBSTACLES = 11;

}  // namespace GraphicsConfig

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_GRAPHICSCONSTANTS_HPP_
