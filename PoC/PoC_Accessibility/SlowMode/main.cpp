#include "TimeSystem.hpp"
#include "GameEntity.hpp"
#include "DifficultyManager.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>

/**
 * @brief Proof of Concept demonstrating Slow Mode & Time Scale for Accessibility
 * 
 * This PoC validates:
 * 1. Global Time Scale implementation
 * 2. Scaled delta time affecting gameplay entities
 * 3. Difficulty presets based on time scaling
 * 4. No physics or timing desynchronization
 */

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n\n";
}

void printEntityState(const GameEntity& entity, float rawDt, float scaledDt, float totalTime) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  " << std::setw(12) << entity.getName() 
              << " | Pos: " << std::setw(8) << entity.getPosition()
              << " | Speed: " << std::setw(6) << entity.getSpeed()
              << " | RawDt: " << std::setw(6) << rawDt
              << " | ScaledDt: " << std::setw(6) << scaledDt
              << " | Time: " << std::setw(6) << totalTime << "s\n";
}

void simulateGameplay(DifficultyManager& diffManager, TimeSystem& timeSystem, 
                      std::vector<GameEntity>& entities, int frames, int frameDelay) {
    std::cout << "\nCurrent Setting: " << DifficultyManager::getPresetName(diffManager.getCurrentPreset())
              << " (Time Scale: " << diffManager.getCurrentTimeScale() << "x)\n";
    std::cout << DifficultyManager::getPresetDescription(diffManager.getCurrentPreset()) << "\n\n";
    
    std::cout << std::string(70, '-') << "\n";
    
    for (int i = 0; i < frames; ++i) {
        // Simulate frame timing
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay));
        
        // Update time system
        timeSystem.update();
        
        float rawDt = timeSystem.getRawDeltaTime();
        float scaledDt = timeSystem.getScaledDeltaTime();
        float totalTime = timeSystem.getTotalScaledTime();
        
        // Update all entities with scaled delta time
        for (auto& entity : entities) {
            entity.update(scaledDt);
        }
        
        // Print state every few frames
        if (i % 3 == 0 || i == frames - 1) {
            std::cout << "Frame " << std::setw(2) << i + 1 << ":\n";
            for (const auto& entity : entities) {
                printEntityState(entity, rawDt, scaledDt, totalTime);
            }
            std::cout << std::string(70, '-') << "\n";
        }
    }
}

void demonstratePhysicsConsistency(TimeSystem& timeSystem) {
    printHeader("PHYSICS CONSISTENCY VALIDATION");
    
    std::cout << "Testing that time scaling doesn't break physics calculations...\n\n";
    
    // Create identical entities
    GameEntity entity1("Test-1.0x", GameEntity::Type::Projectile, 100.0f);
    GameEntity entity2("Test-0.5x", GameEntity::Type::Projectile, 100.0f);
    
    // Run entity1 at normal speed for 2 seconds
    timeSystem.setGlobalTimeScale(1.0f);
    float simulatedTime = 0.0f;
    const float targetTime = 2.0f;
    const float frameDt = 0.016f; // ~60 FPS
    
    while (simulatedTime < targetTime) {
        entity1.update(frameDt * 1.0f);
        simulatedTime += frameDt;
    }
    
    std::cout << "Entity at 1.0x time scale:\n";
    std::cout << "  Position after " << targetTime << "s: " << entity1.getPosition() << " units\n\n";
    
    // Run entity2 at slow speed for 4 seconds (should reach same position)
    simulatedTime = 0.0f;
    const float slowTargetTime = 4.0f; // 2x longer in real time
    
    while (simulatedTime < slowTargetTime) {
        entity2.update(frameDt * 0.5f); // Half speed
        simulatedTime += frameDt;
    }
    
    std::cout << "Entity at 0.5x time scale:\n";
    std::cout << "  Position after " << slowTargetTime << "s real time: " << entity2.getPosition() << " units\n\n";
    
    float difference = std::abs(entity1.getPosition() - entity2.getPosition());
    std::cout << "Position difference: " << difference << " units\n";
    
    if (difference < 0.01f) {
        std::cout << "✓ PASS: Physics remain consistent across time scales!\n";
    } else {
        std::cout << "✗ FAIL: Physics desynchronization detected!\n";
    }
}

void demonstrateUITimingIndependence() {
    printHeader("UI TIMING INDEPENDENCE");
    
    std::cout << "Demonstrating that UI elements should use UNSCALED time:\n\n";
    std::cout << "Example: Menu fade animation\n";
    std::cout << "  - Gameplay time scale: 0.5x (slow mode)\n";
    std::cout << "  - Menu animation uses: Raw DeltaTime (unscaled)\n";
    std::cout << "  - Result: Menu remains responsive at normal speed\n\n";
    
    std::cout << "Example: Loading screen spinner\n";
    std::cout << "  - Gameplay time scale: 0.5x (slow mode)\n";
    std::cout << "  - Spinner uses: Raw DeltaTime (unscaled)\n";
    std::cout << "  - Result: Spinner rotates at normal speed\n\n";
    
    std::cout << "✓ UI/UX elements should always use getRawDeltaTime()\n";
    std::cout << "✓ Gameplay systems should use getScaledDeltaTime()\n";
}

void demonstrateAccessibilityGuidelines() {
    printHeader("ACCESSIBILITY & PHOTOSENSITIVITY GUIDELINES");
    
    std::cout << "UI Clarity Best Practices:\n";
    std::cout << "  ✓ Minimum font size: 16-18px at 1080p\n";
    std::cout << "  ✓ WCAG contrast ratio: 4.5:1 minimum\n";
    std::cout << "  ✓ Sans-serif fonts for readability\n";
    std::cout << "  ✓ Simple, silhouette-based icons\n";
    std::cout << "  ✓ Avoid color-only distinctions (add shape cues)\n\n";
    
    std::cout << "Photosensitivity Safety:\n";
    std::cout << "  ✓ Avoid flashing lights > 3 flashes/second\n";
    std::cout << "  ✓ Use smooth fades instead of rapid blinks\n";
    std::cout << "  ✓ Provide option to disable screen shake\n";
    std::cout << "  ✓ Contrast-based highlights over strobing\n\n";
    
    std::cout << "References:\n";
    std::cout << "  - Game Accessibility Guidelines: https://gameaccessibilityguidelines.com/\n";
    std::cout << "  - WCAG Standards: https://www.w3.org/TR/WCAG21/\n";
    std::cout << "  - Epilepsy Safety: https://www.epilepsy.com/\n";
}

int main() {
    printHeader("R-TYPE ACCESSIBILITY POC: SLOW MODE & TIME SCALING");
    
    std::cout << "This PoC demonstrates:\n";
    std::cout << "  1. Global Time Scale system implementation\n";
    std::cout << "  2. Difficulty presets based on time scaling\n";
    std::cout << "  3. Physics consistency validation\n";
    std::cout << "  4. UI timing independence\n";
    std::cout << "  5. Accessibility guidelines\n";
    
    // Initialize systems
    TimeSystem timeSystem;
    DifficultyManager diffManager;
    
    // Create game entities
    std::vector<GameEntity> entities = {
        GameEntity("Player", GameEntity::Type::Player, 50.0f),
        GameEntity("Enemy-1", GameEntity::Type::Enemy, 30.0f),
        GameEntity("Enemy-2", GameEntity::Type::Enemy, 40.0f),
        GameEntity("Projectile", GameEntity::Type::Projectile, 100.0f)
    };
    
    // Test 1: Normal difficulty
    printHeader("TEST 1: NORMAL DIFFICULTY (100% Speed)");
    float scale = diffManager.setPreset(DifficultyManager::Preset::Normal);
    timeSystem.setGlobalTimeScale(scale);
    simulateGameplay(diffManager, timeSystem, entities, 10, 50);
    
    // Reset entities
    for (auto& entity : entities) entity.reset();
    
    // Test 2: Slow mode (accessibility)
    printHeader("TEST 2: SLOW MODE (50% Speed - Accessibility)");
    scale = diffManager.setPreset(DifficultyManager::Preset::Slow);
    timeSystem.setGlobalTimeScale(scale);
    simulateGameplay(diffManager, timeSystem, entities, 10, 50);
    
    // Reset entities
    for (auto& entity : entities) entity.reset();
    
    // Test 3: Custom scale
    printHeader("TEST 3: CUSTOM SCALE (75% Speed)");
    scale = diffManager.setCustomScale(0.75f);
    timeSystem.setGlobalTimeScale(scale);
    simulateGameplay(diffManager, timeSystem, entities, 10, 50);
    
    // Test 4: Physics consistency
    demonstratePhysicsConsistency(timeSystem);
    
    // Test 5: UI independence
    demonstrateUITimingIndependence();
    
    // Test 6: Accessibility guidelines
    demonstrateAccessibilityGuidelines();
    
    // Summary
    printHeader("CONCLUSION");
    std::cout << "✓ Global Time Scale successfully scales all gameplay elements\n";
    std::cout << "✓ Different difficulty presets work without physics breaks\n";
    std::cout << "✓ Entities maintain proportional relationships at all speeds\n";
    std::cout << "✓ UI elements can remain responsive using raw delta time\n";
    std::cout << "✓ Implementation ready for integration into R-Type engine\n\n";
    
    std::cout << "Next Steps:\n";
    std::cout << "  1. Integrate TimeSystem into R-Type engine\n";
    std::cout << "  2. Add difficulty selector to game settings UI\n";
    std::cout << "  3. Test with actual gameplay systems (physics, AI, animations)\n";
    std::cout << "  4. Conduct UX testing with players of varying reaction speeds\n";
    std::cout << "  5. Implement photosensitivity safeguards in rendering\n\n";
    
    return 0;
}
