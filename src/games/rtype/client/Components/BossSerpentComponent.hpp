/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BossSerpentComponent.hpp - Client-side boss serpent visual state
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_

#include <cmath>
#include <cstdint>

/**
 * @enum BossSerpentState
 * @brief Visual states for boss serpent animation
 */
enum class BossSerpentState : uint8_t {
    SPAWN = 0,   ///< Spawn animation (egg/shell)
    IDLE = 1,    ///< Idle state
    MOVE = 2,    ///< Moving state (head rotation based on direction)
    ATTACK = 3,  ///< Attack animation (stretch)
    DIE = 4      ///< Death animation
};

/**
 * @struct BossSerpentVisual
 * @brief Component for boss serpent head visual state
 *
 * Manages the visual state machine for the serpent boss head.
 * - Head has 16 directional frames (0-15) for rotation
 * - Attack has 8 frames
 * - Spawn/Death use shell animation
 */
struct BossSerpentVisual {
    BossSerpentState state = BossSerpentState::MOVE;  // Start in MOVE state

    // Direction index (0-15) for 16-direction head sprites
    int directionIndex = 0;

    // Previous position for direction calculation
    float prevX = 0.0f;
    float prevY = 0.0f;

    // Animation timing
    float animationTimer = 0.0f;
    int currentFrame = 0;

    // State flags
    bool spawnComplete = true;  // Skip spawn animation for now
    bool isDying = false;

    // Sprite sheet: bdosTextureChaser.gif = 576x430 pixels
    // Analyzing the sheet structure:
    // - Image is 576 wide, likely 8 frames of 72px or 9 frames of 64px
    // - Using 64x64 head frames, 9 per row

    // Row 0-1: 16 directional head frames (64x64 each, 9 per row)
    static constexpr int HEAD_FRAME_WIDTH = 64;
    static constexpr int HEAD_FRAME_HEIGHT = 64;
    static constexpr int HEAD_FRAMES_PER_ROW = 9;
    static constexpr int HEAD_DIRECTION_COUNT = 16;

    // Row 2: Attack animation (wider frames)
    static constexpr int ATTACK_FRAME_WIDTH = 72;
    static constexpr int ATTACK_FRAME_HEIGHT = 64;
    static constexpr int ATTACK_FRAME_COUNT = 8;
    static constexpr int ATTACK_ROW_Y = 128;

    // Row 3-4: Body segments (smaller, ~48x48 or 36x36)
    static constexpr int BODY_FRAME_WIDTH = 36;
    static constexpr int BODY_FRAME_HEIGHT = 36;
    static constexpr int BODY_ROW_Y = 192;

    // Bottom rows: Shell/spawn animation
    static constexpr int SHELL_FRAME_WIDTH = 48;
    static constexpr int SHELL_FRAME_HEIGHT = 48;
    static constexpr int SHELL_ROW_Y = 300;
    static constexpr int SHELL_FRAME_COUNT = 10;

    /**
     * @brief Calculate direction index (0-15) from velocity
     * @param vx Velocity X
     * @param vy Velocity Y
     * @return Direction index 0-15 (0 = right, going counter-clockwise)
     */
    static int calculateDirectionIndex(float vx, float vy) {
        if (vx == 0.0f && vy == 0.0f) {
            return 0;
        }

        float angle = std::atan2(vy, vx);

        float degrees = angle * 180.0f / 3.14159265f;
        if (degrees < 0) {
            degrees += 360.0f;
        }

        int index = static_cast<int>((degrees + 11.25f) / 22.5f) % 16;
        return index;
    }

    /**
     * @brief Get texture rect for head based on direction
     */
    void getHeadRect(int& x, int& y, int& w, int& h) const {
        int row = directionIndex / HEAD_FRAMES_PER_ROW;
        int col = directionIndex % HEAD_FRAMES_PER_ROW;
        x = col * HEAD_FRAME_WIDTH;
        y = row * HEAD_FRAME_HEIGHT;
        w = HEAD_FRAME_WIDTH;
        h = HEAD_FRAME_HEIGHT;
    }

    /**
     * @brief Get texture rect for attack animation
     */
    void getAttackRect(int& x, int& y, int& w, int& h) const {
        x = currentFrame * ATTACK_FRAME_WIDTH;
        y = ATTACK_ROW_Y;
        w = ATTACK_FRAME_WIDTH;
        h = ATTACK_FRAME_HEIGHT;
    }

    /**
     * @brief Get texture rect for body segment
     * @param segmentIndex Which body segment (for variation)
     */
    static void getBodyRect(int segmentIndex, int& x, int& y, int& w, int& h) {
        int col = segmentIndex % 8;
        int row = segmentIndex / 8;
        x = col * BODY_FRAME_WIDTH;
        y = BODY_ROW_Y + row * BODY_FRAME_HEIGHT;
        w = BODY_FRAME_WIDTH;
        h = BODY_FRAME_HEIGHT;
    }

    /**
     * @brief Get texture rect for spawn/death shell animation
     */
    void getShellRect(int& x, int& y, int& w, int& h) const {
        x = currentFrame * SHELL_FRAME_WIDTH;
        y = SHELL_ROW_Y;
        w = SHELL_FRAME_WIDTH;
        h = SHELL_FRAME_HEIGHT;
    }
};

/**
 * @struct BossSerpentBodyVisual
 * @brief Component for boss serpent body segment visuals
 */
struct BossSerpentBodyVisual {
    int segmentIndex = 0;

    void getRect(int& x, int& y, int& w, int& h) const {
        BossSerpentVisual::getBodyRect(segmentIndex, x, y, w, h);
    }
};

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_
