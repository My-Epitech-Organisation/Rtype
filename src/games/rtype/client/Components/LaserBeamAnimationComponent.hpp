/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamAnimationComponent - Multi-phase animation for laser beam
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::client {

/**
 * @enum LaserAnimPhase
 * @brief Animation phases for the laser beam
 */
enum class LaserAnimPhase : uint8_t {
    Startup = 0,  ///< Frames 0-6, plays once at spawn
    Loop,         ///< Frames 7-14, loops while firing
    End,          ///< Frames 15-17, plays once before destruction
    Destroyed     ///< Animation complete, entity can be removed
};

/**
 * @struct LaserBeamAnimationComponent
 * @brief Component for managing multi-phase laser beam animation
 *
 * The laser beam spritesheet is VERTICAL (frames stacked on top of each other).
 * Each frame is 3072x512 pixels (50% scaled), with 18 total frames:
 * - Startup: frames 0-6 (7 frames) - plays once when laser spawns
 * - Loop: frames 7-14 (8 frames) - loops while laser is active
 * - End: frames 15-17 (3 frames) - plays once when laser stops
 */
struct LaserBeamAnimationComponent {
    LaserAnimPhase phase{LaserAnimPhase::Startup};

    // Frame ranges (0-indexed)
    static constexpr int kStartupFirst = 0;
    static constexpr int kStartupLast = 6;  // 7 frames total
    static constexpr int kLoopFirst = 7;
    static constexpr int kLoopLast = 14;  // 8 frames total
    static constexpr int kEndFirst = 15;
    static constexpr int kEndLast = 17;  // 3 frames total

    // Frame dimensions (50% scaled sprite: 3072x9216 total)
    static constexpr int kFrameWidth = 3072;
    static constexpr int kFrameHeight = 512;
    static constexpr int kTotalFrames = 18;

    // Display scale (3072 * 0.2 = ~614 pixels on screen)
    static constexpr float kDisplayScale = 0.2f;

    // Animation state
    int currentFrame{0};
    float elapsedTime{0.0f};
    float frameDuration{0.08f};  // ~12.5 FPS

    // Set to true when server sends destroy event
    // Animation will transition to End phase and play out before destruction
    bool pendingDestroy{false};

    /**
     * @brief Check if animation is in startup phase
     */
    [[nodiscard]] bool isStartup() const noexcept {
        return phase == LaserAnimPhase::Startup;
    }

    /**
     * @brief Check if animation is in loop phase
     */
    [[nodiscard]] bool isLooping() const noexcept {
        return phase == LaserAnimPhase::Loop;
    }

    /**
     * @brief Check if animation is in end phase
     */
    [[nodiscard]] bool isEnding() const noexcept {
        return phase == LaserAnimPhase::End;
    }

    /**
     * @brief Check if animation is complete and entity can be destroyed
     */
    [[nodiscard]] bool isDestroyed() const noexcept {
        return phase == LaserAnimPhase::Destroyed;
    }

    /**
     * @brief Get the Y offset for the current frame in the vertical spritesheet
     */
    [[nodiscard]] int getTextureTopOffset() const noexcept {
        return currentFrame * kFrameHeight;
    }
};

}  // namespace rtype::games::rtype::client
