/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for accessibility helpers, boxing overlay, and input bindings
*/

#include <gtest/gtest.h>

#include <memory>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundChannel.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "ECS.hpp"
#include "client/GameAction.hpp"
#include "client/Graphic/Accessibility.hpp"
#include "client/Graphic/KeyboardActions.hpp"
#include "games/rtype/client/Components/BoxingComponent.hpp"
#include "games/rtype/client/Components/HiddenComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/SoundComponent.hpp"
#include "games/rtype/client/Components/ZIndexComponent.hpp"
#include "games/rtype/client/GameScene/VisualCueFactory.hpp"
#include "games/rtype/client/Systems/BoxingSystem.hpp"
#include "games/rtype/client/Systems/EventSystem.hpp"
#include "AudioLib/AudioLib.hpp"
#include "games/rtype/shared/Components/LifetimeComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace client = rtype::games::rtype::client;
namespace shared = rtype::games::rtype::shared;

// =============================================================================
// KeyboardActions Tests
// =============================================================================

TEST(KeyboardActionsTest, DefaultBindingsArePresent) {
    KeyboardActions actions;

    auto moveUp = actions.getKeyBinding(GameAction::MOVE_UP);
    auto moveDown = actions.getKeyBinding(GameAction::MOVE_DOWN);
    auto moveLeft = actions.getKeyBinding(GameAction::MOVE_LEFT);
    auto moveRight = actions.getKeyBinding(GameAction::MOVE_RIGHT);
    auto shoot = actions.getKeyBinding(GameAction::SHOOT);
    auto pause = actions.getKeyBinding(GameAction::PAUSE);
    auto changeAmmo = actions.getKeyBinding(GameAction::CHANGE_AMMO);

    ASSERT_TRUE(moveUp.has_value());
    ASSERT_TRUE(moveDown.has_value());
    ASSERT_TRUE(moveLeft.has_value());
    ASSERT_TRUE(moveRight.has_value());
    ASSERT_TRUE(shoot.has_value());
    ASSERT_TRUE(pause.has_value());
    ASSERT_TRUE(changeAmmo.has_value());

    EXPECT_EQ(moveUp.value(), sf::Keyboard::Key::Up);
    EXPECT_EQ(moveDown.value(), sf::Keyboard::Key::Down);
    EXPECT_EQ(moveLeft.value(), sf::Keyboard::Key::Left);
    EXPECT_EQ(moveRight.value(), sf::Keyboard::Key::Right);
    EXPECT_EQ(shoot.value(), sf::Keyboard::Key::Space);
    EXPECT_EQ(pause.value(), sf::Keyboard::Key::Escape);
    EXPECT_EQ(changeAmmo.value(), sf::Keyboard::Key::Tab);

    auto shootButton = actions.getJoyButtonBinding(GameAction::SHOOT);
    auto pauseButton = actions.getJoyButtonBinding(GameAction::PAUSE);
    auto changeAmmoButton = actions.getJoyButtonBinding(GameAction::CHANGE_AMMO);
    auto moveUpAxis = actions.getJoyAxisBinding(GameAction::MOVE_UP);
    auto moveLeftAxis = actions.getJoyAxisBinding(GameAction::MOVE_LEFT);

    ASSERT_TRUE(shootButton.has_value());
    ASSERT_TRUE(pauseButton.has_value());
    ASSERT_TRUE(changeAmmoButton.has_value());
    ASSERT_TRUE(moveUpAxis.has_value());
    ASSERT_TRUE(moveLeftAxis.has_value());

    EXPECT_EQ(shootButton.value(), 0u);
    EXPECT_EQ(pauseButton.value(), 7u);
    EXPECT_EQ(changeAmmoButton.value(), 2u);
    EXPECT_EQ(moveUpAxis.value(), sf::Joystick::Axis::Y);
    EXPECT_EQ(moveLeftAxis.value(), sf::Joystick::Axis::X);
}

TEST(KeyboardActionsTest, UpdatesBindingsAndReverseLookupWorks) {
    KeyboardActions actions;

    actions.setKeyBinding(GameAction::SHOOT, sf::Keyboard::Key::LControl);
    auto shootKey = actions.getKeyBinding(GameAction::SHOOT);
    ASSERT_TRUE(shootKey.has_value());
    EXPECT_EQ(shootKey.value(), sf::Keyboard::Key::LControl);

    auto reverse = actions.getKeyBinding(sf::Keyboard::Key::LControl);
    ASSERT_TRUE(reverse.has_value());
    EXPECT_EQ(reverse.value(), GameAction::SHOOT);

    actions.setJoyButtonBinding(GameAction::PAUSE, 5u);
    auto pauseButton = actions.getJoyButtonBinding(GameAction::PAUSE);
    ASSERT_TRUE(pauseButton.has_value());
    EXPECT_EQ(pauseButton.value(), 5u);
}

TEST(KeyboardActionsTest, JoyAxisBindingsAndInversion) {
    KeyboardActions actions;

    actions.setJoyAxisBinding(GameAction::MOVE_LEFT, sf::Joystick::Axis::PovX);
    auto axis = actions.getJoyAxisBinding(GameAction::MOVE_LEFT);
    ASSERT_TRUE(axis.has_value());
    EXPECT_EQ(axis.value(), sf::Joystick::Axis::PovX);

    EXPECT_FALSE(actions.isJoyAxisInverted(GameAction::MOVE_LEFT));
    actions.setJoyAxisInverted(GameAction::MOVE_LEFT, true);
    EXPECT_TRUE(actions.isJoyAxisInverted(GameAction::MOVE_LEFT));
}

TEST(KeyboardActionsTest, InputModeCanBeChanged) {
    KeyboardActions actions;
    actions.setInputMode(InputMode::Controller);
    EXPECT_EQ(actions.getInputMode(), InputMode::Controller);
    actions.setInputMode(InputMode::Keyboard);
    EXPECT_EQ(actions.getInputMode(), InputMode::Keyboard);
}

TEST(KeyboardActionsTest, XboxButtonNamesAreReadable) {
    EXPECT_EQ(KeyboardActions::getXboxButtonName(0), "A");
    EXPECT_EQ(KeyboardActions::getXboxButtonName(7), "Start");
    EXPECT_EQ(KeyboardActions::getXboxButtonName(42), "Button 42");
}

TEST(KeyboardActionsTest, ReverseLookupReturnsEmptyForUnboundKey) {
    KeyboardActions actions;
    // Remove all bindings for a key by rebinding others
    actions.setKeyBinding(GameAction::SHOOT, sf::Keyboard::Key::A);
    
    // Try to look up a key that no action is bound to
    auto result = actions.getKeyBinding(sf::Keyboard::Key::B);
    EXPECT_FALSE(result.has_value());
}

TEST(KeyboardActionsTest, AxisInversionDefaultsToFalse) {
    KeyboardActions actions;
    
    for (int i = 0; i < 8; ++i) {
        auto action = static_cast<GameAction>(i);
        if (action != GameAction::NONE) {
            EXPECT_FALSE(actions.isJoyAxisInverted(action));
        }
    }
}

TEST(KeyboardActionsTest, MultipleButtonBindings) {
    KeyboardActions actions;
    
    actions.setJoyButtonBinding(GameAction::MOVE_UP, 10u);
    actions.setJoyButtonBinding(GameAction::MOVE_DOWN, 11u);
    actions.setJoyButtonBinding(GameAction::MOVE_LEFT, 12u);
    actions.setJoyButtonBinding(GameAction::MOVE_RIGHT, 13u);
    
    EXPECT_EQ(actions.getJoyButtonBinding(GameAction::MOVE_UP).value(), 10u);
    EXPECT_EQ(actions.getJoyButtonBinding(GameAction::MOVE_DOWN).value(), 11u);
    EXPECT_EQ(actions.getJoyButtonBinding(GameAction::MOVE_LEFT).value(), 12u);
    EXPECT_EQ(actions.getJoyButtonBinding(GameAction::MOVE_RIGHT).value(), 13u);
}

TEST(KeyboardActionsTest, OverwriteExistingAxisBinding) {
    KeyboardActions actions;
    
    auto original = actions.getJoyAxisBinding(GameAction::MOVE_UP);
    ASSERT_TRUE(original.has_value());
    
    actions.setJoyAxisBinding(GameAction::MOVE_UP, sf::Joystick::Axis::U);
    auto updated = actions.getJoyAxisBinding(GameAction::MOVE_UP);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated.value(), sf::Joystick::Axis::U);
}

// =============================================================================
// VisualCueFactory Tests
// =============================================================================

TEST(VisualCueFactoryTest, SkipsWhenAccessibilitySingletonMissing) {
    ECS::Registry registry;

    client::VisualCueFactory::createFlash(registry, {10.f, 20.f},
                                          sf::Color::Red);

    EXPECT_EQ(registry.countComponents<client::Rectangle>(), 0u);
    EXPECT_EQ(registry.countComponents<shared::LifetimeComponent>(), 0u);
}

TEST(VisualCueFactoryTest, SkipsWhenVisualCuesDisabled) {
    ECS::Registry registry;
    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, false});

    client::VisualCueFactory::createFlash(registry, {5.f, 5.f},
                                          sf::Color::Blue);

    EXPECT_EQ(registry.countComponents<client::Rectangle>(), 0u);
    EXPECT_EQ(registry.countComponents<shared::LifetimeComponent>(), 0u);
}

TEST(VisualCueFactoryTest, SpawnsFlashWhenEnabled) {
    ECS::Registry registry;
    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, true});

    const sf::Vector2f center{50.f, 60.f};
    const float size = 40.f;
    const float lifetime = 0.25f;
    const int depth = 77;

    client::VisualCueFactory::createFlash(registry, center, sf::Color::Cyan,
                                          size, lifetime, depth);

    bool found = false;
    registry
        .view<client::Rectangle, client::ZIndex, shared::TransformComponent,
              shared::LifetimeComponent>()
        .each([&](ECS::Entity, client::Rectangle& rect, client::ZIndex& z,
                  shared::TransformComponent& pos, shared::LifetimeComponent& life) {
            found = true;
            EXPECT_FLOAT_EQ(rect.size.first, size);
            EXPECT_FLOAT_EQ(rect.size.second, size);
            EXPECT_EQ(rect.currentColor, sf::Color::Cyan);
            EXPECT_EQ(rect.outlineColor, sf::Color::White);
            EXPECT_FLOAT_EQ(rect.outlineThickness, 3.f);
            EXPECT_EQ(z.depth, depth);
            EXPECT_FLOAT_EQ(pos.x, center.x - size / 2.f);
            EXPECT_FLOAT_EQ(pos.y, center.y - size / 2.f);
            EXPECT_FLOAT_EQ(life.remainingTime, lifetime);
        });

    EXPECT_TRUE(found);
}

TEST(VisualCueFactoryTest, MultipleFlashesWithDifferentColors) {
    ECS::Registry registry;
    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, true});

    client::VisualCueFactory::createFlash(registry, {10.f, 10.f}, sf::Color::Red);
    client::VisualCueFactory::createFlash(registry, {20.f, 20.f},
                                          sf::Color::Green);
    client::VisualCueFactory::createFlash(registry, {30.f, 30.f},
                                          sf::Color::Blue);

    size_t count = 0;
    registry.view<client::Rectangle>().each(
        [&](ECS::Entity, client::Rectangle&) { count++; });
    EXPECT_EQ(count, 3u);
}

TEST(VisualCueFactoryTest, FlashWithVariousLifetimes) {
    ECS::Registry registry;
    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, true});

    const float lifetimes[] = {0.1f, 0.5f, 1.0f};
    for (int i = 0; i < 3; ++i) {
        client::VisualCueFactory::createFlash(registry, {float(i * 10), 0.f},
                                              sf::Color::White, 16.f,
                                              lifetimes[i]);
    }

    int idx = 0;
    registry
        .view<shared::LifetimeComponent>()
        .each([&](ECS::Entity, shared::LifetimeComponent& life) {
            EXPECT_FLOAT_EQ(life.remainingTime, lifetimes[idx]);
            idx++;
        });
    EXPECT_EQ(idx, 3);
}

TEST(VisualCueFactoryTest, FlashWithDifferentDepthLayers) {
    ECS::Registry registry;
    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, false, true});

    client::VisualCueFactory::createFlash(registry, {0.f, 0.f}, sf::Color::Red,
                                          64.f, 0.5f, -10);
    client::VisualCueFactory::createFlash(registry, {10.f, 10.f}, sf::Color::Green,
                                          64.f, 0.5f, 0);
    client::VisualCueFactory::createFlash(registry, {20.f, 20.f}, sf::Color::Blue,
                                          64.f, 0.5f, 50);

    std::vector<int> depths;
    registry.view<client::ZIndex>().each(
        [&](ECS::Entity, client::ZIndex& z) { depths.push_back(z.depth); });
    EXPECT_EQ(depths.size(), 3u);
    EXPECT_TRUE(std::find(depths.begin(), depths.end(), -10) != depths.end());
    EXPECT_TRUE(std::find(depths.begin(), depths.end(), 0) != depths.end());
    EXPECT_TRUE(std::find(depths.begin(), depths.end(), 50) != depths.end());
}

// =============================================================================
// BoxingSystem Tests
// =============================================================================

TEST(BoxingSystemTest, EarlyReturnWhenHitboxesDisabled) {
    ECS::Registry registry;
    auto target = std::make_shared<sf::RenderTexture>(sf::Vector2u{200u, 200u});
    client::BoxingSystem system(target);

    sf::Image image({10u, 10u}, sf::Color::White);
    sf::Texture texture;
    ASSERT_TRUE(texture.loadFromImage(image));

    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Image>(entity, texture);
    registry.emplaceComponent<client::BoxingComponent>(
        entity, sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));

    system.update(registry, 0.016f);

    const auto& box = registry.getComponent<client::BoxingComponent>(entity);
    EXPECT_FLOAT_EQ(box.box.getSize().x, 1.f);
    EXPECT_FLOAT_EQ(box.box.getSize().y, 1.f);
}

TEST(BoxingSystemTest, UpdatesBoxesWhenHitboxesEnabled) {
    ECS::Registry registry;
    auto target = std::make_shared<sf::RenderTexture>(sf::Vector2u{200u, 200u});
    client::BoxingSystem system(target);

    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, true, false});

    sf::Image image({100u, 50u}, sf::Color::White);
    sf::Texture texture;
    ASSERT_TRUE(texture.loadFromImage(image));

    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Image>(entity, texture);
    registry.emplaceComponent<client::BoxingComponent>(
        entity, sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));

    auto& img = registry.getComponent<client::Image>(entity);
    img.sprite.setPosition(sf::Vector2f{30.f, 40.f});

    system.update(registry, 0.016f);

    const auto& box = registry.getComponent<client::BoxingComponent>(entity);
    EXPECT_FLOAT_EQ(box.box.getSize().x, 100.f);
    EXPECT_FLOAT_EQ(box.box.getSize().y, 50.f);
    EXPECT_FLOAT_EQ(box.box.getPosition().x, 30.f);
    EXPECT_FLOAT_EQ(box.box.getPosition().y, 40.f);
    EXPECT_EQ(box.box.getFillColor(), box.fillColor);
    EXPECT_EQ(box.box.getOutlineColor(), box.outlineColor);
    EXPECT_FLOAT_EQ(box.box.getOutlineThickness(), box.outlineThickness);
}

TEST(BoxingSystemTest, MultipleBoxesAreUpdated) {
    ECS::Registry registry;
    auto target = std::make_shared<sf::RenderTexture>(sf::Vector2u{400u, 400u});
    client::BoxingSystem system(target);

    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, true, false});

    sf::Image image({50u, 50u}, sf::Color::White);
    sf::Texture texture;
    ASSERT_TRUE(texture.loadFromImage(image));

    // Create 3 entities with boxing components
    for (int i = 0; i < 3; ++i) {
        auto entity = registry.spawnEntity();
        registry.emplaceComponent<client::Image>(entity, texture);
        registry.emplaceComponent<client::BoxingComponent>(
            entity, sf::FloatRect({float(i * 10), float(i * 10)}, {1.f, 1.f}));
        auto& img = registry.getComponent<client::Image>(entity);
        img.sprite.setPosition(sf::Vector2f{float(i * 20), float(i * 20)});
    }

    system.update(registry, 0.016f);

    size_t count = 0;
    registry.view<client::BoxingComponent>().each(
        [&](ECS::Entity, client::BoxingComponent& box) {
            EXPECT_FLOAT_EQ(box.box.getSize().x, 50.f);
            EXPECT_FLOAT_EQ(box.box.getSize().y, 50.f);
            count++;
        });
    EXPECT_EQ(count, 3u);
}

TEST(BoxingSystemTest, NoBoxingComponentsPresent) {
    ECS::Registry registry;
    auto target = std::make_shared<sf::RenderTexture>(sf::Vector2u{200u, 200u});
    client::BoxingSystem system(target);

    registry.setSingleton<AccessibilitySettings>(
        AccessibilitySettings{ColorBlindMode::None, 1.0f, true, false});

    // System should handle empty registry gracefully
    EXPECT_NO_THROW(system.update(registry, 0.016f));
}

TEST(BoxingSystemTest, EarlyReturnWhenNoAccessibilitySettingsSingleton) {
    ECS::Registry registry;
    auto target = std::make_shared<sf::RenderTexture>(sf::Vector2u{200u, 200u});
    client::BoxingSystem system(target);

    // No AccessibilitySettings singleton set
    sf::Image image({10u, 10u}, sf::Color::White);
    sf::Texture texture;
    ASSERT_TRUE(texture.loadFromImage(image));

    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Image>(entity, texture);
    registry.emplaceComponent<client::BoxingComponent>(
        entity, sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));

    // System should handle missing singleton gracefully
    EXPECT_NO_THROW(system.update(registry, 0.016f));
}

// =============================================================================
// AccessibilitySettings Tests
// =============================================================================

TEST(AccessibilitySettingsTest, DefaultColorMode) {
    AccessibilitySettings settings;
    EXPECT_EQ(settings.colorMode, ColorBlindMode::None);
}

TEST(AccessibilitySettingsTest, DefaultIntensity) {
    AccessibilitySettings settings;
    EXPECT_FLOAT_EQ(settings.intensity, 1.0f);
}

TEST(AccessibilitySettingsTest, DefaultToggleStates) {
    AccessibilitySettings settings;
    EXPECT_FALSE(settings.showHitboxes);
    EXPECT_FALSE(settings.showVisualCues);
}

TEST(AccessibilitySettingsTest, CustomInitialization) {
    AccessibilitySettings settings{ColorBlindMode::Deuteranopia, 0.8f, true,
                                   true};
    EXPECT_EQ(settings.colorMode, ColorBlindMode::Deuteranopia);
    EXPECT_FLOAT_EQ(settings.intensity, 0.8f);
    EXPECT_TRUE(settings.showHitboxes);
    EXPECT_TRUE(settings.showVisualCues);
}

TEST(AccessibilitySettingsTest, AllColorBlindModes) {
    static constexpr ColorBlindMode modes[] = {
        ColorBlindMode::None,          ColorBlindMode::Protanopia,
        ColorBlindMode::Deuteranopia,  ColorBlindMode::Tritanopia,
        ColorBlindMode::Achromatopsia, ColorBlindMode::HighContrast};

    for (auto mode : modes) {
        AccessibilitySettings settings{mode, 1.0f, false, false};
        EXPECT_EQ(settings.colorMode, mode);
    }
}

TEST(AccessibilitySettingsTest, IntensityVariations) {
    const float intensities[] = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f};
    for (float intensity : intensities) {
        AccessibilitySettings settings{ColorBlindMode::None, intensity, false,
                                       false};
        EXPECT_FLOAT_EQ(settings.intensity, intensity);
    }
}

TEST(AccessibilitySettingsTest, AllToggleCombinations) {
    bool toggleValues[] = {false, true};
    int count = 0;
    for (auto hitboxes : toggleValues) {
        for (auto cues : toggleValues) {
            AccessibilitySettings settings{ColorBlindMode::None, 1.0f, hitboxes,
                                           cues};
            EXPECT_EQ(settings.showHitboxes, hitboxes);
            EXPECT_EQ(settings.showVisualCues, cues);
            count++;
        }
    }
    EXPECT_EQ(count, 4);
}


// =============================================================================
// EventSystem Tests (joystick debouncing neutral path)
// =============================================================================

TEST(EventSystemTest, JoystickNeutralResetsDebounceStates) {
    // Prepare minimal window and audio lib
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    // Registry with simple buttons (Rectangle + UserEvent)
    ECS::Registry registry;
    std::vector<ECS::Entity> buttons;
    for (int i = 0; i < 2; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<client::Rectangle>(e, client::Rectangle({100, 30}, sf::Color::Blue, sf::Color::Red));
        registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());
        buttons.push_back(e);
    }

    // Create system and feed JoystickMoved events
    client::EventSystem system(window, audio);

    // First: move strongly down (>95) to trigger navigation
    sf::Event::JoystickMoved jmDown;
    jmDown.joystickId = 0;
    jmDown.axis = sf::Joystick::Axis::Y;
    jmDown.position = 96.0f;
    sf::Event evDown(jmDown);
    system.setEvent(evDown);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    // Then: return to neutral to hit the branch that resets debounced state
    sf::Event::JoystickMoved jmNeutral;
    jmNeutral.joystickId = 0;
    jmNeutral.axis = sf::Joystick::Axis::Y;
    jmNeutral.position = 0.0f;
    sf::Event evNeutral(jmNeutral);
    system.setEvent(evNeutral);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    // Finally: move strongly up (<-95) should trigger again (since neutral reset occurred)
    sf::Event::JoystickMoved jmUp;
    jmUp.joystickId = 0;
    jmUp.axis = sf::Joystick::Axis::Y;
    jmUp.position = -96.0f;
    sf::Event evUp(jmUp);
    system.setEvent(evUp);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    window->close();
}

TEST(EventSystemTest, JoystickButtonActivatesMenu) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    std::vector<ECS::Entity> buttons;
    for (int i = 0; i < 2; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<client::Rectangle>(e, client::Rectangle({100, 30}, sf::Color::Blue, sf::Color::Red));
        registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());
        buttons.push_back(e);
    }

    client::EventSystem system(window, audio);

    // Simulate joystick button 0 pressed to trigger activation branch
    sf::Event::JoystickButtonPressed jb;
    jb.joystickId = 0;
    jb.button = 0;
    sf::Event ev(jb);
    system.setEvent(ev);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    window->close();
}



TEST(EventSystemTest, JoystickXAxisDebounceAndNeutralReset) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    // Minimal interactive elements
    for (int i = 0; i < 3; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<client::Rectangle>(e, client::Rectangle({80, 20}, sf::Color::Yellow, sf::Color::Black));
        registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());
    }

    client::EventSystem system(window, audio);

    // Move strongly right (>95) to trigger X-axis navigation
    sf::Event::JoystickMoved jmRight;
    jmRight.joystickId = 0;
    jmRight.axis = sf::Joystick::Axis::X;
    jmRight.position = 97.0f;
    sf::Event evRight(jmRight);
    system.setEvent(evRight);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    // Return to neutral to reset debounce for X-axis
    sf::Event::JoystickMoved jmNeutral;
    jmNeutral.joystickId = 0;
    jmNeutral.axis = sf::Joystick::Axis::X;
    jmNeutral.position = 0.0f;
    sf::Event evNeutral(jmNeutral);
    system.setEvent(evNeutral);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    // Move strongly left (<-95) should be accepted after neutral
    sf::Event::JoystickMoved jmLeft;
    jmLeft.joystickId = 0;
    jmLeft.axis = sf::Joystick::Axis::X;
    jmLeft.position = -98.0f;
    sf::Event evLeft(jmLeft);
    system.setEvent(evLeft);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    window->close();
}

TEST(EventSystemTest, JoystickAxisSmallValuesDoNotTriggerNavigation) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    auto e = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(e, client::Rectangle({80, 20}, sf::Color::White, sf::Color::Black));
    registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());

    client::EventSystem system(window, audio);

    // Values within (-95, 95) should be ignored (no debounce trigger)
    for (float pos : {5.0f, -10.0f, 20.0f, -30.0f, 0.0f}) {
        sf::Event::JoystickMoved jm;
        jm.joystickId = 0;
        jm.axis = sf::Joystick::Axis::Y;
        jm.position = pos;
        sf::Event ev(jm);
        system.setEvent(ev);
        EXPECT_NO_THROW(system.update(registry, 0.0f));
    }

    window->close();
}

TEST(EventSystemTest, JoystickRepeatedLargeMovementWithoutNeutralIsDebounced) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    for (int i = 0; i < 2; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<client::Rectangle>(e, client::Rectangle({80, 20}, sf::Color::Cyan, sf::Color::Black));
        registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());
    }

    client::EventSystem system(window, audio);

    // First large movement triggers
    sf::Event::JoystickMoved jm1;
    jm1.joystickId = 0;
    jm1.axis = sf::Joystick::Axis::Y;
    jm1.position = 99.0f;
    sf::Event ev1(jm1);
    system.setEvent(ev1);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    // Subsequent large movement without neutral should be debounced (no repeat)
    sf::Event::JoystickMoved jm2;
    jm2.joystickId = 0;
    jm2.axis = sf::Joystick::Axis::Y;
    jm2.position = 98.0f;
    sf::Event ev2(jm2);
    system.setEvent(ev2);
    EXPECT_NO_THROW(system.update(registry, 0.0f));

    window->close();
}


TEST(EventSystemTest, MouseHoverPressReleaseFlow) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(
        entity, client::Rectangle({80.f, 40.f}, sf::Color::White,
                                  sf::Color::Yellow));
    registry.emplaceComponent<client::UserEvent>(entity, client::UserEvent());

    auto& rect = registry.getComponent<client::Rectangle>(entity);
    rect.rectangle.setSize({80.f, 40.f});
    rect.rectangle.setPosition({0.f, 0.f});

    client::EventSystem system(window, audio);

    sf::Event::MouseMoved moveInside;
    moveInside.position = {10, 10};
    sf::Event evMove(moveInside);
    system.setEvent(evMove);
    system.update(registry, 0.0f);

    auto& state = registry.getComponent<client::UserEvent>(entity);
    EXPECT_TRUE(state.isHovered);
    EXPECT_FALSE(state.isPressed);
    EXPECT_FALSE(state.isReleased);
    EXPECT_FALSE(state.idle);

    sf::Event::MouseButtonPressed press;
    press.button = sf::Mouse::Button::Left;
    press.position = {10, 10};
    sf::Event evPress(press);
    system.setEvent(evPress);
    system.update(registry, 0.0f);
    EXPECT_TRUE(state.isPressed);

    sf::Event::MouseButtonReleased release;
    release.button = sf::Mouse::Button::Left;
    release.position = {10, 10};
    sf::Event evRelease(release);
    system.setEvent(evRelease);
    system.update(registry, 0.0f);

    EXPECT_TRUE(state.isReleased);
    EXPECT_FALSE(state.isPressed);

    window->close();
}

TEST(EventSystemTest, MouseLeaveWhilePressedResetsFlags) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(
        entity, client::Rectangle({60.f, 30.f}, sf::Color::Red,
                                  sf::Color::Blue));
    registry.emplaceComponent<client::UserEvent>(entity, client::UserEvent());

    auto& rect = registry.getComponent<client::Rectangle>(entity);
    rect.rectangle.setSize({60.f, 30.f});
    rect.rectangle.setPosition({0.f, 0.f});

    client::EventSystem system(window, audio);

    // Start with hover + pressed state
    auto& state = registry.getComponent<client::UserEvent>(entity);
    state.isHovered = true;
    state.isPressed = true;

    sf::Event::MouseMoved moveOutside;
    moveOutside.position = {200, 200};
    sf::Event evMove(moveOutside);
    system.setEvent(evMove);
    system.update(registry, 0.0f);

    EXPECT_FALSE(state.isPressed);
    EXPECT_FALSE(state.isHovered);

    window->close();
}

TEST(EventSystemTest, HiddenComponentSkipsInteraction) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(
        entity, client::Rectangle({50.f, 50.f}, sf::Color::Green,
                                  sf::Color::Yellow));
    registry.emplaceComponent<client::UserEvent>(entity, client::UserEvent());
    registry.emplaceComponent<client::HiddenComponent>(
        entity, client::HiddenComponent{true});

    auto& rect = registry.getComponent<client::Rectangle>(entity);
    rect.rectangle.setSize({50.f, 50.f});
    rect.rectangle.setPosition({0.f, 0.f});

    client::EventSystem system(window, audio);

    sf::Event::MouseMoved moveInside;
    moveInside.position = {10, 10};
    sf::Event evMove(moveInside);
    system.setEvent(evMove);
    system.update(registry, 0.0f);

    const auto& state = registry.getComponent<client::UserEvent>(entity);
    EXPECT_FALSE(state.isHovered);
    EXPECT_TRUE(state.idle);

    window->close();
}

TEST(EventSystemTest, NullWindowPreventsPointCheck) {
    std::shared_ptr<sf::RenderWindow> window;
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(
        entity, client::Rectangle({40.f, 40.f}, sf::Color::Black,
                                  sf::Color::White));
    registry.emplaceComponent<client::UserEvent>(entity, client::UserEvent());

    client::EventSystem system(window, audio);

    sf::Event::MouseMoved moveInside;
    moveInside.position = {5, 5};
    sf::Event evMove(moveInside);
    system.setEvent(evMove);
    system.update(registry, 0.0f);

    const auto& state = registry.getComponent<client::UserEvent>(entity);
    EXPECT_FALSE(state.isHovered);
    EXPECT_TRUE(state.idle);
}

TEST(EventSystemTest, KeyboardNavigationWrapsAndActivates) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    std::vector<ECS::Entity> buttons;
    for (int i = 0; i < 2; ++i) {
        auto e = registry.spawnEntity();
        registry.emplaceComponent<client::Rectangle>(
            e, client::Rectangle({80, 20}, sf::Color::Magenta,
                                 sf::Color::Cyan));
        registry.emplaceComponent<client::UserEvent>(e, client::UserEvent());
        buttons.push_back(e);
    }

    client::EventSystem system(window, audio);

    // Down key should select the first button when none are hovered
    sf::Event::KeyPressed down;
    down.code = sf::Keyboard::Key::Down;
    sf::Event evDown(down);
    system.setEvent(evDown);
    system.update(registry, 0.0f);
    EXPECT_TRUE(registry.getComponent<client::UserEvent>(buttons[0]).isHovered);

    // Up key should wrap to the last button
    sf::Event::KeyPressed up;
    up.code = sf::Keyboard::Key::Up;
    sf::Event evUp(up);
    system.setEvent(evUp);
    system.update(registry, 0.0f);
    EXPECT_FALSE(registry.getComponent<client::UserEvent>(buttons[0]).isHovered);
    EXPECT_TRUE(registry.getComponent<client::UserEvent>(buttons[1]).isHovered);

    // Enter triggers activation on the hovered button
    sf::Event::KeyPressed enter;
    enter.code = sf::Keyboard::Key::Enter;
    sf::Event evEnter(enter);
    system.setEvent(evEnter);
    system.update(registry, 0.0f);

    EXPECT_TRUE(registry.getComponent<client::UserEvent>(buttons[1]).isReleased);

    window->close();
}

TEST(EventSystemTest, HoverAndClickPlaysSoundsWhenAvailable) {
    auto window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({200u, 200u}), "TestWindow");
    auto audio = std::make_shared<AudioLib>();

    ECS::Registry registry;
    const auto entity = registry.spawnEntity();
    registry.emplaceComponent<client::Rectangle>(
        entity, client::Rectangle({40.f, 40.f}, sf::Color::White,
                                  sf::Color::Yellow));
    registry.emplaceComponent<client::UserEvent>(entity, client::UserEvent());

    // Sound buffers with minimal sample data
    auto buffer = std::make_shared<sf::SoundBuffer>();
    std::int16_t sample = 0;
    std::vector<sf::SoundChannel> channels = {sf::SoundChannel::Mono};
    ASSERT_TRUE(buffer->loadFromSamples(&sample, 1, 1, 44100, channels));

    registry.emplaceComponent<client::ButtonSoundComponent>(
        entity, client::ButtonSoundComponent{buffer, buffer});

    auto& rect = registry.getComponent<client::Rectangle>(entity);
    rect.rectangle.setSize({40.f, 40.f});
    rect.rectangle.setPosition({0.f, 0.f});

    client::EventSystem system(window, audio);

    // Hover triggers hover sound branch
    sf::Event::MouseMoved moveInside;
    moveInside.position = {5, 5};
    sf::Event evMove(moveInside);
    system.setEvent(evMove);
    system.update(registry, 0.0f);

    auto& state = registry.getComponent<client::UserEvent>(entity);
    EXPECT_TRUE(state.isHovered);

    // Click triggers click sound branch
    sf::Event::MouseButtonPressed press;
    press.button = sf::Mouse::Button::Left;
    press.position = {5, 5};
    sf::Event evPress(press);
    system.setEvent(evPress);
    system.update(registry, 0.0f);
    EXPECT_TRUE(state.isPressed);

    // Release should mark as released
    sf::Event::MouseButtonReleased release;
    release.button = sf::Mouse::Button::Left;
    release.position = {5, 5};
    sf::Event evRelease(release);
    system.setEvent(evRelease);
    system.update(registry, 0.0f);
    EXPECT_TRUE(state.isReleased);

    window->close();
}


