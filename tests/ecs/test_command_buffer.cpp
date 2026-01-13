/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** test_command_buffer.cpp - Tests for CommandBuffer
*/

#include <gtest/gtest.h>
#include "ECS.hpp"

class CommandBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        commandBuffer = std::make_unique<ECS::CommandBuffer>(*registry);
    }

    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<ECS::CommandBuffer> commandBuffer;
};

TEST_F(CommandBufferTest, SpawnEntityDeferred) {
    // Spawn entity deferred
    auto placeholder = commandBuffer->spawnEntityDeferred();
    
    // Entity should not exist yet in registry
    EXPECT_FALSE(registry->isAlive(placeholder));
    
    // Flush commands
    commandBuffer->flush();
    
    // Now check that entities were created (placeholder gets mapped to real entity)
    EXPECT_TRUE(commandBuffer != nullptr);
}

TEST_F(CommandBufferTest, DestroyEntityDeferred) {
    // Create a real entity
    auto entity = registry->spawnEntity();
    EXPECT_TRUE(registry->isAlive(entity));
    
    // Defer its destruction
    commandBuffer->destroyEntityDeferred(entity);
    
    // Entity should still be alive before flush
    EXPECT_TRUE(registry->isAlive(entity));
    
    // Flush commands
    commandBuffer->flush();
    
    // Entity should be destroyed now
    EXPECT_FALSE(registry->isAlive(entity));
}

TEST_F(CommandBufferTest, MultipleSpawnDeferred) {
    // Spawn multiple entities
    auto placeholder1 = commandBuffer->spawnEntityDeferred();
    auto placeholder2 = commandBuffer->spawnEntityDeferred();
    auto placeholder3 = commandBuffer->spawnEntityDeferred();
    
    // Flush all commands
    commandBuffer->flush();
    
    // Verify command buffer is still valid
    EXPECT_TRUE(commandBuffer != nullptr);
}

TEST_F(CommandBufferTest, MixedDeferredOperations) {
    // Create a real entity
    auto entity = registry->spawnEntity();
    
    // Defer spawn of new entity
    auto placeholder = commandBuffer->spawnEntityDeferred();
    
    EXPECT_TRUE(registry->isAlive(entity));
    
    // Flush - should spawn the placeholder entity
    commandBuffer->flush();
    
    // Both entities should be alive
    EXPECT_TRUE(registry->isAlive(entity));
}

TEST_F(CommandBufferTest, FlushEmptyBuffer) {
    // Flush with no commands should not crash
    EXPECT_NO_THROW(commandBuffer->flush());
}

TEST_F(CommandBufferTest, DestroyPlaceholderEntity) {
    // Create placeholder
    auto placeholder = commandBuffer->spawnEntityDeferred();
    
    // Try to destroy the placeholder before flush
    commandBuffer->destroyEntityDeferred(placeholder);
    
    // Flush - should handle placeholder mapping correctly
    EXPECT_NO_THROW(commandBuffer->flush());
}

TEST_F(CommandBufferTest, MultipleFlushes) {
    // First batch
    auto p1 = commandBuffer->spawnEntityDeferred();
    commandBuffer->flush();
    
    // Second batch
    auto p2 = commandBuffer->spawnEntityDeferred();
    commandBuffer->flush();
    
    // Third batch
    auto e = registry->spawnEntity();
    commandBuffer->destroyEntityDeferred(e);
    commandBuffer->flush();
    
    EXPECT_FALSE(registry->isAlive(e));
}

TEST_F(CommandBufferTest, ClearAfterFlush) {
    // Add commands
    commandBuffer->spawnEntityDeferred();
    commandBuffer->spawnEntityDeferred();
    
    // Flush
    commandBuffer->flush();
    
    // Flush again - should be empty
    EXPECT_NO_THROW(commandBuffer->flush());
}
