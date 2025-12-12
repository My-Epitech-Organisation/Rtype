#include <gtest/gtest.h>

#include "client/Graphic/SceneManager/SceneException.hpp"

TEST(SceneExceptionTest, WhatStrings) {
    SceneNotFound notFound;
    EXPECT_STREQ(notFound.what(), "Scene not found");

    SceneNotInitialized notInit;
    EXPECT_STREQ(notInit.what(), "Scene not initialized");
}
