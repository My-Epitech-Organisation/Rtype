/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Stress tests for event handling and UI systems - tests performance
** of button interactions and event processing under load
*/

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <rtype/ecs.hpp>

#include "../../src/games/rtype/client/AllComponents.hpp"
#include "../../src/games/rtype/client/Systems/ButtonUpdateSystem.hpp"
#include "../../src/games/rtype/client/Systems/EventSystem.hpp"
#include "../../src/games/rtype/client/Systems/ResetTriggersSystem.hpp"
#include "AudioLib/AudioLib.hpp"

// Namespace aliases
namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

/**
 * @brief Test fixture for event system stress tests
 */
class EventStressTest : public ::testing::Test {
   protected:
    std::shared_ptr<ECS::Registry> registry;
    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<AudioLib> audioLib;

    std::unique_ptr<rc::EventSystem> eventSystem;
    std::unique_ptr<rc::ButtonUpdateSystem> buttonUpdateSystem;
    std::unique_ptr<rc::ResetTriggersSystem> resetTriggersSystem;

    std::mt19937 rng{99999};

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();

        window = std::make_shared<sf::RenderWindow>(
            sf::VideoMode({800, 600}), "EventStressTest", sf::Style::None);
        
        audioLib = std::make_shared<AudioLib>();

        eventSystem = std::make_unique<rc::EventSystem>(window, audioLib);
        buttonUpdateSystem = std::make_unique<rc::ButtonUpdateSystem>(window);
        resetTriggersSystem = std::make_unique<rc::ResetTriggersSystem>();
    }

    void TearDown() override {
        window->close();
        registry.reset();
    }

    template <typename Func>
    double measureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count();
    }

    template <typename Func>
    double measureAverageTime(std::size_t iterations, Func&& func) {
        double totalTime = 0.0;
        for (std::size_t i = 0; i < iterations; ++i) {
            totalTime += measureTime(std::forward<Func>(func));
        }
        return totalTime / static_cast<double>(iterations);
    }

    /**
     * @brief Create button entities with all required components
     */
    std::vector<ECS::Entity> createButtons(std::size_t count) {
        std::vector<ECS::Entity> entities;
        entities.reserve(count);

        std::uniform_real_distribution<float> posDist(0.0f, 700.0f);
        std::uniform_real_distribution<float> sizeDist(50.0f, 100.0f);

        for (std::size_t i = 0; i < count; ++i) {
            auto entity = registry->spawnEntity();

            float x = posDist(rng);
            float y = posDist(rng);
            float w = sizeDist(rng);
            float h = sizeDist(rng);

            registry->emplaceComponent<rs::TransformComponent>(entity, x, y);
            registry->emplaceComponent<rc::Rectangle>(
                entity, std::make_pair(w, h),
                sf::Color::Blue, sf::Color::Cyan);
            registry->emplaceComponent<rc::UserEvent>(entity);
            registry->emplaceComponent<rc::ButtonTag>(entity);

            // Setup the rectangle shape for collision detection
            auto& rect = registry->getComponent<rc::Rectangle>(entity);
            rect.rectangle.setSize({w, h});
            rect.rectangle.setPosition({x, y});

            entities.push_back(entity);
        }

        return entities;
    }

    /**
     * @brief Create rectangles (interactive areas without button functionality)
     */
    std::vector<ECS::Entity> createInteractiveRectangles(std::size_t count) {
        std::vector<ECS::Entity> entities;
        entities.reserve(count);

        std::uniform_real_distribution<float> posDist(0.0f, 700.0f);
        std::uniform_real_distribution<float> sizeDist(20.0f, 80.0f);

        for (std::size_t i = 0; i < count; ++i) {
            auto entity = registry->spawnEntity();

            float x = posDist(rng);
            float y = posDist(rng);
            float w = sizeDist(rng);
            float h = sizeDist(rng);

            registry->emplaceComponent<rs::TransformComponent>(entity, x, y);
            registry->emplaceComponent<rc::Rectangle>(
                entity, std::make_pair(w, h),
                sf::Color::Green, sf::Color::Yellow);
            registry->emplaceComponent<rc::UserEvent>(entity);

            auto& rect = registry->getComponent<rc::Rectangle>(entity);
            rect.rectangle.setSize({w, h});
            rect.rectangle.setPosition({x, y});

            entities.push_back(entity);
        }

        return entities;
    }
};

// =============================================================================
// UserEvent Component Stress Tests
// =============================================================================

TEST_F(EventStressTest, UserEventQuery_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    createInteractiveRectangles(COUNT);

    std::size_t queryCount = 0;
    double time = measureTime([this, &queryCount]() {
        registry->view<rc::UserEvent>().each(
            [&queryCount](auto entity, auto& event) {
                ++queryCount;
                event.isHovered = false;
            });
    });

    std::cout << "[PERF] UserEvent query (" << COUNT << " entities): "
              << time << " ms" << std::endl;

    EXPECT_EQ(queryCount, COUNT);
    EXPECT_LT(time, 10.0) << "UserEvent query too slow";
}

TEST_F(EventStressTest, ResetTriggersSystem_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    createInteractiveRectangles(COUNT);

    // Set some triggers
    registry->view<rc::UserEvent>().each([](auto, auto& event) {
        event.isReleased = true;
    });

    double time = measureTime([this]() {
        resetTriggersSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ResetTriggersSystem (" << COUNT << " entities): "
              << time << " ms" << std::endl;

    // Verify all were reset
    std::size_t stillTriggered = 0;
    registry->view<rc::UserEvent>().each([&stillTriggered](auto, auto& event) {
        if (event.isReleased) ++stillTriggered;
    });

    EXPECT_EQ(stillTriggered, 0);
    EXPECT_LT(time, 10.0) << "ResetTriggersSystem too slow";
}

TEST_F(EventStressTest, ResetTriggersSystem_100Iterations) {
    constexpr std::size_t COUNT = 500;
    constexpr std::size_t ITERATIONS = 100;
    createInteractiveRectangles(COUNT);

    double avgTime = measureAverageTime(ITERATIONS, [this]() {
        // Simulate frame: set some triggers, then reset
        registry->view<rc::UserEvent>().each([](auto, auto& event) {
            event.isReleased = true;
        });
        resetTriggersSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ResetTriggers avg (" << ITERATIONS << " iterations): "
              << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 5.0) << "Repeated reset too slow";
}

// =============================================================================
// ButtonUpdateSystem Stress Tests
// =============================================================================

TEST_F(EventStressTest, ButtonUpdateSystem_100Buttons) {
    constexpr std::size_t COUNT = 100;
    createButtons(COUNT);

    double time = measureTime([this]() {
        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ButtonUpdateSystem (" << COUNT << " buttons): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 20.0) << "ButtonUpdateSystem too slow";
}

TEST_F(EventStressTest, ButtonUpdateSystem_500Buttons) {
    constexpr std::size_t COUNT = 500;
    createButtons(COUNT);

    double time = measureTime([this]() {
        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ButtonUpdateSystem (" << COUNT << " buttons): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 50.0) << "ButtonUpdateSystem too slow";
}

TEST_F(EventStressTest, ButtonUpdateSystem_WithHoveredButtons) {
    constexpr std::size_t COUNT = 200;
    auto buttons = createButtons(COUNT);

    // Simulate half buttons being hovered
    std::size_t idx = 0;
    for (auto entity : buttons) {
        auto& event = registry->getComponent<rc::UserEvent>(entity);
        event.isHovered = (idx++ % 2 == 0);
    }

    double time = measureTime([this]() {
        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ButtonUpdate with hover (" << COUNT << " buttons): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 30.0) << "Button hover update too slow";
}

TEST_F(EventStressTest, ButtonUpdateSystem_60FrameSimulation) {
    constexpr std::size_t COUNT = 100;
    constexpr std::size_t FRAMES = 60;
    auto buttons = createButtons(COUNT);

    double avgTime = measureAverageTime(FRAMES, [this, &buttons]() {
        // Simulate changing hover states
        for (auto entity : buttons) {
            auto& event = registry->getComponent<rc::UserEvent>(entity);
            event.isHovered = !event.isHovered;
        }
        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] ButtonUpdate avg (60 frames, " << COUNT
              << " buttons): " << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 10.0) << "Button update frame average too slow";
}

// =============================================================================
// EventSystem Stress Tests
// =============================================================================

TEST_F(EventStressTest, EventSystem_MouseMove_500Rectangles) {
    constexpr std::size_t COUNT = 500;
    createInteractiveRectangles(COUNT);

    // Create a simulated mouse move event
    sf::Event::MouseMoved mouseMove;
    mouseMove.position = sf::Vector2i{400, 300};
    sf::Event event(mouseMove);

    eventSystem->setEvent(event);

    double time = measureTime([this]() {
        eventSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] EventSystem mouse move (" << COUNT << " rects): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 20.0) << "Mouse move event processing too slow";
}

TEST_F(EventStressTest, EventSystem_MouseClick_500Rectangles) {
    constexpr std::size_t COUNT = 500;
    createInteractiveRectangles(COUNT);

    // Create simulated mouse press event
    sf::Event::MouseButtonPressed mousePress;
    mousePress.button = sf::Mouse::Button::Left;
    mousePress.position = sf::Vector2i{400, 300};
    sf::Event event(mousePress);

    eventSystem->setEvent(event);

    double time = measureTime([this]() {
        eventSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] EventSystem mouse click (" << COUNT << " rects): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 20.0) << "Mouse click event processing too slow";
}

TEST_F(EventStressTest, EventSystem_RepeatedEvents_1000Iterations) {
    constexpr std::size_t COUNT = 200;
    constexpr std::size_t ITERATIONS = 1000;
    createInteractiveRectangles(COUNT);

    std::uniform_int_distribution<int> posDist(0, 799);

    double avgTime = measureAverageTime(ITERATIONS, [this, &posDist]() {
        // Simulate random mouse position
        sf::Event::MouseMoved mouseMove;
        mouseMove.position = sf::Vector2i{posDist(rng), posDist(rng)};
        sf::Event event(mouseMove);

        eventSystem->setEvent(event);
        eventSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] EventSystem avg (" << ITERATIONS << " events): "
              << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 5.0) << "Repeated event processing too slow";
}

// =============================================================================
// Combined Event Pipeline Stress Tests
// =============================================================================

TEST_F(EventStressTest, FullEventPipeline_200Buttons_60Frames) {
    constexpr std::size_t COUNT = 200;
    constexpr std::size_t FRAMES = 60;
    createButtons(COUNT);

    std::uniform_int_distribution<int> posDist(0, 799);
    std::uniform_int_distribution<int> clickChance(0, 9);

    double totalTime = 0.0;

    for (std::size_t frame = 0; frame < FRAMES; ++frame) {
        totalTime += measureTime([this, &posDist, &clickChance, frame]() {
            // Phase 1: Reset triggers
            resetTriggersSystem->update(*registry, 0.016f);

            // Phase 2: Mouse move event
            sf::Event::MouseMoved mouseMove;
            mouseMove.position = sf::Vector2i{posDist(rng), posDist(rng)};
            sf::Event moveEvent(mouseMove);
            eventSystem->setEvent(moveEvent);
            eventSystem->update(*registry, 0.016f);

            // Phase 3: Occasionally simulate a click
            if (clickChance(rng) == 0) {
                sf::Event::MouseButtonPressed mousePress;
                mousePress.button = sf::Mouse::Button::Left;
                mousePress.position = sf::Vector2i{posDist(rng), posDist(rng)};
                sf::Event clickEvent(mousePress);
                eventSystem->setEvent(clickEvent);
                eventSystem->update(*registry, 0.016f);
            }

            // Phase 4: Update buttons
            buttonUpdateSystem->update(*registry, 0.016f);
        });
    }

    double avgTime = totalTime / static_cast<double>(FRAMES);

    std::cout << "[PERF] Full event pipeline (" << COUNT << " buttons, "
              << FRAMES << " frames):" << std::endl;
    std::cout << "       Total: " << totalTime << " ms" << std::endl;
    std::cout << "       Avg: " << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 16.67) << "Event pipeline cannot maintain 60 FPS";
}

// =============================================================================
// Mixed Entity Stress Tests
// =============================================================================

TEST_F(EventStressTest, MixedEntities_Buttons_And_Rectangles) {
    constexpr std::size_t BUTTON_COUNT = 100;
    constexpr std::size_t RECT_COUNT = 400;
    constexpr std::size_t FRAMES = 60;

    createButtons(BUTTON_COUNT);
    createInteractiveRectangles(RECT_COUNT);

    std::uniform_int_distribution<int> posDist(0, 799);

    double avgTime = measureAverageTime(FRAMES, [this, &posDist]() {
        resetTriggersSystem->update(*registry, 0.016f);

        sf::Event::MouseMoved mouseMove;
        mouseMove.position = sf::Vector2i{posDist(rng), posDist(rng)};
        sf::Event event(mouseMove);
        eventSystem->setEvent(event);
        eventSystem->update(*registry, 0.016f);

        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Mixed entities (" << BUTTON_COUNT << " buttons + "
              << RECT_COUNT << " rects): " << avgTime << " ms avg" << std::endl;

    EXPECT_LT(avgTime, 16.67) << "Mixed entity event processing too slow";
}

// =============================================================================
// Hidden Entity Performance Tests
// =============================================================================

TEST_F(EventStressTest, HiddenEntities_Performance) {
    constexpr std::size_t TOTAL = 500;
    constexpr std::size_t HIDDEN = 400;
    auto entities = createInteractiveRectangles(TOTAL);

    // Hide most entities
    for (std::size_t i = 0; i < HIDDEN; ++i) {
        registry->emplaceComponent<rc::HiddenComponent>(entities[i], true);
    }

    sf::Event::MouseMoved mouseMove;
    mouseMove.position = sf::Vector2i{400, 300};
    sf::Event event(mouseMove);
    eventSystem->setEvent(event);

    double time = measureTime([this]() {
        eventSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Event with hidden entities (" << HIDDEN << "/"
              << TOTAL << " hidden): " << time << " ms" << std::endl;

    EXPECT_LT(time, 20.0) << "Hidden entity handling too slow";
}

// =============================================================================
// Event System Reuse Tests
// =============================================================================

TEST_F(EventStressTest, EventSystemReuse_NoAllocation) {
    constexpr std::size_t COUNT = 200;
    constexpr std::size_t ITERATIONS = 1000;
    createInteractiveRectangles(COUNT);

    // Pre-create events
    std::vector<sf::Event> events;
    events.reserve(ITERATIONS);

    std::uniform_int_distribution<int> posDist(0, 799);
    for (std::size_t i = 0; i < ITERATIONS; ++i) {
        sf::Event::MouseMoved mouseMove;
        mouseMove.position = sf::Vector2i{posDist(rng), posDist(rng)};
        events.emplace_back(mouseMove);
    }

    // Time processing with event reuse
    double time = measureTime([this, &events, ITERATIONS]() {
        for (std::size_t i = 0; i < ITERATIONS; ++i) {
            eventSystem->setEvent(events[i]);
            eventSystem->update(*registry, 0.016f);
        }
    });

    std::cout << "[PERF] EventSystem reuse (" << ITERATIONS
              << " events): " << time << " ms" << std::endl;
    std::cout << "       Per event: " << (time / ITERATIONS) << " ms"
              << std::endl;

    EXPECT_LT(time / ITERATIONS, 1.0) << "Per-event processing too slow";
}

// =============================================================================
// Button Callback Performance Tests
// =============================================================================

TEST_F(EventStressTest, ButtonCallbacks_ClickSimulation) {
    constexpr std::size_t COUNT = 50;

    // Create buttons with callbacks
    std::size_t callbackCount = 0;
    std::vector<ECS::Entity> entities;

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();

        registry->emplaceComponent<rs::TransformComponent>(entity,
            static_cast<float>(i * 15), static_cast<float>(i * 10));
        registry->emplaceComponent<rc::Rectangle>(
            entity, std::make_pair(50.0f, 30.0f),
            sf::Color::Red, sf::Color::Magenta);
        registry->emplaceComponent<rc::UserEvent>(entity);
        registry->emplaceComponent<rc::ButtonTag>(entity);
        registry->emplaceComponent<rc::Button<>>(entity,
            [&callbackCount]() { ++callbackCount; });

        auto& rect = registry->getComponent<rc::Rectangle>(entity);
        rect.rectangle.setSize({50.0f, 30.0f});
        rect.rectangle.setPosition({static_cast<float>(i * 15),
                                    static_cast<float>(i * 10)});

        entities.push_back(entity);
    }

    // Simulate clicking all buttons
    for (auto entity : entities) {
        auto& event = registry->getComponent<rc::UserEvent>(entity);
        event.isPressed = true;
    }

    double time = measureTime([this]() {
        buttonUpdateSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Button callbacks (" << COUNT << " clicks): "
              << time << " ms" << std::endl;
    std::cout << "       Callbacks executed: " << callbackCount << std::endl;

    EXPECT_EQ(callbackCount, COUNT);
    EXPECT_LT(time, 20.0) << "Button callback execution too slow";
}
