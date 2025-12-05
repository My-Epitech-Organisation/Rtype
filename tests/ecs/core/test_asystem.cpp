/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for ASystem abstract base class
*/

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

using namespace rtype::engine;

// ============================================================================
// TEST CONCRETE IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Simple concrete implementation of ASystem for testing
 */
class TestSystem : public ASystem {
   public:
    explicit TestSystem(std::string name = "TestSystem")
        : ASystem(std::move(name)) {}

    void update(ECS::Registry& registry, float deltaTime) override {
        _lastDeltaTime = deltaTime;
        _updateCount++;
        _registryPtr = &registry;
    }

    // Test helpers
    float getLastDeltaTime() const { return _lastDeltaTime; }
    int getUpdateCount() const { return _updateCount; }
    ECS::Registry* getLastRegistry() const { return _registryPtr; }

   private:
    float _lastDeltaTime = 0.0f;
    int _updateCount = 0;
    ECS::Registry* _registryPtr = nullptr;
};

/**
 * @brief System that tracks enabled state changes
 */
class StateTrackingSystem : public ASystem {
   public:
    StateTrackingSystem() : ASystem("StateTrackingSystem") {}

    void update(ECS::Registry& /*registry*/, float /*deltaTime*/) override {
        if (isEnabled()) {
            _enabledUpdateCount++;
        }
    }

    int getEnabledUpdateCount() const { return _enabledUpdateCount; }

   private:
    int _enabledUpdateCount = 0;
};

/**
 * @brief System with a very long name
 */
class VeryLongNameSystemForTestingPurposesAndNothingElse : public ASystem {
   public:
    VeryLongNameSystemForTestingPurposesAndNothingElse()
        : ASystem("VeryLongNameSystemForTestingPurposesAndNothingElse") {}

    void update(ECS::Registry& /*registry*/, float /*deltaTime*/) override {}
};

/**
 * @brief System with empty name
 */
class EmptyNameSystem : public ASystem {
   public:
    EmptyNameSystem() : ASystem("") {}

    void update(ECS::Registry& /*registry*/, float /*deltaTime*/) override {}
};

/**
 * @brief System with special characters in name
 */
class SpecialNameSystem : public ASystem {
   public:
    SpecialNameSystem() : ASystem("System_With-Special.Characters!@#$%") {}

    void update(ECS::Registry& /*registry*/, float /*deltaTime*/) override {}
};

// ============================================================================
// TEST FIXTURES
// ============================================================================

class ASystemTest : public ::testing::Test {
   protected:
    ECS::Registry registry;
};

// ============================================================================
// NAME TESTS
// ============================================================================

TEST_F(ASystemTest, GetName_ReturnsCorrectName) {
    TestSystem system("MovementSystem");
    EXPECT_EQ(system.getName(), "MovementSystem");
}

TEST_F(ASystemTest, GetName_DefaultName) {
    TestSystem system;
    EXPECT_EQ(system.getName(), "TestSystem");
}

TEST_F(ASystemTest, GetName_EmptyName) {
    EmptyNameSystem system;
    EXPECT_EQ(system.getName(), "");
}

TEST_F(ASystemTest, GetName_LongName) {
    VeryLongNameSystemForTestingPurposesAndNothingElse system;
    EXPECT_EQ(system.getName(),
              "VeryLongNameSystemForTestingPurposesAndNothingElse");
}

TEST_F(ASystemTest, GetName_SpecialCharacters) {
    SpecialNameSystem system;
    EXPECT_EQ(system.getName(), "System_With-Special.Characters!@#$%");
}

TEST_F(ASystemTest, GetName_IsConst) {
    const TestSystem system("ConstSystem");
    EXPECT_EQ(system.getName(), "ConstSystem");
}

// ============================================================================
// ENABLED STATE TESTS
// ============================================================================

TEST_F(ASystemTest, IsEnabled_DefaultTrue) {
    TestSystem system;
    EXPECT_TRUE(system.isEnabled());
}

TEST_F(ASystemTest, SetEnabled_False) {
    TestSystem system;
    system.setEnabled(false);
    EXPECT_FALSE(system.isEnabled());
}

TEST_F(ASystemTest, SetEnabled_True) {
    TestSystem system;
    system.setEnabled(false);
    system.setEnabled(true);
    EXPECT_TRUE(system.isEnabled());
}

TEST_F(ASystemTest, SetEnabled_MultipleChanges) {
    TestSystem system;
    for (int i = 0; i < 10; ++i) {
        system.setEnabled(false);
        EXPECT_FALSE(system.isEnabled());
        system.setEnabled(true);
        EXPECT_TRUE(system.isEnabled());
    }
}

TEST_F(ASystemTest, SetEnabled_SameValueTwice) {
    TestSystem system;
    system.setEnabled(true);
    system.setEnabled(true);
    EXPECT_TRUE(system.isEnabled());

    system.setEnabled(false);
    system.setEnabled(false);
    EXPECT_FALSE(system.isEnabled());
}

TEST_F(ASystemTest, IsEnabled_IsNoexcept) {
    TestSystem system;
    EXPECT_TRUE(noexcept(system.isEnabled()));
}

TEST_F(ASystemTest, SetEnabled_IsNoexcept) {
    TestSystem system;
    EXPECT_TRUE(noexcept(system.setEnabled(true)));
}

// ============================================================================
// UPDATE TESTS
// ============================================================================

TEST_F(ASystemTest, Update_RecordsDeltaTime) {
    TestSystem system;
    system.update(registry, 0.016f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 0.016f);
}

TEST_F(ASystemTest, Update_ZeroDeltaTime) {
    TestSystem system;
    system.update(registry, 0.0f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 0.0f);
}

TEST_F(ASystemTest, Update_LargeDeltaTime) {
    TestSystem system;
    system.update(registry, 1.0f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 1.0f);
}

TEST_F(ASystemTest, Update_NegativeDeltaTime) {
    TestSystem system;
    system.update(registry, -0.016f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), -0.016f);
}

TEST_F(ASystemTest, Update_MultipleCalls) {
    TestSystem system;
    EXPECT_EQ(system.getUpdateCount(), 0);

    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 1);

    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 2);

    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 3);
}

TEST_F(ASystemTest, Update_ManyIterations) {
    TestSystem system;
    for (int i = 0; i < 1000; ++i) {
        system.update(registry, 0.016f);
    }
    EXPECT_EQ(system.getUpdateCount(), 1000);
}

TEST_F(ASystemTest, Update_ReceivesRegistry) {
    TestSystem system;
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getLastRegistry(), &registry);
}

TEST_F(ASystemTest, Update_DifferentRegistries) {
    TestSystem system;
    ECS::Registry registry2;

    system.update(registry, 0.016f);
    EXPECT_EQ(system.getLastRegistry(), &registry);

    system.update(registry2, 0.016f);
    EXPECT_EQ(system.getLastRegistry(), &registry2);
}

// ============================================================================
// ENABLED STATE INTERACTION WITH UPDATE
// ============================================================================

TEST_F(ASystemTest, EnabledState_AffectsUpdate) {
    StateTrackingSystem system;

    // Update while enabled
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getEnabledUpdateCount(), 1);

    // Disable and update
    system.setEnabled(false);
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getEnabledUpdateCount(), 1);  // No change

    // Re-enable and update
    system.setEnabled(true);
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getEnabledUpdateCount(), 2);
}

// ============================================================================
// MULTIPLE SYSTEMS TESTS
// ============================================================================

TEST_F(ASystemTest, MultipleSystems_IndependentState) {
    TestSystem system1("System1");
    TestSystem system2("System2");
    TestSystem system3("System3");

    system1.setEnabled(true);
    system2.setEnabled(false);
    system3.setEnabled(true);

    EXPECT_TRUE(system1.isEnabled());
    EXPECT_FALSE(system2.isEnabled());
    EXPECT_TRUE(system3.isEnabled());

    EXPECT_EQ(system1.getName(), "System1");
    EXPECT_EQ(system2.getName(), "System2");
    EXPECT_EQ(system3.getName(), "System3");
}

TEST_F(ASystemTest, MultipleSystems_IndependentUpdates) {
    TestSystem system1("System1");
    TestSystem system2("System2");

    system1.update(registry, 0.016f);
    system1.update(registry, 0.016f);

    system2.update(registry, 0.032f);

    EXPECT_EQ(system1.getUpdateCount(), 2);
    EXPECT_EQ(system2.getUpdateCount(), 1);
    EXPECT_FLOAT_EQ(system1.getLastDeltaTime(), 0.016f);
    EXPECT_FLOAT_EQ(system2.getLastDeltaTime(), 0.032f);
}

// ============================================================================
// POINTER/REFERENCE TESTS
// ============================================================================

TEST_F(ASystemTest, AsPointer_WorksCorrectly) {
    auto system = std::make_unique<TestSystem>("PointerSystem");

    EXPECT_EQ(system->getName(), "PointerSystem");
    EXPECT_TRUE(system->isEnabled());

    system->update(registry, 0.016f);
    EXPECT_EQ(system->getUpdateCount(), 1);
}

TEST_F(ASystemTest, AsISystemPointer_Polymorphism) {
    std::unique_ptr<ISystem> system = std::make_unique<TestSystem>("PolySystem");

    EXPECT_EQ(system->getName(), "PolySystem");
    EXPECT_TRUE(system->isEnabled());

    system->setEnabled(false);
    EXPECT_FALSE(system->isEnabled());

    system->update(registry, 0.016f);
}

TEST_F(ASystemTest, VectorOfSystems_WorksCorrectly) {
    std::vector<std::unique_ptr<ISystem>> systems;
    systems.push_back(std::make_unique<TestSystem>("System1"));
    systems.push_back(std::make_unique<TestSystem>("System2"));
    systems.push_back(std::make_unique<TestSystem>("System3"));

    for (const auto& system : systems) {
        EXPECT_TRUE(system->isEnabled());
        system->update(registry, 0.016f);
    }

    EXPECT_EQ(systems.size(), 3);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(ASystemTest, VerySmallDeltaTime) {
    TestSystem system;
    system.update(registry, 0.0000001f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 0.0000001f);
}

TEST_F(ASystemTest, VeryLargeDeltaTime) {
    TestSystem system;
    system.update(registry, 1000000.0f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 1000000.0f);
}

TEST_F(ASystemTest, InfinityDeltaTime) {
    TestSystem system;
    system.update(registry, std::numeric_limits<float>::infinity());
    EXPECT_TRUE(std::isinf(system.getLastDeltaTime()));
}

TEST_F(ASystemTest, NaNDeltaTime) {
    TestSystem system;
    system.update(registry, std::numeric_limits<float>::quiet_NaN());
    EXPECT_TRUE(std::isnan(system.getLastDeltaTime()));
}

// ============================================================================
// LIFECYCLE TESTS
// ============================================================================

TEST_F(ASystemTest, Lifecycle_CreateEnableDisableUpdate) {
    TestSystem system("LifecycleSystem");

    // Initially enabled
    EXPECT_TRUE(system.isEnabled());
    EXPECT_EQ(system.getName(), "LifecycleSystem");

    // Update while enabled
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 1);

    // Disable
    system.setEnabled(false);
    EXPECT_FALSE(system.isEnabled());

    // Update while disabled (still gets called, system should check internally)
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 2);

    // Re-enable
    system.setEnabled(true);
    EXPECT_TRUE(system.isEnabled());

    // Final update
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 3);
}

TEST_F(ASystemTest, Lifecycle_WithRegistryOperations) {
    TestSystem system("EntitySystem");

    // Spawn some entities
    auto e1 = registry.spawnEntity();
    auto e2 = registry.spawnEntity();
    auto e3 = registry.spawnEntity();

    // Update system with entities present
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 1);

    // Kill an entity
    registry.killEntity(e2);

    // Update again
    system.update(registry, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 2);

    // Verify registry pointer is correct
    EXPECT_EQ(system.getLastRegistry(), &registry);
}
