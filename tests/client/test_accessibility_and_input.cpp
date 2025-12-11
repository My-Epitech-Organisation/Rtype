/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for accessibility helpers, boxing overlay, and input bindings
*/

#include <gtest/gtest.h>

#include <memory>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ECS.hpp"
#include "client/GameAction.hpp"
#include "client/Graphic/Accessibility.hpp"
#include "client/Graphic/KeyboardActions.hpp"
#include "games/rtype/client/Components/BoxingComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/ZIndexComponent.hpp"
#include "games/rtype/client/GameScene/VisualCueFactory.hpp"
#include "games/rtype/client/Systems/BoxingSystem.hpp"
#include "games/rtype/shared/Components/LifetimeComponent.hpp"
#include "games/rtype/shared/Components/PositionComponent.hpp"

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
        .view<client::Rectangle, client::ZIndex, shared::Position,
              shared::LifetimeComponent>()
        .each([&](ECS::Entity, client::Rectangle& rect, client::ZIndex& z,
                  shared::Position& pos, shared::LifetimeComponent& life) {
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


