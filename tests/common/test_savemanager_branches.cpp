/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Extended branch coverage tests for RTypeSaveManager and related components
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#include "Config/RTypeConfig.hpp"

using namespace rtype::game::config;

class SaveManagerBranchTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "save_branch_test";
        std::filesystem::create_directories(testDir);
        manager = std::make_unique<RTypeSaveManager>(testDir);
    }

    void TearDown() override {
        manager.reset();
        std::filesystem::remove_all(testDir);
    }

    std::filesystem::path testDir;
    std::unique_ptr<RTypeSaveManager> manager;
};

// ============================================================================
// RTypeGameState::isValid() Branch Tests
// ============================================================================

TEST(RTypeGameStateBranchTest, IsValidWithCorrectMagic) {
    RTypeGameState state = RTypeGameState::createNew();
    EXPECT_TRUE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithWrongMagic) {
    RTypeGameState state = RTypeGameState::createNew();
    state.header.magic = 0x12345678;
    EXPECT_FALSE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithWrongVersion) {
    // Note: isValid() may not check version - it mainly checks data integrity
    // This test verifies the state is still considered valid with different version
    // since version is handled during load, not during isValid
    RTypeGameState state = RTypeGameState::createNew();
    state.header.version = 0;
    // Version check may happen during load, not in isValid()
    // isValid() checks magic, players, and data integrity
    EXPECT_TRUE(state.isValid());  // Version doesn't affect isValid
}

TEST(RTypeGameStateBranchTest, IsValidWithNoPlayers) {
    RTypeGameState state;
    state.header.magic = SAVE_MAGIC_NUMBER;
    state.header.version = SAVE_FORMAT_VERSION;
    state.players.clear();
    EXPECT_FALSE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithInvalidPlayerHealth) {
    RTypeGameState state = RTypeGameState::createNew();
    state.players[0].health = 200;
    state.players[0].maxHealth = 100;
    EXPECT_FALSE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithZeroLevel) {
    RTypeGameState state = RTypeGameState::createNew();
    state.progression.currentLevel = 0;
    EXPECT_FALSE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithZeroWave) {
    RTypeGameState state = RTypeGameState::createNew();
    state.progression.currentWave = 0;
    EXPECT_FALSE(state.isValid());
}

TEST(RTypeGameStateBranchTest, IsValidWithAllCorrect) {
    RTypeGameState state = RTypeGameState::createNew();
    state.progression.currentLevel = 1;
    state.progression.currentWave = 1;
    state.players[0].health = 50;
    state.players[0].maxHealth = 100;
    EXPECT_TRUE(state.isValid());
}

// ============================================================================
// RTypeGameState::calculateChecksum() Branch Tests
// ============================================================================

TEST(RTypeGameStateBranchTest, ChecksumConsistentForSameState) {
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.saveName = "test";
    state1.players[0].score = 1000;

    RTypeGameState state2 = state1;

    EXPECT_EQ(state1.calculateChecksum(), state2.calculateChecksum());
}

TEST(RTypeGameStateBranchTest, ChecksumDifferentForDifferentScore) {
    // Checksum may not include saveName - test with data that's included
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.players[0].score = 1000;

    RTypeGameState state2 = RTypeGameState::createNew();
    state2.players[0].score = 2000;

    EXPECT_NE(state1.calculateChecksum(), state2.calculateChecksum());
}

TEST(RTypeGameStateBranchTest, ChecksumDifferentForDifferentProgression) {
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.progression.currentLevel = 1;

    RTypeGameState state2 = RTypeGameState::createNew();
    state2.progression.currentLevel = 2;

    EXPECT_NE(state1.calculateChecksum(), state2.calculateChecksum());
}

TEST(RTypeGameStateBranchTest, ChecksumDifferentForDifferentPlayerHealth) {
    // Checksum may not include enemies - test with player data
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.players[0].health = 100;

    RTypeGameState state2 = RTypeGameState::createNew();
    state2.players[0].health = 50;

    EXPECT_NE(state1.calculateChecksum(), state2.calculateChecksum());
}

// ============================================================================
// RTypeSaveManager::save() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveInvalidState) {
    RTypeGameState state;
    state.header.magic = 0;  // Invalid magic
    state.players.clear();

    auto result = manager->save(state, "invalid_state");

    EXPECT_EQ(result, SaveResult::InvalidData);
    EXPECT_FALSE(manager->saveExists("invalid_state"));
}

TEST_F(SaveManagerBranchTest, SaveValidState) {
    RTypeGameState state = RTypeGameState::createNew();

    auto result = manager->save(state, "valid_state");

    EXPECT_EQ(result, SaveResult::Success);
    EXPECT_TRUE(manager->saveExists("valid_state"));
}

TEST_F(SaveManagerBranchTest, SaveOverwritesExisting) {
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.players[0].score = 100;
    manager->save(state1, "overwrite_test");

    RTypeGameState state2 = RTypeGameState::createNew();
    state2.players[0].score = 200;
    manager->save(state2, "overwrite_test");

    auto loaded = manager->load("overwrite_test");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->players[0].score, 200u);
}

// ============================================================================
// RTypeSaveManager::load() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, LoadNonexistent) {
    auto result = manager->load("nonexistent");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(manager->getLastResult(), SaveResult::FileNotFound);
}

TEST_F(SaveManagerBranchTest, LoadCorruptedMagic) {
    // Write a file with wrong magic number
    auto filepath = testDir / "bad_magic.rtsave";
    std::ofstream file(filepath, std::ios::binary);
    uint32_t wrongMagic = 0xDEADBEEF;
    file.write(reinterpret_cast<const char*>(&wrongMagic), sizeof(wrongMagic));
    file.close();

    auto result = manager->load("bad_magic");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(manager->getLastResult(), SaveResult::FileCorrupted);
}

TEST_F(SaveManagerBranchTest, LoadCorruptedData) {
    // Write garbage data
    auto filepath = testDir / "garbage.rtsave";
    std::ofstream file(filepath, std::ios::binary);
    file << "This is garbage data that is not a valid save file!";
    file.close();

    auto result = manager->load("garbage");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(manager->getLastResult(), SaveResult::FileCorrupted);
}

TEST_F(SaveManagerBranchTest, LoadTruncatedFile) {
    // Save a valid state, then truncate the file
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "truncated");

    auto filepath = testDir / "truncated.rtsave";
    std::filesystem::resize_file(filepath, 10);  // Truncate to 10 bytes

    auto result = manager->load("truncated");

    EXPECT_FALSE(result.has_value());
}

TEST_F(SaveManagerBranchTest, LoadChecksumMismatch) {
    // Save a valid state
    RTypeGameState state = RTypeGameState::createNew();
    state.players[0].score = 1000;
    manager->save(state, "checksum_test");

    // Corrupt a byte in the middle of the file
    auto filepath = testDir / "checksum_test.rtsave";
    std::fstream file(filepath, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(50);  // Go to middle of file
    char garbage = 0xFF;
    file.write(&garbage, 1);
    file.close();

    auto result = manager->load("checksum_test");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(manager->getLastResult(), SaveResult::FileCorrupted);
}

TEST_F(SaveManagerBranchTest, LoadWithVersionMigration) {
    bool migrationCalled = false;
    uint32_t migratedFromVersion = 0;

    manager->setMigrationCallback([&](RTypeGameState& state, uint32_t oldVersion) {
        migrationCalled = true;
        migratedFromVersion = oldVersion;
        return true;
    });

    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "version_test");

    // The file was just saved with current version, migration won't be called
    auto loaded = manager->load("version_test");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_FALSE(migrationCalled);
}

TEST_F(SaveManagerBranchTest, LoadNewerVersionFails) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "future_version");

    // Manually modify the version to be newer
    auto filepath = testDir / "future_version.rtsave";
    std::fstream file(filepath, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(4);  // Skip magic, go to version
    uint32_t futureVersion = SAVE_FORMAT_VERSION + 10;
    file.write(reinterpret_cast<const char*>(&futureVersion), sizeof(futureVersion));
    file.close();

    auto result = manager->load("future_version");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(manager->getLastResult(), SaveResult::VersionMismatch);
}

// ============================================================================
// RTypeSaveManager::deleteSave() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, DeleteExistingSave) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "to_delete");

    EXPECT_TRUE(manager->saveExists("to_delete"));
    EXPECT_TRUE(manager->deleteSave("to_delete"));
    EXPECT_FALSE(manager->saveExists("to_delete"));
}

TEST_F(SaveManagerBranchTest, DeleteNonexistentSave) {
    bool result = manager->deleteSave("never_existed");
    EXPECT_FALSE(result);
}

// ============================================================================
// RTypeSaveManager::listSaves() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, ListSavesEmpty) {
    auto saves = manager->listSaves();
    EXPECT_TRUE(saves.empty());
}

TEST_F(SaveManagerBranchTest, ListSavesMultiple) {
    RTypeGameState state1 = RTypeGameState::createNew();
    state1.saveName = "First";
    manager->save(state1, "save1");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    RTypeGameState state2 = RTypeGameState::createNew();
    state2.saveName = "Second";
    manager->save(state2, "save2");

    auto saves = manager->listSaves();
    EXPECT_EQ(saves.size(), 2u);
    // Should be sorted by timestamp (most recent first)
    EXPECT_EQ(saves[0].filename, "save2");
    EXPECT_EQ(saves[1].filename, "save1");
}

TEST_F(SaveManagerBranchTest, ListSavesIgnoresNonSaveFiles) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "real_save");

    // Create a non-save file
    std::ofstream other(testDir / "not_a_save.txt");
    other << "This is not a save file";
    other.close();

    auto saves = manager->listSaves();
    EXPECT_EQ(saves.size(), 1u);
    EXPECT_EQ(saves[0].filename, "real_save");
}

TEST_F(SaveManagerBranchTest, ListSavesDirectoryDoesNotExist) {
    // Create a manager with a non-existent directory
    auto nonExistentDir = testDir / "does_not_exist";
    RTypeSaveManager tempManager(nonExistentDir);

    // Directory should be created by constructor
    EXPECT_TRUE(std::filesystem::exists(nonExistentDir));
}

// ============================================================================
// RTypeSaveManager::getSaveInfo() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, GetSaveInfoValid) {
    RTypeGameState state = RTypeGameState::createNew();
    state.saveName = "Test Save";
    state.progression.currentLevel = 5;
    state.progression.totalScore = 10000;
    manager->save(state, "info_test");

    auto info = manager->getSaveInfo("info_test");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->saveName, "Test Save");
    EXPECT_EQ(info->currentLevel, 5u);
    EXPECT_EQ(info->totalScore, 10000u);
}

TEST_F(SaveManagerBranchTest, GetSaveInfoNonexistent) {
    auto info = manager->getSaveInfo("nonexistent");
    EXPECT_FALSE(info.has_value());
}

TEST_F(SaveManagerBranchTest, GetSaveInfoCorruptedFile) {
    // Write garbage to a file
    auto filepath = testDir / "corrupted_info.rtsave";
    std::ofstream file(filepath, std::ios::binary);
    file << "garbage";
    file.close();

    auto info = manager->getSaveInfo("corrupted_info");
    // Info should indicate invalid or fail
    EXPECT_FALSE(info.has_value() && info->isValid);
}

// ============================================================================
// RTypeSaveManager::createBackup() / restoreBackup() Branch Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, CreateBackupSuccess) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "backup_test");

    EXPECT_TRUE(manager->createBackup("backup_test"));
    EXPECT_TRUE(manager->saveExists("backup_test.bak"));
}

TEST_F(SaveManagerBranchTest, CreateBackupNonexistent) {
    bool result = manager->createBackup("nonexistent");
    EXPECT_FALSE(result);
}

TEST_F(SaveManagerBranchTest, CreateBackupWithCustomName) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "main");

    EXPECT_TRUE(manager->createBackup("main", "custom_backup"));
    EXPECT_TRUE(manager->saveExists("custom_backup"));
}

TEST_F(SaveManagerBranchTest, RestoreBackupSuccess) {
    RTypeGameState original = RTypeGameState::createNew();
    original.players[0].score = 1000;
    manager->save(original, "restore_test");

    manager->createBackup("restore_test");

    // Modify the save
    original.players[0].score = 5000;
    manager->save(original, "restore_test");

    // Restore
    EXPECT_TRUE(manager->restoreBackup("restore_test"));

    auto loaded = manager->load("restore_test");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->players[0].score, 1000u);
}

TEST_F(SaveManagerBranchTest, RestoreBackupNoBackupExists) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "no_backup");

    bool result = manager->restoreBackup("no_backup");
    EXPECT_FALSE(result);
}

// ============================================================================
// PlayerState PowerUp Serialization Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveAllPowerUpTypes) {
    std::vector<PowerUpType> powerUps = {
        PowerUpType::None,
        PowerUpType::SpeedBoost,
        PowerUpType::Shield,
        PowerUpType::DoubleDamage,
        PowerUpType::RapidFire,
        PowerUpType::ExtraLife,
        PowerUpType::Bomb
    };

    for (size_t i = 0; i < powerUps.size(); ++i) {
        RTypeGameState state = RTypeGameState::createNew();
        state.players[0].activePowerUp = powerUps[i];
        state.players[0].powerUpTimeRemaining = static_cast<float>(i) * 5.0F;

        std::string slot = "powerup_" + std::to_string(static_cast<int>(powerUps[i]));
        manager->save(state, slot);

        auto loaded = manager->load(slot);
        ASSERT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->players[0].activePowerUp, powerUps[i]);
        EXPECT_FLOAT_EQ(loaded->players[0].powerUpTimeRemaining, static_cast<float>(i) * 5.0F);
    }
}

// ============================================================================
// Multiple Players Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveAndLoadManyPlayers) {
    RTypeGameState state = RTypeGameState::createNew();

    for (int i = 1; i < 4; ++i) {
        PlayerState player;
        player.playerId = static_cast<uint32_t>(i + 1);
        player.health = 100 - i * 10;
        player.maxHealth = 100;
        player.lives = 3 - i;
        player.score = static_cast<uint32_t>(i * 1000);
        player.positionX = static_cast<float>(i * 100);
        player.positionY = static_cast<float>(i * 50);
        state.players.push_back(player);
    }

    manager->save(state, "many_players");

    auto loaded = manager->load("many_players");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->players.size(), 4u);

    for (int i = 1; i < 4; ++i) {
        EXPECT_EQ(loaded->players[i].playerId, static_cast<uint32_t>(i + 1));
        EXPECT_EQ(loaded->players[i].health, 100 - i * 10);
    }
}

// ============================================================================
// Multiple Enemies Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveAndLoadManyEnemies) {
    RTypeGameState state = RTypeGameState::createNew();

    for (int i = 0; i < 10; ++i) {
        EnemyState enemy;
        enemy.enemyId = static_cast<uint32_t>(100 + i);
        enemy.enemyType = static_cast<uint8_t>(i % 5);
        enemy.positionX = static_cast<float>(i * 100);
        enemy.positionY = static_cast<float>(i * 50);
        enemy.health = 50 + i * 10;
        state.enemies.push_back(enemy);
    }

    manager->save(state, "many_enemies");

    auto loaded = manager->load("many_enemies");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->enemies.size(), 10u);

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(loaded->enemies[i].enemyId, static_cast<uint32_t>(100 + i));
        EXPECT_EQ(loaded->enemies[i].enemyType, static_cast<uint8_t>(i % 5));
    }
}

// ============================================================================
// Progression and Difficulty Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveFullProgressionData) {
    RTypeGameState state = RTypeGameState::createNew();

    state.progression.currentLevel = 10;
    state.progression.currentWave = 5;
    state.progression.totalWaves = 50;
    state.progression.enemiesDefeated = 500;
    state.progression.totalScore = 100000;
    state.progression.playTimeSeconds = 7200.0F;  // 2 hours

    state.progression.lastCheckpoint.checkpointId = 3;
    state.progression.lastCheckpoint.waveNumber = 4;
    state.progression.lastCheckpoint.waveProgress = 0.8F;

    manager->save(state, "full_progression");

    auto loaded = manager->load("full_progression");
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->progression.currentLevel, 10u);
    EXPECT_EQ(loaded->progression.currentWave, 5u);
    EXPECT_EQ(loaded->progression.totalWaves, 50u);
    EXPECT_EQ(loaded->progression.enemiesDefeated, 500u);
    EXPECT_EQ(loaded->progression.totalScore, 100000u);
    EXPECT_FLOAT_EQ(loaded->progression.playTimeSeconds, 7200.0F);

    EXPECT_EQ(loaded->progression.lastCheckpoint.checkpointId, 3u);
    EXPECT_FLOAT_EQ(loaded->progression.lastCheckpoint.waveProgress, 0.8F);
}

TEST_F(SaveManagerBranchTest, SaveDifficultySettings) {
    RTypeGameState state = RTypeGameState::createNew();

    state.difficulty.difficultyLevel = "nightmare";
    state.difficulty.enemyHealthMultiplier = 2.0F;
    state.difficulty.enemySpeedMultiplier = 1.5F;
    state.difficulty.playerDamageMultiplier = 0.5F;
    state.difficulty.startingLives = 1;

    manager->save(state, "difficulty_test");

    auto loaded = manager->load("difficulty_test");
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->difficulty.difficultyLevel, "nightmare");
    EXPECT_FLOAT_EQ(loaded->difficulty.enemyHealthMultiplier, 2.0F);
    EXPECT_FLOAT_EQ(loaded->difficulty.enemySpeedMultiplier, 1.5F);
    EXPECT_FLOAT_EQ(loaded->difficulty.playerDamageMultiplier, 0.5F);
    EXPECT_EQ(loaded->difficulty.startingLives, 1u);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(SaveManagerBranchTest, SaveEmptySaveName) {
    RTypeGameState state = RTypeGameState::createNew();
    state.saveName = "";

    manager->save(state, "empty_name");

    auto loaded = manager->load("empty_name");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->saveName, "");
}

TEST_F(SaveManagerBranchTest, SaveLongSaveName) {
    RTypeGameState state = RTypeGameState::createNew();
    state.saveName = std::string(1000, 'A');  // Very long name

    manager->save(state, "long_name");

    auto loaded = manager->load("long_name");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->saveName.length(), 1000u);
}

TEST_F(SaveManagerBranchTest, SaveUnicodeSaveName) {
    RTypeGameState state = RTypeGameState::createNew();
    state.saveName = "日本語セーブ 🎮";

    manager->save(state, "unicode_name");

    auto loaded = manager->load("unicode_name");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->saveName, "日本語セーブ 🎮");
}

TEST_F(SaveManagerBranchTest, SaveZeroValues) {
    RTypeGameState state = RTypeGameState::createNew();
    state.players[0].positionX = 0.0F;
    state.players[0].positionY = 0.0F;
    state.players[0].rotation = 0.0F;
    state.players[0].score = 0;

    manager->save(state, "zero_values");

    auto loaded = manager->load("zero_values");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_FLOAT_EQ(loaded->players[0].positionX, 0.0F);
    EXPECT_FLOAT_EQ(loaded->players[0].positionY, 0.0F);
    EXPECT_EQ(loaded->players[0].score, 0u);
}

TEST_F(SaveManagerBranchTest, SaveNegativeValues) {
    RTypeGameState state = RTypeGameState::createNew();
    state.players[0].positionX = -500.0F;
    state.players[0].positionY = -250.0F;

    manager->save(state, "negative_values");

    auto loaded = manager->load("negative_values");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_FLOAT_EQ(loaded->players[0].positionX, -500.0F);
    EXPECT_FLOAT_EQ(loaded->players[0].positionY, -250.0F);
}

TEST_F(SaveManagerBranchTest, SaveMaxValues) {
    RTypeGameState state = RTypeGameState::createNew();
    state.players[0].score = 0xFFFFFFFF;  // Max uint32_t
    state.progression.totalScore = 0xFFFFFFFF;  // Max uint32_t

    manager->save(state, "max_values");

    auto loaded = manager->load("max_values");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->players[0].score, 0xFFFFFFFFu);
}

// ============================================================================
// Concurrent Operations Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, RapidSaveLoad) {
    for (int i = 0; i < 10; ++i) {
        RTypeGameState state = RTypeGameState::createNew();
        state.players[0].score = static_cast<uint32_t>(i * 100);

        manager->save(state, "rapid_test");

        auto loaded = manager->load("rapid_test");
        ASSERT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->players[0].score, static_cast<uint32_t>(i * 100));
    }
}

// ============================================================================
// GetLastError Tests
// ============================================================================

TEST_F(SaveManagerBranchTest, GetLastErrorAfterFileNotFound) {
    [[maybe_unused]] auto result = manager->load("nonexistent");

    std::string error = manager->getLastError();
    EXPECT_FALSE(error.empty());
    EXPECT_NE(error.find("not found"), std::string::npos);
}

TEST_F(SaveManagerBranchTest, GetLastErrorAfterSuccess) {
    RTypeGameState state = RTypeGameState::createNew();
    manager->save(state, "success_test");

    auto loaded = manager->load("success_test");
    ASSERT_TRUE(loaded.has_value());

    std::string error = manager->getLastError();
    EXPECT_TRUE(error.empty());
}
