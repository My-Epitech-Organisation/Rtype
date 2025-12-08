/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for ClientApp
*/

#include <gtest/gtest.h>
#include <memory>
#include <ECS.hpp>
#include "../src/client/ClientApp.hpp"

class ClientAppTest : public ::testing::Test {
protected:
    std::shared_ptr<ECS::Registry> registry;

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
    }

    void TearDown() override {
        registry.reset();
    }
};

TEST_F(ClientAppTest, Constructor_InitializesGraphic) {
    EXPECT_NO_THROW({
        ClientApp app(registry);
    });
}
