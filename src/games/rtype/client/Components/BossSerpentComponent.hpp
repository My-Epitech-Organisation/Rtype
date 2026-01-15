/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BossSerpentComponent.hpp - Client-side boss serpent visual state
**
** Sprite sheets (each 677x369, 5 frames of 135x369):
** - serpent_head.png: Head idle/movement animation
** - serpent_attack.png: Head attack animation
** - serpent_body.png: Body segment animation
** - serpent_tail.png: Tail animation
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_

#include <cmath>
#include <cstdint>
#include <string>

/**
 * @enum BossSerpentState
 * @brief Visual states for boss serpent animation
 */
enum class BossSerpentState : uint8_t {
    IDLE = 0,
    MOVE = 1,
    ATTACK = 2,
    DIE = 3
};

/**
 * @enum BossSerpentPartType
 * @brief Type of serpent body part for texture selection
 */
enum class BossSerpentPartType : uint8_t { HEAD = 0, BODY = 1, TAIL = 2 };

/**
 * @struct BossSerpentVisual
 * @brief Component for boss serpent visual state
 *
 * All 4 sprite sheets are 677x369 with 5 frames each.
 * Frame size: 135x369 pixels (677/5 ≈ 135.4, using 135)
 *
 * Texture names:
 * - "boss_serpent_head" for head idle/movement
 * - "boss_serpent_attack" for attack animation
 * - "boss_serpent_body" for body segments
 * - "boss_serpent_tail" for tail segment
 */
struct BossSerpentVisual {
    BossSerpentState state = BossSerpentState::MOVE;
    BossSerpentPartType partType = BossSerpentPartType::HEAD;
    float animationTimer = 0.0f;
    int currentFrame = 0;
    float prevX = 0.0f;
    float prevY = 0.0f;
    bool isAttacking = false;
    bool isDying = false;
    static constexpr int SHEET_WIDTH = 677;
    static constexpr int SHEET_HEIGHT = 369;
    static constexpr int FRAME_COUNT = 5;
    static constexpr int FRAME_WIDTH = 135;
    static constexpr int FRAME_HEIGHT = 369;
    static constexpr float ANIMATION_SPEED = 0.1f;
    static constexpr const char* TEXTURE_HEAD = "boss_serpent_head";
    static constexpr const char* TEXTURE_ATTACK = "boss_serpent_attack";
    static constexpr const char* TEXTURE_BODY = "boss_serpent_body";
    static constexpr const char* TEXTURE_TAIL = "boss_serpent_tail";

    /**
     * @brief Get the appropriate texture name based on state and part type
     */
    [[nodiscard]] const char* getTextureName() const;

    /**
     * @brief Get texture rect for current animation frame
     * @param x Output X position in sprite sheet
     * @param y Output Y position in sprite sheet
     * @param w Output frame width
     * @param h Output frame height
     */
    void getTextureRect(int& x, int& y, int& w, int& h) const;

    /**
     * @brief Advance animation frame
     * @param deltaTime Time elapsed since last update
     */
    void updateAnimation(float deltaTime);

    /**
     * @brief Reset animation to first frame
     */
    void resetAnimation();
};

/**
 * @struct BossSerpentBodyVisual
 * @brief Component for boss serpent body/tail segment visuals
 */
struct BossSerpentBodyVisual {
    BossSerpentPartType partType = BossSerpentPartType::BODY;
    int segmentIndex = 0;
    float animationTimer = 0.0f;
    int currentFrame = 0;

    /**
     * @brief Get the texture name for this segment
     */
    [[nodiscard]] const char* getTextureName() const;

    /**
     * @brief Get texture rect for current frame
     */
    void getTextureRect(int& x, int& y, int& w, int& h) const;

    /**
     * @brief Update animation
     */
    void updateAnimation(float deltaTime);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOSSSERPENTCOMPONENT_HPP_
