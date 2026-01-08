/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LevelCreatorScene.hpp
*/

/**
 * @file LevelCreatorScene.hpp
 * @brief Scene for creating and editing levels through a UI.
 *
 * Provides UI controls for editing level metadata (name, id, background, scroll
 * speed, boss) and configuring waves (spawns and powerups). Supports exporting
 * the level configuration to a TOML file via saveToToml().
 */

#ifndef R_TYPE_LEVELCREATORSCENE_HPP
#define R_TYPE_LEVELCREATORSCENE_HPP


#include <functional>
#include <memory>
#include <string>
#include <map>

#include "Graphic/KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "SceneManager/Scenes/AScene.hpp"
#include "Systems/TextInputSystem.hpp"

/** @brief Width (pixels) for a standard level UI section. */
constexpr float kLevelSectionWidth = 700.f;
/** @brief Height (pixels) for a standard level UI section. */
constexpr float kLevelSectionHeight = 500.f;
/** @brief Default left X position (pixels) for level sections on screen. */
constexpr float kLevelSectionPosLeft = 50.f;
/** @brief Default top Y position (pixels) for level sections on screen. */
constexpr float kLevelSectionPosTop = 200.f;

/**
 * @class LevelCreatorScene
 * @brief Handles the level creation editor UI and serialization.
 *
 * Manages UI elements and state for editing level metadata and waves. It
 * exposes helpers to update UI layout, save the configuration to TOML, and
 * manage temporary UI elements (like save status messages).
 */
class LevelCreatorScene : public AScene {
public:
    /**
     * @brief Single spawn configuration inside a wave.
     *
     * Contains enemy type, spawn delay and count as well as associated UI
     * entity identifiers for the input fields.
     */
    struct Spawn {
        std::string enemy = "basic";
        float delay = 0.0f;
        int count = 1;

        ECS::Entity enemyInputVal{0};
        ECS::Entity delayInputVal{0};
        ECS::Entity countInputVal{0};
    };

    /**
     * @brief Powerup configuration to spawn within a wave.
     *
     * Stores powerup id, spawn delay and vertical position, plus UI entity
     * identifiers for the input fields.
     */
    struct Powerup {
        std::string id = "health_small";
        float delay = 0.0f;
        float y = 540.0f;

        ECS::Entity idInputVal{0};
        ECS::Entity delayInputVal{0};
        ECS::Entity yInputVal{0};
    };

    /**
     * @brief A wave of spawns and powerups.
     *
     * Contains its index number, a spawn delay, and lists of spawn and
     * powerup configurations. Also keeps the UI entity for the spawn delay
     * input field.
     */
    struct Wave {
        int number;
        float spawn_delay = 1.0f;
        std::vector<Spawn> spawns;
        std::vector<Powerup> powerups;

        ECS::Entity spawnDelayInputVal{0};
    };

private:

    /** @brief Number of waves for legacy display; prefer `_waves.size()` for authoritative count. */
    unsigned int _nbrWaves = 0;

    /**
     * @name Level metadata input entities
     * Entities for UI inputs that represent the level metadata (name, id, background, etc.).
     * @{
     */
    /** @brief Entity id for the level name input field. */
    ECS::Entity _levelNameInput;
    /** @brief Entity id for the level identifier input field. */
    ECS::Entity _levelIdInput;
    /** @brief Entity id for the background selection/input field. */
    ECS::Entity _bgInputInput;
    /** @brief Entity id for the scroll speed input field. */
    ECS::Entity _scrollSpeedInput;
    /** @brief Entity id for the boss identifier input field. */
    ECS::Entity _bossInput;
    /** @} */

    /** @brief List of waves defined for the level. */
    std::vector<Wave> _waves;
    /** @brief Index of the current wave being edited; -1 when none is selected. */
    int _currentWaveIndex = -1;
    
    /** @brief Dynamic UI entities created by the scene (labels, inputs, buttons). */
    std::vector<ECS::Entity> _uiEntities;
    /** @brief Entities used for the wave section layout (background/title rows and static layout elements). */
    std::vector<ECS::Entity> _waveUiEntities; 

    /**
     * @brief Represents a scrollable section of UI and its tracked entities.
     *
     * Each section has an identifier and visible bounds. The `entities` vector
     * pairs an entity with a Y offset (relative to the section's top) so that
     * the scene can update absolute positions when scrolling.
     */
    struct ScrollSection {
        /** @brief Unique identifier for the section. */
        std::string id;
        /** @brief Visible rectangle area of the section in screen coordinates. */
        rtype::display::Rect<float> bounds;
        /** @brief Current vertical scroll offset in pixels (positive moves content up). */
        float currentScroll = 0.0f;
        /** @brief Maximum scrollable offset computed from content size. */
        float maxScroll = 0.0f;
        /** @brief Entities contained in the section paired with their relative Y offset. */
        std::vector<std::pair<ECS::Entity, float>> entities;
    };
    /** @brief All scroll sections managed by this scene. */
    std::vector<ScrollSection> _sections;
    
    /**
     * @brief Adds an entity to a scrollable section.
     * @param sectionId Identifier of the section to attach the entity to.
     * @param entity The ECS entity that should be tracked by the section.
     * @param relativeY Vertical offset (in pixels) of the entity relative to the
     *        top of the section bounds. Used to compute absolute Y when scrolling.
     *
     * This registers the entity with the internal section so it will be moved
     * when the section's scroll position changes.
     */
    void addElementToSection(const std::string& sectionId, ECS::Entity entity, float relativeY);
    /**
     * @brief Creates a new scrollable section with a title and bounds.
     * @param id Unique identifier for the section.
     * @param title Display title shown in the UI.
     * @param bounds Screen rectangle that defines the section's visible area.
     *
     * The created section is appended to the scene's _sections list and used to
     * group related UI entities for independent scrolling.
     */
    void createSection(const std::string& id, const std::string& title, const rtype::display::Rect<float>& bounds);

    /**
     * @brief Text input handling system.
     *
     * Owned reference to the TextInputSystem used by this scene to process and
     * update `TextInput` components (cursor, editing, validation). It is
     * initialized by the scene and used on each update to respond to keyboard
     * input for active fields.
     */
    std::shared_ptr<rtype::games::rtype::client::TextInputSystem>
        _textInputSystem;

    /**
     * @brief Callback used to request a scene switch.
     * @param scene The target scene to switch to.
     */
    std::function<void(const SceneManager::Scene &)> _switchToScene;
    
    /**
     * @brief Temporary status message entity.
     *
     * Holds an entity id pointing to a transient message shown after operations
     * such as saving (e.g., "File saved as ..."). The scene ensures only one
     * status message exists by removing the previous entity before creating a new one.
     */
    ECS::Entity _statusMessageEntity{0};

    /**
     * @brief Refreshes the wave configuration UI to reflect `_waves[_currentWaveIndex]`.
     *
     * Rebuilds or updates UI elements for the current wave (spawn/powerup rows,
     * labels, input fields) and applies stored values to the input components.
     */
    void refreshWaveUI();

    /**
     * @brief Recomputes absolute positions for section entities after scroll changes.
     *
     * Updates entity transforms based on each section's currentScroll so that
     * scrolling is visually reflected in the scene.
     */
    void updateScrollPositions();

    /**
     * @brief Save current wave fields into `_waves[_currentWaveIndex]`.
     *
     * Reads values from input entities and writes them to the Wave struct so
     * that switching waves or exporting reflects the edited values.
     */
    void saveCurrentWaveStats();

    /**
     * @brief Serializes the current level configuration to a TOML file.
     *
     * Uses the current UI values to create a TOML file on disk. On success, it
     * spawns a temporary green status message entity to notify the user.
     */
    void saveToToml();

    /**
     * @brief Adds a new wave to the level.
     *
     * Appends a default-initialized `Wave`, switches the current index, and
     * refreshes the UI to edit the new wave.
     */
    void addWave();

    /**
     * @brief Switches to an existing wave index and updates the UI.
     * @param index Index of the wave to switch to (0-based).
     */
    void switchWave(int index);

    /**
     * @brief Returns the current string value of a TextInput entity.
     * @param entity The TextInput entity to query.
     * @return The current textual value stored in the input component.
     *
     * Helper used when reading UI data prior to saving to TOML or when copying
     * values between the UI and internal data structures.
     */
    std::string getInputValue(ECS::Entity entity);

public:
    /**
     * @brief Construct a LevelCreatorScene.
     * @param ecs Shared registry used to spawn and manage entities.
     * @param assetsManager Asset manager used by widgets and text rendering.
     * @param window Display reference for coordinate conversions if required.
     * @param keybinds Keyboard action mappings used by input handling.
     * @param audio Audio subsystem for button/notification sounds.
     * @param switchToScene Callback invoked to request scene switches.
     */
    LevelCreatorScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<::rtype::display::IDisplay> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<AudioLib> audio,
        std::function<void(const SceneManager::Scene&)> switchToScene);

    /**
     * @brief Destroy the LevelCreatorScene and clean up UI entities.
     */
    ~LevelCreatorScene() override;

    /**
     * @brief Process a display event (keyboard, mouse).
     * @param e Event received from the display.
     *
     * Forwards events to input systems and handles UI interactions like button clicks.
     */
    void pollEvents(const rtype::display::Event& e) override;

    /**
     * @brief Update scene state.
     * @param dt Delta time since last frame in seconds.
     *
     * Runs input system updates, animations and any time-based transitions.
     */
    void update(float dt) override;

    /**
     * @brief Render the scene UI to the provided window.
     * @param window Display interface to draw onto.
     */
    void render(std::shared_ptr<::rtype::display::IDisplay> window) override;
};


#endif //R_TYPE_LEVELCREATORSCENE_HPP