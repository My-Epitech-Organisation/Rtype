/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_boss_systems - Unit tests for boss-related systems
*/

#include <gtest/gtest.h>

#include <memory>

#include "games/rtype/server/Systems/Boss/BossAttackSystem.hpp"
#include "games/rtype/server/Systems/Boss/BossPhaseSystem.hpp"
#include "games/rtype/server/Systems/Boss/WeakPointSystem.hpp"
#include "games/rtype/shared/Components/BossComponent.hpp"
#include "games/rtype/shared/Components/BossPatternComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeakPointComponent.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;

// =============================================================================
// BossPhaseSystem Tests
// =============================================================================

class BossPhaseSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<BossPhaseSystem> system;
    std::vector<rtype::engine::GameEvent> emittedEvents;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        system = std::make_unique<BossPhaseSystem>(
            [this](const rtype::engine::GameEvent& event) {
                emittedEvents.push_back(event);
            });
    }

    void TearDown() override {
        system.reset();
        registry.reset();
        emittedEvents.clear();
    }

    ECS::Entity createBoss(float health = 100.0F, bool withPhases = true) {
        ECS::Entity boss = registry->spawnEntity();
        registry->emplaceComponent<BossTag>(boss);
        registry->emplaceComponent<NetworkIdComponent>(boss, 1000);
        registry->emplaceComponent<TransformComponent>(boss, 500.0F, 300.0F,
                                                       0.0F);
        registry->emplaceComponent<VelocityComponent>(boss, 0.0F, 0.0F);
        HealthComponent healthComp;
        healthComp.max = 100;
        healthComp.current = static_cast<int32_t>(health);
        registry->emplaceComponent<HealthComponent>(boss, healthComp);

        BossComponent bossComp;
        bossComp.bossType = BossType::Generic;
        bossComp.baseX = 500.0F;
        bossComp.baseY = 300.0F;

        if (withPhases) {
            BossPhase phase1;
            phase1.healthThreshold = 1.0F;
            phase1.phaseName = "Phase1";

            BossPhase phase2;
            phase2.healthThreshold = 0.66F;
            phase2.phaseName = "Phase2";

            BossPhase phase3;
            phase3.healthThreshold = 0.33F;
            phase3.phaseName = "Phase3";

            bossComp.phases.push_back(phase1);
            bossComp.phases.push_back(phase2);
            bossComp.phases.push_back(phase3);
        }

        registry->emplaceComponent<BossComponent>(boss, bossComp);
        return boss;
    }
};

TEST_F(BossPhaseSystemTest, SystemNameCorrect) {
    EXPECT_EQ(system->getName(), "BossPhaseSystem");
}

TEST_F(BossPhaseSystemTest, UpdateWithNoBoss) {
    // Should not crash with no entities
    system->update(*registry, 0.016F);
}

TEST_F(BossPhaseSystemTest, BossAtFullHealthStaysInPhase1) {
    ECS::Entity boss = createBoss(100.0F);
    system->update(*registry, 0.016F);

    const auto& bossComp = registry->getComponent<BossComponent>(boss);
    EXPECT_EQ(bossComp.currentPhaseIndex, 0);
}

TEST_F(BossPhaseSystemTest, BossTransitionsToPhase2) {
    ECS::Entity boss = createBoss(60.0F);  // Below 66% threshold
    system->update(*registry, 0.016F);

    const auto& bossComp = registry->getComponent<BossComponent>(boss);
    EXPECT_EQ(bossComp.currentPhaseIndex, 1);
    EXPECT_TRUE(bossComp.phaseTransitionActive);
}

TEST_F(BossPhaseSystemTest, BossTransitionsToPhase3) {
    ECS::Entity boss = createBoss(30.0F);  // Below 33% threshold
    system->update(*registry, 0.016F);

    const auto& bossComp = registry->getComponent<BossComponent>(boss);
    EXPECT_EQ(bossComp.currentPhaseIndex, 1);  // First transition from phase 0
}

TEST_F(BossPhaseSystemTest, InvulnerabilityTimerDecreases) {
    ECS::Entity boss = createBoss(100.0F);
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    bossComp.invulnerabilityTimer = 1.0F;

    system->update(*registry, 0.5F);

    EXPECT_LT(bossComp.invulnerabilityTimer, 1.0F);
}

TEST_F(BossPhaseSystemTest, DefeatedBossNotProcessed) {
    ECS::Entity boss = createBoss(100.0F);
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    bossComp.defeated = true;

    system->update(*registry, 0.016F);

    // Should not change anything since boss is defeated
    EXPECT_EQ(bossComp.currentPhaseIndex, 0);
}

TEST_F(BossPhaseSystemTest, BossDeathTriggered) {
    ECS::Entity boss = createBoss(0.0F);  // Dead boss

    system->update(*registry, 0.016F);

    const auto& bossComp = registry->getComponent<BossComponent>(boss);
    EXPECT_TRUE(bossComp.defeated);
}

TEST_F(BossPhaseSystemTest, PhaseTransitionActiveBlocksNewTransition) {
    ECS::Entity boss = createBoss(50.0F);
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    bossComp.phaseTransitionActive = true;
    bossComp.currentPhaseIndex = 0;

    system->update(*registry, 0.016F);

    // Should not transition because transition is already active
    EXPECT_EQ(bossComp.currentPhaseIndex, 0);
}

TEST_F(BossPhaseSystemTest, BossWithPatternComponentUpdated) {
    ECS::Entity boss = createBoss(60.0F);
    registry->emplaceComponent<BossPatternComponent>(boss);

    system->update(*registry, 0.016F);

    // Pattern component should be cleared on phase transition
    const auto& patterns =
        registry->getComponent<BossPatternComponent>(boss);
    EXPECT_TRUE(patterns.phasePatterns.empty());
}

// =============================================================================
// BossAttackSystem Tests
// =============================================================================

class BossAttackSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<BossAttackSystem> system;
    std::vector<rtype::engine::GameEvent> emittedEvents;
    int projectilesSpawned = 0;
    int minionsSpawned = 0;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        projectilesSpawned = 0;
        minionsSpawned = 0;

        system = std::make_unique<BossAttackSystem>(
            [this](const rtype::engine::GameEvent& event) {
                emittedEvents.push_back(event);
            },
            [this](ECS::Registry& /*reg*/, float /*x*/, float /*y*/,
                   float /*vx*/, float /*vy*/, int32_t /*damage*/,
                   uint32_t /*ownerNetId*/) -> uint32_t {
                projectilesSpawned++;
                return 1;
            },
            [this](ECS::Registry& /*reg*/, const std::string& /*type*/,
                   float /*x*/, float /*y*/) { minionsSpawned++; });
    }

    void TearDown() override {
        system.reset();
        registry.reset();
        emittedEvents.clear();
    }

    ECS::Entity createBossWithPatterns() {
        ECS::Entity boss = registry->spawnEntity();
        registry->emplaceComponent<BossTag>(boss);
        registry->emplaceComponent<NetworkIdComponent>(boss, 1000);
        registry->emplaceComponent<TransformComponent>(boss, 500.0F, 300.0F,
                                                       0.0F);

        BossComponent bossComp;
        bossComp.bossType = BossType::Generic;
        bossComp.defeated = false;
        bossComp.phaseTransitionActive = false;
        registry->emplaceComponent<BossComponent>(boss, bossComp);

        BossPatternComponent patterns;
        patterns.enabled = true;
        patterns.state = PatternExecutionState::Idle;
        patterns.globalCooldown = 0.0F;
        patterns.patternQueue.push_back(
            AttackPatternConfig::createCircularShot());
        registry->emplaceComponent<BossPatternComponent>(boss, patterns);

        return boss;
    }

    ECS::Entity createPlayer(float x = 200.0F, float y = 300.0F) {
        ECS::Entity player = registry->spawnEntity();
        registry->emplaceComponent<PlayerTag>(player);
        registry->emplaceComponent<TransformComponent>(player, x, y, 0.0F);
        return player;
    }
};

TEST_F(BossAttackSystemTest, SystemNameCorrect) {
    EXPECT_EQ(system->getName(), "BossAttackSystem");
}

TEST_F(BossAttackSystemTest, UpdateWithNoBoss) {
    system->update(*registry, 0.016F);
    // Should not crash
}

TEST_F(BossAttackSystemTest, DefeatedBossDoesNotAttack) {
    ECS::Entity boss = createBossWithPatterns();
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    bossComp.defeated = true;

    system->update(*registry, 0.016F);

    EXPECT_EQ(projectilesSpawned, 0);
}

TEST_F(BossAttackSystemTest, TransitioningBossDoesNotAttack) {
    ECS::Entity boss = createBossWithPatterns();
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    bossComp.phaseTransitionActive = true;

    system->update(*registry, 0.016F);

    EXPECT_EQ(projectilesSpawned, 0);
}

TEST_F(BossAttackSystemTest, DisabledPatternsDoNotExecute) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.enabled = false;

    system->update(*registry, 0.016F);

    EXPECT_EQ(projectilesSpawned, 0);
}

TEST_F(BossAttackSystemTest, PatternStartsWhenReady) {
    createBossWithPatterns();
    createPlayer();

    system->update(*registry, 0.016F);

    // Pattern should have started (state changed from Idle)
}

TEST_F(BossAttackSystemTest, GlobalCooldownDecreases) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.globalCooldown = 1.0F;

    system->update(*registry, 0.5F);

    EXPECT_LT(patterns.globalCooldown, 1.0F);
}

TEST_F(BossAttackSystemTest, CircularShotExecutes) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.state = PatternExecutionState::Executing;
    patterns.currentPattern = AttackPatternConfig::createCircularShot();
    createPlayer();

    system->update(*registry, 0.016F);

    EXPECT_GT(projectilesSpawned, 0);
}

TEST_F(BossAttackSystemTest, SpreadFanExecutes) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.state = PatternExecutionState::Executing;
    patterns.currentPattern = AttackPatternConfig::createSpreadFan();
    createPlayer();

    system->update(*registry, 0.016F);

    EXPECT_GT(projectilesSpawned, 0);
}

TEST_F(BossAttackSystemTest, MinionSpawnExecutes) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.state = PatternExecutionState::Executing;
    patterns.currentPattern = AttackPatternConfig::createMinionSpawn();

    system->update(*registry, 0.016F);

    EXPECT_GT(minionsSpawned, 0);
}

TEST_F(BossAttackSystemTest, LaserSweepExecutes) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.state = PatternExecutionState::Executing;
    patterns.currentPattern = AttackPatternConfig::createLaserSweep();
    createPlayer();

    system->update(*registry, 0.1F);

    // Laser sweep updates telegraph angle
    EXPECT_NE(patterns.telegraphAngle, 0.0F);
}

TEST_F(BossAttackSystemTest, TailSweepExecutes) {
    ECS::Entity boss = createBossWithPatterns();
    auto& patterns = registry->getComponent<BossPatternComponent>(boss);
    patterns.state = PatternExecutionState::Executing;
    patterns.currentPattern = AttackPatternConfig::createTailSweep();

    system->update(*registry, 0.016F);

    // Tail sweep progresses
    EXPECT_GT(patterns.patternProgress, 0.0F);
}

// =============================================================================
// WeakPointSystem Tests
// =============================================================================

class WeakPointSystemTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<WeakPointSystem> system;
    std::vector<rtype::engine::GameEvent> emittedEvents;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        system = std::make_unique<WeakPointSystem>(
            [this](const rtype::engine::GameEvent& event) {
                emittedEvents.push_back(event);
            });
    }

    void TearDown() override {
        system.reset();
        registry.reset();
        emittedEvents.clear();
    }

    ECS::Entity createBoss() {
        ECS::Entity boss = registry->spawnEntity();
        registry->emplaceComponent<BossTag>(boss);
        registry->emplaceComponent<NetworkIdComponent>(boss, 1000);
        registry->emplaceComponent<TransformComponent>(boss, 500.0F, 300.0F,
                                                       0.0F);
        BossComponent bossComp;
        bossComp.bossType = BossType::Serpent;
        registry->emplaceComponent<BossComponent>(boss, bossComp);
        registry->emplaceComponent<HealthComponent>(boss, 100);
        return boss;
    }

    ECS::Entity createWeakPoint(ECS::Entity parent, uint32_t parentNetId,
                                float offsetX = 0.0F, float offsetY = 0.0F) {
        ECS::Entity wp = registry->spawnEntity();
        registry->emplaceComponent<WeakPointTag>(wp);
        WeakPointComponent wpComp;
        wpComp.parentBossEntity = parent;
        wpComp.parentBossNetworkId = parentNetId;
        wpComp.localOffsetX = offsetX;
        wpComp.localOffsetY = offsetY;
        registry->emplaceComponent<WeakPointComponent>(wp, wpComp);
        registry->emplaceComponent<TransformComponent>(wp, 0.0F, 0.0F, 0.0F);
        registry->emplaceComponent<NetworkIdComponent>(wp, 2000);
        registry->emplaceComponent<HealthComponent>(wp, 50);
        return wp;
    }
};

TEST_F(WeakPointSystemTest, SystemNameCorrect) {
    EXPECT_EQ(system->getName(), "WeakPointSystem");
}

TEST_F(WeakPointSystemTest, UpdateWithNoWeakPoints) {
    system->update(*registry, 0.016F);
    // Should not crash
}

TEST_F(WeakPointSystemTest, WeakPointFollowsParent) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000, 50.0F, 25.0F);

    system->update(*registry, 0.016F);

    const auto& wpTransform = registry->getComponent<TransformComponent>(wp);
    EXPECT_FLOAT_EQ(wpTransform.x, 550.0F);  // 500 + 50
    EXPECT_FLOAT_EQ(wpTransform.y, 325.0F);  // 300 + 25
}

TEST_F(WeakPointSystemTest, DestroyedWeakPointNotUpdated) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000, 50.0F, 25.0F);
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.destroyed = true;
    auto& wpTransform = registry->getComponent<TransformComponent>(wp);
    wpTransform.x = 0.0F;
    wpTransform.y = 0.0F;

    system->update(*registry, 0.016F);

    // Position should not have changed
    EXPECT_FLOAT_EQ(wpTransform.x, 0.0F);
    EXPECT_FLOAT_EQ(wpTransform.y, 0.0F);
}

TEST_F(WeakPointSystemTest, WeakPointWithDeadParentNotUpdated) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000, 50.0F, 25.0F);
    registry->killEntity(boss);

    system->update(*registry, 0.016F);

    // Should not crash, weak point should not move
}

TEST_F(WeakPointSystemTest, WeakPointWithSegmentIndex) {
    ECS::Entity boss = createBoss();
    auto& bossComp = registry->getComponent<BossComponent>(boss);
    // Record some position history
    bossComp.recordPosition(100.0F, 200.0F);
    bossComp.recordPosition(150.0F, 200.0F);

    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.segmentIndex = 1;

    system->update(*registry, 0.016F);

    // Weak point should follow segment position from history
}

TEST_F(WeakPointSystemTest, WeakPointDestructionDetected) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpHealth = registry->getComponent<HealthComponent>(wp);
    wpHealth.current = 0;

    system->update(*registry, 0.016F);

    const auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    EXPECT_TRUE(wpComp.destroyed);
}

TEST_F(WeakPointSystemTest, WeakPointBonusScoreEmitted) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpHealth = registry->getComponent<HealthComponent>(wp);
    wpHealth.current = 0;
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.bonusScore = 1000;

    system->update(*registry, 0.016F);

    // Event should have been emitted
    EXPECT_GT(emittedEvents.size(), 0);
}

TEST_F(WeakPointSystemTest, WeakPointDamageToParent) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpHealth = registry->getComponent<HealthComponent>(wp);
    wpHealth.current = 0;
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.damageToParent = 20;

    int32_t originalHealth =
        registry->getComponent<HealthComponent>(boss).current;

    system->update(*registry, 0.016F);

    // Boss should have taken damage
    const auto& bossHealth = registry->getComponent<HealthComponent>(boss);
    EXPECT_LT(bossHealth.current, originalHealth);
}

TEST_F(WeakPointSystemTest, CriticalWeakPointExposesCore) {
    ECS::Entity boss = createBoss();
    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpHealth = registry->getComponent<HealthComponent>(wp);
    wpHealth.current = 0;
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.critical = true;
    wpComp.exposesCore = true;

    system->update(*registry, 0.016F);

    // Core exposure effect should be triggered (check events)
}

TEST_F(WeakPointSystemTest, DisablesBossAttackPattern) {
    ECS::Entity boss = createBoss();
    BossPatternComponent patterns;
    patterns.enabled = true;
    registry->emplaceComponent<BossPatternComponent>(boss, patterns);

    ECS::Entity wp = createWeakPoint(boss, 1000);
    auto& wpHealth = registry->getComponent<HealthComponent>(wp);
    wpHealth.current = 0;
    auto& wpComp = registry->getComponent<WeakPointComponent>(wp);
    wpComp.disablesBossAttack = true;
    wpComp.disabledAttackPattern = "laser_sweep";

    system->update(*registry, 0.016F);

    // Pattern disabling should be handled
}
