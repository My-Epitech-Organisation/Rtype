/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ScrollComponent - Component for scrollable map elements
*/

#pragma once

namespace rtype::games::rtype::shared {

/**
 * @struct ScrollComponent
 * @brief Component for entities that scroll with the level
 *
 * Entities with this component will be moved leftward by the ScrollingSystem.
 * The initialX stores the original X position in level coordinates.
 */
struct ScrollComponent {
    float initialX = 0.0F;   ///< Initial X position in level coordinates
    bool isActive = false;   ///< Whether entity is currently in active zone
    bool hasSpawned = false; ///< Whether entity has been spawned into the game

    ScrollComponent() = default;
    explicit ScrollComponent(float x) : initialX(x) {}
};

/**
 * @struct LevelScrollState
 * @brief Component/State tracking the current scroll position of the level
 *
 * This is typically attached to a "level manager" entity or used as a singleton.
 * The scrollOffset represents how far the level has scrolled from the start.
 */
struct LevelScrollState {
    float scrollOffset = 0.0F;      ///< Current scroll offset (increases over time)
    float scrollSpeed = 50.0F;      ///< Scroll speed in pixels per second
    float levelWidth = 10000.0F;    ///< Total width of the level
    float viewportWidth = 1920.0F;  ///< Width of the visible viewport
    float spawnAheadDistance = 100.0F; ///< Spawn elements this far ahead of viewport
    float despawnBehindDistance = 100.0F; ///< Despawn elements this far behind viewport
    bool isPaused = false;          ///< Whether scrolling is paused

    /**
     * @brief Get the left edge of the visible area in level coordinates
     */
    [[nodiscard]] float getViewportLeft() const noexcept {
        return scrollOffset;
    }

    /**
     * @brief Get the right edge of the visible area in level coordinates
     */
    [[nodiscard]] float getViewportRight() const noexcept {
        return scrollOffset + viewportWidth;
    }

    /**
     * @brief Get the spawn threshold (elements spawn when they cross this)
     */
    [[nodiscard]] float getSpawnThreshold() const noexcept {
        return scrollOffset + viewportWidth + spawnAheadDistance;
    }

    /**
     * @brief Get the despawn threshold (elements despawn when they cross this)
     */
    [[nodiscard]] float getDespawnThreshold() const noexcept {
        return scrollOffset - despawnBehindDistance;
    }

    /**
     * @brief Check if level has completed scrolling
     */
    [[nodiscard]] bool isComplete() const noexcept {
        return scrollOffset >= levelWidth - viewportWidth;
    }

    /**
     * @brief Convert level X coordinate to screen X coordinate
     * @param levelX Position in level coordinates
     * @return Position in screen coordinates
     */
    [[nodiscard]] float levelToScreenX(float levelX) const noexcept {
        return levelX - scrollOffset;
    }

    /**
     * @brief Convert screen X coordinate to level X coordinate
     * @param screenX Position in screen coordinates
     * @return Position in level coordinates
     */
    [[nodiscard]] float screenToLevelX(float screenX) const noexcept {
        return screenX + scrollOffset;
    }
};

}  // namespace rtype::games::rtype::shared
